/*
    Dungeon : Eye of Azshara 100-110
    Encounter: Warlord Parjesh
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "eye_of_azshara.h"

enum Says
{
    SAY_INTRO           = 0,
    SAY_INTRO_1         = 1,
    SAY_AGGRO           = 2,
    SAY_CALL            = 3,
    SAY_SPEAR_NPC       = 4,
    SAY_TARGET          = 5,
    SAY_SPEAR_PLR       = 6,
    SAY_ENRAGE          = 7,
    SAY_DEATH           = 8,
    // 9 - Сш-ш!.. Сокруши их!  10 - Ха-ха, от меня не спрятаться!    (пока хз, куда)
}; 

enum Spells
{
    SPELL_EMPTY_ENERGY              = 202146,
    SPELL_ENERGIZE                  = 202143,
    SPELL_CALL_REINFORCEMENTS_1     = 192072,
    SPELL_CALL_REINFORCEMENTS_2     = 192073,
    SPELL_CALL_REINFORCEMENTS       = 196563, //Heroic+
    SPELL_THROW_SPEAR               = 192131,
    SPELL_CRASHING_WAVE             = 191900,
    SPELL_IMPALING_SPEAR_FILTER     = 191927,
    SPELL_IMPALING_SPEAR_FIXATE     = 192094,
    SPELL_IMPALING_SPEAR            = 191946,
    SPELL_IMPALING_SPEAR_DMG_NPC    = 191975,
    SPELL_IMPALING_SPEAR_DMG_PLR    = 191977,
    SPELL_IMPALING_SPEAR_KNOCK      = 193183,
    SPELL_ENRAGE                    = 197064,
};

enum eEvents
{
    EVENT_CALL_REINFORC     = 1,
    EVENT_THROW_SPEAR       = 2,
    EVENT_CRASHING_WAVE     = 3,
    EVENT_IMPALING_SPEAR    = 4,
};

enum eMisc
{
    DATA_TARGET_GUID        = 0,
};

Position const centrPos = { -3682.64f, 4417.02f, 32.44f };

//91784
class boss_warlord_parjesh : public CreatureScript
{
public:
    boss_warlord_parjesh() : CreatureScript("boss_warlord_parjesh") {}

    struct boss_warlord_parjeshAI : public BossAI
    {
        boss_warlord_parjeshAI(Creature* creature) : BossAI(creature, DATA_PARJESH) {}

        bool randSum = false;
        bool enrage = false;
        bool introDone = false;
        bool introDone_1 = false;
        ObjectGuid targetPlayer;

        void SetGUID(ObjectGuid const& guid, int32 type) override
        {
            if (type == DATA_TARGET_GUID)
                targetPlayer = guid;
        }

        ObjectGuid GetGUID(int32 type) override
        {
            if (type == DATA_TARGET_GUID)
                return targetPlayer;

            return ObjectGuid::Empty;
        }

        void Reset() override
        {
            _Reset();
            me->RemoveAurasDueToSpell(SPELL_ENERGIZE);
            me->RemoveAurasDueToSpell(SPELL_ENRAGE);
            DoCast(me, SPELL_EMPTY_ENERGY, true);
            randSum = false;
            enrage = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_AGGRO);
            _EnterCombat();
            DoCast(me, SPELL_ENERGIZE, true);

            events.RescheduleEvent(EVENT_CALL_REINFORC, 3500 + urand(0, 1000));
            events.RescheduleEvent(EVENT_THROW_SPEAR, 11000);
            events.RescheduleEvent(EVENT_CRASHING_WAVE, 3000);
        }
        
        void MoveInLineOfSight(Unit* who) override
        {  
            if (!who->IsPlayer())
                return;

            if (!introDone && me->IsWithinDistInMap(who, 190.0f))
            {
                introDone = true;
                ZoneTalk(SAY_INTRO);
            }
            if (!introDone_1 && me->IsWithinDistInMap(who, 100.0f))
            {
                introDone_1 = true;
                ZoneTalk(SAY_INTRO_1);
            }
            BossAI::MoveInLineOfSight(who);
        }          

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            events.RescheduleEvent(EVENT_IMPALING_SPEAR, 0);
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_IMPALING_SPEAR_FILTER:
                    Talk(SAY_TARGET, target->GetGUID());
                    SetGUID(target->GetGUID(), DATA_TARGET_GUID);
                    break;
                case SPELL_IMPALING_SPEAR_DMG_NPC:
                    Talk(SAY_SPEAR_NPC);
                    break;
                case SPELL_IMPALING_SPEAR_DMG_PLR:
                    Talk(SAY_SPEAR_PLR);
                    DoCast(target, SPELL_IMPALING_SPEAR_KNOCK, true);
                    break;
            }
        }

        void SpellFinishCast(const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_CRASHING_WAVE:
                    if (me->getVictim())
                        me->SetFacingTo(me->getVictim());
                    break;
                case SPELL_CALL_REINFORCEMENTS:
                    DoCast(me, SPELL_CALL_REINFORCEMENTS_1, true);
                    DoCast(me, SPELL_CALL_REINFORCEMENTS_2, true);
                    break;
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPct(31) && !enrage)
            {
                enrage = true;
                DoCast(me, SPELL_ENRAGE, true);
                Talk(SAY_ENRAGE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (CheckHomeDistToEvade(diff, 60.0f))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CALL_REINFORC:
                    {
                        if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
                            DoCast(SPELL_CALL_REINFORCEMENTS);
                        else
                        {
                            if (!randSum)
                            {
                                randSum = true;
                                DoCast(SPELL_CALL_REINFORCEMENTS_1);
                            }
                            else
                            {
                                randSum = false;
                                DoCast(SPELL_CALL_REINFORCEMENTS_2);
                            }
                        }
                        Talk(SAY_CALL);
                        events.RescheduleEvent(EVENT_CALL_REINFORC, urand(28000, 35000));
                        break;
                    }
                    case EVENT_THROW_SPEAR:
                        if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                            DoCast(pTarget, SPELL_THROW_SPEAR);
                        events.RescheduleEvent(EVENT_THROW_SPEAR, 16000);
                        break;
                    case EVENT_CRASHING_WAVE:
                        DoCast(SPELL_CRASHING_WAVE);
                        events.RescheduleEvent(EVENT_CRASHING_WAVE, 3000);
                        break;
                    case EVENT_IMPALING_SPEAR:
                        targetPlayer.Clear();
                        DoCast(me, SPELL_IMPALING_SPEAR_FILTER, true);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_warlord_parjeshAI (creature);
    }
};

//trash 91785
class npc_wandering_shellback : public CreatureScript
{
public:
    npc_wandering_shellback() : CreatureScript("npc_wandering_shellback") {}

    struct npc_wandering_shellbackAI : public ScriptedAI
    {
        npc_wandering_shellbackAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;    
        
        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 15000);
            events.RescheduleEvent(EVENT_2, 25000);
        }            

        void SpellHit(Unit* caster, SpellInfo const* spell) override   
        {
            if (spell->Id == 197141)
            {
                DoCast(me, 15716);
                DoZoneInCombat(me, 50.0f);
            }

            if (me->HasAura(195103) && caster->GetTypeId() == TYPEID_PLAYER)
                me->CastSpell(caster, 195104);
        } 
    
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(195105);
                        events.RescheduleEvent(EVENT_1, 15000);
                        break;                     
                    case EVENT_2:
                        DoCast(195103);
                        events.RescheduleEvent(EVENT_2, 25000);
                        break;        
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wandering_shellbackAI (creature);
    }
};

//191946
class spell_parjesh_impaling_spear : public SpellScriptLoader
{
    public:
        spell_parjesh_impaling_spear() : SpellScriptLoader("spell_parjesh_impaling_spear") { }

        class spell_parjesh_impaling_spear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_parjesh_impaling_spear_SpellScript);

            ObjectGuid targetGUID;
            WorldObject* objTarget = nullptr;

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                Creature* caster = GetCaster()->ToCreature();
                if (!caster || targets.empty())
                    return;

                targetGUID = caster->AI()->GetGUID(DATA_TARGET_GUID);
                if (!targetGUID)
                {
                    targets.clear();
                    return;
                }

                Player* pPlayer = ObjectAccessor::GetPlayer(*caster, targetGUID);
                if (!pPlayer)
                {
                    targets.clear();
                    return;
                }

                std::list<Creature*> creList;
                caster->GetAliveCreatureListWithEntryInGrid(creList, NPC_HATECOIL_SHELLBREAKER, 100.0f);
                caster->GetAliveCreatureListWithEntryInGrid(creList, NPC_HATECOIL_CRESTRIDER, 100.0f);
                for (auto const& trashTarget : creList)
                    targets.push_back(trashTarget);

                for (auto const& target : targets)
                    if (target->IsInBetween(caster, pPlayer, 3.0f))
                        if (!objTarget || (caster->GetDistance(objTarget) > caster->GetDistance(target)))
                            objTarget = target;

                if (!objTarget)
                    objTarget = pPlayer;

                targets.clear();
                targets.push_back(objTarget);
            }

            void HandleOnHit()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (GetHitUnit() != objTarget)
                    return;

                if (GetHitUnit()->GetTypeId() != TYPEID_PLAYER)
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_IMPALING_SPEAR_DMG_NPC, true);
                else
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_IMPALING_SPEAR_DMG_PLR, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_parjesh_impaling_spear_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_ENEMY_BETWEEN_DEST2);
                OnHit += SpellHitFn(spell_parjesh_impaling_spear_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_parjesh_impaling_spear_SpellScript();
        }
};

//192072, 192073
class spell_parjesh_call_reinforcements : public SpellScript
{
    PrepareSpellScript(spell_parjesh_call_reinforcements);

    Position pos;

    void HandleScript(SpellEffIndex effIndex)
    {
        if (effIndex == EFFECT_0)
        {
          centrPos.SimplePosXYRelocationByAngle(pos, 38.0f, frand(0.0f, 6.28f));
          pos.m_positionZ = GetCaster()->GetHeight(pos.m_positionX, pos.m_positionY, pos.m_positionZ + 2.0f, true);
        }

        GetHitDest()->Relocate(pos);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_parjesh_call_reinforcements::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnEffectLaunch += SpellEffectFn(spell_parjesh_call_reinforcements::HandleScript, EFFECT_1, SPELL_EFFECT_SUMMON);
    }
};

void AddSC_boss_warlord_parjesh()
{
    new boss_warlord_parjesh();
    new npc_wandering_shellback();
    new spell_parjesh_impaling_spear();
    RegisterSpellScript(spell_parjesh_call_reinforcements);
}