/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef SC_SCRIPTMGR_H
#define SC_SCRIPTMGR_H

#include <atomic>
#include "Common.h"
#include "Player.h"
#include "SharedDefines.h"
#include "World.h"
#include "Weather.h"
#include "ObjectMgr.h"

class AreaTriggerAI;
class AuctionHouseObject;
class Aura;
class AuraScript;
class Battleground;
class BattlegroundMap;
class Channel;
class ChatCommand;
class Creature;
class CreatureAI;
class DynamicObject;
class EventObject;
class GameObject;
class GameObjectAI;
class GridMap;
class Group;
class Guild;
class InstanceMap;
class InstanceScript;
class Item;
class Map;
class OutdoorPvP;
class Player;
class Quest;
class ScriptMgr;
class Spell;
class SpellCastTargets;
class SpellScript;
class Transport;
class Unit;
class Vehicle;
class WorldObject;
class WorldPacket;
class WorldSession;
class WorldSocket;

struct AchievementCriteriaData;
struct AuctionEntry;
struct ConditionSourceInfo;
struct Condition;
struct ItemTemplate;
struct OutdoorPvPData;

#define VISIBLE_RANGE       166.0f                          //MAX visible range (size of grid)

// Generic scripting text function.
void DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target = nullptr);

class ScriptObject
{
    friend class ScriptMgr;
    const std::string _name;

public:

    // Do not override this in scripts; it should be overridden by the various script type classes. It indicates
    // whether or not this script type must be assigned in the database.
    virtual bool IsDatabaseBound() const { return false; }
    std::string const& GetName() const { return _name; }

protected:
    ScriptObject(std::string name) : _name(name) { }
    virtual ~ScriptObject() = default;
};

template<class TObject>
class UpdatableScript
{
protected:
    UpdatableScript() = default;
    virtual ~UpdatableScript() = default;

public:
    virtual void OnUpdate(TObject* /*obj*/, uint32 /*diff*/) { }
};

class SpellScriptLoader : public ScriptObject
{
    protected:

        SpellScriptLoader(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Should return a fully valid SpellScript pointer.
        virtual SpellScript* GetSpellScript() const { return nullptr; }

        // Should return a fully valid AuraScript pointer.
        virtual AuraScript* GetAuraScript() const { return nullptr; }
};

class WorldScript : public ScriptObject
{
    protected:

        WorldScript(std::string name);

    public:

        // Called when the open/closed state of the world changes.
        virtual void OnOpenStateChange(bool /*open*/) { }

        // Called after the world configuration is (re)loaded.
        virtual void OnConfigLoad(bool /*reload*/) { }

        // Called before the message of the day is changed.
        virtual void OnMotdChange(std::string& /*newMotd*/) { }

        // Called when a world shutdown is initiated.
        virtual void OnShutdownInitiate(ShutdownExitCode /*code*/, ShutdownMask /*mask*/) { }

        // Called when a world shutdown is cancelled.
        virtual void OnShutdownCancel() { }

        // Called on every world tick (don't execute too heavy code here).
        virtual void OnUpdate(uint32 /*diff*/) { }

        // Called when the world is started.
        virtual void OnStartup() { }

        // Called when the world is actually shut down.
        virtual void OnShutdown() { }
};

class FormulaScript : public ScriptObject
{
    protected:

        FormulaScript(std::string name);

    public:

        // Called after calculating honor.
        virtual void OnHonorCalculation(float& /*honor*/, uint8 /*level*/, float /*multiplier*/) { }

        // Called after gray level calculation.
        virtual void OnGrayLevelCalculation(uint8& /*grayLevel*/, uint8 /*playerLevel*/) { }

        // Called after calculating experience color.
        virtual void OnColorCodeCalculation(XPColorChar& /*color*/, uint8 /*playerLevel*/, uint8 /*mobLevel*/) { }

        // Called after calculating zero difference.
        virtual void OnZeroDifferenceCalculation(uint8& /*diff*/, uint8 /*playerLevel*/) { }

        // Called after calculating base experience gain.
        virtual void OnBaseGainCalculation(uint32& /*gain*/, uint8 /*playerLevel*/, uint8 /*mobLevel*/) { }

        // Called after calculating experience gain.
        virtual void OnGainCalculation(uint32& /*gain*/, Player* /*player*/, Unit* /*unit*/) { }

        // Called when calculating the experience rate for group experience.
        virtual void OnGroupRateCalculation(float& /*rate*/, uint32 /*count*/, bool /*isRaid*/) { }
};

namespace Battlepay
{
    struct Product;
}

class BattlePayProductScript : public ScriptObject
{
protected:
    explicit BattlePayProductScript(std::string scriptName);
public:
    virtual void OnProductDelivery(WorldSession* /*session*/, Battlepay::Product const& /*product*/) { }
    virtual bool CanShow(WorldSession* /*session*/, Battlepay::Product const& /*product*/) { return true; }
    virtual bool CanBuy(WorldSession* /*session*/, Battlepay::Product const& /*product*/, std::string& /*reason*/) { return true; }
    virtual std::string GetCustomData(Battlepay::Product const& /*product*/) { return ""; }
};

template<class TMap>
class MapScript : public UpdatableScript<TMap>
{
    MapEntry const* _mapEntry;

    protected:

        MapScript(uint32 mapId) : _mapEntry(sMapStore.LookupEntry(mapId))
        {
            if (!_mapEntry)
                TC_LOG_ERROR(LOG_FILTER_TSCR, "Invalid MapScript for %u; no such map ID.", mapId);
        }

    public:

        // Gets the MapEntry structure associated with this script. Can return NULL.
        MapEntry const* GetEntry() { return _mapEntry; }

        // Called when the map is created.
        virtual void OnCreate(TMap* /*map*/) { }

        // Called just before the map is destroyed.
        virtual void OnDestroy(TMap* /*map*/) { }

        // Called when a grid map is loaded.
        virtual void OnLoadGridMap(TMap* /*map*/, GridMap* /*gmap*/, uint32 /*gx*/, uint32 /*gy*/) { }

        // Called when a grid map is unloaded.
        virtual void OnUnloadGridMap(TMap* /*map*/, GridMap* /*gmap*/, uint32 /*gx*/, uint32 /*gy*/)  { }

        // Called when a player enters the map.
        virtual void OnPlayerEnter(TMap* /*map*/, Player* /*player*/) { }

        // Called when a player leaves the map.
        virtual void OnPlayerLeave(TMap* /*map*/, Player* /*player*/) { }

        // Called on every map update tick.
        virtual void OnUpdate(TMap* /*map*/, uint32 /*diff*/) { }
};

class WorldMapScript : public ScriptObject, public MapScript<Map>
{
    protected:

        WorldMapScript(std::string name, uint32 mapId);
};

class InstanceMapScript : public ScriptObject, public MapScript<InstanceMap>
{
    protected:

        InstanceMapScript(std::string name, uint32 mapId);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Gets an InstanceScript object for this instance.
        virtual InstanceScript* GetInstanceScript(InstanceMap* /*map*/) const { return nullptr; }
};

class BattlegroundMapScript : public ScriptObject, public MapScript<BattlegroundMap>
{
    protected:

        BattlegroundMapScript(std::string name, uint32 mapId);
};

class ItemScript : public ScriptObject
{
    protected:

        ItemScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when a dummy spell effect is triggered on the item.
        virtual bool OnDummyEffect(Unit* /*caster*/, uint32 /*spellId*/, SpellEffIndex /*effIndex*/, Item* /*target*/) { return false; }

        // Called when a player accepts a quest from the item.
        virtual bool OnQuestAccept(Player* /*player*/, Item* /*item*/, Quest const* /*quest*/) { return false; }

        // Called when a player uses the item.
        virtual bool OnUse(Player* /*player*/, Item* /*item*/, SpellCastTargets const& /*targets*/) { return false; }

        // Called when the item expires (is destroyed).
        virtual bool OnExpire(Player* /*player*/, ItemTemplate const* /*proto*/) { return false; }

        virtual bool OnCreate(Player* /*player*/, Item* /*item*/) { return false; }
};

class CreatureScript : public ScriptObject, public UpdatableScript<Creature>
{
    protected:

        CreatureScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when a dummy spell effect is triggered on the creature.
        virtual bool OnDummyEffect(Unit* /*caster*/, uint32 /*spellId*/, SpellEffIndex /*effIndex*/, Creature* /*target*/) { return false; }

        // Called when a player opens a gossip dialog with the creature.
        virtual bool OnGossipHello(Player* /*player*/, Creature* /*creature*/) { return false; }

        // Called when a player selects a gossip item in the creature's gossip menu.
        virtual bool OnGossipSelect(Player* /*player*/, Creature* /*creature*/, uint32 /*sender*/, uint32 /*action*/) { return false; }

        // Called when a player selects a gossip with a code in the creature's gossip menu.
        virtual bool OnGossipSelectCode(Player* /*player*/, Creature* /*creature*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) { return false; }

        // Called when a player accepts a quest from the creature.
        virtual bool OnQuestAccept(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/) { return false; }

        // Called when a player selects a quest in the creature's quest menu.
        virtual bool OnQuestSelect(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/) { return false; }

        // Called when a player completes a quest with the creature.
        virtual bool OnQuestComplete(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/) { return false; }

        // Called when a player selects a quest reward.
        virtual bool OnQuestReward(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/, uint32 /*opt*/) { return false; }

        // Called when the dialog status between a player and the creature is requested.
        virtual uint32 GetDialogStatus(Player* /*player*/, Creature* /*creature*/) { return 100; }

        // Called when a CreatureAI object is needed for the creature.
        virtual CreatureAI* GetAI(Creature* /*creature*/) const { return nullptr; }
};

template<class AI>
CreatureAI* GetAIForInstance(Creature* creature, const char *Name)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId(Name))
                return new AI(creature);
    return nullptr;
}

class GameObjectScript : public ScriptObject, public UpdatableScript<GameObject>
{
    protected:

        GameObjectScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when a dummy spell effect is triggered on the gameobject.
        virtual bool OnDummyEffect(Unit* /*caster*/, uint32 /*spellId*/, SpellEffIndex /*effIndex*/, GameObject* /*target*/) { return false; }

        // Called when a player opens a gossip dialog with the gameobject.
        virtual bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) { return false; }

        // Called when a player selects a gossip item in the gameobject's gossip menu.
        virtual bool OnGossipSelect(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/) { return false; }

        // Called when a player selects a gossip with a code in the gameobject's gossip menu.
        virtual bool OnGossipSelectCode(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) { return false; }

        // Called when a player accepts a quest from the gameobject.
        virtual bool OnQuestAccept(Player* /*player*/, GameObject* /*go*/, Quest const* /*quest*/) { return false; }

        // Called when a player selects a quest reward.
        virtual bool OnQuestReward(Player* /*player*/, GameObject* /*go*/, Quest const* /*quest*/, uint32 /*opt*/) { return false; }

        // Called when the dialog status between a player and the gameobject is requested.
        virtual uint32 GetDialogStatus(Player* /*player*/, GameObject* /*go*/) { return 100; }

        // Called when the game object is destroyed (destructible buildings only).
        virtual void OnDestroyed(GameObject* /*go*/, Player* /*player*/) { }

        // Called when the game object is damaged (destructible buildings only).
        virtual void OnDamaged(GameObject* /*go*/, Player* /*player*/) { }

        // Called when the game object loot state is changed.
        virtual void OnLootStateChanged(GameObject* /*go*/, uint32 /*state*/, Unit* /*unit*/) { }

        // Called when the game object state is changed.
        virtual void OnGameObjectStateChanged(GameObject* /*go*/, uint32 /*state*/) { }

        // Called when a GameObjectAI object is needed for the gameobject.
        virtual GameObjectAI* GetAI(GameObject* /*go*/) const { return nullptr; }
};

class AreaTriggerScript : public ScriptObject
{
    protected:

        AreaTriggerScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when the area trigger is activated by a player.
        virtual bool OnTrigger(Player* /*player*/, AreaTriggerEntry const* /*trigger*/, bool /*enter*/) { return false; }

        // Called when a AreaTriggerAI object is needed for the areatrigger.
        virtual AreaTriggerAI* GetAI(AreaTrigger* /*at*/) const { return nullptr; }
};

class SceneTriggerScript : public ScriptObject
{
protected:

    SceneTriggerScript(std::string name);

public:

    bool IsDatabaseBound() const override { return true; }

    // Called when the scene event trigger by call CMSG_SCENE_TRIGGER_EVENT
    virtual bool OnTrigger(Player* /*player*/, SpellScene const* /*trigger*/, std::string /*triggername*/) { return false; }
};

class EventObjectScript : public ScriptObject
{
protected:

    EventObjectScript(std::string name);

public:

    bool IsDatabaseBound() const override { return true; }

    virtual bool IsTriggerMeets(Player* /*player*/, EventObject* /*trigger*/) { return true; }
    virtual bool OnTrigger(Player* /*player*/, EventObject* /*trigger*/, bool /*enter*/) { return false; }
};

class BattlegroundScript : public ScriptObject
{
    protected:

        BattlegroundScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Should return a fully valid Battleground object for the type ID.
        virtual Battleground* GetBattleground() const = 0;
};

class OutdoorPvPScript : public ScriptObject
{
    protected:

        OutdoorPvPScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Should return a fully valid OutdoorPvP object for the type ID.
        virtual OutdoorPvP* GetOutdoorPvP() const = 0;
};

class CommandScript : public ScriptObject
{
    protected:

        CommandScript(std::string name);

    public:

        // Should return a pointer to a valid command table (ChatCommand array) to be used by ChatHandler.
        virtual std::vector<ChatCommand> GetCommands() const = 0;
};

class WeatherScript : public ScriptObject, public UpdatableScript<Weather>
{
    protected:

        WeatherScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when the weather changes in the zone this script is associated with.
        virtual void OnChange(Weather* /*weather*/, WeatherState /*state*/, float /*grade*/) { }
};

class AuctionHouseScript : public ScriptObject
{
    protected:

        AuctionHouseScript(std::string name);

    public:

        // Called when an auction is added to an auction house.
        virtual void OnAuctionAdd(AuctionHouseObject* /*ah*/, AuctionEntry* /*entry*/) { }

        // Called when an auction is removed from an auction house.
        virtual void OnAuctionRemove(AuctionHouseObject* /*ah*/, AuctionEntry* /*entry*/) { }

        // Called when an auction was succesfully completed.
        virtual void OnAuctionSuccessful(AuctionHouseObject* /*ah*/, AuctionEntry* /*entry*/) { }

        // Called when an auction expires.
        virtual void OnAuctionExpire(AuctionHouseObject* /*ah*/, AuctionEntry* /*entry*/) { }
};

class ConditionScript : public ScriptObject
{
    protected:

        ConditionScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when a single condition is checked for a player.
        virtual bool OnConditionCheck(Condition* /*condition*/, ConditionSourceInfo& /*sourceInfo*/) { return true; }
};

class VehicleScript : public ScriptObject
{
    protected:

        VehicleScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called after a vehicle is installed.
        virtual void OnInstall(Vehicle* /*veh*/) { }

        // Called after a vehicle is uninstalled.
        virtual void OnUninstall(Vehicle* /*veh*/) { }

        // Called when a vehicle resets.
        virtual void OnReset(Vehicle* /*veh*/) { }

        // Called after an accessory is installed in a vehicle.
        virtual void OnInstallAccessory(Vehicle* /*veh*/, Creature* /*accessory*/) { }

        // Called after a passenger is added to a vehicle.
        virtual void OnAddPassenger(Vehicle* /*veh*/, Unit* /*passenger*/, int8 /*seatId*/) { }

        // Called after a passenger is removed from a vehicle.
        virtual void OnRemovePassenger(Vehicle* /*veh*/, Unit* /*passenger*/) { }

        // Called when a CreatureAI object is needed for the creature.
        virtual CreatureAI* GetAI(Creature* /*creature*/) const { return nullptr; }
};

class DynamicObjectScript : public ScriptObject, public UpdatableScript<DynamicObject>
{
    protected:

        DynamicObjectScript(std::string name);
};

class TransportScript : public ScriptObject, public UpdatableScript<Transport>
{
    protected:

        TransportScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when a player boards the transport.
        virtual void OnAddPassenger(Transport* /*transport*/, Player* /*player*/) { }

        // Called when a creature boards the transport.
        virtual void OnAddCreaturePassenger(Transport* /*transport*/, Creature* /*creature*/) { }

        // Called when a player exits the transport.
        virtual void OnRemovePassenger(Transport* /*transport*/, Player* /*player*/) { }

        // Called when a transport moves.
        virtual void OnRelocate(Transport* /*transport*/, uint32 /*waypointId*/, uint32 /*mapId*/, float /*x*/, float /*y*/, float /*z*/) { }
};

class AchievementCriteriaScript : public ScriptObject
{
    protected:

        AchievementCriteriaScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called when an additional criteria is checked.
        virtual bool OnCheck(Player* source, Unit* target) = 0;
};

class AchievementRewardScript : public ScriptObject
{
    protected:

        AchievementRewardScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return true; }

        // Called ow handling reward
        virtual bool OnGet(Player* /*source*/, AchievementReward const* /*data*/) {  return false; };
        virtual uint32 SelectItem(Player* /*source*/, AchievementReward const* /*data*/) { return 0; };
};

class PlayerScript : public ScriptObject
{
    protected:

        PlayerScript(std::string name);

    public:

        // Called when a player kills another player
		virtual void OnPVPKill(Player* killer, Player* killed)
		{
			uint32 killerlvl = killer->getLevel();
			uint32 killedlvl = killed->getLevel();
			int32 diff = killerlvl - killedlvl;
			uint32 XPLow = (killedlvl * 5 + 45)*(1 + 0.05*diff);
			uint32 XPHigh = (killedlvl * 5 + 45)*(1 + 0.05*diff);
			uint32 minusgold = killer->GetMoney() - (diff * 10000);
			uint32 plusgold = killed->GetMoney() + (diff * 10000);
			uint32 killergold = killer->GetMoney();
			uint32 killedgold = killed->GetMoney();
			uint32 plusgold2 = killedgold + killergold;

			if (killerlvl < killedlvl + 1)
				killer->GiveXP(XPHigh, killed);
			else
				if (diff > 10)
					if (killergold > minusgold)
					{
						killer->SetMoney(minusgold);
						killed->SetMoney(plusgold);
					}
					else
					{
						killed->SetMoney(plusgold2);
						killer->SetMoney(0);
					}
				else
					if (0 < diff && diff < 10)
						killer->GiveXP(XPLow, killed);
			return;
		}

        // Called when a player kills a creature
        virtual void OnCreatureKill(Player* /*killer*/, Creature* /*killed*/) { }

        // Called when a player is killed by a creature
        virtual void OnPlayerKilledByCreature(Creature* /*killer*/, Player* /*killed*/) { }

        // Called when a player's level changes (right before the level is applied)
        virtual void OnLevelChanged(Player* /*player*/, uint8 /*newLevel*/) { }

        // Called when a player's free talent points change (right before the change is applied)
        virtual void OnFreeTalentPointsChanged(Player* /*player*/, uint32 /*points*/) { }

        // Called when a player's talent points are reset (right before the reset is done)
        virtual void OnTalentsReset(Player* /*player*/, bool /*noCost*/) { }

        // Called when a player's money is modified (before the modification is done)
        virtual void OnMoneyChanged(Player* /*player*/, int64& /*amount*/) { }

        // Called when a player gains XP (before anything is given)
        virtual void OnGiveXP(Player* /*player*/, uint32& /*amount*/, Unit* /*victim*/) { }

        // Called when a player's reputation changes (before it is actually changed)
        virtual void OnReputationChange(Player* /*player*/, uint32 /*factionId*/, int32& /*standing*/, bool /*incremental*/) { }

        // Called when a duel is requested
        virtual void OnDuelRequest(Player* /*target*/, Player* /*challenger*/) { }

        // Called when a duel starts (after 3s countdown)
        virtual void OnDuelStart(Player* /*player1*/, Player* /*player2*/) { }

        // Called when a duel ends
        virtual void OnDuelEnd(Player* /*winner*/, Player* /*loser*/, DuelCompleteType /*type*/) { }

        // The following methods are called when a player sends a chat message.
        virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/) { }

        virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Player* /*receiver*/) { }

        virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Group* /*group*/) { }

        virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Guild* /*guild*/) { }

        virtual void OnChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/, Channel* /*channel*/) { }

        // Both of the below are called on emote opcodes.
        virtual void OnClearEmote(Player* /*player*/) { }

        virtual void OnTextEmote(Player* /*player*/, uint32 /*textEmote*/, uint32 /*emoteNum*/, ObjectGuid const& /*guid*/) { }

        // Called in Spell::Cast.
        virtual void OnSpellCast(Player* /*player*/, Spell* /*spell*/, bool /*skipCheck*/) { }

        // Called when a player logs in.
        virtual void OnLogin(Player* /*player*/) { }
        virtual void OnLogin(Player* /*player*/, bool firstLogin) { }

        // Called when a player logs out.
        virtual void OnLogout(Player* /*player*/) { }

        // Called when a player is created.
        virtual void OnCreate(Player* /*player*/) { }

        // Called when a player is deleted.
        virtual void OnDelete(ObjectGuid const& /*guid*/) { }

        // Called when a player is bound to an instance
        virtual void OnBindToInstance(Player* /*player*/, Difficulty /*difficulty*/, uint32 /*mapId*/, bool /*permanent*/) { }

        // Called when a player switches to a new zone
        virtual void OnUpdateZone(Player* /*player*/, uint32 /*newZone*/, uint32 /*newArea*/) { }

        virtual void OnPetBattleFinish(Player* /*player*/) { }

        // Called when a player changes to a new map (after moving to new map)
        virtual void OnMapChanged(Player* /*player*/) { }

        virtual void OnMovementInform(Player* /*player*/, uint32 /*moveType*/, uint32 /*ID*/) { }

        virtual void OnUpdate(Player* /*player*/, uint32 /*diff*/) { }

        virtual void OnSpellLearned(Player* /*player*/, uint32 /*spellID*/) { }
        
        virtual void OnWhoListCall(Player* /*player*/, const std::set<ObjectGuid>& /*players*/ ) { }
        
        virtual void OnSendMail(Player* /*player*/, std::string& subject, std::string& body, ObjectGuid receiver) {}
        
        virtual void OnQuestReward(Player* player, Quest const* quest) {}

        virtual void OnEnterCombat(Player* player, Unit* target) {}
		
        //After looting item
        virtual void OnLootItem(Player* player, Item* item, uint32 count) { }

        //After creating item (eg profession item creation)
        virtual void OnCreateItem(Player* player, Item* item, uint32 count) { }

        //After receiving item as a quest reward
        virtual void OnQuestRewardItem(Player* player, Item* item, uint32 count) { }
};

class SessionScript : public ScriptObject
{
    protected:

        SessionScript(std::string name);

    public:

        // Called when a account logs in.
        virtual void OnLogin(WorldSession* /*session*/) { }
};

class GuildScript : public ScriptObject
{
    protected:

        GuildScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return false; }

        // Called when a member is added to the guild.
        virtual void OnAddMember(Guild* /*guild*/, Player* /*player*/, uint8& /*plRank*/) { }

        // Called when a member is removed from the guild.
        virtual void OnRemoveMember(Guild* /*guild*/, Player* /*player*/, bool /*isDisbanding*/, bool /*isKicked*/) { }

        // Called when the guild MOTD (message of the day) changes.
        virtual void OnMOTDChanged(Guild* /*guild*/, std::string const& /*newMotd*/) { }

        // Called when the guild info is altered.
        virtual void OnInfoChanged(Guild* /*guild*/, std::string const& /*newInfo*/) { }

        // Called when a guild is created.
        virtual void OnCreate(Guild* /*guild*/, Player* /*leader*/, std::string const& /*name*/) { }

        // Called when a guild is disbanded.
        virtual void OnDisband(Guild* /*guild*/) { }

        // Called when a guild member withdraws money from a guild bank.
        virtual void OnMemberWitdrawMoney(Guild* /*guild*/, Player* /*player*/, uint64& /*amount*/, bool /*isRepair*/) { }

        // Called when a guild member deposits money in a guild bank.
        virtual void OnMemberDepositMoney(Guild* /*guild*/, Player* /*player*/, uint64& /*amount*/) { }

        // Called when a guild member moves an item in a guild bank.
        virtual void OnItemMove(Guild* /*guild*/, Player* /*player*/, Item* /*pItem*/, bool /*isSrcBank*/, uint8 /*srcContainer*/, uint8 /*srcSlotId*/,
            bool /*isDestBank*/, uint8 /*destContainer*/, uint8 /*destSlotId*/) { }

        virtual void OnEvent(Guild* /*guild*/, uint8 /*eventType*/, uint32 /*playerGuid1*/, uint32 /*playerGuid2*/, uint8 /*newRank*/) { }

        virtual void OnBankEvent(Guild* /*guild*/, uint8 /*eventType*/, uint8 /*tabId*/, uint32 /*playerGuid*/, uint32 /*itemOrMoney*/, uint16 /*itemStackCount*/, uint8 /*destTabId*/) { }
};

class GroupScript : public ScriptObject
{
    protected:

        GroupScript(std::string name);

    public:

        bool IsDatabaseBound() const override { return false; }

        // Called when a member is added to a group.
        virtual void OnAddMember(Group* /*group*/, ObjectGuid const& /*guid*/) { }

        // Called when a member is invited to join a group.
        virtual void OnInviteMember(Group* /*group*/, ObjectGuid const& /*guid*/) { }

        // Called when a member is removed from a group.
        virtual void OnRemoveMember(Group* /*group*/, ObjectGuid const& /*guid*/, RemoveMethod /*method*/, ObjectGuid const& /*kicker*/, const char* /*reason*/) { }

        // Called when the leader of a group is changed.
        virtual void OnChangeLeader(Group* /*group*/, ObjectGuid const& /*newLeaderGuid*/, ObjectGuid const& /*oldLeaderGuid*/) { }

        // Called when a group is disbanded.
        virtual void OnDisband(Group* /*group*/) { }
};


class WorldStateScript : public ScriptObject
{
protected:
    WorldStateScript(std::string name);

public:

    bool IsDatabaseBound() const override { return false; }

    virtual void OnCreate(uint32 /*variableID*/, uint32 /*value*/, uint8 /*type*/) { }
    virtual void OnDelete(uint32 /*variableID*/, uint8 /*type*/) { }
};

// Placed here due to ScriptRegistry::AddScript dependency.
#define sScriptMgr ScriptMgr::instance()

typedef std::list<std::string> UnusedScriptNamesContainer;
extern UnusedScriptNamesContainer UnusedScriptNames;

// Manages registration, loading, and execution of scripts.
class ScriptMgr
{
    friend class ScriptObject;

        ScriptMgr();
        virtual ~ScriptMgr();

    public: /* Initialization */
        static ScriptMgr* instance()
        {
            static ScriptMgr instance;
            return &instance;
        }

        void Initialize();
        void LoadDatabase();
        void FillSpellSummary();

        const char* ScriptsVersion() const { return "Integrated Trinity Scripts"; }

        void IncrementScriptCount() { ++_scriptCount; }
        uint32 GetScriptCount() const { return _scriptCount; }

        typedef void(*ScriptLoaderCallbackType)();

        /// Sets the script loader callback which is invoked to load scripts
        /// (Workaround for circular dependency game <-> scripts)
        void SetScriptLoader(ScriptLoaderCallbackType script_loader_callback)
        {
            _script_loader_callback = script_loader_callback;
        }

        /* Unloading */
        void Unload();

        /* SpellScriptLoader */
        void CreateSpellScripts(uint32 spellId, std::vector<SpellScript*>& scriptVector, Spell* invoker);
        void CreateAuraScripts(uint32 spellId, std::vector<AuraScript*>& scriptVector, Aura* invoker);
        SpellScriptLoader* GetSpellScriptLoader(uint32 scriptId);

        /* WorldScript */
        void OnOpenStateChange(bool open);
        void OnConfigLoad(bool reload);
        void OnMotdChange(std::string& newMotd);
        void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask);
        void OnShutdownCancel();
        void OnStartup();
        void OnShutdown();

        /* FormulaScript */
        void OnHonorCalculation(float& honor, uint8 level, float multiplier);
        void OnGrayLevelCalculation(uint8& grayLevel, uint8 playerLevel);
        void OnColorCodeCalculation(XPColorChar& color, uint8 playerLevel, uint8 mobLevel);
        void OnZeroDifferenceCalculation(uint8& diff, uint8 playerLevel);
        void OnBaseGainCalculation(uint32& gain, uint8 playerLevel, uint8 mobLevel);
        void OnGainCalculation(uint32& gain, Player* player, Unit* unit);
        void OnGroupRateCalculation(float& rate, uint32 count, bool isRaid);

        /* MapScript */
        void OnCreateMap(Map* map);
        void OnDestroyMap(Map* map);
        void OnLoadGridMap(Map* map, GridMap* gmap, uint32 gx, uint32 gy);
        void OnUnloadGridMap(Map* map, GridMap* gmap, uint32 gx, uint32 gy);
        void OnPlayerEnterMap(Map* map, Player* player);
        void OnPlayerLeaveMap(Map* map, Player* player);

        /* InstanceMapScript */
        InstanceScript* CreateInstanceData(InstanceMap* map);

        /* ItemScript */
        bool OnDummyEffect(Unit* caster, uint32 spellId, SpellEffIndex effIndex, Item* target);
        bool OnQuestAccept(Player* player, Item* item, Quest const* quest);
        bool OnItemUse(Player* player, Item* item, SpellCastTargets const& targets);
        bool OnItemExpire(Player* player, ItemTemplate const* proto);
        bool OnItemCreate(Player* player, ItemTemplate const* proto, Item* item);

        /* CreatureScript */
        bool OnDummyEffect(Unit* caster, uint32 spellId, SpellEffIndex effIndex, Creature* target);
        bool OnGossipHello(Player* player, Creature* creature);
        bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action);
        bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code);
        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest);
        bool OnQuestSelect(Player* player, Creature* creature, Quest const* quest);
        bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest);
        bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt);
        uint32 GetDialogStatus(Player* player, Creature* creature);
        CreatureAI* GetCreatureAI(Creature* creature);

        /* GameObjectScript */
        bool OnDummyEffect(Unit* caster, uint32 spellId, SpellEffIndex effIndex, GameObject* target);
        bool OnGossipHello(Player* player, GameObject* go);
        bool OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action);
        bool OnGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code);
        bool OnQuestAccept(Player* player, GameObject* go, Quest const* quest);
        bool OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt);
        uint32 GetDialogStatus(Player* player, GameObject* go);
        void OnGameObjectDestroyed(GameObject* go, Player* player);
        void OnGameObjectDamaged(GameObject* go, Player* player);
        void OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* unit);
        void OnGameObjectStateChanged(GameObject* go, uint32 state);
        GameObjectAI* GetGameObjectAI(GameObject* go);

        /* AreaTriggerScript */
        bool OnAreaTrigger(Player* player, AreaTriggerEntry const* trigger, bool enter);
        AreaTriggerAI* GetAreaTriggerAI(AreaTrigger* areaTrigger);

        /* SceneTriggerScript */
        bool OnSceneTrigger(Player* player, SpellScene const* trigger, std::string triggername);

        /* EventObjectScript */
        bool IsEOMeetConditions(Player* player, EventObject* event);
        bool OnEventObject(Player* player, EventObject* event, bool enter);

        /* BattlegroundScript */
        Battleground* CreateBattleground(uint16 typeId);

        /* OutdoorPvPScript */
        OutdoorPvP* CreateOutdoorPvP(OutdoorPvPData const* data);

        /* CommandScript */
        std::vector<ChatCommand> GetChatCommands();

        /* WeatherScript */
        void OnWeatherChange(Weather* weather, WeatherState state, float grade);
        void OnWeatherUpdate(Weather* weather, uint32 diff);

        /* AuctionHouseScript */

        void OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry);
        void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry);

        /* ConditionScript */
        bool OnConditionCheck(Condition* condition, ConditionSourceInfo& sourceInfo);

        /* VehicleScript */
        void OnInstall(Vehicle* veh);
        void OnUninstall(Vehicle* veh);
        void OnReset(Vehicle* veh);
        void OnInstallAccessory(Vehicle* veh, Creature* accessory);
        void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatId);
        void OnRemovePassenger(Vehicle* veh, Unit* passenger);

        /* TransportScript */
        void OnAddPassenger(Transport* transport, Player* player);
        void OnAddCreaturePassenger(Transport* transport, Creature* creature);
        void OnRemovePassenger(Transport* transport, Player* player);
        void OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z);

        /* AchievementCriteriaScript */
        bool OnCriteriaCheck(AchievementCriteriaData const* data, Player* source, Unit* target);

        /* AchievementRewardScript */
        bool OnRewardCheck(AchievementReward const* data, Player* source);
        uint32 OnSelectItemReward(AchievementReward const* data, Player* source);

        /* PlayerScript */
        void OnPVPKill(Player* killer, Player* killed);
        void OnCreatureKill(Player* killer, Creature* killed);
        void OnPlayerKilledByCreature(Creature* killer, Player* killed);
        void OnPlayerLevelChanged(Player* player, uint8 oldLevel);
        void OnPlayerFreeTalentPointsChanged(Player* player, uint32 newPoints);
        void OnPlayerTalentsReset(Player* player, bool noCost);
        void OnPlayerMoneyChanged(Player* player, int64& amount);
        void OnGivePlayerXP(Player* player, uint32& amount, Unit* victim);
        void OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental);
        void OnPlayerDuelRequest(Player* target, Player* challenger);
        void OnPlayerDuelStart(Player* player1, Player* player2);
        void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel);
        void OnPlayerClearEmote(Player* player);
        void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid const& guid);
        void OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck);
        void OnPlayerLogin(Player* player, bool firstLogin = false);
        void OnPlayerLogout(Player* player);
        void OnPlayerCreate(Player* player);
        void OnPlayerDelete(ObjectGuid const& guid);
        void OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent);
        void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea);
        void OnPetBattleFinish(Player* player);
        void OnMovementInform(Player* player, uint32 moveType, uint32 ID);
        void OnUpdate(Player* player, uint32 diff);
        void OnPlayerSpellLearned(Player* player, uint32 spellID);
        void OnPlayerWhoListCall(Player* player, const std::set<ObjectGuid> & playersGuids);
        void OnPlayerSendMail(Player* player, std::string& subject, std::string& body, ObjectGuid receiverGuid);
        void OnPlayerQuestReward(Player* player, Quest const* quest);
        void OnPlayerEnterCombat(Player* player, Unit* target);
        void OnLootItem(Player* player, Item* item, uint32 count);
        void OnCreateItem(Player* player, Item* item, uint32 count);
        void OnQuestRewardItem(Player* player, Item* item, uint32 count);
        
        /* SessionScript */
        void OnSessionLogin(WorldSession* session);

        /* GuildScript */
        void OnGuildAddMember(Guild* guild, Player* player, uint8& plRank);
        void OnGuildRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool isKicked);
        void OnGuildMOTDChanged(Guild* guild, std::string const& newMotd);
        void OnGuildInfoChanged(Guild* guild, std::string const& newInfo);
        void OnGuildCreate(Guild* guild, Player* leader, std::string const& name);
        void OnGuildDisband(Guild* guild);
        void OnGuildMemberWitdrawMoney(Guild* guild, Player* player, uint64 &amount, bool isRepair);
        void OnGuildMemberDepositMoney(Guild* guild, Player* player, uint64 &amount);
        void OnGuildItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId, bool isDestBank, uint8 destContainer, uint8 destSlotId);
        void OnGuildEvent(Guild* guild, uint8 eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank);
        void OnGuildBankEvent(Guild* guild, uint8 eventType, uint8 tabId, uint32 playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId);

        /* GroupScript */
        void OnGroupAddMember(Group* group, ObjectGuid const& guid);
        void OnGroupInviteMember(Group* group, ObjectGuid const& guid);
        void OnGroupRemoveMember(Group* group, ObjectGuid const& guid, RemoveMethod method, ObjectGuid const& kicker, const char* reason);
        void OnGroupChangeLeader(Group* group, ObjectGuid const& newLeaderGuid, ObjectGuid const& oldLeaderGuid);
        void OnGroupDisband(Group* group);

        /* Scheduled scripts */
        uint32 IncreaseScheduledScriptsCount() { return ++_scheduledScripts; }
        uint32 DecreaseScheduledScriptCount() { return --_scheduledScripts; }
        uint32 DecreaseScheduledScriptCount(size_t count) { return _scheduledScripts -= count; }
        bool IsScriptScheduled() const { return _scheduledScripts > 0; }

        void OnWorldStateCreate(uint32 variableID, uint32 value, uint8 type);
        void OnWorldStateDelete(uint32 variableID, uint8 type);

        /* BattlePayProductScript */
        void OnBattlePayProductDelivery(WorldSession* session, Battlepay::Product const& product);
        bool BattlePayCanBuy(WorldSession* session, Battlepay::Product const& product, std::string& reason);
        std::string BattlePayGetCustomData(Battlepay::Product const& product);
    private:

        uint32 _scriptCount;

        //atomic op counter for active scripts amount
        std::atomic_long _scheduledScripts;
        ScriptLoaderCallbackType _script_loader_callback;
};

template <class S>
class GenericSpellScriptLoader : public SpellScriptLoader
{
    public:
        GenericSpellScriptLoader(char const* name) : SpellScriptLoader(name) { }
        SpellScript* GetSpellScript() const override { return new S(); }
};
#define RegisterSpellScript(spell_script) new GenericSpellScriptLoader<spell_script>(#spell_script)

template <class A>
class GenericAuraScriptLoader : public SpellScriptLoader
{
    public:
        GenericAuraScriptLoader(char const* name) : SpellScriptLoader(name) { }
        AuraScript* GetAuraScript() const override { return new A(); }
};
#define RegisterAuraScript(aura_script) new GenericAuraScriptLoader<aura_script>(#aura_script)

template <class S, class A>
class GenericSpellAndAuraScriptLoader : public SpellScriptLoader
{
    public:
        GenericSpellAndAuraScriptLoader(char const* name) : SpellScriptLoader(name) { }
        SpellScript* GetSpellScript() const override { return new S(); }
        AuraScript* GetAuraScript() const override { return new A(); }
};
#define RegisterSpellAndAuraScriptPair(spell_script, aura_script) new GenericSpellAndAuraScriptLoader<spell_script, aura_script>(#spell_script)

template <class AI>
class GenericCreatureScript : public CreatureScript
{
    public:
        GenericCreatureScript(char const* name) : CreatureScript(name) { }
        CreatureAI* GetAI(Creature* me) const override { return new AI(me); }
};
#define RegisterCreatureAI(ai_name) new GenericCreatureScript<ai_name>(#ai_name)

template <class AI, AI*(*AIFactory)(Creature*)>
class FactoryCreatureScript : public CreatureScript
{
    public:
        FactoryCreatureScript(char const* name) : CreatureScript(name) { }
        CreatureAI* GetAI(Creature* me) const override { return AIFactory(me); }
};
#define RegisterCreatureAIWithFactory(ai_name, factory_fn) new FactoryCreatureScript<ai_name, &factory_fn>(#ai_name)

template <class AI>
class GenericGameObjectScript : public GameObjectScript
{
    public:
        GenericGameObjectScript(char const* name) : GameObjectScript(name) { }
        GameObjectAI* GetAI(GameObject* go) const override { return new AI(go); }
};
#define RegisterGameObjectAI(ai_name) new GenericGameObjectScript<ai_name>(#ai_name)

template <class AI>
class GenericAreaTriggerEntityScript : public AreaTriggerScript
{
    public:
        GenericAreaTriggerEntityScript(char const* name) : AreaTriggerScript(name) { }
        AreaTriggerAI* GetAI(AreaTrigger* at) const override { return new AI(at); }
};
#define RegisterAreaTriggerAI(ai_name) new GenericAreaTriggerEntityScript<ai_name>(#ai_name)

template <class AI>
class GenericInstanceMapScript : public InstanceMapScript
{
public:
    GenericInstanceMapScript(char const* name, uint32 mapId) : InstanceMapScript(name, mapId) { }
    InstanceScript* GetInstanceScript(InstanceMap* map) const override { return new AI(map); }
};
#define RegisterInstanceScript(ai_name, mapId) new GenericInstanceMapScript<ai_name>(#ai_name, mapId)

#endif
