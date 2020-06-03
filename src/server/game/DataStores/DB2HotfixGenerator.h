
#ifndef DB2HotfixGenerator_h__
#define DB2HotfixGenerator_h__

#include "DB2Store.h"
#include <initializer_list>

class DB2HotfixGeneratorBase
{
public:
    static void LogMissingRecord(std::string const& storageName, uint32 recordId);
    static void AddClientHotfix(uint32 tableHash, uint32 recordId);
};

template<class T>
class DB2HotfixGenerator : private DB2HotfixGeneratorBase
{
public:
    explicit DB2HotfixGenerator(DB2Storage<T>& storage) : _storage(storage), _count(0) { }

    void ApplyHotfix(uint32 id, void(*fixer)(T*), bool notifyClient = false) { ApplyHotfix({ id }, fixer, notifyClient); }
    void ApplyHotfix(std::initializer_list<uint32> ids, void(*fixer)(T*), bool notifyClient = false) { ApplyHotfix(ids.begin(), ids.end(), fixer, notifyClient); }

    template<class I, class = typename std::enable_if<!std::is_void<decltype(*std::begin(std::declval<I>()))>::value>::type>
    void ApplyHotfix(I const& ids, void(*fixer)(T*), bool notifyClient = false) { ApplyHotfix(std::begin(ids), std::end(ids), fixer, notifyClient); }

    uint32 GetAppliedHotfixesCount() const { return _count; }

private:
    void ApplyHotfix(uint32 const* begin, uint32 const* end, void(*fixer)(T*), bool notifyClient)
    {
        while (begin != end)
        {
            uint32 id = *begin++;
            T const* entry = _storage.LookupEntry(id);
            if (!entry)
            {
                DB2HotfixGeneratorBase::LogMissingRecord(_storage.GetFileName().c_str(), id);
                continue;
            }

            fixer(const_cast<T*>(entry));
            ++_count;

            if (notifyClient)
                DB2HotfixGeneratorBase::AddClientHotfix(_storage.GetTableHash(), id);
        }
    }


    DB2Storage<T>& _storage;
    uint32 _count;
};

#endif // DB2HotfixGenerator_h__
