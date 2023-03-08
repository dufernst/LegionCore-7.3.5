#include <chrono>
#include <thread>
#include <algorithm>
#include <filesystem>
#include <optional>

#include <httplib.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <SMTPMail.h>

#include "Database/DatabaseEnv.h"
#include "Configuration/Config.h"
#include "AccountMgr.h"
#include "SHA1.h"

#include "nacl.h"

extern "C" {
    // from captcha library
    const int gifsize = 17646;
    void captcha(unsigned char im[70 * 200], unsigned char l[6]);
    void makegif(unsigned char im[70 * 200], unsigned char gif[gifsize]);
}

bool LoadConfig()
{
    char const* cfg_file = "webapi.conf";

    std::string error;
    if (!sConfigMgr->LoadInitial(cfg_file, error))
    {
        printf("Invalid or missing configuration file : %s\n", cfg_file);
        printf("Verify that the file exists and has \'[webapi]\' written in the top of the file!\n");

        printf(error.c_str());
        return false;
    }

    return true;
}

bool StartLoginDB()
{
    LoginDatabase = LoginDatabaseWorkerPool();

    MySQL::Library_Init();

    std::string dbstring = sConfigMgr->GetStringDefault("LoginDatabaseInfo", "");
    if (dbstring.empty())
    {
        printf("Database not specified\n");
        return false;
    }

    int32 worker_threads = 0;
    int32 synch_threads = 1;

    // NOTE: Authserver is singlethreaded you should keep synch_threads == 1, only 1 will ever be used anyway.
    if (!LoginDatabase.Open(dbstring.c_str(), uint8(worker_threads), uint8(synch_threads)))
    {
        printf("Cannot connect to database\n");
        return false;
    }

    //printf("Started auth database connection pool.");
    return true;
}

void StopLoginDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}

/*
All possible URLs:
(with api post) /account/creation/email/
(with api post) /account/creation/email/error-wrong/
(with api post) /account/creation/email/error-double/
(with api post) /account/creation/password/
(with api post) /account/creation/password/error-wrong/
(with api post (api get)) /account/creation/captcha/(#/api/)
(with api post (api get)) /account/creation/captcha/error-wrong/(#/api/)
(with api post (api get)) /account/creation/captcha/error-internal/(#/api/)
(static) /account/creation/verify/
(api get) /account/creation/success/#/api/
(static) /account/creation/success/old/
(static) /account/creation/success/done/
(static) /account/creation/success/error-internal/
*/
struct StateDataRegistration
{
    std::string email = "";
    bool emailAvailable = false;
    bool emailVerified = false;
    std::vector<unsigned char> captcha;
    std::string password = "";

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(email, emailAvailable, emailVerified, captcha, password);
    }

    std::string whereToNext()
    {
        if (email == "")
            return std::string("/account/creation/email/");
        if (password == "")
            return std::string("/account/creation/password/");
        if (!emailAvailable)
            return std::string("/account/creation/captcha/");
        if (emailVerified)
            return std::string("/account/creation/success/");

        return std::string("/account/creation/verify/");
    }
};

/*
All possible URLs:
(with api post) /account/reset/email/
(with api post) /account/reset/email/error-wrong/
(with api post) /account/reset/email/not-found/ (if not found, maybe need convert to WoD account)
(with api post) /account/reset/password/
(with api post) /account/reset/password/error-wrong/
(with api post (api get)) /account/reset/captcha/(#/api/)
(with api post (api get)) /account/reset/captcha/error-wrong/(#/api/)
(with api post (api get)) /account/reset/captcha/error-internal/(#/api/)
(static) /account/reset/check-email/
(api get) /account/reset/success/#/api/
(static) /account/reset/success/error-internal/
(static) /account/reset/success/done/
(static) /account/reset/success/expired/ (here able to click back to start of reset)
*/
struct StateDataPasswordReset
{
    std::string email = "";
    uint64_t validTillEpoch = 0;
    bool emailVerified = false;
    std::vector<unsigned char> captcha;
    std::string password = "";

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(email, validTillEpoch, emailVerified, captcha, password);
    }

    std::string whereToNext()
    {
        if (email == "")
            return std::string("/account/reset/email/");
        if (password == "")
            return std::string("/account/reset/password/");
        if (validTillEpoch == 0)
            return std::string("/account/reset/captcha/");
        if (!emailVerified)
            return std::string("/account/reset/check-email/");

        return std::string("/account/reset/success/");
    }
};

/*
All possible URLs:
(with api post) /account/upgrade/current/username/
(with api post) /account/upgrade/current/username/not-found/
(with api post) /account/upgrade/current/username/error-wrong/

(with api post) /account/upgrade/current/password/
(with api post) /account/upgrade/current/password/error-wrong/
OR DIRECTLY
(with api post (api get)) /account/upgrade/current/captcha/(#/api/)
(with api post (api get)) /account/upgrade/current/captcha/error-wrong/(#/api/)
(with api post (api get)) /account/upgrade/current/captcha/error-internal/(#/api/)

(static) /account/upgrade/current/check-email/wrong-password/
(static) /account/upgrade/current/check-email/no-password/
(static) /account/upgrade/current/check-email/no-email/
OR DIRECTLY
(with api post) /account/upgrade/new/email/ (ready to migrate your account to the new expansion)
(with api post) /account/upgrade/new/email/error-wrong/
(with api post) /account/upgrade/new/email/error-double/
(with api post) /account/upgrade/new/password/
(with api post) /account/upgrade/new/password/error-wrong/
(with api post (api get)) /account/upgrade/new/captcha/(#/api/)
(with api post (api get)) /account/upgrade/new/captcha/error-wrong/(#/api/)
(with api post (api get)) /account/upgrade/new/captcha/error-internal/(#/api/)
(static) /account/upgrade/new/check-email/
(api get) /account/upgrade/success/#/api/
(static) /account/upgrade/success/error-internal/
(static) /account/upgrade/success/done/
(static) /account/upgrade/success/old/
*/
struct StateDataUpgrade
{
    std::string currentUsername = "";
    std::string currentPassword = "";
    bool skipPassword = false;
    bool usernameFound = false;
    bool currentAccountVerified = false;
    std::string currentEmail = "";
    std::string newEmail = "";
    std::string newPassword = "";
    bool newEmailAvailable = false;
    bool newEmailVerified = false;
    std::vector<unsigned char> captcha;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(currentUsername, currentPassword, skipPassword, usernameFound, currentAccountVerified,
            currentEmail, newEmail, newPassword, newEmailAvailable, newEmailVerified, captcha);
    }

    std::string whereToNext()
    {
        if (currentUsername == "")
            return std::string("/account/upgrade/current/username/");
        if (currentPassword == "" && !skipPassword)
            return std::string("/account/upgrade/current/password/");
        if (!usernameFound)
            return std::string("/account/upgrade/current/captcha/");
        if (!currentAccountVerified && currentEmail == "")
            return std::string("/account/upgrade/current/check-email/no-email/");
        if (!currentAccountVerified && skipPassword)
            return std::string("/account/upgrade/current/check-email/no-password/");
        if (!currentAccountVerified && !skipPassword)
            return std::string("/account/upgrade/current/check-email/wrong-password/");
        if (newEmail == "")
            return std::string("/account/upgrade/new/email/");
        if (newPassword == "")
            return std::string("/account/upgrade/new/password/");
        if (!newEmailAvailable)
            return std::string("/account/upgrade/new/captcha/");
        if (!newEmailVerified)
            return std::string("/account/upgrade/new/check-email/");

        return std::string("/account/upgrade/success/");
    }
};

struct StateUnsubscribe
{
    std::string emailAddress;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(emailAddress);
    }
};

struct StateLogin
{
    std::string returnUrl;
    std::string emailAddress;
    std::string password;
    std::vector<unsigned char> captcha;
    uint64_t validTill;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(returnUrl, emailAddress, password, captcha, validTill);
    }

    std::string whereToNext()
    {
        if (emailAddress == "" || password == "")
            return std::string("/account/login/credentials/");

        return std::string("/account/login/captcha/");
    }
};

struct StateLoggedIn
{
    uint32_t accountId;
    uint64_t validTill;
    std::string email;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(accountId, validTill, email);
    }
};

struct StateAccountReference
{
    uint32_t aId;
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(aId);
    }
};

template <typename T>
std::string StateToHexBytesString(const T& data, const unsigned char* k, const bool onlyDigits = false)
{
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
	randombytes(nonce, crypto_secretbox_NONCEBYTES);

    std::ostringstream ostream;
    cereal::BinaryOutputArchive archive(ostream);
    archive(data);

    std::string dataAsString = ostream.str();
    std::vector<unsigned char> unsignedData(dataAsString.data(), dataAsString.data() + dataAsString.length());

    std::vector<unsigned char> toEncryptV = std::vector<unsigned char>(crypto_secretbox_ZEROBYTES, '\0');
    toEncryptV.insert(toEncryptV.end(), unsignedData.begin(), unsignedData.end());
    std::vector<unsigned char> encryptedV = std::vector<unsigned char>(toEncryptV.size() +
        crypto_secretbox_NONCEBYTES - crypto_secretbox_BOXZEROBYTES, '\0');

    crypto_secretbox(encryptedV.data() + crypto_secretbox_NONCEBYTES - crypto_secretbox_BOXZEROBYTES, toEncryptV.data(), toEncryptV.size(), nonce, k);
    for (int i = 0; i < crypto_secretbox_NONCEBYTES; i++)
        encryptedV[i] = nonce[i];

    char buffer[onlyDigits ? 4 : 3];
    std::string result;
    for (const unsigned char byte : encryptedV)
        if (snprintf(buffer, onlyDigits ? 4 : 3, onlyDigits ? "%.3u" : "%.2X", byte) == onlyDigits ? 3 : 2)
            result += std::string(buffer);

    return result;
}

template <typename T>
bool HexBytesStringToState(T& data, const unsigned char* k, const std::string& bytesAsHexString, const bool onlyDigits = false)
{
    uint64_t charsPerByte = onlyDigits ? 3 : 2;
    if (bytesAsHexString.length() % charsPerByte || bytesAsHexString.length() <= crypto_secretbox_NONCEBYTES * charsPerByte)
        return false;

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    for (int i = 0; i < crypto_secretbox_NONCEBYTES * charsPerByte; i += charsPerByte)
        sscanf(bytesAsHexString.c_str() + i, onlyDigits ? "%3hhu" : "%2hhx", &nonce[i / charsPerByte]);


    std::vector<unsigned char> encryptedV(crypto_secretbox_BOXZEROBYTES + (bytesAsHexString.length() / charsPerByte) - crypto_secretbox_NONCEBYTES, '\0');
    for (int i = crypto_secretbox_NONCEBYTES * charsPerByte; i < bytesAsHexString.length(); i += charsPerByte)
        sscanf(bytesAsHexString.c_str() + i, onlyDigits ? "%3hhu" : "%2hhx",
                &encryptedV[crypto_secretbox_BOXZEROBYTES + (i - crypto_secretbox_NONCEBYTES * charsPerByte) / charsPerByte]);

    std::vector<unsigned char> m(encryptedV.size(), '\0');
    if (crypto_secretbox_open(m.data(), encryptedV.data(), encryptedV.size(), nonce, k))
        return false;

    std::string decryptedString(m.begin() + crypto_secretbox_ZEROBYTES, m.end());

    std::istringstream istream(decryptedString);
    cereal::BinaryInputArchive archive(istream);
    archive(data);

    return true;
}

template <typename T>
std::string ReturnCaptcha(const std::string& hexString, const unsigned char* k)
{
    T input;
    HexBytesStringToState(input, k, hexString);

    if (input.captcha.empty())
    {
        input.captcha = std::vector<unsigned char>(6);
        randombytes(input.captcha.data(), 6);
    }

    unsigned char im[70 * 200];
    unsigned char gif[gifsize];

    captcha(im, input.captcha.data());
    makegif(im, gif);

    return std::string((const char*)gif, gifsize);
}

template <typename T>
bool GetCookieState(T& data, const unsigned char* k, const std::string& name, const httplib::Request& req)
{
    if (!req.has_header("Cookie"))
        return false;

    std::string cookies = req.get_header_value("Cookie");
    std::string nameWithEqual(name + std::string("="));
    size_t startPos = cookies.find(nameWithEqual);

    if (startPos == std::string::npos)
        return false;

    return HexBytesStringToState(data, k, cookies.substr(
        startPos + nameWithEqual.size(), cookies.find(';', startPos) - startPos - nameWithEqual.size()));
}

struct SMTPData
{
    std::string SMTPPort;
    std::string SMTPIP;
    std::string SMTPUser;
    std::string SMTPPassword;
    std::string SMTPFromAddress;
    std::string SMTPFromName;
    std::string SMTPMailRegistrationSubject;
    std::string SMTPMailResetSubject;
    std::string SMTPMailValidationSubject;
    std::string SMTPMailConfirmationSubject;
    std::string SMTPReturnDomain;
};

struct VoteWebsiteData
{
    uint8_t id;
    std::string name;
    std::string callbackKeyName;
    std::string callbackHostname;
    std::vector<std::string> callbackIps;
    std::string callbackUrl;
    bool callbackIdOnlyDigits;
    std::string voteUrl;
    std::string checkKeyName;
    std::string checkKeyValue;
    uint8_t voteTokenType;
    uint8_t voteTokensGiven;
};

std::vector<std::string> LoadJSonDatabase(const std::string& filename)
{
    std::ifstream file(filename);
    cereal::JSONInputArchive archive(file);
    std::vector<std::string> vec;
    archive(vec);

    return vec;
}

std::string strToLower(const std::string& s)
{
    std::string res(s);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) {return std::tolower(c);});
    return res;
}

bool invalidEmail(const std::string& email)
{
    const std::regex pattern(R"(^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*$)");
    return !std::regex_match(email, pattern);
}

std::optional<std::string> getFile(const std::string& file)
{
    std::string fileContent("");

    if (!std::filesystem::exists(file))
        return std::nullopt;

    std::ifstream fileToTokenize(file);
    for(std::string line; getline(fileToTokenize, line);)
    {
        fileContent += line;
        fileContent += std::string("\n");
    }
    fileToTokenize.close();

    return fileContent;
}

std::vector<std::string> getTokenizedFile(const std::string& file, const std::string& delimiter)
{
    std::vector<std::string> result;

    std::optional<std::string> optionalFileContent = getFile(file);
    if (!optionalFileContent)
        return result;

    std::string fileContent = *optionalFileContent;

    if (delimiter.empty())
    {
        result.push_back(fileContent);
        return result;
    }

    size_t start = 0;
    size_t end = fileContent.find(delimiter);

    while (end != std::string::npos)
    {
        result.push_back(fileContent.substr(start, end - start));
        start = end + delimiter.length();
        end = fileContent.find(delimiter, start);
    }

    result.push_back(fileContent.substr(start, end));

    return result;
}

void sendNewsletter(const SMTPData& smtpData, const unsigned char* k)
{
    std::vector<std::string> emailAddresses = LoadJSonDatabase("emailAddresses.json");
    std::transform(emailAddresses.begin(), emailAddresses.end(), emailAddresses.begin(), strToLower);
    std::vector<std::string> unsubscribedEmailAddresses = LoadJSonDatabase("unsubscribedEmailAddresses.json");
    std::transform(unsubscribedEmailAddresses.begin(), unsubscribedEmailAddresses.end(), unsubscribedEmailAddresses.begin(), strToLower);
    const std::unordered_set<std::string> filterEmailAddresses(unsubscribedEmailAddresses.begin(), unsubscribedEmailAddresses.end());

    auto cleanedBegin = emailAddresses.begin();
    auto cleanedEnd = std::remove_if(emailAddresses.begin(), emailAddresses.end(), [&filterEmailAddresses](const std::string& email)
    {
        return invalidEmail(email) || filterEmailAddresses.count(email);
    });
    const std::unordered_set<std::string> cleanedUniqueAll(cleanedBegin, cleanedEnd);
    
    std::string emailBody = getFile("newsletter.html").value();

    std::error_code ec;
    uint64_t epoch = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch() + std::chrono::hours(7 * 24)).count();
    std::string newName = std::string("newsletter-") + std::to_string(epoch) + std::string(".html");
    std::filesystem::rename("newsletter.html", newName, ec);
    if (ec.value() != 0)
    {
        printf("Could not move newsletter.html to newsletter-epoch.html... abort...\n");
        return;
    }

    std::string unsubscribeDomain = sConfigMgr->GetStringDefault("NewsLetterUnsubDomain", "https://unsubscribe.wow.com/unsubscribe/");
    std::string subject = sConfigMgr->GetStringDefault("NewsLetterSubject", "WoW Subject");
    std::string unsubEmail = sConfigMgr->GetStringDefault("NewsLetterMailForUnsubscribe", "unsubscribe@wow.com");

    uint64_t index = 0;
    for (const std::string& email : cleanedUniqueAll)
    {
        StateUnsubscribe sUnsubscribe;
        sUnsubscribe.emailAddress = email;
        std::string unsubscribeURL = unsubscribeDomain + StateToHexBytesString(sUnsubscribe, k) + std::string("/");

        SMTPMail mail;
        try
        {
            mail.open(smtpData.SMTPIP.c_str(), smtpData.SMTPPort.c_str(), smtp_connection_security::SMTP_SECURITY_NONE, smtp_flag::SMTP_NO_CERT_VERIFY, NULL);
            mail.auth(smtp_authentication_method::SMTP_AUTH_LOGIN, smtpData.SMTPUser.c_str(), smtpData.SMTPPassword.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_FROM, smtpData.SMTPFromAddress.c_str(), smtpData.SMTPFromName.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_TO, email.c_str(), NULL);
            mail.header_add("Subject", subject.c_str());
            mail.header_add("List-Unsubscribe", std::string(std::string("<mailto: ") + unsubEmail + std::string("?subject=unsubscribe>, <") + unsubscribeURL + std::string(">")).c_str());
            mail.header_add("MIME-Version", "1.0");
            mail.header_add("Content-Type", "multipart/alternative; boundary=dakjqwmnejadhasdkljkaeiasndansdahjdhjasd");
            mail.mail(emailBody.c_str());
        }
        catch (SMTPMailException& e)
        {
            printf(e.what());
        }

        try
        {
            mail.close();
        }
        catch (SMTPMailException&) {}

        printf("[%llu] Mailed to: %s\n", index, email.c_str());
        index++;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

std::vector<VoteWebsiteData> loadVoteWebsites()
{
    std::vector<VoteWebsiteData> voteWebsites;

    for (int i = 1; i < std::numeric_limits<uint8_t>::max(); i++)
    {
        if (sConfigMgr->GetStringDefault(std::string("WebsiteName") + std::to_string(i), "") == "")
            break;

        voteWebsites.emplace_back();
        voteWebsites[i - 1].id = i;
        voteWebsites[i - 1].name = sConfigMgr->GetStringDefault(std::string("WebsiteName") + std::to_string(i), "");
        voteWebsites[i - 1].callbackKeyName = sConfigMgr->GetStringDefault(std::string("CallbackKeyName") + std::to_string(i), "");
        voteWebsites[i - 1].callbackHostname = sConfigMgr->GetStringDefault(std::string("CallbackHostname") + std::to_string(i), "");

        voteWebsites[i - 1].callbackIps = std::vector<std::string>();
        std::string callbackIps = sConfigMgr->GetStringDefault(std::string("CallbackIps") + std::to_string(i), "");
        size_t startSearch = 0;
        while (true)
        {
            size_t endSearch = callbackIps.find(' ', startSearch);
            std::string callbackIp = callbackIps.substr(startSearch, endSearch);
            if (callbackIp == "")
                break;

            voteWebsites[i - 1].callbackIps.emplace_back(std::move(callbackIp));

            if (endSearch == std::string::npos)
                break;

            // skip the space on the next round
            startSearch = endSearch + 1;
        }

        voteWebsites[i - 1].callbackUrl = sConfigMgr->GetStringDefault(std::string("CallbackUrl") + std::to_string(i), "");
        voteWebsites[i - 1].callbackIdOnlyDigits = sConfigMgr->GetBoolDefault(std::string("CallbackIdOnlyDigits") + std::to_string(i), false);
        voteWebsites[i - 1].voteUrl = sConfigMgr->GetStringDefault(std::string("VoteUrl") + std::to_string(i), "");
        voteWebsites[i - 1].checkKeyName = sConfigMgr->GetStringDefault(std::string("CheckKeyName") + std::to_string(i), "");
        voteWebsites[i - 1].checkKeyValue = sConfigMgr->GetStringDefault(std::string("CheckKeyValue") + std::to_string(i), "");
        voteWebsites[i - 1].voteTokenType = sConfigMgr->GetIntDefault(std::string("VoteToken") + std::to_string(i), 0);
        voteWebsites[i - 1].voteTokensGiven = sConfigMgr->GetIntDefault(std::string("VoteTokensGiven") + std::to_string(i), 0);

        printf("Loaded vote site: %s, with CallbackKeyName: %s, CallbackHostname: %s, CallbackUrl: %s, CallbackIdOnlyDigits: %u, VoteUrl: %s, CheckKeyName: %s, CheckKeyValue: %s, VoteToken: %d, VoteTokensGiven: %d\n",
            voteWebsites[i - 1].name.c_str(), voteWebsites[i - 1].callbackKeyName.c_str(), voteWebsites[i - 1].callbackHostname.c_str(),
            voteWebsites[i - 1].callbackUrl.c_str(), voteWebsites[i - 1].callbackIdOnlyDigits,
            voteWebsites[i - 1].voteUrl.c_str(), voteWebsites[i - 1].checkKeyName.c_str(), voteWebsites[i - 1].checkKeyValue.c_str(),
            voteWebsites[i - 1].voteTokenType, voteWebsites[i - 1].voteTokensGiven);
        for (auto& ip : voteWebsites[i - 1].callbackIps)
            printf("Added callbackIp: %s\n", ip.c_str());
    }

    return voteWebsites;
}

void printReqData(const httplib::Request& req)
{
    printf("Did get the following parameters:\n");
    for (const auto& param: req.params)
        printf("%s:%s\n", param.first.c_str(), param.second.c_str());
    printf("Did get the following headers:\n");
    for (const auto& header: req.headers)
        printf("%s:%s\n", header.first.c_str(), header.second.c_str());
}

void handleVoteCallback(const VoteWebsiteData& voteWebsite, const std::string& ipFromHeader, const httplib::Request& req, const unsigned char* k)
{
    if (ipFromHeader != "" && !req.has_header(ipFromHeader))
    {
        printf("Vote callback lacking the configured header: %s\n", ipFromHeader.c_str());
        printReqData(req);
        return;
    }

    std::string realIp = (ipFromHeader != "") ? req.get_header_value(ipFromHeader) : req.remote_addr;
    if (std::find(voteWebsite.callbackIps.begin(), voteWebsite.callbackIps.end(), realIp) == voteWebsite.callbackIps.end())
    {
        if (voteWebsite.callbackHostname == "")
        {
            printf("Vote callback for website: %s, came in from unexpected IP: %s\n", voteWebsite.name.c_str(), realIp.c_str());
            return;
        }

        std::vector<std::string> foundIps;
        httplib::hosted_at(voteWebsite.callbackHostname, foundIps);
        if (std::find(foundIps.begin(), foundIps.end(), realIp) == foundIps.end())
        {
            printf("Vote callback for website: %s, came in from unexpected IP: %s\n", voteWebsite.name.c_str(), realIp.c_str());
            return;
        }
    }

    if (voteWebsite.checkKeyName != "")
    {
        if (!req.has_param(voteWebsite.checkKeyName) && !req.has_header(voteWebsite.checkKeyName))
        {
            printf("Vote callback for website: %s, came in without the expected parameter or header value.\n", voteWebsite.name.c_str());
            printReqData(req);
            return;
        }

        if (req.has_param(voteWebsite.checkKeyName) ?
            req.get_param_value(voteWebsite.checkKeyName) != voteWebsite.checkKeyValue :
            req.get_header_value(voteWebsite.checkKeyName) != voteWebsite.checkKeyValue)
        {
            printf("Vote check param/header value is incorrect.\n");
            printReqData(req);
            return;
        }
    }

    if (!req.has_param(voteWebsite.callbackKeyName) && !req.has_header(voteWebsite.callbackKeyName))
    {
        printf("Vote callback missing required param/header containing the userRef.\n");
        printReqData(req);
        return;
    }

    std::string userRef = req.has_param(voteWebsite.callbackKeyName) ?
        req.get_param_value(voteWebsite.callbackKeyName) : req.get_header_value(voteWebsite.callbackKeyName);

    StateAccountReference sar;
    if (!HexBytesStringToState(sar, k, userRef, voteWebsite.callbackIdOnlyDigits))
    {
        printf("Unable to parse StateAccountReference from provided userRef.\n");
        printReqData(req);
        return;
    }

    if (voteWebsite.voteTokensGiven <= 0)
        return;

    if (!StartLoginDB())
    {
        printf("Unable to reward vote points due to a connection issue with the database.\n");
        printReqData(req);
        return;
    }

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_OR_UPD_TOKEN);
    stmt->setUInt32(0, sar.aId);
    stmt->setUInt8(1, voteWebsite.voteTokenType);
    stmt->setInt64(2, voteWebsite.voteTokensGiven);
    stmt->setInt64(3, voteWebsite.voteTokensGiven);
    LoginDatabase.DirectExecute(stmt);

    StopLoginDB();
}

int main(int argc, char* argv[])
{    
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!LoadConfig())
        return 1;

    if (!StartLoginDB())
        return 1;

    StopLoginDB();

	unsigned char k[crypto_secretbox_KEYBYTES];

	if (FILE* pFile = fopen("salsa-key.txt", "rb"))
	{
        fread(k, sizeof(unsigned char), crypto_secretbox_KEYBYTES, pFile);

		fclose(pFile);
	}
	else if (FILE* pFile = fopen("salsa-key.txt", "wb"))
	{
		randombytes(k, crypto_secretbox_KEYBYTES);
        fwrite(k, sizeof(unsigned char), crypto_secretbox_KEYBYTES, pFile);

		fclose(pFile);
	}
	else
		return 1;

    int32 port = sConfigMgr->GetIntDefault("Port", 8090);
    if (port < 0 || port > 0xFFFF)
    {
        printf("Specified port out of allowed range (1-65535).\n");
        return 1;
    }

    std::string bind_ip = sConfigMgr->GetStringDefault("BindIP", "127.0.0.1");

    SMTPData smtpData;
    smtpData.SMTPPort = sConfigMgr->GetStringDefault("SMTPPort", "");
    smtpData.SMTPIP = sConfigMgr->GetStringDefault("SMTPIP", "");
    smtpData.SMTPUser = sConfigMgr->GetStringDefault("SMTPUser", "");
    smtpData.SMTPPassword = sConfigMgr->GetStringDefault("SMTPPassword", "");
    smtpData.SMTPFromAddress = sConfigMgr->GetStringDefault("SMTPFromAddress", "");
    smtpData.SMTPFromName = sConfigMgr->GetStringDefault("SMTPFromName", "");
    smtpData.SMTPMailRegistrationSubject = sConfigMgr->GetStringDefault("SMTPMailRegistrationSubject", "");
    smtpData.SMTPMailResetSubject = sConfigMgr->GetStringDefault("SMTPMailResetSubject", "");
    smtpData.SMTPMailValidationSubject = sConfigMgr->GetStringDefault("SMTPMailValidationSubject", "");
    smtpData.SMTPMailConfirmationSubject = sConfigMgr->GetStringDefault("SMTPMailConfirmationSubject", "");
    smtpData.SMTPReturnDomain = sConfigMgr->GetStringDefault("SMTPReturnDomain", "");

    if (std::filesystem::exists("newsletter.html"))
    {
        sendNewsletter(smtpData, k);
        return 0;
    }

    std::vector<VoteWebsiteData> voteWebsites(loadVoteWebsites());
    std::string refUrlHeader = sConfigMgr->GetStringDefault("RefUrlHeader", "");
    std::string refUrlFooter = sConfigMgr->GetStringDefault("RefUrlFooter", "");
    std::string ipFromHeader = sConfigMgr->GetStringDefault("IpFromHeader", "");

    #undef CPPHTTPLIB_THREAD_POOL_COUNT
    #define CPPHTTPLIB_THREAD_POOL_COUNT 0
    httplib::Server svr;

    svr.Get("/hello", [](const httplib::Request& req, httplib::Response& res)
    {
        res.set_content("Hello World!", "text/plain");
    });
    // FOR NEW ACCOUNT
	svr.Post(R"(/account/creation/email(?:/error-wrong)?(?:/error-double)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"(^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*$)");
        std::string hexString = req.matches[1];

        StateDataRegistration sdr;
        if (hexString != "")
            HexBytesStringToState(sdr, k, hexString);
		
        sdr.email = "";
        sdr.emailAvailable = false;
        sdr.emailVerified = false;
        sdr.captcha = std::vector<unsigned char>(6);
        randombytes(sdr.captcha.data(), 6);

		if (!req.has_param("emailAddress") || !std::regex_match(req.get_param_value("emailAddress"), pattern))
		{
            std::string newURL = sdr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdr.email = req.get_param_value("emailAddress");

        std::string newURL = sdr.whereToNext() + StateToHexBytesString(sdr, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/creation/password(?:/error-wrong)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"((?=[a-zA-Z0-9]{8,16}$)(?=.*\d)(?=.*[a-zA-Z]).*)");
        std::string hexString = req.matches[1];

        StateDataRegistration sdr;
        if (!HexBytesStringToState(sdr, k, hexString))
        {
			res.set_redirect(sdr.whereToNext().c_str());
			return;
        }

		if (!req.has_param("password") || !std::regex_match(req.get_param_value("password"), pattern))
		{
            std::string newURL = sdr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdr.password = req.get_param_value("password");

        std::string newURL = sdr.whereToNext() + StateToHexBytesString(sdr, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/creation/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/*$)",
        [k, &smtpData](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataRegistration sdr;
        if (!HexBytesStringToState(sdr, k, hexString))
        {
			res.set_redirect(sdr.whereToNext().c_str());
			return;
        }

        // should only be here if email and password have already been set
        if (sdr.email == "" || sdr.password == "")
        {
            res.set_redirect(sdr.whereToNext().c_str());
			return;
        }

        if (!req.has_param("captcha") || req.get_param_value("captcha").length() != 5)
		{
            sdr.captcha = std::vector<unsigned char>(6);
            randombytes(sdr.captcha.data(), 6);

            std::string newURL = sdr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        unsigned char im[70 * 200];
        captcha(im, sdr.captcha.data());
        unsigned char foundWrong = 0;
        for (int i = 0; i < 5; i++)
            if (req.get_param_value("captcha")[i] != (char)sdr.captcha[i])
                foundWrong++;

        // allow one mistake
        if (foundWrong > 1)
        {
            sdr.captcha = std::vector<unsigned char>(6);
            randombytes(sdr.captcha.data(), 6);

            std::string newURL = sdr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sdr.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        bool foundEmail = AccountMgr::GetId(sdr.email);
        StopLoginDB();

        if (foundEmail)
        {
            sdr.email = "";
            std::string newURL = sdr.whereToNext() + std::string("error-double/") + StateToHexBytesString(sdr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        std::string newURLFailure = sdr.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdr, k) + std::string("/");
        sdr.emailAvailable = true;
        std::string newURLSuccess = sdr.whereToNext() + StateToHexBytesString(sdr, k) + std::string("/");

        sdr.emailVerified = true;

        std::string verifiedEmailURL = smtpData.SMTPReturnDomain + sdr.whereToNext() + StateToHexBytesString(sdr, k) + std::string("/api");
        std::vector<std::string> tokenizedMailContent = getTokenizedFile("registration.mail", "<insert-url>");
        if (tokenizedMailContent.empty())
        {
			res.set_redirect(newURLFailure.c_str());
			return;
        }
        std::string emailBody(tokenizedMailContent[0]);
        for (size_t i = 1; i < tokenizedMailContent.size(); i++)
            emailBody += verifiedEmailURL + tokenizedMailContent[i];

        bool mailSuccess = true;
        SMTPMail mail;
        try
        {
            mail.open(smtpData.SMTPIP.c_str(), smtpData.SMTPPort.c_str(), smtp_connection_security::SMTP_SECURITY_NONE, smtp_flag::SMTP_NO_CERT_VERIFY, NULL);
            mail.auth(smtp_authentication_method::SMTP_AUTH_LOGIN, smtpData.SMTPUser.c_str(), smtpData.SMTPPassword.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_FROM, smtpData.SMTPFromAddress.c_str(), smtpData.SMTPFromName.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_TO, sdr.email.c_str(), NULL);
            mail.header_add("Subject", smtpData.SMTPMailRegistrationSubject.c_str());
            mail.header_add("MIME-Version", "1.0");
            mail.header_add("Content-Type", "multipart/alternative; boundary=dakjqwmnejadhasdkljkaeiasndansdahjdhjasd");
            mail.mail(emailBody.c_str());
        }
        catch (SMTPMailException&)
        {
            mailSuccess = false;
        }
        
        try
        {
            mail.close();
        }
        catch (SMTPMailException&) {}

        if (mailSuccess)
            res.set_redirect(newURLSuccess.c_str());
        else
            res.set_redirect(newURLFailure.c_str());
    });
    svr.Get(R"(/account/creation/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];
        res.set_content(ReturnCaptcha<StateDataRegistration>(req.matches[1], k), "image/gif");
    });
    svr.Get(R"(/account/creation/success(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataRegistration sdr;
        if (!HexBytesStringToState(sdr, k, hexString))
        {
			res.set_redirect(sdr.whereToNext().c_str());
			return;
        }

        // should only be here if the email verified has been set to true
        if (!sdr.emailVerified)
        {
            res.set_redirect(sdr.whereToNext().c_str());
			return;
        }

        // if the cookie is set this account is created after a referral
        StateAccountReference sar;
        uint32_t refererId = 0;
        if (GetCookieState(sar, k, "ref", req))
            refererId = sar.aId;

        if (!StartLoginDB())
        {
            std::string newURL = sdr.whereToNext() + std::string("error-internal/");
			res.set_redirect(newURL.c_str());
			return;
        }

        AccountOpResult result = AccountMgr::CreateAccount(sdr.email, sdr.password, false);
        // only set the referer if the creation was a success
        if (result == AccountOpResult::AOR_OK)
        {
            uint32_t createdAccountId = AccountMgr::GetId(sdr.email);

            if (createdAccountId)
            {
                PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_REFERER);
                stmt->setUInt32(0, refererId);
                stmt->setUInt32(1, createdAccountId);
                LoginDatabase.DirectExecute(stmt);
            }
        }

        StopLoginDB();

        if (result == AccountOpResult::AOR_NAME_ALREADY_EXIST)
        {
            std::string newURL = std::string("/account/creation/success/old/");
			res.set_redirect(newURL.c_str());
			return;
        }

        if (result != AccountOpResult::AOR_OK)
        {
            std::string newURL = sdr.whereToNext() + std::string("error-internal/");
			res.set_redirect(newURL.c_str());
			return;
        }

        std::string newURL = std::string("/account/creation/success/done/");
        res.set_redirect(newURL.c_str());
    });

    // RESET PASSWORD
	svr.Post(R"(/account/reset/email(?:/error-wrong)?(?:/not-found)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"(^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*$)");
        std::string hexString = req.matches[1];

        StateDataPasswordReset sdpr;
        if (hexString != "")
            HexBytesStringToState(sdpr, k, hexString);
		
        sdpr.validTillEpoch = 0;
        sdpr.email = "";
        sdpr.password = "";
        sdpr.emailVerified = false;
        sdpr.captcha = std::vector<unsigned char>(6);
        randombytes(sdpr.captcha.data(), 6);

		if (!req.has_param("emailAddress") || !std::regex_match(req.get_param_value("emailAddress"), pattern))
		{
            std::string newURL = sdpr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdpr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdpr.email = req.get_param_value("emailAddress");

        std::string newURL = sdpr.whereToNext() + StateToHexBytesString(sdpr, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/reset/password(?:/error-wrong)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"((?=[a-zA-Z0-9]{8,16}$)(?=.*\d)(?=.*[a-zA-Z]).*)");
        std::string hexString = req.matches[1];

        StateDataPasswordReset sdpr;
        if (!HexBytesStringToState(sdpr, k, hexString))
        {
			res.set_redirect(sdpr.whereToNext().c_str());
			return;
        }

		if (!req.has_param("password") || !std::regex_match(req.get_param_value("password"), pattern))
		{
            std::string newURL = sdpr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdpr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdpr.password = req.get_param_value("password");

        std::string newURL = sdpr.whereToNext() + StateToHexBytesString(sdpr, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/reset/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/*$)",
        [k, &smtpData](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataPasswordReset sdpr;
        if (!HexBytesStringToState(sdpr, k, hexString))
        {
			res.set_redirect(sdpr.whereToNext().c_str());
			return;
        }

        // should only be here if email and password have already been set
        if (sdpr.email == "" || sdpr.password == "")
        {
            res.set_redirect(sdpr.whereToNext().c_str());
			return;
        }

        if (!req.has_param("captcha") || req.get_param_value("captcha").length() != 5)
		{
            sdpr.captcha = std::vector<unsigned char>(6);
            randombytes(sdpr.captcha.data(), 6);

            std::string newURL = sdpr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdpr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        unsigned char im[70 * 200];
        captcha(im, sdpr.captcha.data());
        unsigned char foundWrong = 0;
        for (int i = 0; i < 5; i++)
            if (req.get_param_value("captcha")[i] != (char)sdpr.captcha[i])
                foundWrong++;

        // allow one mistake
        if (foundWrong > 1)
        {
            sdpr.captcha = std::vector<unsigned char>(6);
            randombytes(sdpr.captcha.data(), 6);

            std::string newURL = sdpr.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdpr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sdpr.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdpr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        bool foundEmail = AccountMgr::GetId(sdpr.email);
        StopLoginDB();

        if (!foundEmail)
        {
            sdpr.email = "";
            std::string newURL = sdpr.whereToNext() + std::string("not-found/") + StateToHexBytesString(sdpr, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        std::string newURLFailure = sdpr.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdpr, k) + std::string("/");
        // link from email is valid for 7 * 24 hours
        sdpr.validTillEpoch = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch() + std::chrono::hours(7 * 24)).count();
        std::string newURLSuccess = sdpr.whereToNext() + StateToHexBytesString(sdpr, k) + std::string("/");
        sdpr.emailVerified = true;

        std::string verifiedEmailURL = smtpData.SMTPReturnDomain + sdpr.whereToNext() + StateToHexBytesString(sdpr, k) + std::string("/api");
        std::vector<std::string> tokenizedMailContent = getTokenizedFile("reset.mail", "<insert-url>");
        if (tokenizedMailContent.empty())
        {
			res.set_redirect(newURLFailure.c_str());
			return;
        }
        std::string emailBody(tokenizedMailContent[0]);
        for (size_t i = 1; i < tokenizedMailContent.size(); i++)
            emailBody += verifiedEmailURL + tokenizedMailContent[i];

        bool mailSuccess = true;
        SMTPMail mail;
        try
        {
            mail.open(smtpData.SMTPIP.c_str(), smtpData.SMTPPort.c_str(), smtp_connection_security::SMTP_SECURITY_NONE, smtp_flag::SMTP_NO_CERT_VERIFY, NULL);
            mail.auth(smtp_authentication_method::SMTP_AUTH_LOGIN, smtpData.SMTPUser.c_str(), smtpData.SMTPPassword.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_FROM, smtpData.SMTPFromAddress.c_str(), smtpData.SMTPFromName.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_TO, sdpr.email.c_str(), NULL);
            mail.header_add("Subject", smtpData.SMTPMailResetSubject.c_str());
            mail.header_add("MIME-Version", "1.0");
            mail.header_add("Content-Type", "multipart/alternative; boundary=dakjqwmnejadhasdkljkaeiasndansdahjdhjasd");
            mail.mail(emailBody.c_str());
        }
        catch (SMTPMailException&)
        {
            mailSuccess = false;
        }
        
        try
        {
            mail.close();
        }
        catch (SMTPMailException&) {}

        if (mailSuccess)
            res.set_redirect(newURLSuccess.c_str());
        else
            res.set_redirect(newURLFailure.c_str());
    });
    svr.Get(R"(/account/reset/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];
        res.set_content(ReturnCaptcha<StateDataPasswordReset>(hexString, k), "image/gif");
    });
    svr.Get(R"(/account/reset/success(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataPasswordReset sdpr;
        if (!HexBytesStringToState(sdpr, k, hexString))
        {
			res.set_redirect(sdpr.whereToNext().c_str());
			return;
        }

        // should only be here if the email verified has been set to true
        if (!sdpr.emailVerified)
        {
            std::string newURL = sdpr.whereToNext() + StateToHexBytesString(sdpr, k) + std::string("/");
            res.set_redirect(newURL.c_str());
			return;
        }

        // this reset link is not valid anymore
        if (std::chrono::seconds(sdpr.validTillEpoch) < std::chrono::system_clock::now().time_since_epoch())
        {
            std::string newURL("/account/reset/success/expired/");
            res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sdpr.whereToNext() + std::string("error-internal/");
			res.set_redirect(newURL.c_str());
			return;
        }

        AccountMgr::ChangePassword(AccountMgr::GetId(sdpr.email), sdpr.password, false);
        StopLoginDB();

        std::string newURL("/account/reset/success/done/");
        res.set_redirect(newURL.c_str());
    });

    // CONVERT OLD ACCOUNT
    svr.Post(R"(/account/upgrade/current/username(?:/error-wrong)?(?:/not-found)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"(^.+$)");
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (hexString != "")
            HexBytesStringToState(sdu, k, hexString);
		
        sdu.currentAccountVerified = false;
        sdu.currentPassword = "";
        sdu.currentUsername = "";
        sdu.currentEmail = "";
        sdu.newEmail = "";
        sdu.newEmailAvailable = false;
        sdu.newEmailVerified = false;
        sdu.newPassword = "";
        sdu.usernameFound = false;
        sdu.skipPassword = false;
        sdu.captcha = std::vector<unsigned char>(6);
        randombytes(sdu.captcha.data(), 6);

		if (!req.has_param("username") || !std::regex_match(req.get_param_value("username"), pattern))
		{
            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdu.currentUsername = req.get_param_value("username");

        std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/upgrade/current/password(?:/error-wrong)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"(^[a-zA-Z0-9]*$)");
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (!HexBytesStringToState(sdu, k, hexString))
        {
			res.set_redirect(sdu.whereToNext().c_str());
			return;
        }

		if (!req.has_param("password") || !std::regex_match(req.get_param_value("password"), pattern))
		{
            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdu.currentPassword = req.get_param_value("password");
        sdu.currentAccountVerified = false;
        if (sdu.currentPassword == "")
            sdu.skipPassword = true;

        // we set a new captcha here, otherwise can keep trying passwords without a new captcha
        sdu.captcha = std::vector<unsigned char>(6);
        randombytes(sdu.captcha.data(), 6);

        std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/upgrade/current/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/*$)",
        [k, &smtpData](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (!HexBytesStringToState(sdu, k, hexString))
        {
			res.set_redirect(sdu.whereToNext().c_str());
			return;
        }

        // should only be here if email and password have already been set
        if (sdu.currentUsername == "" || (sdu.currentPassword == "" && !sdu.skipPassword))
        {
            res.set_redirect(sdu.whereToNext().c_str());
			return;
        }

        if (!req.has_param("captcha") || req.get_param_value("captcha").length() != 5)
		{
            sdu.captcha = std::vector<unsigned char>(6);
            randombytes(sdu.captcha.data(), 6);

            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        unsigned char im[70 * 200];
        captcha(im, sdu.captcha.data());
        unsigned char foundWrong = 0;
        for (int i = 0; i < 5; i++)
            if (req.get_param_value("captcha")[i] != (char)sdu.captcha[i])
                foundWrong++;

        // allow one mistake
        if (foundWrong > 1)
        {
            sdu.captcha = std::vector<unsigned char>(6);
            randombytes(sdu.captcha.data(), 6);

            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sdu.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_FOR_ACCOUNT_CONVERT);
        stmt->setString(0, sdu.currentUsername);
        stmt->setString(1, sdu.currentUsername);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        uint32 foundUsername = (result) ? (*result)[0].GetUInt32() : 0;
        bool correctPassword = false;
        std::string emailIfAvailable = "";
        if (foundUsername != 0)
        {
            correctPassword = AccountMgr::CheckPassword(foundUsername, sdu.currentPassword);

            if (!correctPassword)
            {
                // also check with our WoD password hash
                std::string toUpperUsername = sdu.currentUsername;
                std::string toUpperPassword = sdu.currentPassword;

                Utf8ToUpperOnlyLatin(toUpperUsername);
                Utf8ToUpperOnlyLatin(toUpperPassword);

                SHA1Hash sha;
                sha.Initialize();
                sha.UpdateData(toUpperUsername);
                sha.UpdateData(":");
                sha.UpdateData(toUpperPassword);
                sha.Finalize();

                stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_CHECK_PASSWORD);
                stmt->setUInt32(0, foundUsername);
                stmt->setString(1, ByteArrayToHexStr(sha.GetDigest(), sha.GetLength()));
                result = LoginDatabase.Query(stmt);
                correctPassword = (result) ? true : false;

                // even with the old hash method the password appears to be incorrect
                if (!correctPassword)
                {
                    stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_EMAIL_BY_ID_FOR_ACCOUNT_CONVERT);
                    stmt->setUInt32(0, foundUsername);
                    result = LoginDatabase.Query(stmt);

                    if (result)
                        emailIfAvailable = (*result)[0].GetString();
                }
            }
        }

        StopLoginDB();

        // whatever happens now, make sure we have a new captcha
        sdu.captcha = std::vector<unsigned char>(6);
        randombytes(sdu.captcha.data(), 6);

        if (foundUsername == 0)
        {
            sdu.currentUsername = "";
            std::string newURL = sdu.whereToNext() + std::string("not-found/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        sdu.usernameFound = true;
        if (correctPassword)
        {
            sdu.currentAccountVerified = true;
            std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        sdu.currentEmail = emailIfAvailable;
        const std::regex mailPattern(R"(^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*$)");
        if (!std::regex_match(emailIfAvailable, mailPattern))
        {
            sdu.currentEmail = "";
            std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
            return;
        }

        std::string newURLFailure = sdu.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdu, k) + std::string("/");
        std::string newURLSuccess = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
        sdu.currentAccountVerified = true;
        std::string verifiedEmailURL = smtpData.SMTPReturnDomain + sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
        std::vector<std::string> tokenizedMailContent = getTokenizedFile("validation.mail", "<insert-url>");
        if (tokenizedMailContent.empty())
        {
			res.set_redirect(newURLFailure.c_str());
			return;
        }
        std::string emailBody(tokenizedMailContent[0]);
        for (size_t i = 1; i < tokenizedMailContent.size(); i++)
            emailBody += verifiedEmailURL + tokenizedMailContent[i];

        bool mailSuccess = true;
        SMTPMail mail;
        try
        {
            mail.open(smtpData.SMTPIP.c_str(), smtpData.SMTPPort.c_str(), smtp_connection_security::SMTP_SECURITY_NONE, smtp_flag::SMTP_NO_CERT_VERIFY, NULL);
            mail.auth(smtp_authentication_method::SMTP_AUTH_LOGIN, smtpData.SMTPUser.c_str(), smtpData.SMTPPassword.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_FROM, smtpData.SMTPFromAddress.c_str(), smtpData.SMTPFromName.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_TO, sdu.currentEmail.c_str(), NULL);
            mail.header_add("Subject", smtpData.SMTPMailValidationSubject.c_str());
            mail.header_add("MIME-Version", "1.0");
            mail.header_add("Content-Type", "multipart/alternative; boundary=dakjqwmnejadhasdkljkaeiasndansdahjdhjasd");
            mail.mail(emailBody.c_str());
        }
        catch (SMTPMailException&)
        {
            mailSuccess = false;
        }
        
        try
        {
            mail.close();
        }
        catch (SMTPMailException&) {}

        if (mailSuccess)
            res.set_redirect(newURLSuccess.c_str());
        else
            res.set_redirect(newURLFailure.c_str());
    });
    svr.Get(R"(/account/upgrade/current/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];
        res.set_content(ReturnCaptcha<StateDataUpgrade>(hexString, k), "image/gif");
    });
    svr.Post(R"(/account/upgrade/new/email(?:/error-wrong)?(?:/error-double)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"(^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*$)");
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (hexString != "")
            HexBytesStringToState(sdu, k, hexString);
		
        sdu.captcha = std::vector<unsigned char>(6);
        randombytes(sdu.captcha.data(), 6);

		if (!req.has_param("emailAddress") || !std::regex_match(req.get_param_value("emailAddress"), pattern))
		{
            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdu.newEmail = req.get_param_value("emailAddress");
        sdu.newEmailAvailable = false;
        sdu.newEmailVerified = false;
        sdu.newPassword = "";

        std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/upgrade/new/password(?:/error-wrong)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
		const std::regex pattern(R"((?=[a-zA-Z0-9]{8,16}$)(?=.*\d)(?=.*[a-zA-Z]).*)");
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (!HexBytesStringToState(sdu, k, hexString))
        {
			res.set_redirect(sdu.whereToNext().c_str());
			return;
        }

		if (!req.has_param("password") || !std::regex_match(req.get_param_value("password"), pattern))
		{
            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sdu.newPassword = req.get_param_value("password");

        std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Post(R"(/account/upgrade/new/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/*$)",
        [k, &smtpData](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (!HexBytesStringToState(sdu, k, hexString))
        {
			res.set_redirect(sdu.whereToNext().c_str());
			return;
        }

        // should only be here if the new password/email have been set and the account has been verified
        if (sdu.newPassword == "" || sdu.newEmail == "" || !sdu.currentAccountVerified)
        {
            std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
            res.set_redirect(newURL.c_str());
			return;
        }

        if (!req.has_param("captcha") || req.get_param_value("captcha").length() != 5)
		{
            sdu.captcha = std::vector<unsigned char>(6);
            randombytes(sdu.captcha.data(), 6);

            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        unsigned char im[70 * 200];
        captcha(im, sdu.captcha.data());
        unsigned char foundWrong = 0;
        for (int i = 0; i < 5; i++)
            if (req.get_param_value("captcha")[i] != (char)sdu.captcha[i])
                foundWrong++;

        // allow one mistake
        if (foundWrong > 1)
        {
            sdu.captcha = std::vector<unsigned char>(6);
            randombytes(sdu.captcha.data(), 6);

            std::string newURL = sdu.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sdu.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        bool foundEmail = AccountMgr::GetId(sdu.newEmail);
        StopLoginDB();

        if (foundEmail)
        {
            sdu.newEmail = "";
            std::string newURL = sdu.whereToNext() + std::string("error-double/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        std::string newURLFailure = sdu.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sdu, k) + std::string("/");
        sdu.newEmailAvailable = true;
        std::string newURLSuccess = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");

        sdu.newEmailVerified = true;

        std::string verifiedEmailURL = smtpData.SMTPReturnDomain + sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/api");
        std::vector<std::string> tokenizedMailContent = getTokenizedFile("confirmation.mail", "<insert-url>");
        if (tokenizedMailContent.empty())
        {
			res.set_redirect(newURLFailure.c_str());
			return;
        }
        std::string emailBody(tokenizedMailContent[0]);
        for (size_t i = 1; i < tokenizedMailContent.size(); i++)
            emailBody += verifiedEmailURL + tokenizedMailContent[i];

        bool mailSuccess = true;
        SMTPMail mail;
        try
        {
            mail.open(smtpData.SMTPIP.c_str(), smtpData.SMTPPort.c_str(), smtp_connection_security::SMTP_SECURITY_NONE, smtp_flag::SMTP_NO_CERT_VERIFY, NULL);
            mail.auth(smtp_authentication_method::SMTP_AUTH_LOGIN, smtpData.SMTPUser.c_str(), smtpData.SMTPPassword.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_FROM, smtpData.SMTPFromAddress.c_str(), smtpData.SMTPFromName.c_str());
            mail.address_add(smtp_address_type::SMTP_ADDRESS_TO, sdu.newEmail.c_str(), NULL);
            mail.header_add("Subject", smtpData.SMTPMailConfirmationSubject.c_str());
            mail.header_add("MIME-Version", "1.0");
            mail.header_add("Content-Type", "multipart/alternative; boundary=dakjqwmnejadhasdkljkaeiasndansdahjdhjasd");
            mail.mail(emailBody.c_str());
        }
        catch (SMTPMailException&)
        {
            mailSuccess = false;
        }
        
        try
        {
            mail.close();
        }
        catch (SMTPMailException&) {}

        if (mailSuccess)
            res.set_redirect(newURLSuccess.c_str());
        else
            res.set_redirect(newURLFailure.c_str());
    });
    svr.Get(R"(/account/upgrade/new/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];
        res.set_content(ReturnCaptcha<StateDataUpgrade>(hexString, k), "image/gif");
    });
    svr.Get(R"(/account/upgrade/success/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateDataUpgrade sdu;
        if (!HexBytesStringToState(sdu, k, hexString))
        {
			res.set_redirect(sdu.whereToNext().c_str());
			return;
        }

        // should only be here if the newEmail verified has been set to true
        if (!sdu.newEmailVerified)
        {
            std::string newURL = sdu.whereToNext() + StateToHexBytesString(sdu, k) + std::string("/");
            res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sdu.whereToNext() + std::string("error-internal/");
			res.set_redirect(newURL.c_str());
			return;
        }

        // make absolutely certain no account already uses this email
        bool foundEmail = AccountMgr::GetId(sdu.newEmail);

        // but we should still check if the account needs to be upgraded
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_FOR_ACCOUNT_CONVERT);
        stmt->setString(0, sdu.currentUsername);
        stmt->setString(1, sdu.currentUsername);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        uint32 foundUser = (result) ? (*result)[0].GetUInt32() : 0;
        AccountOpResult changeUserNameResult = AccountOpResult::AOR_OK;

        // only if we still need to upgrade and no other account uses this email can we upgrade
        if (foundUser != 0 && !foundEmail)
        {
            changeUserNameResult = AccountMgr::ChangeUsername(foundUser, sdu.newEmail, sdu.newPassword, false);
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_EXPANSION);
            stmt->setUInt32(0, 6);
            stmt->setUInt32(1, foundUser);
            LoginDatabase.DirectExecute(stmt);
        }

        StopLoginDB();

        if (foundUser == 0)
        {
            std::string newURL = sdu.whereToNext() + std::string("old/");
            res.set_redirect(newURL.c_str());
			return;
        }
        else if (foundEmail)
        {
            sdu.newEmail = "";
            sdu.newPassword = "";
            sdu.newEmailAvailable = false;
            sdu.newEmailVerified = false;
            std::string newURL = sdu.whereToNext() + std::string("error-double/") + StateToHexBytesString(sdu, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }
        else if (changeUserNameResult != AccountOpResult::AOR_OK)
        {
            std::string newURL = sdu.whereToNext() + std::string("error-internal/");
			res.set_redirect(newURL.c_str());
			return;
        }
        else
        {
            std::string newURL = sdu.whereToNext() + std::string("done/");
			res.set_redirect(newURL.c_str());
			return;
        }
    });
    svr.Get(R"(/unsubscribe/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateUnsubscribe su;
        if (!HexBytesStringToState(su, k, hexString))
        {
			res.set_content("The URL for unsubscribing is invalid, please contant an admin if this problem persists.", "text/plain");
			return;
        }

        std::vector<std::string> unsubscribed = LoadJSonDatabase("unsubscribedEmailAddresses.json");
        auto it = find(unsubscribed.begin(), unsubscribed.end(), su.emailAddress);
        if (it != unsubscribed.end())
        {
            res.set_content("You have already been unsubscribed from any further emails, if you are still incorrectly receiving our emails, please contact an admin.", "text/plain");
            return;
        }

        unsubscribed.push_back(su.emailAddress);
        std::ofstream file("unsubscribedEmailAddresses.json");
        cereal::JSONOutputArchive archive(file);
        archive(unsubscribed);

        res.set_content("You have been successfully unsubscribed from any further emails! Please contact an admin to undo this if you change your mind :)", "text/plain");
    });

    // LOGIN
    svr.Post(R"(/account/login/credentials(?:/error-wrong)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateLogin sl;
        if (!HexBytesStringToState(sl, k, hexString))
        {
            sl.captcha = std::vector<unsigned char>(6);
            randombytes(sl.captcha.data(), 6);

            std::string newURL = sl.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        sl.emailAddress = "";
        sl.password = "";
        sl.captcha = std::vector<unsigned char>(6);
        randombytes(sl.captcha.data(), 6);

		if (!req.has_param("emailAddress") || !req.has_param("password"))
		{
            std::string newURL = sl.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        sl.emailAddress = req.get_param_value("emailAddress");
        sl.password = req.get_param_value("password");
        uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        uint64_t secondsTillExpire = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(1)).count();
        sl.validTill = now + secondsTillExpire;

        std::string newURL = sl.whereToNext() + StateToHexBytesString(sl, k) + std::string("/");
        res.set_redirect(newURL.c_str());
    });
    svr.Get(R"(/account/login/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];
        res.set_content(ReturnCaptcha<StateLogin>(hexString, k), "image/gif");
    });
    svr.Post(R"(/account/login/captcha(?:/error-wrong)?(?:/error-internal)?/*([a-zA-z0-9]*)/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateLogin sl;
        if (!HexBytesStringToState(sl, k, hexString))
        {
            sl.captcha = std::vector<unsigned char>(6);
            randombytes(sl.captcha.data(), 6);

            std::string newURL = sl.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        if (now >= sl.validTill || !req.has_param("captcha") || req.get_param_value("captcha").length() != 5 || sl.captcha.size() == 0)
		{
            sl.captcha = std::vector<unsigned char>(6);
            randombytes(sl.captcha.data(), 6);

            std::string newURL = sl.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
		}

        unsigned char im[70 * 200];
        captcha(im, sl.captcha.data());
        unsigned char foundWrong = 0;
        for (int i = 0; i < 5; i++)
            if (req.get_param_value("captcha")[i] != (char)sl.captcha[i])
                foundWrong++;

        // allow one mistake
        if (foundWrong > 1)
        {
            sl.captcha = std::vector<unsigned char>(6);
            randombytes(sl.captcha.data(), 6);

            std::string newURL = sl.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        if (!StartLoginDB())
        {
            std::string newURL = sl.whereToNext() + std::string("error-internal/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        uint32 accountId = AccountMgr::GetId(sl.emailAddress);
        bool correctPassword = false;
        if (accountId)
            correctPassword = AccountMgr::CheckPassword(accountId, sl.password);

        StopLoginDB();

        if (!correctPassword)
        {
            sl.emailAddress = "";
            sl.password = "";
            sl.captcha = std::vector<unsigned char>(6);
            randombytes(sl.captcha.data(), 6);

            std::string newURL = sl.whereToNext() + std::string("error-wrong/") + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
			return;
        }

        StateLoggedIn sli;
        sli.accountId = accountId;
        sli.email = sl.emailAddress;

        uint64_t secondsTillExpire = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(62 * 24)).count();
        sli.validTill = now + secondsTillExpire;

        std::string cookie = StateToHexBytesString(sli, k);

        res.set_header("Set-Cookie", std::string("loginSession=") + cookie + std::string("; Path=/; Max-Age=") + std::to_string(secondsTillExpire));
        res.set_redirect(sl.returnUrl.size() > 0 ? sl.returnUrl : "/");
    });

    // REFERENCE URL
    svr.Get(R"(/ref/*([a-zA-z0-9]*)/+api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        std::string hexString = req.matches[1];

        StateAccountReference sar;
        if (HexBytesStringToState(sar, k, hexString))
        {
            uint64_t secondsTillExpire = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(14 * 24)).count();
            res.set_header("Set-Cookie", std::string("ref=") + hexString + std::string("; Path=/; Max-Age=") + std::to_string(secondsTillExpire));
        }

        res.set_redirect("/");
    });
    svr.Get(R"(/account/myref/login/api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        StateLoggedIn sli;
        if (!GetCookieState(sli, k, "loginSession", req) || std::chrono::seconds(sli.validTill) < std::chrono::system_clock::now().time_since_epoch())
        {
            StateLogin sl;
            sl.returnUrl = "/account/myref/login/api/";
            std::string newURL = sl.whereToNext() + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
            return;
        }

        res.set_redirect("/account/myref/");
    });
    svr.Get(R"(/account/myref/switch/api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        StateLogin sl;
        sl.returnUrl = "/account/myref/login/api/";
        std::string newURL = sl.whereToNext() + StateToHexBytesString(sl, k) + std::string("/");
        res.set_redirect(newURL.c_str());
        return;
    });
    svr.Get(R"(/account/myref/api/*$)",
        [k, &smtpData](const httplib::Request& req, httplib::Response& res)
    {
        StateLoggedIn sli;
        if (!GetCookieState(sli, k, "loginSession", req) || std::chrono::seconds(sli.validTill) < std::chrono::system_clock::now().time_since_epoch())
        {
            res.set_content("", "text/css");
            return;
        }

        StateAccountReference sar;
        sar.aId = sli.accountId;
        std::string refUrl(smtpData.SMTPReturnDomain + std::string("/ref/") + StateToHexBytesString(sar, k) + std::string("/api/"));

        std::string output = "";
        output += std::string("#idaccount::before {\n");
        output += std::string("content:\"") + sli.email + std::string("\";\n");
        output += std::string("}\n");
        output += std::string("#idurl::before {\n");
        output += std::string("content:\"") + refUrl + std::string("\";\n");
        output += std::string("}\n");
    
        res.set_content(output, "text/css");
    });
    svr.Get(R"(/account/myref/iframe/api/*$)",
        [k, &smtpData, &refUrlHeader, &refUrlFooter](const httplib::Request& req, httplib::Response& res)
    {
        StateLoggedIn sli;
        if (!GetCookieState(sli, k, "loginSession", req) || std::chrono::seconds(sli.validTill) < std::chrono::system_clock::now().time_since_epoch())
        {
            res.set_content("", "text/html");
            return;
        }

        StateAccountReference sar;
        sar.aId = sli.accountId;
        std::string refUrl(smtpData.SMTPReturnDomain + std::string("/ref/") + StateToHexBytesString(sar, k) + std::string("/api/"));

        std::string output = "";
        output += refUrlHeader;
        output += std::string("\n") + refUrl + std::string("\n");
        output += refUrlFooter;

        res.set_content(output, "text/html");
    });

    // VOTING
    for (const auto& voteWebsite : voteWebsites)
    {
        std::string callbackRegex = voteWebsite.callbackUrl + std::string("/*$");
        svr.Get(callbackRegex,
            [k, &voteWebsite, &ipFromHeader](const httplib::Request& req, httplib::Response& res)
        {
            handleVoteCallback(voteWebsite, ipFromHeader, req, k);
        });
        svr.Post(callbackRegex,
            [k, &voteWebsite, &ipFromHeader](const httplib::Request& req, httplib::Response& res)
        {
            handleVoteCallback(voteWebsite, ipFromHeader, req, k);
        });
        svr.Post(std::string("/account/vote/redirect/") + std::to_string(voteWebsite.id) + std::string("/api/*$"),
            [k, &voteWebsite](const httplib::Request& req, httplib::Response& res)
        {
            StateLoggedIn sli;
            if (!GetCookieState(sli, k, "loginSession", req) || std::chrono::seconds(sli.validTill) < std::chrono::system_clock::now().time_since_epoch())
            {
                StateLogin sl;
                sl.returnUrl = std::string("/account/vote/redirect/") + std::to_string(voteWebsite.id) + std::string("/api/");
                std::string newURL = sl.whereToNext() + StateToHexBytesString(sl, k) + std::string("/");
                res.set_redirect(newURL.c_str());
                return;
            }

            uint64_t secondsTillExpire = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(16)).count();
            std::string cookie = StateToHexBytesString(sli, k);
            res.set_header("Set-Cookie", std::string("vote") + std::to_string(voteWebsite.id) + std::string("=") +
                cookie + std::string("; Path=/; Max-Age=") + std::to_string(secondsTillExpire));

            StateAccountReference sar;
            sar.aId = sli.accountId;
            std::string redirectUrl = voteWebsite.voteUrl + StateToHexBytesString(sar, k, voteWebsite.callbackIdOnlyDigits);
            res.set_redirect(redirectUrl);
        });
    }
    svr.Get(R"(/account/vote/login/api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        StateLoggedIn sli;
        if (!GetCookieState(sli, k, "loginSession", req) || std::chrono::seconds(sli.validTill) < std::chrono::system_clock::now().time_since_epoch())
        {
            StateLogin sl;
            sl.returnUrl = "/account/vote/login/api/";
            std::string newURL = sl.whereToNext() + StateToHexBytesString(sl, k) + std::string("/");
			res.set_redirect(newURL.c_str());
            return;
        }

        res.set_redirect("/account/vote/");
    });
    svr.Get(R"(/account/vote/switch/api/*$)",
        [k](const httplib::Request& req, httplib::Response& res)
    {
        StateLogin sl;
        sl.returnUrl = "/account/vote/login/api/";
        std::string newURL = sl.whereToNext() + StateToHexBytesString(sl, k) + std::string("/");
        res.set_redirect(newURL.c_str());
        return;
    });
    svr.Get(R"(/account/vote/api/*$)",
        [k, &voteWebsites](const httplib::Request& req, httplib::Response& res)
    {
        StateLoggedIn sli;
        if (!GetCookieState(sli, k, "loginSession", req) || std::chrono::seconds(sli.validTill) < std::chrono::system_clock::now().time_since_epoch())
        {
            res.set_content("", "text/css");
            return;
        }

        std::string output = "";
        output += std::string("#idaccount::before {\n");
        output += std::string("content:\"") + sli.email + std::string("\";\n");
        output += std::string("}\n");

        for (const auto& voteWebsite : voteWebsites)
        {
            output += std::string("#vote-div") + std::to_string(voteWebsite.id) +  std::string("{display:block;}\n");
            output += std::string(".vote-name") + std::to_string(voteWebsite.id) + std::string("::after {\n");
            output += std::string("content:\"") + voteWebsite.name + std::string("\";\n");
            output += std::string("}\n");

            StateLoggedIn sli;
            if (GetCookieState(sli, k, std::string("vote") + std::to_string(voteWebsite.id), req))
                output += std::string("#vote-button-disabled") + std::to_string(voteWebsite.id) + std::string("{display:block;}\n");
            else
                output += std::string("#vote-button-enable") + std::to_string(voteWebsite.id) + std::string("{display:block;}\n");
        }

        res.set_content(output, "text/css");
    });

    svr.listen(bind_ip.c_str(), port);
	return 0;
}
