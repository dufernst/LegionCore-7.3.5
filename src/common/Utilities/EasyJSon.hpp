
#pragma once

#include <sstream>
#include <string>
#include <map>
#include <type_traits>

namespace EasyJSon
{
    enum NodeType
    {
        NT_INDEXED_ARRAY,
        NT_ARRAY,
        NT_DATA,
        NT_DATA_INT
    };

    template <class StringAllocator> class Node
    {
    public:
        const NodeType & Type()
        {
            return _type;
        }

        Node<StringAllocator> & operator[](const StringAllocator & name)
        {
            _type = NT_INDEXED_ARRAY;

            if (_childNodes.find(name) == _childNodes.end())
                _childNodes[name] = Node();

            return _childNodes[name];
        }

        Node<StringAllocator> & operator[](const size_t & id);

        size_t Size()
        {
            return _childNodes.size();
        }

        const StringAllocator & GetData()
        {
            return _data;
        }

        inline void SetData(const StringAllocator & data, bool isArithmetic = false)
        {
            _type = isArithmetic ? NT_DATA_INT : NT_DATA;
            _data = data;
        }

        const std::map<StringAllocator, Node<StringAllocator>> & GetChilds()
        {
            return _childNodes;
        }

        template<class StringAllocatorStream> inline StringAllocator Serialize(bool pretty, int level = 0);

        template<typename T> inline Node & operator=(T data);

    private:
        NodeType _type;
        StringAllocator _data;
        std::map<StringAllocator, Node<StringAllocator>> _childNodes;
    };

    template <> inline Node<std::string> & Node<std::string>::operator[](const size_t & id)
    {
        _type = NT_ARRAY;

        std::string keyName = std::to_string((unsigned long)id);
        if (_childNodes.find(keyName) == _childNodes.end())
            _childNodes[keyName] = Node();

        return _childNodes[keyName];
    }

    template <> inline Node<std::wstring> & Node<std::wstring>::operator[](const size_t & id)
    {
        _type = NT_ARRAY;

        std::wstring keyName = std::to_wstring((unsigned long)id);
        if (_childNodes.find(keyName) == _childNodes.end())
            _childNodes[keyName] = Node();

        return _childNodes[keyName];
    }

    template <class StringAllocator>
    template <class StringAllocatorStream> inline StringAllocator Node<StringAllocator>::Serialize(bool pretty, int level)
    {
        if (_type == NT_DATA)
        {
            return StringAllocator("\"" + _data + "\"");
        }
        else if (_type == NT_DATA_INT)
        {
            return StringAllocator(_data);
        }
        else if (_type == NT_ARRAY)
        {
            StringAllocatorStream output;

            if (pretty && level != 0)
                output << "\n";

            output << std::string(level, '\t') << "[" << (pretty ? "\n" : "");

            for (typename std::map<StringAllocator, Node<StringAllocator>>::iterator itr = _childNodes.begin(); itr != _childNodes.end(); itr++)
            {
                if (itr != _childNodes.begin())
                    output << ",";

                output << itr->second.template Serialize<StringAllocatorStream>(pretty, level + 1);
            }

            if (pretty)
                output << "\n";

            output << std::string(level, '\t') << "]";

            return output.str();
        }
        else
        {
            StringAllocatorStream output;

            if (pretty && level != 0)
                output << "\n";

            output << std::string(level, '\t') << "{" << (pretty ? "\n" : "");

            for (typename std::map<StringAllocator, Node<StringAllocator>>::iterator itr = _childNodes.begin(); itr != _childNodes.end(); itr++)
            {
                if (itr != _childNodes.begin())
                    output << ",\n";

                output << std::string(level + 1, '\t') << "\"" << itr->first << "\": " << itr->second.template Serialize<StringAllocatorStream>(pretty, level + 1);
            }

            if (pretty)
                output << "\n";

            output << std::string(level, '\t') << "}";

            return output.str();
        }

        return StringAllocator("");
    }

    template <>
    template <class StringAllocatorStream> inline std::wstring Node<std::wstring>::Serialize(bool pretty, int level)
    {
        if (_type == NT_DATA)
        {
            return std::wstring(L"\"" + _data + L"\"");
        }
        else if (_type == NT_DATA_INT)
        {
            return std::wstring(_data);
        }
        else if (_type == NT_ARRAY)
        {
            StringAllocatorStream output;

            if (pretty && level != 0)
                output << L"\n";

            output << std::wstring(level, L'\t') << L"[" << (pretty ? L"\n" : L"");

            for (std::map<std::wstring, Node<std::wstring>>::iterator itr = _childNodes.begin(); itr != _childNodes.end(); itr++)
            {
                if (itr != _childNodes.begin())
                    output << L",";

                output << itr->second.Serialize<StringAllocatorStream>(pretty, level + 1);
            }

            if (pretty)
                output << L"\n";

            output << std::wstring(level, L'\t') << L"]";

            return output.str();
        }
        else
        {
            StringAllocatorStream output;

            if (pretty && level != 0)
                output << L"\n";

            output << std::wstring(level, L'\t') << L"{" << (pretty ? L"\n" : L"");

            for (std::map<std::wstring, Node<std::wstring>>::iterator itr = _childNodes.begin(); itr != _childNodes.end(); itr++)
            {
                if (itr != _childNodes.begin())
                    output << L",\n";

                output << std::wstring(level + 1, L'\t') << L"\"" << itr->first << L"\": " << itr->second.Serialize<StringAllocatorStream>(pretty, level + 1);
            }

            if (pretty)
                output << L"\n";

            output << std::wstring(level, L'\t') << L"}";

            return output.str();
        }

        return std::wstring(L"");
    }

    template<>
    template<typename T> inline Node<std::string> & Node<std::string>::operator=(T data)
    {
        SetData(std::to_string(data), std::is_arithmetic<T>::value);
        return *this;
    }

    template<>
    template<typename T> inline Node<std::wstring> & Node<std::wstring>::operator=(T data)
    {
        SetData(std::to_wstring(data), std::is_arithmetic<T>::value);
        return *this;
    }

    template<>
    template<> inline Node<std::string> & Node<std::string>::operator=(const char * data)
    {
        SetData(data);
        return *this;
    }

    template<>
    template<> inline Node<std::string> & Node<std::string>::operator=(std::string data)
    {
        SetData(data);
        return *this;
    }

    template<>
    template<> inline Node<std::wstring> & Node<std::wstring>::operator=(const wchar_t * data)
    {
        SetData(data);
        return *this;
    }

    template<>
    template<> inline Node<std::wstring> & Node<std::wstring>::operator=(std::wstring data)
    {
        SetData(data);
        return *this;
    }
}
