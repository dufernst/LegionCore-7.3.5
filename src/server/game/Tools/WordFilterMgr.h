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

#ifndef TRINITYCORE_WORDFILTERMGR_H
#define TRINITYCORE_WORDFILTERMGR_H

// #include <locale>
// #include <codecvt>

struct BadSentenceInfo
{
    std::string text{};
    uint32 id = 0; 
    int32 penalty = 0; 
    uint32 sourceMask = 0;
};


typedef std::map<ObjectGuid, uint64> ComplaintsByUsers; // guid -> complaint id
struct ComplaintInfoUser
{
    ComplaintsByUsers m_complaintsByUsers{};
    uint64 m_muteTime = 0;
    uint32 m_muteCount = 0;
};

typedef std::unordered_map<ObjectGuid, ComplaintInfoUser> ComplaintsInfo; // guid of offender -> him complaints

enum BadSentenceMask : uint32
{
    BS_PM       = 0x00000001,
    BS_MAIL     = 0x00000002,
    BS_LFG      = 0x00000004,
};

struct PlayerFloodInfo
{
    uint32 lastSayTimeLFG = 0;
    uint32 lastSayTimeGeneral = 0;
    std::map<size_t, uint32> phrasesCheck{};
    std::set<ObjectGuid> players{};

    std::set<std::string> mailPhrases{};
    std::set<size_t> mailFoundedBadWords{};
};

class WordFilterMgr
{
    WordFilterMgr();
    ~WordFilterMgr();

public:
    static WordFilterMgr* instance();

    /// Note for 0.6v : Will used std::map instead of std::unordered_map, because of problems with cross-platform compilation.
    // [letter][analogs] 
    typedef std::map<wchar_t, wchar_t> LetterAnalogMap;
    // [converted][original]
    typedef std::set<std::wstring> BadWordMap;
    typedef std::set<std::wstring> BadWordMapMail;
    typedef std::map <size_t, BadSentenceInfo> BadSentences;

    void LoadLetterAnalogs();
    void LoadBadWords();
    void LoadBadSentences();
    void LoadComplaints();

    std::string FindBadWord(std::string const& text, bool mail = false);

    // manipulations with container 
    bool AddBadWord(std::string const& badWord, bool toDB = false);
    void AddConvert(std::string const& badWord);
    bool AddBadWordMail(std::string const& badWord, bool toDB = false);
    bool RemoveBadWord(std::string const& badWord, bool fromDB = false);

    bool AddBadSentence(std::string badSentence, size_t hash, uint32 mask, int32 penalty = 10);
    void UpdatePenaltyForBadSentenceById(uint32 id, int32 penalty);

    size_t FilterMsgAndGetHash(std::string lowedMsg, LocaleConstant locale = LOCALE_enUS, bool* isValid = nullptr);
    void GeneralFilterWstring(std::wstring& msg, LocaleConstant locale = LOCALE_enUS, bool* isValid = nullptr);

    // element (const) accessor 
    BadWordMap GetBadWords() const;

    uint32 GetIdOfBadSentence(size_t hash);
    int32 GetPenaltyForBadSentence(size_t hash);
    uint32 GetSourceMaskForBadSentence(size_t hash);
    std::vector<BadSentenceInfo> GetBadSentenceList(uint32 page);
    uint32 GetLastIdBadSentences() const { return lastIdBadSentences;}

    PlayerFloodInfo& GetFloodInfo(uint32 accountid) { return m_floodInfo[accountid]; }
    bool AddComplaintForUser(const ObjectGuid& offender, const ObjectGuid& complainant, uint64 complaintId, const std::string& text);
private:
    LetterAnalogMap m_letterAnalogs;
    BadWordMap m_badWords;
    BadWordMapMail m_badWordsMail;

    BadSentences m_badSentences{};
    std::map<uint32, size_t> hashById{};
    uint32 lastIdBadSentences{};
    std::hash<std::wstring> hash_fn;
    
    ComplaintsInfo m_complaints{};
    std::map<uint32, PlayerFloodInfo> m_floodInfo{}; // account -> info
};

#define sWordFilterMgr WordFilterMgr::instance()

#endif