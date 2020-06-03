////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "AshranMgr.hpp"

/// A'shran Herald - 84113
class npc_ashran_herald : public CreatureScript
{
public:
    npc_ashran_herald() : CreatureScript("npc_ashran_herald") { }

    struct npc_ashran_heraldAI : public ScriptedAI
    {
        npc_ashran_heraldAI(Creature* creature) : ScriptedAI(creature) { }

        enum eTalk
        {
            AnnounceNeutralGraveyard,
            AnnounceHordeGraveyard,
            AnnounceAllianceGraveyard
        };

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case AnnounceMarketplaceGraveyard:
                    Talk(AnnounceNeutralGraveyard);
                    break;
                case eAshranActions::AnnounceHordeGraveyard:
                    Talk(AnnounceHordeGraveyard);
                    break;
                case eAshranActions::AnnounceAllianceGraveyard:
                    Talk(AnnounceAllianceGraveyard);
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_heraldAI(creature);
    }
};

/// SLG Generic MoP (Large AOI) - 68553
class npc_slg_generic_mop : public CreatureScript
{
public:
    npc_slg_generic_mop() : CreatureScript("npc_slg_generic_mop") { }

    struct npc_slg_generic_mopAI : public ScriptedAI
    {
        npc_slg_generic_mopAI(Creature* creature) : ScriptedAI(creature) { }

        enum eTalk
        {
            HordeVictory,
            AllianceKillBoss,
            AllianceVictory,
            HordeKillBoss
        };

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case AnnounceHordeVictory:
                    Talk(HordeVictory);
                    break;
                case AnnounceAllianceKillBoss:
                    Talk(AllianceKillBoss);
                    break;
                case AnnounceAllianceVictory:
                    Talk(AllianceVictory);
                    break;
                case AnnounceHordeKillBoss:
                    Talk(HordeKillBoss);
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_slg_generic_mopAI(creature);
    }
};

/// High Warlord Volrath - 82877
/// Grand Marshal Tremblade - 82876
class npc_faction_boss : public CreatureScript
{
public:
    npc_faction_boss() : CreatureScript("npc_faction_boss") { }

    struct npc_faction_bossAI : public ScriptedAI
    {
        npc_faction_bossAI(Creature* creature) : ScriptedAI(creature)
        {
            m_ZoneScript = sOutdoorPvPMgr->GetZoneScript(creature->GetZoneId());
        }

        enum eSpells
        {
            SpellBladeTwisterSearcher = 178798,   ///< Uses 178797 on the target (Only 1)
            SpellBladeTwisterMissile = 178797,   ///< Launch 178795, Summons 89320
            SpellMortalCleave = 177147,
            SpellEnableUnitFrame = 177684,

            SpellAshranLaneMobScaling = 178838,
            AshranLaneMobScalingAura = 164310
        };

        enum eTalk
        {
            TalkIntro,
            TalkAggro,
            TalkSlay,
            TalkDeath,
            TalkVictory
        };

        enum eEvents
        {
            EventMortalCleave = 1,
            EventBladeTwister
        };

        EventMap m_Events;
        ZoneScript* m_ZoneScript;

        void Reset() override
        {
            m_Events.Reset();

            me->RemoveAura(SpellEnableUnitFrame);
            me->RemoveAura(SpellAshranLaneMobScaling);

            if (me->GetEntry() == GrandMarshalTremblade)
                me->setFaction(12); ///< Alliance
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            Talk(TalkAggro);

            m_Events.ScheduleEvent(EventMortalCleave, 5000);
            m_Events.ScheduleEvent(EventBladeTwister, 15000);

            me->CastSpell(me, SpellEnableUnitFrame, true);
        }

        void KilledUnit(Unit* p_Who) override
        {
            if (p_Who->GetTypeId() == TYPEID_PLAYER)
                Talk(TalkSlay);
        }

        void JustDied(Unit* killer) override
        {
            Talk(TalkDeath);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(m_ZoneScript))
            {
                auto l_GenericGuid = l_Ashran->GetFactionGenericMoP(me->GetEntry() == GrandMarshalTremblade ? TEAM_ALLIANCE : TEAM_HORDE);
                if (Creature* l_GenericMoP = sObjectAccessor->FindCreature(l_GenericGuid))
                    l_GenericMoP->AI()->DoAction(me->GetEntry() == GrandMarshalTremblade ? AnnounceHordeKillBoss : AnnounceAllianceKillBoss);

                l_Ashran->HandleFactionBossDeath(me->GetEntry() == GrandMarshalTremblade ? TEAM_HORDE : TEAM_ALLIANCE);
            }

            /// Upon successfully defeating the enemy leader, those present receive 50 Honor and 250 Conquest
            std::list<Player*> l_PlayerList;
            me->GetPlayerListInGrid(l_PlayerList, 100.0f);

            l_PlayerList.remove_if([this](Player* player) -> bool
            {
                if (player == nullptr)
                    return true;

                if (!me->IsValidAttackTarget(player))
                    return true;

                return false;
            });

            for (Player* l_Player : l_PlayerList)
                l_Player->RewardHonor(l_Player, 1, 50 * 100);

            /// Trigger strongboxes loot for near players
            if (me->GetEntry() == GrandMarshalTremblade)
                killer->CastSpell(killer, SpellAllianceReward, true);
            else
                killer->CastSpell(killer, SpellHordeReward, true);
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case WarspearOutpostInFight:
                case StormshieldStrongholdInFight:
                    Talk(TalkIntro);
                    break;
                case WarspearVictory:
                case StormshieldVictory:
                    Talk(TalkVictory);
                    break;
                default:
                    break;
            }
        }

        void SpellHit(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_SpellInfo->Id == SpellBladeTwisterSearcher)
                me->CastSpell(p_Target, SpellBladeTwisterMissile, false);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
            {
                if (me->isInCombat())
                    EnterEvadeMode();
                return;
            }

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventMortalCleave:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellMortalCleave, false);
                    m_Events.ScheduleEvent(EventMortalCleave, 15000);
                    break;
                case EventBladeTwister:
                    me->CastSpell(me, SpellBladeTwisterSearcher, true);
                    m_Events.ScheduleEvent(EventBladeTwister, 30000);
                    break;
                default:
                    break;
            }

            EnterEvadeIfOutOfCombatArea(p_Diff);
            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_faction_bossAI(creature);
    }
};

/// Shevan Manille <Flight Master> - 87672
/// Tina Kelatara <Flight Master> - 87617
class npc_ashran_flight_masters : public CreatureScript
{
public:
    npc_ashran_flight_masters() : CreatureScript("npc_ashran_flight_masters") { }

    bool OnGossipSelect(Player* player, Creature*, uint32, uint32) override
    {
        if (player == nullptr || !player->IsInWorld())
            return true;

        if (player->GetTeamId() == TEAM_ALLIANCE)
            player->ActivateTaxiPathTo(TaxiPathBaseHordeToAlliance);
        else
            player->ActivateTaxiPathTo(TaxiPathBaseAllianceToHorde);

        return false;
    }

    struct npc_ashran_flight_mastersAI : public ScriptedAI
    {
        npc_ashran_flight_mastersAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_NPC);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_flight_mastersAI(creature);
    }
};

/// Alliance Spirit Guide - 80723
/// Horde Spirit Guide - 80724
class npc_ashran_spirit_healer : public CreatureScript
{
public:
    npc_ashran_spirit_healer() : CreatureScript("npc_ashran_spirit_healer") { }

    struct npc_ashran_spirit_healerAI : public ScriptedAI
    {
        npc_ashran_spirit_healerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->setDeathState(DEAD);
            me->AddChannelObject(me->GetGUID());
            //me->SetChannelSpellID(eAshranSpells::SpellSpiritHeal);
            me->SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, 1.0f);
            me->SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, 1.0f);
            DoCast(me, SpellSpiritHeal);
        }

        void JustRespawned() override { }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (!me->HasUnitState(UNIT_STATE_CASTING))
                DoCast(me, SpellSpiritHeal);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_spirit_healerAI(creature);
    }
};

/// Kor'lok <The Ogre King> - 80858
class npc_ashran_korlok : public CreatureScript
{
public:
    npc_ashran_korlok() : CreatureScript("npc_ashran_korlok") { }

    struct npc_ashran_korlokAI : public ScriptedAI
    {
        npc_ashran_korlokAI(Creature* creature) : ScriptedAI(creature)
        {
            m_OutdoorPvP = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());
            m_IsAwake = false;
            m_InFight = false;
        }

        enum eSpells
        {
            SpellBoomingShout = 177150,
            SpellBoonOfKorlok = 177164,
            SpellCrushingLeap = 164819,
            SpellMASSIVEKick = 177157,
            SpellOgreicLanding = 165096,
            SpellOgreicLeap = 164854,

            /// Debuff self casted
            SpellCurseOfKorlok = 165192,
            SpellCurseOfKrong = 165134,

            /// Cosmetic
            ShadowyGhostCosmeticSpawnSpellBlue = 156204,
            AshranLaneMobScalingAura = 164310
        };

        enum eTalk
        {
            TalkAwake,
            TalkRecruitedByAlliance,
            TalkRecruitedByHorde,
            TalkSlay,
            TalkDeath
        };

        enum eEvents
        {
            EventCrushingLeap = 1,
            EventBoomingShout,
            EventMASSIVEKick,
            EventBoonOfKorlok,
            EventCurseOfKorlok
        };

        enum eActions
        {
            ActionHordeRecruit,
            ActionAllianceRecruit
        };

        EventMap m_Events;
        OutdoorPvP* m_OutdoorPvP;

        bool m_IsAwake;
        bool m_InFight;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            if (!m_IsAwake)
                Talk(TalkAwake);

            m_Events.Reset();

            m_IsAwake = true;

            if (!m_InFight)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->setFaction(KorlokNeutral);
                me->CastSpell(me, ShadowyGhostCosmeticSpawnSpellBlue, true);
                me->CastSpell(me, AshranLaneMobScalingAura, true);
            }
        }

        void EnterEvadeMode() override
        {
            /// Copy/Paste of classic EnterEvade mode but without RemoveAllAuras
            /// Sometimes bosses stuck in combat?
            me->DeleteThreatList();
            me->CombatStop(true);
            me->LoadCreaturesAddon();
            me->SetLootRecipient(NULL);
            me->ResetPlayerDamageReq();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_InFight = true;

            m_Events.Reset();

            m_Events.ScheduleEvent(EventCrushingLeap, 10000);
            m_Events.ScheduleEvent(EventBoomingShout, 5000);
            m_Events.ScheduleEvent(EventMASSIVEKick, 20000);
            m_Events.ScheduleEvent(EventBoonOfKorlok, 30000);
        }

        void KilledUnit(Unit* p_Who) override
        {
            if (p_Who->GetTypeId() == TYPEID_PLAYER)
                Talk(TalkSlay);
        }

        void DamageTaken(Unit* /*who*/, uint32& p_Damage, DamageEffectType dmgType) override
        {
            if (me->getFaction() == KorlokNeutral)
            {
                p_Damage = 0;
                return;
            }

            if (p_Damage >= me->GetHealth())
            {
                Talk(TalkDeath);
                p_Damage = 0;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->setFaction(KorlokNeutral);
                me->SetReactState(REACT_PASSIVE);

                if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(m_OutdoorPvP))
                    l_Ashran->EndEvent(EventKorlokTheOgreKing);
            }
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case ActionAllianceRecruit:
                    Talk(TalkRecruitedByAlliance);
                    me->setFaction(KorlokForAlliance);
                    HandleJumpToFight();
                    break;
                case ActionHordeRecruit:
                    Talk(TalkRecruitedByHorde);
                    me->setFaction(KorlokForHorde);
                    HandleJumpToFight();
                    break;
                default:
                    break;
            }
        }

        void SpellHit(Unit* /*p_Target*/, SpellInfo const* p_SpellInfo) override
        {
            if (p_SpellInfo->Id == SpellOgreicLeap)
            {
                OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(m_OutdoorPvP);
                if (l_Ashran == nullptr)
                    return;

                if (Creature* l_SLG = Creature::GetCreature(*me, l_Ashran->GetGenericMoPGuid(l_Ashran->GetCurrentBattleType())))
                {
                    Position l_Pos;
                    l_SLG->GetPosition(&l_Pos);
                    me->GetMotionMaster()->MoveJump(l_Pos.m_positionX, l_Pos.m_positionY, l_Pos.m_positionZ, 100.0f, 100.0f, me->GetOrientation(), 1);
                }
            }
        }

        void MovementInform(uint32 /*type*/, uint32 p_ID) override
        {
            if (p_ID == 1)
            {
                me->CastSpell(me, SpellOgreicLanding, true);

                Position l_Pos;
                me->GetPosition(&l_Pos);
                me->SetHomePosition(l_Pos);
            }
        }

        void UpdateAI(uint32 p_Diff) override
        {
            /// Only for Curse of Kor'lok
            m_Events.Update(p_Diff);

            switch (m_Events.ExecuteEvent())
            {
                case EventCurseOfKorlok:
                    me->CastSpell(me, SpellCurseOfKorlok, true);   ///< Stacks every 30s ... +5% damage taken
                    m_Events.ScheduleEvent(EventCurseOfKorlok, 30000);
                    break;
                default:
                    break;
            }

            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventCrushingLeap:
                {
                    me->CastSpell(me, SpellCrushingLeap, false);
                    m_Events.ScheduleEvent(EventCrushingLeap, 40000);
                    break;
                }
                case EventBoomingShout:
                {
                    me->CastSpell(me, SpellBoomingShout, false);
                    m_Events.ScheduleEvent(EventBoomingShout, 50000);
                    break;
                }
                case EventMASSIVEKick:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    {
                        me->CastSpell(l_Target, SpellMASSIVEKick, true);
                        DoModifyThreatPercent(l_Target, -100);
                    }

                    m_Events.ScheduleEvent(EventMASSIVEKick, 60000);
                    break;
                }
                case EventBoonOfKorlok:
                {
                    me->CastSpell(me, SpellBoonOfKorlok, true);
                    m_Events.ScheduleEvent(EventBoonOfKorlok, 60000);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }

        void HandleJumpToFight()
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(m_OutdoorPvP))
                l_Ashran->EndEvent(EventKorlokTheOgreKing, false);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_AGGRESSIVE);

            me->CastSpell(me, SpellCurseOfKrong, true);
            me->CastSpell(me, SpellCurseOfKorlok, true);   ///< Stacks every 30s ... +5% damage taken
            m_Events.ScheduleEvent(EventCurseOfKorlok, 30000);

            me->CastSpell(me, SpellOgreicLeap, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_korlokAI(creature);
    }
};

/// Muk'Mar Raz <Horde Champion> - 81725
/// Gaul Dun Firok <Alliance Champion> - 81726
class npc_ashran_faction_champions : public CreatureScript
{
public:
    npc_ashran_faction_champions() : CreatureScript("npc_ashran_faction_champions") { }

    struct npc_ashran_faction_championsAI : public ScriptedAI
    {
        npc_ashran_faction_championsAI(Creature* creature) : ScriptedAI(creature)
        {
            m_OutdoorPvP = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());

            m_Rewarded = false;
        }

        enum eSpells
        {
            SpellBoomingShout = 177150,
            SpellCrushingLeap = 164819,
            SpellMASSIVEKick = 177157,
            SpellEnrage = 164811,
            AshranLaneMobScalingAura = 164310
        };

        enum eEvents
        {
            EventCrushingLeap = 1,
            EventBoomingShout,
            EventMASSIVEKick
        };

        enum eActions
        {
            ActionHordeRecruit,
            ActionAllianceRecruit
        };

        EventMap m_Events;
        OutdoorPvP* m_OutdoorPvP;

        bool m_Rewarded;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            m_Events.Reset();

            me->CastSpell(me, AshranLaneMobScalingAura, true);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventCrushingLeap, 10000);
            m_Events.ScheduleEvent(EventBoomingShout, 5000);
            m_Events.ScheduleEvent(EventMASSIVEKick, 20000);
        }

        void JustDied(Unit* killer) override
        {
            if (m_OutdoorPvP == nullptr)
                return;

            Creature* l_Korlok = sObjectAccessor->FindCreature(m_OutdoorPvP->GetCreature(NeutralKorlokTheOgreKing));
            if (l_Korlok == nullptr || !l_Korlok->IsAIEnabled)    ///< Shouldn't happens
                return;

            if (killer->GetTypeId() == TYPEID_PLAYER)
            {
                if (killer->ToPlayer()->GetTeamId() == TEAM_ALLIANCE)
                    l_Korlok->AI()->DoAction(ActionAllianceRecruit);
                else
                    l_Korlok->AI()->DoAction(ActionHordeRecruit);
            }
            else if (killer->GetOwner() && killer->GetOwner()->GetTypeId() == TYPEID_PLAYER)
            {
                if (Player* l_Owner = killer->GetOwner()->ToPlayer())
                {
                    if (l_Owner->ToPlayer()->GetTeamId() == TEAM_ALLIANCE)
                        l_Korlok->AI()->DoAction(ActionAllianceRecruit);
                    else
                        l_Korlok->AI()->DoAction(ActionHordeRecruit);
                }
            }
        }

        void DamageTaken(Unit* /*who*/, uint32& p_Damage, DamageEffectType dmgType) override
        {
            if (p_Damage < me->GetHealth())
            {
                if (me->HasAura(SpellEnrage))
                    return;

                if (me->HealthBelowPctDamaged(50, p_Damage))
                    me->CastSpell(me, SpellEnrage, true);

                return;
            }

            if (m_Rewarded)
                return;

            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
                l_Ashran->CastSpellOnTeam(me, TEAM_ALLIANCE, SpellEventAllianceReward);

            m_Rewarded = true;
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventCrushingLeap:
                {
                    me->CastSpell(me, SpellCrushingLeap, false);
                    m_Events.ScheduleEvent(EventCrushingLeap, 40000);
                    break;
                }
                case EventBoomingShout:
                {
                    me->CastSpell(me, SpellBoomingShout, false);
                    m_Events.ScheduleEvent(EventBoomingShout, 50000);
                    break;
                }
                case EventMASSIVEKick:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    {
                        me->CastSpell(l_Target, SpellMASSIVEKick, true);
                        DoModifyThreatPercent(l_Target, -100);
                    }

                    m_Events.ScheduleEvent(EventMASSIVEKick, 60000);
                    break;
                }
                default:
                    break;
            }

            EnterEvadeIfOutOfCombatArea(p_Diff);
            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_faction_championsAI(creature);
    }
};

/// Mandragoraster - 83683
class npc_ashran_mandragoraster : public CreatureScript
{
public:
    npc_ashran_mandragoraster() : CreatureScript("npc_ashran_mandragoraster") { }

    struct npc_ashran_mandragorasterAI : public ScriptedAI
    {
        npc_ashran_mandragorasterAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpells
        {
            AshranLaneMobScalingAura = 164310,

            SpellSplittingBreath = 161520,

            SpellPiercingChomp = 161932,
            SpellPiercingChompAura = 161933
        };

        enum eEvents
        {
            EventSplittingBreath = 1,
            EventPiercingChomp
        };

        EventMap m_Events;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            me->CastSpell(me, AshranLaneMobScalingAura, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventSplittingBreath, 5000);
            m_Events.ScheduleEvent(EventPiercingChomp, 8000);
        }

        void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_Target == nullptr)
                return;

            if (p_SpellInfo->Id == SpellPiercingChomp)
                me->CastSpell(p_Target, SpellPiercingChompAura, true);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventSplittingBreath:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellSplittingBreath, false);
                    m_Events.ScheduleEvent(EventSplittingBreath, 15000);
                    break;
                case EventPiercingChomp:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellPiercingChomp, false);
                    m_Events.ScheduleEvent(EventPiercingChomp, 8000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_mandragorasterAI(creature);
    }
};

/// Panthora - 83691
class npc_ashran_panthora : public CreatureScript
{
public:
    npc_ashran_panthora() : CreatureScript("npc_ashran_panthora") { }

    struct npc_ashran_panthoraAI : public ScriptedAI
    {
        npc_ashran_panthoraAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpells
        {
            AshranLaneMobScalingAura = 164310,

            SpellShadowClaws = 176542
        };

        enum eEvents
        {
            EventShadowClaws = 1
        };

        EventMap m_Events;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            me->CastSpell(me, AshranLaneMobScalingAura, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventShadowClaws, 5000);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventShadowClaws:
                    me->CastSpell(me, SpellShadowClaws, true);
                    m_Events.ScheduleEvent(EventShadowClaws, 15000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_panthoraAI(creature);
    }
};

/// Ancient Inferno - 84875
class npc_ashran_ancient_inferno : public CreatureScript
{
public:
    npc_ashran_ancient_inferno() : CreatureScript("npc_ashran_ancient_inferno") { }

    struct npc_ashran_ancient_infernoAI : public ScriptedAI
    {
        npc_ashran_ancient_infernoAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpells
        {
            AshranLaneMobScalingAura = 164310,

            SpellLavaBurst = 176170,
            SpellMoltenFirestorm = 176171,
            SpellVolcanicActivitySearch = 176130,
            SpellVolcanicActivityMissil = 176132
        };

        enum eEvents
        {
            EventLavaBurst = 1,
            EventVolcanicActivity,
            EventMoltenFirestorm
        };

        EventMap m_Events;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            me->CastSpell(me, AshranLaneMobScalingAura, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventLavaBurst, 2000);
            m_Events.ScheduleEvent(EventVolcanicActivity, 8000);
            m_Events.ScheduleEvent(EventMoltenFirestorm, 10000);
        }

        void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_Target == nullptr)
                return;

            if (p_SpellInfo->Id == SpellVolcanicActivitySearch)
                me->CastSpell(p_Target, SpellVolcanicActivityMissil, true);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventLavaBurst:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellLavaBurst, false);
                    m_Events.ScheduleEvent(EventLavaBurst, 6000);
                    break;
                case EventVolcanicActivity:
                    me->CastSpell(me, SpellVolcanicActivitySearch, true);
                    m_Events.ScheduleEvent(EventVolcanicActivity, 15000);
                    break;
                case EventMoltenFirestorm:
                    me->CastSpell(me, SpellMoltenFirestorm, true);
                    m_Events.ScheduleEvent(EventMoltenFirestorm, 20000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_ancient_infernoAI(creature);
    }
};

/// Volcano - 88227
class npc_ashran_volcano : public CreatureScript
{
public:
    npc_ashran_volcano() : CreatureScript("npc_ashran_volcano") { }

    struct npc_ashran_volcanoAI : public ScriptedAI
    {
        npc_ashran_volcanoAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpell
        {
            VolcanoAreaTrigger = 176144
        };

        void Reset() override
        {
            me->CastSpell(me, VolcanoAreaTrigger, true);

            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void UpdateAI(uint32 /*p_Diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_volcanoAI(creature);
    }
};

/// Goregore - 84893
class npc_ashran_goregore : public CreatureScript
{
public:
    npc_ashran_goregore() : CreatureScript("npc_ashran_goregore") { }

    struct npc_ashran_goregoreAI : public ScriptedAI
    {
        npc_ashran_goregoreAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpells
        {
            AshranLaneMobScalingAura = 164310,
            SpellCranky = 169710
        };

        enum eEvents
        {
            EventCranky = 1
        };

        EventMap m_Events;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            me->CastSpell(me, AshranLaneMobScalingAura, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventCranky, 1000);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventCranky:
                    me->CastSpell(me, SpellCranky, true);
                    m_Events.ScheduleEvent(EventCranky, 10000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_goregoreAI(creature);
    }
};

/// Ashmaul Magma Caster - 84906
class npc_ashran_ashmaul_magma_caster : public CreatureScript
{
public:
    npc_ashran_ashmaul_magma_caster() : CreatureScript("npc_ashran_ashmaul_magma_caster") { }

    struct npc_ashran_ashmaul_magma_casterAI : public ScriptedAI
    {
        npc_ashran_ashmaul_magma_casterAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpells
        {
            SpellLavaBurstVolley = 169725,
            SpellVolcanicGround = 169724
        };

        enum eEvents
        {
            EventLavaBurstVolley = 1,
            EventVolcanicGround
        };

        EventMap m_Events;

        void Reset() override
        {
            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventLavaBurstVolley, 3000);
            m_Events.ScheduleEvent(EventVolcanicGround, 8000);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventLavaBurstVolley:
                    me->CastSpell(me, SpellLavaBurstVolley, false);
                    m_Events.ScheduleEvent(EventLavaBurstVolley, 10000);
                    break;
                case EventVolcanicGround:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellVolcanicGround, false);
                    m_Events.ScheduleEvent(EventVolcanicGround, 15000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_ashmaul_magma_casterAI(creature);
    }
};

/// Volcanic Ground - 84952
class npc_ashran_volcanic_ground : public CreatureScript
{
public:
    npc_ashran_volcanic_ground() : CreatureScript("npc_ashran_volcanic_ground") { }

    struct npc_ashran_volcanic_groundAI : public ScriptedAI
    {
        npc_ashran_volcanic_groundAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpell
        {
            VolcanicGround = 169723
        };

        void Reset() override
        {
            me->CastSpell(me, VolcanicGround, true);

            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void UpdateAI(uint32 /*p_Diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_volcanic_groundAI(creature);
    }
};

/// Elder Darkweaver Kath - 85771
class npc_ashran_elder_darkweaver_kath : public CreatureScript
{
public:
    npc_ashran_elder_darkweaver_kath() : CreatureScript("npc_ashran_elder_darkweaver_kath") { }

    struct npc_ashran_elder_darkweaver_kathAI : public ScriptedAI
    {
        npc_ashran_elder_darkweaver_kathAI(Creature* creature) : ScriptedAI(creature), m_SummonCount{0}
        {
        }

        enum eSpells
        {
            AshranLaneMobScalingAura = 164310,

            SpellDarknessWithin = 158830,
            SpellDarknessWithinSearcher = 158844,
            SpellDarknessWithinMissile = 158845,
            SpellShadowFigurinesSearch = 158854,
            SpellShadowFigurinesSpawn = 158719
        };

        enum eEvents
        {
            EventDarknessWithin = 1,
            EventShadowFigurines
        };

        EventMap m_Events;
        uint8 m_SummonCount;

        void Reset() override
        {
            m_SummonCount = 0;

            me->setRegeneratingHealth(false);

            me->CastSpell(me, AshranLaneMobScalingAura, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventDarknessWithin, 2000);
            m_Events.ScheduleEvent(EventShadowFigurines, 10000);
        }

        void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_Target == nullptr)
                return;

            switch (p_SpellInfo->Id)
            {
                case SpellDarknessWithinSearcher:
                    me->CastSpell(p_Target, SpellDarknessWithinMissile, true);
                    break;
                case SpellShadowFigurinesSearch:
                    if (m_SummonCount >= 3)
                        break;
                    me->CastSpell(p_Target, SpellShadowFigurinesSpawn, true);
                    ++m_SummonCount;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            HandleHealthAndDamageScaling();

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventDarknessWithin:
                    me->CastSpell(me, SpellDarknessWithin, false);
                    m_Events.ScheduleEvent(EventDarknessWithin, 15000);
                    break;
                case EventShadowFigurines:
                    m_SummonCount = 0;
                    me->CastSpell(me, SpellShadowFigurinesSearch, false);
                    m_Events.ScheduleEvent(EventShadowFigurines, 20000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void HandleHealthAndDamageScaling()
        {
            std::list<HostileReference*> l_ThreatList = me->getThreatManager().getThreatList();
            uint32 l_Count = static_cast<uint32>(std::count_if(l_ThreatList.begin(), l_ThreatList.end(), [this](HostileReference* p_HostileRef) -> bool
            {
                Unit* l_Unit = Unit::GetUnit(*me, p_HostileRef->getUnitGuid());
                return l_Unit && l_Unit->GetTypeId() == TYPEID_PLAYER;
            }));

            if (Aura* l_Scaling = me->GetAura(AshranLaneMobScalingAura))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_Count);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_elder_darkweaver_kathAI(creature);
    }
};

/// Shadow Figurine - 78620
class npc_ashran_shadow_figurine : public CreatureScript
{
public:
    npc_ashran_shadow_figurine() : CreatureScript("npc_ashran_shadow_figurine") { }

    struct npc_ashran_shadow_figurineAI : public ScriptedAI
    {
        npc_ashran_shadow_figurineAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpell
        {
            ShadowChains = 158714
        };

        void Reset() override
        {
            me->CastSpell(me, ShadowChains, true);

            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void UpdateAI(uint32 /*p_Diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_shadow_figurineAI(creature);
    }
};

/// Ashmaul Destroyer - 84876
class npc_ashran_ashmaul_destroyer : public CreatureScript
{
public:
    npc_ashran_ashmaul_destroyer() : CreatureScript("npc_ashran_ashmaul_destroyer") { }

    struct npc_ashran_ashmaul_destroyerAI : public ScriptedAI
    {
        npc_ashran_ashmaul_destroyerAI(Creature* creature) : ScriptedAI(creature) { }

        enum eSpells
        {
            SpellEarthSmash = 176187
        };

        enum eEvents
        {
            EventEarthSmash = 1
        };

        EventMap m_Events;

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventEarthSmash, 1000);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventEarthSmash:
                    me->CastSpell(me, SpellEarthSmash, false);
                    m_Events.ScheduleEvent(EventEarthSmash, 10000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_ashmaul_destroyerAI(creature);
    }
};

/// Fen Tao - 91483
class npc_ashran_fen_tao : public CreatureScript
{
public:
    npc_ashran_fen_tao() : CreatureScript("npc_ashran_fen_tao") { }

    enum SpellIds
    {
        SpellAddFollowerFenTao = 181526
    };

    enum MiscDatas
    {
        GossipMenuId = 90007,
        FirstNpcTextID = 92005,
        SecondNpcTextID = 92006
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PrepareGossipMenu(creature, GossipMenuId, false);
        GossipMenuItem const* l_Item = player->PlayerTalkClass->GetGossipMenu().GetItem(FirstNpcTextID);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, l_Item ? l_Item->Message : "Why are you here?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(MiscDatas::FirstNpcTextID, creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 p_Sender, uint32 p_Action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (p_Action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
            {
                //creature->SendAddFollowerQuery(player, p_Sender, GOSSIP_ACTION_INFO_DEF + 2, "Fen Tao");
                player->SEND_GOSSIP_MENU(MiscDatas::SecondNpcTextID, creature->GetGUID());
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->CastSpell(player, SpellAddFollowerFenTao, true);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            default:
                player->PlayerTalkClass->SendCloseGossip();
                break;
        }

        return true;
    }
};

void AddSC_AshranNPCNeutral()
{
    new npc_ashran_herald();
    //new npc_slg_generic_mop();
    new npc_faction_boss();
    new npc_ashran_flight_masters();
    new npc_ashran_spirit_healer();
    new npc_ashran_korlok();
    new npc_ashran_faction_champions();
    new npc_ashran_mandragoraster();
    new npc_ashran_panthora();
    new npc_ashran_ancient_inferno();
    new npc_ashran_volcano();
    new npc_ashran_goregore();
    new npc_ashran_ashmaul_magma_caster();
    new npc_ashran_volcanic_ground();
    new npc_ashran_elder_darkweaver_kath();
    new npc_ashran_shadow_figurine();
    new npc_ashran_ashmaul_destroyer();
    new npc_ashran_fen_tao();
}
