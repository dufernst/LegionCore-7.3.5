#ifndef DataContainersH
#define DataContainersH

#include <boost/any.hpp>

namespace Trinity
{
    class AnyDataContainer
    {
        std::unordered_map<std::string, boost::any> dataMap;

    public:
        template<typename T>
        void Set(std::string const& key, T value)
        {
            dataMap[key] = value;
        }

        template<typename T>
        T GetValue(std::string const& key, T defaultValue = T()) const
        {
            auto itr = dataMap.find(key);
            if (itr != dataMap.end())
                return boost::any_cast<T>(itr->second);
            return defaultValue;
        }

        bool Exist(std::string const& key) const
        {
            return dataMap.find(key) != dataMap.end();
        }

        void Remove(std::string const& key)
        {
            dataMap.erase(key);
        }

        uint32 Increment(std::string const& key, uint32 increment = 1)
        {
            auto currentValue = GetValue<uint32>(key, uint32(0));
            Set(key, currentValue += increment);
            return currentValue;
        }

        bool IncrementOrProcCounter(std::string const& key, uint32 maxVal, uint32 increment = 1)
        {
            auto newValue = Increment(key, increment);
            if (newValue < maxVal)
                return false;

            Remove(key);
            return true;
        }
    };
}

#endif
