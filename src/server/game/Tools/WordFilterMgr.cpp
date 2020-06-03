/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WordFilterMgr.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include <boost/locale/encoding_utf.hpp>

#define MAX_SIZE_SENTENCE 27

WordFilterMgr::WordFilterMgr() 
{
}

WordFilterMgr::~WordFilterMgr()
{
}

WordFilterMgr* WordFilterMgr::instance()
{
    static WordFilterMgr instance;
    return &instance;
}

void WordFilterMgr::LoadLetterAnalogs()
{
    uint32 oldMSTime = getMSTime();

    m_letterAnalogs.clear();

    QueryResult result = WorldDatabase.Query("SELECT letter, analogs FROM letter_analogs");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,">> Loaded 0 letter analogs. DB table `letter_analogs` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        std::wstring letter = boost::locale::conv::utf_to_utf<wchar_t>(fields[0].GetString()); // !fields[0].GetInt8()
        std::wstring analogs = boost::locale::conv::utf_to_utf<wchar_t>(fields[1].GetString());

        for (uint32 i = 0; i < analogs.size(); ++i)
            m_letterAnalogs[analogs[i]] = letter[0];

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,">> Loaded %u (%u) letter analogs in %u ms", count, m_letterAnalogs.size(), GetMSTimeDiffToNow(oldMSTime));
}


void WordFilterMgr::LoadBadWords()
{
    uint32 oldMSTime = getMSTime();

    m_badWords.clear();
    m_badWordsMail.clear();

    QueryResult result = WorldDatabase.Query("SELECT `bad_word`, `convert` FROM bad_word");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,">> Loaded 0 bad words. DB table `bad_word` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        std::string analog = fields[0].GetString();
        std::string convert = fields[1].GetString();

        if (convert.empty())
            AddBadWord(analog);
        else
            AddConvert(convert);

        ++count;
    }
    while (result->NextRow());

    result = WorldDatabase.Query("SELECT bad_word FROM bad_word_mail");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,">> Loaded 0 bad words. DB table `bad_word_mail` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        std::string analog = fields[0].GetString();

        AddBadWordMail(analog);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING,">> Loaded %u bad words in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WordFilterMgr::LoadBadSentences()
{
    uint32 oldMSTime = getMSTime();

    m_badSentences.clear();
    hashById.clear();

    QueryResult result = CharacterDatabase.Query("SELECT hash, sentence, id, penalty, sourceMask, output FROM bad_sentences order by id");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 bad sentences. DB table `bad_sentences` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        size_t hash_bd = fields[0].GetUInt64();
               
        size_t hash = FilterMsgAndGetHash(fields[1].GetString());

        if (m_badSentences.find(hash) != m_badSentences.end())
        {
            CharacterDatabase.PExecute("DELETE FROM bad_sentences where id = %u", fields[2].GetUInt32());
            continue;
        }
        
        if (hash != hash_bd)
            CharacterDatabase.PExecute("UPDATE bad_sentences set hash = " UI64FMTD " where id = %u", hash, fields[2].GetUInt32());
        
        
        BadSentenceInfo & info = m_badSentences[hash];
        info.text = fields[1].GetString();
        info.id = fields[2].GetUInt32();
        lastIdBadSentences = info.id;
        info.penalty = fields[3].GetInt32();
        info.sourceMask = fields[4].GetUInt32();
        if (fields[5].GetString().empty())
        {
            std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(info.text);
            GeneralFilterWstring(_badWord);
            if (_badWord.size() > MAX_SIZE_SENTENCE)
                _badWord.resize(MAX_SIZE_SENTENCE);
            std::string _badWordEsc = boost::locale::conv::utf_to_utf<char>(_badWord);
            CharacterDatabase.EscapeString(_badWordEsc);
            CharacterDatabase.PExecute("UPDATE bad_sentences set output = '%s' where id = %u", _badWordEsc.c_str(), info.id);
        }

        hashById[info.id] = hash;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u bad sentences in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WordFilterMgr::LoadComplaints()
{
    uint32 oldMSTime = getMSTime();

    m_complaints.clear();

    QueryResult result = CharacterDatabase.Query("SELECT ID, ReportPlayer, SpammerGuid, MessageLog FROM report_complaints where JustBanned = 0 and (UNIX_TIMESTAMP() - (ReportTime-TimeSinceOffence)) <= 86400");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 report_complaints. DB table `report_complaints` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        AddComplaintForUser(ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64()),
                            ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64()), fields[0].GetUInt32(), fields[3].GetString());
        
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u report_complaints in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}


std::string WordFilterMgr::FindBadWord(std::string const& text, bool mail)
{
    std::wstring _text = boost::locale::conv::utf_to_utf<wchar_t>(text);
    
    GeneralFilterWstring(_text);

    if (_text.empty() || m_badWords.empty())
        return "";

    for (BadWordMap::const_iterator it = m_badWords.begin(); it != m_badWords.end(); ++it)
    {
        if (_text.find(*it) != std::wstring::npos)
        {
            // TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Compare text %s _text %s *it %s", text.c_str(), boost::locale::conv::utf_to_utf<char>(_text).c_str(), boost::locale::conv::utf_to_utf<char>(*it).c_str());
            return boost::locale::conv::utf_to_utf<char>(*it);
        }
    }

    if(mail)
    {
        for (BadWordMapMail::const_iterator it = m_badWordsMail.begin(); it != m_badWordsMail.end(); ++it)
        {
        //    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Compare %s and %s", boost::locale::conv::utf_to_utf<char>(*it).c_str(), boost::locale::conv::utf_to_utf<char>(_text).c_str());
            if (_text.find(*it) != std::wstring::npos)
                return boost::locale::conv::utf_to_utf<char>(*it);
        }
    }
    return "";
}

bool WordFilterMgr::AddBadWord(std::string const& badWord, bool toDB)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);

    GeneralFilterWstring(_badWord);

    // is already exist
    if (m_badWords.find(_badWord) != m_badWords.end())
    {
        WorldDatabase.PQuery("DELETE FROM bad_word WHERE bad_word = '%s'", badWord.c_str());
        return false;
    }

    m_badWords.insert(_badWord);

    if (toDB)
        WorldDatabase.PQuery("REPLACE INTO bad_word VALUES ('%s', '%s')", badWord.c_str(), boost::locale::conv::utf_to_utf<char>(_badWord).c_str());
    else
        WorldDatabase.PQuery("UPDATE bad_word SET `convert` = '%s' WHERE bad_word = '%s'", boost::locale::conv::utf_to_utf<char>(_badWord).c_str(), badWord.c_str());

    return true;
}

void WordFilterMgr::AddConvert(std::string const& badWord)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);

    // is already exist
    if (m_badWords.find(_badWord) != m_badWords.end())
        return;

    m_badWords.insert(_badWord);
}

bool WordFilterMgr::AddBadWordMail(std::string const& badWord, bool toDB)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);
    GeneralFilterWstring(_badWord);

    // is already exist
    if (m_badWordsMail.find(_badWord) != m_badWordsMail.end())
        return false;

    m_badWordsMail.insert(_badWord);

    if (toDB)
        WorldDatabase.PQuery("REPLACE INTO bad_word_mail VALUES ('%s')", badWord.c_str());

    return true;
}

bool WordFilterMgr::RemoveBadWord(std::string const& badWord, bool fromDB)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);
    GeneralFilterWstring(_badWord);

    // is not exist
    BadWordMap::iterator it = m_badWords.find(_badWord);
    if (it == m_badWords.end())
        return false;

    m_badWords.erase(it);

    if (fromDB)
        WorldDatabase.PExecute("DELETE FROM bad_word WHERE `bad_word` = '%s'", badWord.c_str());

    return true;
}

bool WordFilterMgr::AddBadSentence(std::string badSentence, size_t hash, uint32 mask, int32 penalty /* = 10 */)
{
    if(m_badSentences[hash].id != 0 && mask == m_badSentences[hash].sourceMask && penalty == m_badSentences[hash].penalty)
        return false;

    m_badSentences[hash].text = badSentence;
    m_badSentences[hash].id = (m_badSentences[hash].id == 0 ? ++lastIdBadSentences : m_badSentences[hash].id);
    m_badSentences[hash].penalty = penalty;
    m_badSentences[hash].sourceMask = m_badSentences[hash].sourceMask | mask;

    hashById[m_badSentences[hash].id] = hash;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_BAD_SENTENCES);
    stmt->setUInt32(0, m_badSentences[hash].id);
    stmt->setString(1, m_badSentences[hash].text);
    stmt->setUInt64(2, hash);
    stmt->setInt32(3, penalty);
    stmt->setUInt32(4, mask);

    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(m_badSentences[hash].text);

    GeneralFilterWstring(_badWord);

    if (_badWord.size() > MAX_SIZE_SENTENCE)
        _badWord.resize(MAX_SIZE_SENTENCE);

    stmt->setString(5, boost::locale::conv::utf_to_utf<char>(_badWord).c_str());

    CharacterDatabase.Execute(stmt);

    return true;
}

void WordFilterMgr::UpdatePenaltyForBadSentenceById(uint32 id, int32 penalty)
{
    size_t hash = hashById[id];
    m_badSentences[hash].penalty = penalty;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_BAD_SENTENCES);
    stmt->setInt32(0, penalty);
    stmt->setUInt32(1, id);
    CharacterDatabase.Execute(stmt);
}

size_t WordFilterMgr::FilterMsgAndGetHash(std::string lowedMsg, LocaleConstant locale /*= LOCALE_enUS*/, bool* isValid /*= nullptr*/)
{
    std::wstring letters = boost::locale::conv::utf_to_utf<wchar_t>(sObjectMgr->GetTrinityString(20087, LOCALE_ruRU)); // all eng + all rus chars

    std::wstring wstrLowed = boost::locale::conv::utf_to_utf<wchar_t>(lowedMsg);

    GeneralFilterWstring(wstrLowed, locale, isValid);

    //if (isValid && !(*isValid))
    //    return 0;

    if (wstrLowed.size() > MAX_SIZE_SENTENCE)
        wstrLowed.resize(MAX_SIZE_SENTENCE);
    
    size_t hash;
    if (wstrLowed.size() <= 2) // but was 5
        hash = hash_fn(boost::locale::conv::utf_to_utf<wchar_t>(lowedMsg)); // origial message
    else
        hash = hash_fn(wstrLowed);

    return hash;
}

void WordFilterMgr::GeneralFilterWstring(std::wstring& _text, LocaleConstant locale /*= LOCALE_enUS*/, bool* isValid /*= nullptr*/)
{
    if (_text.empty())
        return;

    std::transform(_text.begin(), _text.end(), _text.begin(), ::tolower);

    std::wstring letters = boost::locale::conv::utf_to_utf<wchar_t>(sObjectMgr->GetTrinityString(20087, LOCALE_ruRU)); // all eng + all rus chars
    for (uint16 i = 0; i < _text.size(); ++i)
    {
        auto posItr = letters.find(_text[i]);
        if (posItr != std::wstring::npos && posItr >= 52 + 33)  // big russian symbols to lower
            _text[i] = letters[posItr - 33];
    }
    

    std::string examples = sObjectMgr->GetTrinityString(20087, LOCALE_enUS); // some strange symbols like {треугольник}
    Tokenizer tokens(examples, ' ');

    for (auto exstring : tokens)
    {
        std::wstring example = boost::locale::conv::utf_to_utf<wchar_t>(exstring);
        do
        {
            auto first = _text.find(example);
            if (first == std::wstring::npos)
                break;
            _text.erase(first, example.size());
        } while (true);
    }

    std::wstring symbols = boost::locale::conv::utf_to_utf<wchar_t>(sObjectMgr->GetTrinityString(20087, LOCALE_koKR)); // all correct symbols

    bool tempValid = true;
    uint8 rusSymbols = 0;
    for (uint8 i = 0; i < _text.size() && i < 50;)
    {
        
     /*   auto it = m_letterAnalogs.find(_text[i]);
        if (it != m_letterAnalogs.end())
            _text[i] = (*it).second; */

        auto posItr = letters.find(_text[i]);
        if (posItr == letters.npos) // if this char not a-z а-я, then erase it
        {
            if (isValid)
                if (symbols.find(_text[i]) == symbols.npos) // it is not correct symbols too
                {
                    if (locale == LOCALE_ruRU || locale == LOCALE_enUS)
                        *isValid = false;
                    else
                        tempValid = false;
                }

            _text.erase(i, 1);
        }
        else
        {
            if (posItr >= 52)
                ++rusSymbols;
            ++i;
        }
    }

    if (!tempValid && (float(rusSymbols) / std::min(float(_text.size()), 50.0f)) >= 0.9f)
        *isValid = false;
}

WordFilterMgr::BadWordMap WordFilterMgr::GetBadWords() const
{
    return m_badWords;
}


uint32 WordFilterMgr::GetIdOfBadSentence(size_t hash)
{
    return m_badSentences[hash].id;
}

int32 WordFilterMgr::GetPenaltyForBadSentence(size_t hash)
{
    if (m_badSentences[hash].id)
        return m_badSentences[hash].penalty;
    else
        return 0;
}

uint32 WordFilterMgr::GetSourceMaskForBadSentence(size_t hash)
{
    if (m_badSentences[hash].id)
        return m_badSentences[hash].sourceMask;
    else
        return 0;
}

std::vector<BadSentenceInfo> WordFilterMgr::GetBadSentenceList(uint32 page)
{
    std::vector<BadSentenceInfo> list{};
    for (uint32 i = 1 + (page - 1) * 25; i <= page * 25 && i <= lastIdBadSentences; ++i)
        if (hashById[i])
            list.push_back(m_badSentences[hashById[i]]);

    return list;
}

bool WordFilterMgr::AddComplaintForUser(const ObjectGuid & offender, const ObjectGuid & complainant, uint64 complaintId, const std::string & fullInfo)
{
    ComplaintInfoUser& info = m_complaints[offender];

    auto itr = fullInfo.find("Text: [");
    if (itr == std::string::npos)
        return false;

    std::string text(fullInfo, itr+7);

    if (text.empty())
        return false;

    text.pop_back(); // delete ]

    if (text.empty())
        return false;

    if (info.m_muteTime > time(NULL) && info.m_muteCount >= 2) // just banned maximum ??
        return false;

    if (info.m_complaintsByUsers.find(complainant) != info.m_complaintsByUsers.end())
        return false;

    info.m_complaintsByUsers[complainant] = complaintId;

    WorldIntConfigs config = (info.m_muteCount <= 2 ? WorldIntConfigs(CONFIG_COMPLAINTS_PENALTY1 + info.m_muteCount) : WorldIntConfigs(CONFIG_COMPLAINTS_PENALTY1 + 2));

    if (sWorld->getIntConfig(CONFIG_COMPLAINTS_REQUIRED) != 0 && sWorld->getIntConfig(config) != 0
        && info.m_complaintsByUsers.size() >= sWorld->getIntConfig(CONFIG_COMPLAINTS_REQUIRED))
    {
        std::stringstream ss_complaintIds;
        for (auto& pair : info.m_complaintsByUsers)
            ss_complaintIds << pair.second << ", ";

        std::string complaintIds = ss_complaintIds.str();
        for (int i = 0; i < 2; ++i) // clear ", "
            complaintIds.pop_back();

        std::stringstream ss;
        ss << "Complaints: " << complaintIds.c_str() << " Text: " << text.c_str();

        if (sWorld->getIntConfig(config) < 0)
        {
            std::string name;
            if (ObjectMgr::GetPlayerNameByGUID(offender, name))
                sWorld->BanCharacter(name, "-1", ss.str().c_str(), "Server");
            else if (uint32 accountid = ObjectMgr::GetPlayerAccountIdByGUID(offender))
            {
                PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO);
                stmt->setInt32(0, int32(realm.Id.Realm));
                stmt->setUInt32(1, accountid);
                PreparedQueryResult result = LoginDatabase.Query(stmt);

                if (result)
                {
                    Field* fields = result->Fetch();
                    std::string accountName = fields[0].GetString();

                    sWorld->BanAccount(BAN_ACCOUNT, accountName.c_str(), "-1", ss.str().c_str(), "Server");
                    info.m_muteTime = 4294967295;
                }
            }
        }
        else if (uint32 accountid = ObjectMgr::GetPlayerAccountIdByGUID(offender))
        {
            sWorld->MuteAccount(accountid, sWorld->getIntConfig(config), ss.str().c_str(), "Server");

            info.m_muteTime = time(NULL) + sWorld->getIntConfig(config)*MINUTE;
        }

        ++info.m_muteCount;

        CharacterDatabase.PExecute("UPDATE report_complaints set JustBanned = 1 where ID in (%s)", complaintIds.c_str());
        info.m_complaintsByUsers.clear();
    }

    return true;
}
