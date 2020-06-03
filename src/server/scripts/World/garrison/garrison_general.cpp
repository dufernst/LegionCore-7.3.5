#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "CreatureTextMgr.h"
#include "GameObjectAI.h"
#include "Garrison.h"
#include "ChatPackets.h"
#include "Packets/MiscPackets.h"

Position const hearhstoneHorde[3] = {
    {5560.4f, 4507.7f, 132.67f, 206.1f},         // Horde lvl1 map 1152
    {5565.86f, 4593.5f, 140.893f, 2.332632f},    // Horde lvl2 spell 171325 map 1330
    {5563.7f, 4599.9f, 141.71f, 131.75f},        // Horde lvl3 spell 171325 map 1153
};
Position const hearhstoneAlliance[3] = {
    {1850.71f, 254.43f, 78.083f, 1.886526f},     // ALLiance lvl1 spell 158825
    {1936.9f, 341.35f, 90.28f, 123.479f},        // Alliance lvl2 
    {1947.0f, 324.88f, 90.28f, 118.664f},        // Alliance lvl3
};

//http://www.wowhead.com/spell=171253/garrison-hearthstone
class spell_garrison_hearthstone : public SpellScriptLoader
{
public:
    spell_garrison_hearthstone() : SpellScriptLoader("spell_garrison_hearthstone") { }

    class spell_garrison_hearthstone_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_garrison_hearthstone_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* caster = GetCaster())
            {
                Player* plr = caster->ToPlayer();
                if (!plr)
                    return;

                Garrison *garr = plr->GetGarrison();
                if (!garr || garr->GetGarrisonMapID() == -1)
                    return;
                uint8 idx = garr->GetGarrisonLevel() - 1;

                if (plr->GetTeam() == ALLIANCE)
                    plr->TeleportTo(garr->GetGarrisonMapID(), hearhstoneAlliance[idx].m_positionX, hearhstoneAlliance[idx].m_positionY, hearhstoneAlliance[idx].m_positionZ, hearhstoneAlliance[idx].m_orientation, TELE_TO_SPELL, 171253);
                else
                    plr->TeleportTo(garr->GetGarrisonMapID(), hearhstoneHorde[idx].m_positionX, hearhstoneHorde[idx].m_positionY, hearhstoneHorde[idx].m_positionZ, hearhstoneHorde[idx].m_orientation, TELE_TO_SPELL, 171253);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_garrison_hearthstone_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_TELEPORT_L);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_garrison_hearthstone_SpellScript();
    }
};

//http://www.wowhead.com/spell=173847/loot
//! HORDE Q: 34824 ALLIANCE Q: 35176
class spell_garrison_cache_loot : public SpellScriptLoader
{
public:
    spell_garrison_cache_loot() : SpellScriptLoader("spell_garrison_cache_loot") { }

    class spell_garrison_cache_loot_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_garrison_cache_loot_SpellScript);

        enum data
        {
            NPC__ = 80223,
            QUESTA = 35176,
            QUESTH = 34824,
        };

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* caster = GetCaster())
            {
                Player* plr = caster->ToPlayer();
                if (!plr)
                    return;

                Garrison *garr = plr->GetGarrison();
                if (!garr || !garr->GetResNumber())
                    return;
                
                plr->ModifyCurrency(CURRENCY_TYPE_GARRISON_RESOURCES, garr->GetResNumber(), false, false, true, true, true);
                garr->UpdateResTakenTime();

                if (plr->GetQuestStatus(QUESTA) == QUEST_STATUS_INCOMPLETE ||
                    plr->GetQuestStatus(QUESTH) == QUEST_STATUS_INCOMPLETE)
                if (Creature *c = plr->FindNearestCreature(NPC__, 100.0f))
                    sCreatureTextMgr->SendChat(c, TEXT_GENERIC_0, plr->GetGUID());
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_garrison_cache_loot_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_garrison_cache_loot_SpellScript();
    }
};

//162714
class spell_q34824 : public SpellScriptLoader
{
public:
    spell_q34824() : SpellScriptLoader("spell_q34824") { }

    class spell_q34824_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_q34824_SpellScript);


        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* caster = GetCaster())
            {
                std::list<Player*> playerList;
                caster->GetPlayerListInGrid(playerList, 10.0f);
                for (auto player : playerList)
                    player->KilledMonsterCredit(caster->GetEntry(), ObjectGuid::Empty);

                if (Creature *c = caster->ToCreature())
                    sCreatureTextMgr->SendChat(c, TEXT_GENERIC_0);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_q34824_SpellScript::HandleScriptEffect, 2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_q34824_SpellScript();
    }
};

//161033 for the gord cast 160767 too
//! HORDE Q: 34378, ALLIANCE Q: 34586
class spell_161033 : public SpellScriptLoader
{
public:
    spell_161033() : SpellScriptLoader("spell_161033") { }

    class spell_161033_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_161033_SpellScript);

#define __spell 160767
        void Handle(SpellEffIndex effIndex)
        {
            Unit* caster = GetCaster();
            if (!caster || !caster->ToPlayer())
                return;

            if (caster->ToPlayer()->GetTeam() == ALLIANCE)
                return;

            caster->CastSpell(caster, __spell, true);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_161033_SpellScript::Handle, 0, SPELL_EFFECT_KILL_CREDIT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_161033_SpellScript();
    }
};

//- 155071
/*
UPDATE `npc_spellclick_spells` set cast_flags = 1 WHERE`spell_id` = 155071; -- need plr caster to creature
*/

class spell_interract : public SpellScriptLoader
{
public:
    spell_interract() : SpellScriptLoader("spell_interract") { }

    class spell_interract_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_interract_SpellScript);

        enum data
        {
            NPC__ = 79446,

            SPELL_CREDIT_Q34646 = 177492,

        };

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* caster = GetCaster())
            {
                Player* plr = caster->ToPlayer();
                if (!plr)
                    return;

                Unit * target = GetOriginalTarget(); 
                if (!target)
                    return;

                volatile uint32 entry = target->GetEntry();
                switch (target->GetEntry())
                {
                    case NPC__:
                        //cast 164640
                        //cast 164649
                        plr->TalkedToCreature(target->GetEntry(), target->GetGUID());
                        caster->CastSpell(caster, SPELL_CREDIT_Q34646, true);
                        break;
                    default:
                        break;
                }

            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_interract_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_interract_SpellScript();
    }
};

// Lumber mill
// Summon Lumberjack Faction Picker
//! Spell: 168641 -> 167962 -> H: 167961 A: 167902 (NPC: H:83985, A: 83950)
//! Q: A: 36189, H: 36137
//! Go: 234021

class spell_garr_lumberjack : public SpellScriptLoader
{
public:
    spell_garr_lumberjack() : SpellScriptLoader("spell_garr_lumberjack") { }

    class spell_garr_lumberjack_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_garr_lumberjack_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* caster = GetCaster())
            {
                Player* plr = caster->ToPlayer();
                if (!plr)
                    return;

                if (plr->GetTeam() == ALLIANCE)
                    plr->CastSpell(plr, 167902, false);
                else
                    plr->CastSpell(plr, 167961, false); 
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_garr_lumberjack_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_garr_lumberjack_SpellScript();
    }
};

// 83985 83950
class mob_garr_lumberjack : public CreatureScript
{
public:
    mob_garr_lumberjack() : CreatureScript("mob_garr_lumberjack") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_garr_lumberjackAI(creature);
    }

    struct mob_garr_lumberjackAI : public ScriptedAI
    {

        mob_garr_lumberjackAI(Creature* creature) : ScriptedAI(creature), isQuest(false)
        {
        }

        enum data
        {
            SOUNDID = 7514,
            GO_TREE_H = 233922,
            GO_TREE_A = 234021,
            SPELL_PEON_AXE = 167957,
            SPELL_CARRY_LUMBER = 167958,//Carry Lumber
            SPELL_CREATE_TIMBER = 168647,

            SPELL_STUMP = 170079,

            SPELL_TIMBER_5 = 168523,
        };

        ObjectGuid treeGUID;
        ObjectGuid plrGUID;
        EventMap _events;
        bool isQuest;

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                if (GameObject * tree = me->GetMap()->GetGameObject(treeGUID))
                    me->SetFacingToObject(tree);

                me->CastSpell(me, SPELL_PEON_AXE, true);
                me->HandleEmoteCommand(EMOTE_STATE_WORK_CHOPWOOD_2);
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_WORK_CHOPWOOD_2);
                _events.ScheduleEvent(EVENT_1, 5000);
            }
            else
            {
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                if (Player* player = ObjectAccessor::GetPlayer(*me, plrGUID))
                {
                    if (!isQuest)
                    {
                        float count = urand(5, 7);
                        WorldPackets::Chat::WorldText packet;
                        packet.Guid = ObjectGuid::Empty;
                        packet.Arg1 = count;
                        packet.Arg2 = 0;
                        packet.Text = sObjectMgr->GetTrinityStringForDBCLocale(LANG_LUMBER_RESULT);
                        player->SendDirectMessage(packet.Write());
                        me->CastCustomSpell(player, SPELL_TIMBER_5, &count, NULL, NULL, false);
                    }else
                        me->CastSpell(player, SPELL_CREATE_TIMBER, true);
                }
                me->DespawnOrUnsummon(1);
            }
        }

        void IsSummonedBy(Unit* summoner) override
        {
            Player *player = summoner->ToPlayer();
            if (!player)
            {
                me->MonsterSay("SCRIPT::mob_garr_lumberjack summoner is not player", LANG_UNIVERSAL, ObjectGuid::Empty);
                return;
            }
            plrGUID = summoner->GetGUID();
            me->PlayDirectSound(SOUNDID, player);

            QuestStatus status = player->GetQuestStatus(player->GetTeam() == ALLIANCE ? 36189 : 36137);
            isQuest = !(status == QUEST_STATUS_REWARDED);

            std::list<GameObject*> gameobjectList;
            CellCoord pair(Trinity::ComputeCellCoord(summoner->GetPositionX(), summoner->GetPositionY()));
            Cell cell(pair);
            cell.SetNoCreate();

            Trinity::GameObjectInRangeCheck check(summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ(), 10.0f);
            Trinity::GameObjectListSearcher<Trinity::GameObjectInRangeCheck> searcher(summoner, gameobjectList, check);

            cell.Visit(pair, Trinity::makeGridVisitor(searcher), *(summoner->GetMap()), *summoner, 10.0f);
            for (auto tree : gameobjectList)
            {
                if (tree->GetUInt32Value(GAMEOBJECT_FIELD_FLAGS) & GO_FLAG_LUMBER)
                {
                    Position pos;
                    tree->GetRandomNearPosition(pos, 5.0f);
                    me->GetMotionMaster()->MovePoint(1, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                    treeGUID = tree->GetGUID();

                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (Player* player = ObjectAccessor::GetPlayer(*me, plrGUID))
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                            if (GameObject * tree = me->GetMap()->GetGameObject(treeGUID))
                            {
                                me->CastSpell(me, SPELL_STUMP, true);
                                tree->setVisibilityCDForPlayer(plrGUID, 300);
                                tree->DestroyForPlayer(player);
                            }
                            me->CastSpell(me, SPELL_CARRY_LUMBER, true);
                            _events.ScheduleEvent(EVENT_2, 5000);
                            me->GetMotionMaster()->MovePoint(2, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
                            break;
                        case EVENT_2:
                            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                            me->CastSpell(player, SPELL_CREATE_TIMBER, true);
                            break;
                    }
                }
            }
        }
    };
};

//! A: 36194 H: 36142
// Go: 234000
// Spell: 167970 Summon Lumberjack Faction Picker 
class spell_garr_lumberjack_lvl2 : public SpellScriptLoader
{
public:
    spell_garr_lumberjack_lvl2() : SpellScriptLoader("spell_garr_lumberjack_lvl2") { }

    class spell_garr_lumberjack_lvl2_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_garr_lumberjack_lvl2_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* caster = GetCaster())
            {
                Player* plr = caster->ToPlayer();
                if (!plr)
                    return;

                //AiID: 6784
                if (plr->GetTeam() == ALLIANCE)
                    plr->CastSpell(plr, 167969, false);
                else
                    plr->CastSpell(plr, 168043, false);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_garr_lumberjack_lvl2_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_garr_lumberjack_lvl2_SpellScript();
    }
};
// 84003 84004
// 170069->167972(animKIt)->167974->168028->167958
class mob_garr_lumberjack2 : public CreatureScript
{
public:
    mob_garr_lumberjack2() : CreatureScript("mob_garr_lumberjack2") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_garr_lumberjack2AI(creature);
    }

    struct mob_garr_lumberjack2AI : public ScriptedAI
    {

        mob_garr_lumberjack2AI(Creature* creature) : ScriptedAI(creature), isQuest(false)
        {
            me->UpdatePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ() + 6.0f, true);
        }

        enum data
        {
            SPELL_PEON_AXE = 167957,
            SPELL_CARRY_LUMBER = 167958, //Carry Lumber
            SPELL_CREATE_TIMBER2 = 168942,

            SPELL_STUMP = 170079,

            SPELL_TIMBER_10 = 168524,
        };

        ObjectGuid treeGUID;
        ObjectGuid plrGUID;
        EventMap _events;
        bool isQuest;

        void IsSummonedBy(Unit* summoner) override
        {
            Player *player = summoner->ToPlayer();
            if (!player)
            {
                me->MonsterSay("SCRIPT::mob_garr_lumberjack summoner is not player", LANG_UNIVERSAL, ObjectGuid::Empty);
                return;
            }
            plrGUID = summoner->GetGUID();

            QuestStatus status = player->GetQuestStatus(player->GetTeam() == ALLIANCE ? 36194 : 36142);
            isQuest = !(status == QUEST_STATUS_REWARDED);
            me->UpdatePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 6.0f, true);
            me->CastSpell(me, 170069, true);

            //167972
            me->SetAnimKitId(6784);
            //me->SetDisableGravity(true, false);
            WorldPackets::Misc::SetPlayHoverAnim packet;
            packet.UnitGUID = me->GetGUID();
            packet.PlayHoverAnim = true;
            player->SendDirectMessage(packet.Write());
            me->CastSpell(me, 167972, true);

            _events.ScheduleEvent(EVENT_1, 1000);
           
            std::list<GameObject*> gameobjectList;
            CellCoord pair(Trinity::ComputeCellCoord(summoner->GetPositionX(), summoner->GetPositionY()));
            Cell cell(pair);
            cell.SetNoCreate();

            Trinity::GameObjectInRangeCheck check(summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ(), 10.0f);
            Trinity::GameObjectListSearcher<Trinity::GameObjectInRangeCheck> searcher(summoner, gameobjectList, check);

            cell.Visit(pair, Trinity::makeGridVisitor(searcher), *(summoner->GetMap()), *summoner, 10.0f);
            for (auto tree : gameobjectList)
            {
                if (tree->GetUInt32Value(GAMEOBJECT_FIELD_FLAGS) & GO_FLAG_LUMBER)
                {
                    Position pos;
                    tree->GetNearPoint(tree, pos.m_positionX, pos.m_positionY, pos.m_positionZ, 1.0f, 1.0f, 360.0f);
                    treeGUID = tree->GetGUID();
                    me->UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 6.0f, true);
                    me->SetFacingToObject(tree);
                    break;
                }
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE && type  != EFFECT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                me->CastSpell(me, 167974, true);
                me->RemoveAurasDueToSpell(167972);
                me->SetAnimKitId(0);
                me->SetDisableGravity(false);
                WorldPackets::Misc::SetPlayHoverAnim packet;
                packet.UnitGUID = me->GetGUID();
                packet.PlayHoverAnim = false;
                if (Player* player = ObjectAccessor::GetPlayer(*me, plrGUID))
                    player->SendDirectMessage(packet.Write());
                _events.ScheduleEvent(EVENT_2, 4000);
                me->GetMotionMaster()->MoveFall();
                if (GameObject * tree = me->GetMap()->GetGameObject(treeGUID))
                    me->SetFacingToObject(tree);
            }
            else if (id == 2)
            {
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, plrGUID);
                if (Player* player = ObjectAccessor::GetPlayer(*me, plrGUID))
                {
                    if (!isQuest)
                    {
                        float count = urand(10, 15);
                        WorldPackets::Chat::WorldText packet;
                        packet.Guid = ObjectGuid::Empty;
                        packet.Arg1 = count;
                        packet.Arg2 = 0;
                        packet.Text = sObjectMgr->GetTrinityStringForDBCLocale(LANG_LUMBER_RESULT);
                        player->SendDirectMessage(packet.Write());
                        me->CastCustomSpell(player, SPELL_TIMBER_10, &count, NULL, NULL, false);
                    }
                    else
                        me->CastSpell(player, SPELL_CREATE_TIMBER2, true);
                }
                me->DespawnOrUnsummon(1);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (Player* player = ObjectAccessor::GetPlayer(*me, plrGUID))
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                            me->GetMotionMaster()->MoveTakeoff(1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() - 6.0f);
                            break;
                        case EVENT_2:
                            me->CastSpell(me, 168028, true);
                            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, plrGUID);
                            _events.ScheduleEvent(EVENT_3, 1000);
                            break;
                        case EVENT_3:
                            me->CastSpell(me, 159560, true);
                            _events.ScheduleEvent(EVENT_4, 2000);

                            /*if (GameObject * tree = me->GetMap()->GetGameObject(treeGUID))
                            {
                                Position pos;
                                tree->GetRandomNearPosition(pos, 5.0f);
                                me->GetMotionMaster()->MovePoint(0, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                            }*/
                            break;
                        case EVENT_4:
                            me->RemoveAurasDueToSpell(167974);
                            me->SetAnimKitId(6778);
                            me->HandleEmoteCommand(EMOTE_STATE_WORK_CHOPWOOD_2);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_WORK_CHOPWOOD_2);
                            _events.ScheduleEvent(EVENT_5, 5000);
                            break;
                        case EVENT_5:
                            me->SetAnimKitId(0);
                            me->RemoveAurasDueToSpell(168028);
                            if (GameObject * tree = me->GetMap()->GetGameObject(treeGUID))
                            {
                                me->CastSpell(me, SPELL_STUMP, true);
                                me->CastSpell(me, 167958, true);
                                tree->setVisibilityCDForPlayer(plrGUID, 300);
                                tree->DestroyForPlayer(player);
                            }
                            me->GetMotionMaster()->MovePoint(2, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
                            break;
                    }
                }
            }
        }
    };
};

// Q: A : 37088 H : 37062
// spell: 174569
class spell_q37088_q37062 : public SpellScriptLoader
{
public:
    spell_q37088_q37062() : SpellScriptLoader("spell_q37088_q37062") { }

    class spell_q37088_q37062_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_q37088_q37062_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            Unit* caster = GetCaster();
            if (!caster)
                return;

            Player *player = caster->ToPlayer();
            if (!player)
                return;

            player->KilledMonsterCredit(87254, ObjectGuid::Empty);

            if (Unit* target = GetExplTargetUnit())
                sCreatureTextMgr->SendChat(target->ToCreature(), TEXT_GENERIC_0, player->GetGUID());
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_q37088_q37062_SpellScript::HandleScriptEffect, 0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_q37088_q37062_SpellScript();
    }
};

void AddSC_garrison_general()
{
    new spell_garrison_hearthstone();
    new spell_garrison_cache_loot();
    new spell_q34824();
    new spell_161033();
    new spell_interract();
    new spell_garr_lumberjack();
    new mob_garr_lumberjack();
    new spell_garr_lumberjack_lvl2();
    new mob_garr_lumberjack2();
    new spell_q37088_q37062();
}