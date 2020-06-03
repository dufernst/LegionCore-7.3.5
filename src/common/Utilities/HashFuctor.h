/*
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_HAS_FUCTOR_H
#define TRINITY_HAS_FUCTOR_H

#include "Define.h"

#include <cds/container/feldman_hashmap_hp.h>
#include <cds/container/feldman_hashset_hp.h>
#include <cds/container/iterable_list_hp.h>

class ObjectGuid;
class WorldObject;
class Item;
class Transport;
class Player;
class Unit;
class AuraEffect;

struct int8Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<int8_t> hash;
};

struct uint8Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<uint8_t> hash;
};

struct boolTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<bool> hash;
};

struct charTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<char> hash;
};

struct int16Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<short> hash;
};
struct uint16Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<unsigned short> hash;
};

struct int32Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<int> hash;
};

struct uint32Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<unsigned int> hash;
};

struct int64Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<long> hash;
};

struct uint64Traits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<unsigned long> hash;
};

struct floatTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<float> hash;
};

struct doubleTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<double> hash;
};

struct wstringTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<std::wstring> hash;
};

struct stringTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<std::string> hash;
};

struct guidTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<ObjectGuid> hash;
};

template< class T >
struct classTraits: public cds::container::feldman_hashmap::traits
{
    typedef std::hash<T*&> hash;
};

struct WorldObjectHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( WorldObject* src ) const
        {
            return std::hash<WorldObject*>()(src);
        }
    };
};
static std::hash<WorldObject*> worldObjectHashGen;

struct ItemHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( Item* src ) const
        {
            return std::hash<Item*>()(src);
        }
    };
};
static std::hash<Item*> itemHashGen;

struct TransportHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( Transport* t ) const
        {
            return std::hash<Transport*>()(t);
        }
    };
};
static std::hash<Transport*> transportHashGen;

struct uint32HashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( uint32 src ) const
        {
            return std::hash<uint32>()(src);
        }
    };
};

struct PlayerHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( Player* src ) const
        {
            return std::hash<Player*>()(src);
        }
    };
};
static std::hash<Player*> PlayerHashGen;

struct UnitHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( Unit* src ) const
        {
            return std::hash<Unit*>()(src);
        }
    };
};
static std::hash<Unit*> UnitHashGen;

struct ObjectGuidHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( ObjectGuid src ) const
        {
            return std::hash<ObjectGuid>()(src);
        }
    };
};
static std::hash<ObjectGuid> ObjectGuidHashGen;

struct AuraEffectHashAccessor: public cds::container::feldman_hashset::traits
{
    struct hash_accessor {
        size_t operator()( AuraEffect* src ) const
        {
            return std::hash<AuraEffect*>()(src);
        }
    };
};
static std::hash<AuraEffect*> AuraEffectHashGen;

static std::hash<int8> I8Hash;
static std::hash<uint8> U8Hash;
static std::hash<int16> I16Hash;
static std::hash<uint16> U16Hash;
static std::hash<int32> I32Hash;
static std::hash<uint32> U32Hash;
static std::hash<int64> I64Hash;
static std::hash<uint64> U64Hash;
static std::hash<float> FHash;
static std::hash<double> DHash;
static std::hash<std::string> StringHash;
static std::hash<std::wstring> WstringHash;

#endif
