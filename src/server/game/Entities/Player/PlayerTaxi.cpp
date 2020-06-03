#include "Player.h"
#include "TaxiPackets.h"
#include "ObjectMgr.h"

PlayerTaxi::PlayerTaxi()
{
    m_taximask.fill(0);
}

void PlayerTaxi::InitTaxiNodesForLevel(uint32 race, uint32 chrClass, uint8 level)
{
    // class specific initial known nodes
    TaxiMask const& factionMask = Player::TeamForRace(race) == HORDE ? sHordeTaxiNodesMask : sAllianceTaxiNodesMask;
    switch (chrClass)
    {
        case CLASS_DEATH_KNIGHT:
            for (uint16 i = 0; i < TaxiMaskSize; ++i)
                m_taximask[i] |= sOldContinentsNodesMask[i] & factionMask[i];
            break;
        default:
            break;
    }

    // race specific initial known nodes: capital and taxi hub masks
    switch (race)
    {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_DRAENEI:
        case RACE_WORGEN:
        case RACE_PANDAREN_ALLIANCE:
        case RACE_VOID_ELF:
        case RACE_LIGHTFORGED_DRAENEI:
            SetTaximaskNode(2);     // Stormwind, Elwynn
            SetTaximaskNode(6);     // Ironforge, Dun Morogh
            SetTaximaskNode(26);    // Lor'danel, Darkshore
            SetTaximaskNode(27);    // Rut'theran Village, Teldrassil
            SetTaximaskNode(49);    // Moonglade (Alliance)
            SetTaximaskNode(94);    // The Exodar
            SetTaximaskNode(456);   // Dolanaar, Teldrassil
            SetTaximaskNode(457);   // Darnassus, Teldrassil
            SetTaximaskNode(582);   // Goldshire, Elwynn
            SetTaximaskNode(589);   // Eastvale Logging Camp, Elwynn
            SetTaximaskNode(619);   // Kharanos, Dun Morogh
            SetTaximaskNode(620);   // Gol'Bolar Quarry, Dun Morogh
            SetTaximaskNode(624);   // Azure Watch, Azuremyst Isle
            break;
        case RACE_ORC:
        case RACE_UNDEAD_PLAYER:
        case RACE_TAUREN:
        case RACE_TROLL:
        case RACE_BLOODELF:
        case RACE_GOBLIN:
        case RACE_PANDAREN_HORDE:
        case RACE_NIGHTBORNE:
        case RACE_HIGHMOUNTAIN_TAUREN:
            SetTaximaskNode(11);    // Undercity, Tirisfal
            SetTaximaskNode(22);    // Thunder Bluff, Mulgore
            SetTaximaskNode(23);    // Orgrimmar, Durotar
            SetTaximaskNode(69);    // Moonglade (Horde)
            SetTaximaskNode(82);    // Silvermoon City
            SetTaximaskNode(384);   // The Bulwark, Tirisfal
            SetTaximaskNode(402);   // Bloodhoof Village, Mulgore
            SetTaximaskNode(460);   // Brill, Tirisfal Glades
            SetTaximaskNode(536);   // Sen'jin Village, Durotar
            SetTaximaskNode(537);   // Razor Hill, Durotar
            SetTaximaskNode(625);   // Fairbreeze Village, Eversong Woods
            SetTaximaskNode(631);   // Falconwing Square, Eversong Woods
            break;
        default:
            break;
    }

    //Temporary add for alfa test legion, when not ready artefact
    SetTaximaskNode(1673);   // Val'sharah, start quest location

    switch (Player::TeamForRace(race))
    {
        case ALLIANCE:
            SetTaximaskNode(100);
            break;
        case HORDE:
            SetTaximaskNode(99);
            break;
        default:
            break;
    }

    if (level >= 68)
        SetTaximaskNode(213);                               //Shattered Sun Staging Area

    // Argus Taxi
    if (IsTaximaskNodeKnown(1944) || IsTaximaskNodeKnown(1977) || IsTaximaskNodeKnown(1994))
    {
        SetTaximaskNode(1985); // [Hidden] Argus Ground Points Hub (Ground TP out to here, TP to Vindicaar from here)
        SetTaximaskNode(1986); // [Hidden] Argus Vindicaar Ground Hub (Vindicaar TP out to here, TP to ground from here)
        SetTaximaskNode(1987); // [Hidden] Argus Vindicaar No Load Hub (Vindicaar No Load transition goes through here)
    }
}

void PlayerTaxi::LoadTaxiMask(std::string const &data)
{
    Tokenizer tokens(data, ' ');

    uint16 index = 0;
    for (Tokenizer::const_iterator iter = tokens.begin(); index < TaxiMaskSize && iter != tokens.end(); ++iter, ++index)
        m_taximask[index] = sTaxiNodesMask[index] & atoul(*iter);
}

bool PlayerTaxi::IsTaximaskNodeKnown(uint32 nodeidx) const
{
    uint16 field = uint16((nodeidx - 1) / 8);
    uint32 submask = 1 << ((nodeidx - 1) % 8);
    return (m_taximask[field] & submask) == submask;
}

bool PlayerTaxi::SetTaximaskNode(uint32 nodeidx)
{
    uint16 field = uint16((nodeidx - 1) / 8);
    uint32 submask = 1 << ((nodeidx - 1) % 8);
    if ((m_taximask[field] & submask) != submask)
    {
        m_taximask[field] |= submask;
        return true;
    }
    return false;
}

void PlayerTaxi::AppendTaximaskTo(WorldPackets::Taxi::ShowTaxiNodes& data, bool all, Player* player)
{
    if (all)
    {
        data.CanLandNodes = sTaxiNodesMaskV;              // all existed nodes
        data.CanUseNodes = sTaxiNodesMaskV;
    }
    else
    {
        std::vector<uint8> CanUseNodes;
        std::vector<uint8> CanLandNodes;
        MapEntry const* map = sMapStore.LookupEntry(player->GetMapId());
        if (!map)
            return;

        int16 ParentID = map->ParentMapID != -1 ? map->ParentMapID : map->CosmeticParentMapID;
        uint32 requireFlag = (player->GetTeam() == ALLIANCE) ? TAXI_NODE_FLAG_ALLIANCE : TAXI_NODE_FLAG_HORDE;
        for (TaxiNodesEntry const* node : sTaxiNodesStore)
        {
            if (!node || !(node->Flags & requireFlag))
                continue;

            MapEntry const* mapEntry = sMapStore.LookupEntry(node->ContinentID);
            if (!mapEntry)
                continue;

            int16 ParentMapID = mapEntry->ParentMapID != -1 ? mapEntry->ParentMapID : mapEntry->CosmeticParentMapID;
            if (ParentID != -1)
            {
                if (node->ContinentID != player->GetMapId() && ParentMapID != player->GetMapId() && node->ContinentID != ParentID && ParentMapID != ParentID)
                    continue;
            }
            else if (node->ContinentID != player->GetMapId())
                    continue;

            if (node->ConditionID)
            {
                if ((node->Flags & TAXI_NODE_FLAG_UNK2) && !sPlayerConditionStore.LookupEntry(node->ConditionID)) //TAXI_NODE_FLAG_UNK2 not use if condition not found in db2
                    continue;
                if (!sConditionMgr->IsPlayerMeetingCondition(player, node->ConditionID))
                    continue;
            }

            bool landNodes = true;
            switch (node->ID)
            {
                case 1985: // 1944
                    // if (player->GetCurrentZoneID() != 8574) // Крокуун
                        landNodes = false;
                    break;
                case 1986: // 1987
                    // if (player->GetCurrentZoneID() != 8701) // Мак\'Ари
                        landNodes = false;
                    break;
                case 1987: // 1994
                    // if (player->GetCurrentZoneID() != 8899) // Пустоши Анторуса
                        landNodes = false;
                    break;
            }

            uint16 field = static_cast<uint16>((node->ID - 1) / 8);
            uint32 submask = 1 << ((node->ID - 1) % 8);

            if (m_taximask[field] & submask)
            {
                if (CanLandNodes.size() <= field)
                    CanLandNodes.resize(field + 1, 0);
                if (CanUseNodes.size() <= field)
                    CanUseNodes.resize(field + 1, 0);
                if (landNodes)
                    CanLandNodes[field] |= submask;
                CanUseNodes[field] |= submask;
            }
        }

        data.CanLandNodes = CanLandNodes;                  // known nodes
        data.CanUseNodes = CanUseNodes;
    }
}

TaxiMask const& PlayerTaxi::GetTaxiMask() const
{
    return m_taximask;
}

bool PlayerTaxi::LoadTaxiDestinationsFromString(std::string const& values, uint32 team)
{
    ClearTaxiDestinations();

    Tokenizer tokens(values, ' ');

    for (Tokenizer::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
        AddTaxiDestination(atoul(*iter));

    if (m_TaxiDestinations.empty())
        return true;

    // Check integrity
    if (m_TaxiDestinations.size() < 2)
        return false;

    for (size_t i = 1; i < m_TaxiDestinations.size(); ++i)
    {
        uint32 cost;
        uint32 path;
        sObjectMgr->GetTaxiPath(m_TaxiDestinations[i - 1], m_TaxiDestinations[i], path, cost);
        if (!path)
            return false;
    }

    // can't load taxi path without mount set (quest taxi path?)
    if (!sObjectMgr->GetTaxiMountDisplayId(GetTaxiSource(), team, true))
        return false;

    return true;
}

std::string PlayerTaxi::SaveTaxiDestinationsToString()
{
    if (m_TaxiDestinations.empty())
        return "";

    std::ostringstream ss;

    for (size_t i = 0; i < m_TaxiDestinations.size(); ++i)
        ss << m_TaxiDestinations[i] << ' ';

    return ss.str();
}

void PlayerTaxi::ClearTaxiDestinations()
{
    m_TaxiDestinations.clear();
}

void PlayerTaxi::AddTaxiDestination(uint32 dest)
{
    m_TaxiDestinations.push_back(dest);
}

void PlayerTaxi::SetTaxiDestination(std::vector<uint32>& nodes)
{
    m_TaxiDestinations.clear();
    m_TaxiDestinations.insert(m_TaxiDestinations.begin(), nodes.begin(), nodes.end());
}

uint32 PlayerTaxi::GetTaxiSource() const
{
    return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front();
}

uint32 PlayerTaxi::GetTaxiDestination() const
{
    return m_TaxiDestinations.size() < 2 ? 0 : m_TaxiDestinations[1];
}

uint32 PlayerTaxi::GetCurrentTaxiPath() const
{
    if (m_TaxiDestinations.size() < 2)
        return 0;

    uint32 path;
    uint32 cost;

    sObjectMgr->GetTaxiPath(m_TaxiDestinations[0], m_TaxiDestinations[1], path, cost);

    return path;
}

uint32 PlayerTaxi::NextTaxiDestination()
{
    m_TaxiDestinations.pop_front();
    return GetTaxiDestination();
}

std::ostringstream& operator<< (std::ostringstream& ss, PlayerTaxi const& taxi)
{
    for (uint16 i = 0; i < TaxiMaskSize; ++i)
        ss << uint32(taxi.m_taximask[i]) << ' ';
    return ss;
}

bool PlayerTaxi::RequestEarlyLanding()
{
    if (m_TaxiDestinations.size() <= 2)
        return false;

    // start from first destination - m_TaxiDestinations[0] is the current starting node
    for (std::deque<uint32>::iterator it = ++m_TaxiDestinations.begin(); it != m_TaxiDestinations.end(); ++it)
    {
        if (IsTaximaskNodeKnown(*it))
        {
            if (++it == m_TaxiDestinations.end())
                return false;   // if we are left with only 1 known node on the path don't change the spline, its our final destination anyway

            m_TaxiDestinations.erase(it, m_TaxiDestinations.end());
            return true;
        }
    }

    return false;
}

std::deque<uint32> const& PlayerTaxi::GetPath() const
{
    return m_TaxiDestinations;
}

bool PlayerTaxi::empty() const
{
    return m_TaxiDestinations.empty();
}
