#include "antorus.h"
#include "AreaTriggerAI.h"

enum Says
{
    //Shatug
    SAY_WARN_SIPHON_CORRUPTION          = 0,

    //Fharg
    SAY_WARN_ENFLAME_CORRUPTION         = 0,
    SAY_WARN_DESOLATE_PATH              = 1,
};

enum Spells
{
    SPELL_CONVERS_OUTRO                 = 249152,
    SPELL_DAILY_ESSENCE_FELHOUNDS       = 305308,

    SPELL_DESTROYER_BOON                = 244049,
    SPELL_DESTROYER_BOON_ALTER          = 244050,
    SPELL_SARGERAS_BLESSING             = 246057,
    SPELL_FOCUSING_POWER                = 251356,

    //Shatug
    SPELL_CORRUPTING_MAW_AURA           = 251447,
    SPELL_CORRUPTING_MAW_DMG            = 254760,
    SPELL_DECAY                         = 245098,
    SPELL_DECAY_DMG                     = 254383,
    SPELL_CONSUMING_SPHERE_FILTER       = 244159,
    SPELL_CONSUMING_SPHERE_AT           = 244107,
    SPELL_SIPHON_CORRUPTION             = 244056,
    SPELL_SIPHONED_MARK                 = 248819,
    SPELL_SIPHONED_DMG                  = 244583,

    //Fharg
    SPELL_FHARG_START_LEAP              = 253244,
    SPELL_BURNING_MAW_AURA              = 251448,
    SPELL_BURNING_MAW_DMG               = 254747,
    SPELL_SMOULDERING                   = 251445,
    SPELL_SMOULDERING_DMG               = 254384,
    SPELL_DESOLATE_PATH_UNK             = 244442, //unk aura
    SPELL_DESOLATE_PATH_FILTER          = 244441,
    SPELL_DESOLATE_PATH_CHANNEL         = 244064,
    SPELL_DESOLATE_PATH_MARK            = 244768,
    SPELL_DESOLATE_PATH_CAST            = 244825,
    SPELL_DESOLATE_PATH_VISUAL          = 244831,
    SPELL_DESOLATE_PATH_VISUAL_CHARGE   = 244833,
    SPELL_DESOLATE_PATH_AT              = 244767,
    SPELL_ENFLAME_CORRUPTION            = 244057,
    SPELL_ENFLAME_CORRUPTION_MARK       = 248815,
    SPELL_ENFLAME_CORRUPTION_DMG        = 244473,

///Heroic+

    //Shatug
    SPELL_WEIGHT_OF_DARKNESS_FILTER     = 244069,
    SPELL_WEIGHT_OF_DARKNESS_AT         = 254429,
    SPELL_WEIGHT_OF_DARKNESS_MOD_SPEED  = 244679,
    SPELL_WEIGHT_OF_DARKNESS_FEAR       = 244071,

    //Fharg
    SPELL_MOLTEN_TOUCH_FILTER           = 244072,
    SPELL_MOLTEN_TOUCH_TELEPORT         = 244084,
    SPELL_MOLTEN_TOUCH_PERIODIC_DUMMY   = 244086,
    SPELL_MOLTEN_TOUCH_VISUAL           = 244099,
    SPELL_MOLTEN_TOUCH_VISUAL_2         = 244100,
    SPELL_MOLTEN_TOUCH_MARK             = 249119,
    SPELL_MOLTEN_TOUCH_CHANNEL          = 249227,
    SPELL_MOLTEN_TOUCH_STUN             = 249241,
    SPELL_MOLTEN_TOUCH_SINGED           = 244091,
    SPELL_MOLTEN_FLARE                  = 244162,
    SPELL_MOLTEN_FLARE_VISUAL           = 244169,

///Mythic
    SPELL_DARK_RECONSTITUTION           = 249113,
    SPELL_SOULTOUCHED                   = 251444,
    SPELL_SOULTOUCHED_FILTER            = 244053,

    SPELL_FLAMETOUCHED                  = 244054,
    SPELL_BURNING_FLASH                 = 245021,
    SPELL_BURNING_REMNANT_AT            = 245023,

    SPELL_SHADOWTOUCHED                 = 244055,
    SPELL_SHADOWSCAR_FILTER             = 245149,
    SPELL_SHADOWSCAR_DMG                = 245100,
    SPELL_SHADOWSCAR_JUMP               = 245151,
    SPELL_CONSUMING_DETONATION_AT       = 251366,
};

enum eEvents
{
    //Shatug
    EVENT_CORRUPTING                    = 1,
    EVENT_CORRUPTING_DMG                = 2,
    EVENT_CONSUMING_SPHERE              = 3,
    EVENT_SIPHON_CORRUPTION             = 4,
    EVENT_WEIGHT_OF_DARKNESS            = 5,

    //Fharg
    EVENT_BURNING_MAW                   = 1,
    EVENT_BURNING_MAW_DMG               = 2,
    EVENT_DESOLATE_PATH                 = 3,
    EVENT_ENFLAME_CORRUPTION            = 4,
    EVENT_MOLTEN_TOUCH                  = 5,
};

struct boss_felhounds_encounters : public BossAI
{
    boss_felhounds_encounters(Creature* creature) : BossAI(creature, DATA_FELHOUNDS)
    {
        otherEntry = me->GetEntry() == NPC_SHATUG ? NPC_FHARG : NPC_SHATUG;
    }

    std::list<ObjectGuid> GroupA;
    std::list<ObjectGuid> GroupB;
    bool darkRes = false;
    bool swapTouched = false;
    uint32 otherEntry = 0;
    uint32 checkCombatTimer = 0;

    void Reset() override
    {
        _Reset();
        me->RemoveAllAuras();
        me->SetPower(POWER_ENERGY, 0);
        me->SetReactState(REACT_AGGRESSIVE);
        darkRes = false;
        swapTouched = false;
        checkCombatTimer = 0;
        GroupA.clear();
        GroupB.clear();
    }

    void EnterCombat(Unit* who) override
    {
        _EnterCombat();
        if (!IsMythicRaid())
            DoCast(me, SPELL_DESTROYER_BOON, true);
        DoCast(me, SPELL_DESTROYER_BOON_ALTER, true);
        DoCast(me, SPELL_FOCUSING_POWER, true);
        checkCombatTimer = 100;
        
        if (IsMythicRaid() && me->GetEntry() == NPC_SHATUG)
        {
            me->AddDelayedCombat(1000, [this]() -> void { CreateTouchedList(); });
        }
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
        me->NearTeleportTo(me->GetHomePosition());
        RemoveDebuff();

        if (auto other = instance->instance->GetCreature(instance->GetGuidData(otherEntry)))
        {
            other->AddDelayedEvent(100, [other]() -> void
            {
                if (other && other->isInCombat())
                    other->AI()->EnterEvadeMode();
            });
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        RemoveDebuff();

        if (me->GetEntry() == NPC_SHATUG)
        {
            instance->instance->ApplyOnEveryPlayer([&](Player* player)
            {
                player->CastSpell(player, SPELL_DAILY_ESSENCE_FELHOUNDS, true);
            });
        }
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (!IsMythicRaid())
            return;

        if (me->HealthBelowPct(51) && !swapTouched)
        {
            if (auto shatug = instance->instance->GetCreature(instance->GetGuidData(NPC_SHATUG)))
                shatug->GetAI()->DoAction(ACTION_4);
        }

        if (damage >= me->GetHealth())
        {
            if (auto other = instance->instance->GetCreature(instance->GetGuidData(otherEntry)))
            {
                if (other->GetHealth() > 1)
                {
                    damage = 0;

                    if (!darkRes)
                    {
                        darkRes = true;
                        me->SetHealth(1);
                        me->StopAttack(true);
                        me->CastSpell(me, SPELL_DARK_RECONSTITUTION);
                    }
                }
                else
                    other->Kill(other);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (checkCombatTimer)
        {
            if (checkCombatTimer <= diff)
            {
                checkCombatTimer = 1000;

                if (auto other = instance->instance->GetCreature(instance->GetGuidData(otherEntry)))
                {
                    if (!me->IsInEvadeMode() && me->isInCombat() && !other->isInCombat() && !other->IsInEvadeMode())
                        other->AI()->DoZoneInCombat(other, 100.0f);
                }
            }
            else
                checkCombatTimer -= diff;
        }
    }

    void CreateTouchedList()
    {
        std::list<ObjectGuid> tempHealers[2];
        std::list<ObjectGuid> tempDamagers[2];
        std::list<ObjectGuid> tempTanks[2];

        std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
        Trinity::Containers::RandomResizeList(threatList, threatList.size());
        for (auto ref : threatList)
        {
            if (auto player = Player::GetPlayer(*me, ref->getUnitGuid()))
            {
                if (player->GetSpecializationRole() == ROLES_HEALER)
                {
                    if (tempHealers[0] > tempHealers[1])
                        tempHealers[1].push_back(player->GetGUID());
                    else
                        tempHealers[0].push_back(player->GetGUID());
                }
                else if (player->GetSpecializationRole() == ROLES_DPS)
                {
                    if (tempDamagers[0] > tempDamagers[1])
                        tempDamagers[1].push_back(player->GetGUID());
                    else
                        tempDamagers[0].push_back(player->GetGUID());
                }
                else if (player->GetSpecializationRole() == ROLES_TANK)
                {
                    if (tempTanks[0] > tempTanks[1])
                        tempTanks[1].push_back(player->GetGUID());
                    else
                        tempTanks[0].push_back(player->GetGUID());
                }
            }
        }

        GroupA.splice(GroupA.end(), tempHealers[0]);
        GroupA.splice(GroupA.end(), tempDamagers[0]);
        GroupA.splice(GroupA.end(), tempTanks[0]);

        GroupB.splice(GroupB.end(), tempHealers[1]);
        GroupB.splice(GroupB.end(), tempDamagers[1]);
        GroupB.splice(GroupB.end(), tempTanks[1]);

        SoulTouched(true);
    }

    void SoulTouched(bool first = false)
    {
        if (!me->isAlive())
            return;

        for (auto guid : GroupA)
        {
            if (auto player = Player::GetPlayer(*me, guid))
                player->CastSpell(player, first ? SPELL_FLAMETOUCHED : SPELL_SHADOWTOUCHED, true);
        }

        for (auto guid : GroupB)
        {
            if (auto player = Player::GetPlayer(*me, guid))
                player->CastSpell(player, first ? SPELL_SHADOWTOUCHED : SPELL_FLAMETOUCHED, true);
        }
    }

    void RemoveDebuff()
    {
        if (me->GetEntry() == NPC_SHATUG)
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DECAY);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SIPHONED_MARK);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOWTOUCHED);
        }
        else
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SMOULDERING);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DESOLATE_PATH_MARK);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ENFLAME_CORRUPTION_MARK);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_TOUCH_FILTER);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_TOUCH_PERIODIC_DUMMY);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MOLTEN_TOUCH_MARK);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FLAMETOUCHED);
        }
    }
};

//122135
struct npc_felhounds_shatug : public boss_felhounds_encounters
{
    npc_felhounds_shatug(Creature* creature) : boss_felhounds_encounters(creature) {}

    void Reset() override
    {
        boss_felhounds_encounters::Reset();
    }

    void EnterCombat(Unit* who) override
    {
        boss_felhounds_encounters::EnterCombat(who);

        me->SetPower(me->getPowerType(), 86);
        events.RescheduleEvent(EVENT_CORRUPTING, 11000);
    }

    void EnterEvadeMode() override
    {
        boss_felhounds_encounters::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        boss_felhounds_encounters::JustDied(nullptr);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        boss_felhounds_encounters::DamageTaken(attacker, damage, dmgType);
    }

    void DoAction(int32 const actionID) override
    {
        if (darkRes)
            return;

        switch (actionID)
        {
            case ACTION_1: //33
                events.RescheduleEvent(EVENT_CONSUMING_SPHERE, 500);
                break;
            case ACTION_2: //100
                events.RescheduleEvent(EVENT_SIPHON_CORRUPTION, 500);
                break;
            case ACTION_3: //66
                if (IsHeroicPlusRaid())
                    events.RescheduleEvent(EVENT_WEIGHT_OF_DARKNESS, 500);
                break;
            case ACTION_4:
                if (!swapTouched)
                {
                    swapTouched = true;
                    me->CastSpell(me, SPELL_SOULTOUCHED, true);
                }
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (me->GetGUID() == target->GetGUID())
            return;

        switch (spell->Id)
        {
            case SPELL_CONSUMING_SPHERE_FILTER:
                me->CastSpell(target, SPELL_CONSUMING_SPHERE_AT, true);
                break;
            case SPELL_WEIGHT_OF_DARKNESS_FILTER:
                me->CastSpell(target, SPELL_WEIGHT_OF_DARKNESS_AT, true);
                break;
        }
    }

    void SpellFinishCast(const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_DARK_RECONSTITUTION)
        {
            darkRes = false;
            me->SetReactState(REACT_AGGRESSIVE);
        }

        if (spell->Id == SPELL_SOULTOUCHED_FILTER)
            SoulTouched();
    }

    void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
    {
        if (apply || !me->isInCombat())
            return;

        switch (spellId)
        {
            case SPELL_SIPHON_CORRUPTION:
                me->SetReactState(REACT_AGGRESSIVE, 500);
                break;
            case SPELL_CORRUPTING_MAW_AURA:
                events.CancelEvent(EVENT_CORRUPTING_DMG);
                break;
        }
    }

    void OnRemoveAuraTarget(Unit* target, uint32 spellId, AuraRemoveMode mode) override
    {
        if (!me->isInCombat() || mode != AURA_REMOVE_BY_EXPIRE)
            return;

        switch (spellId)
        {
            case SPELL_SIPHONED_MARK:
                me->CastSpell(target, SPELL_SIPHONED_DMG, true);
                break;
        }
    }

    void OnAreaTriggerDespawn(uint32 spellId, Position pos, bool duration) override
    {
        if (IsMythicRaid() && spellId == SPELL_CONSUMING_SPHERE_AT)
            me->RemoveAreaObject(SPELL_CONSUMING_DETONATION_AT);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        boss_felhounds_encounters::UpdateAI(diff);
        events.Update(diff);

        if (CheckHomeDistToEvade(diff, 55.0f, -3248.99f, 10396.73f, -155.46f))
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CORRUPTING:
                    DoCast(me, SPELL_CORRUPTING_MAW_AURA, true);
                    events.RescheduleEvent(EVENT_CORRUPTING, 11000);
                    events.RescheduleEvent(EVENT_CORRUPTING_DMG, 2500);
                    break;
                case EVENT_CORRUPTING_DMG:
                    DoCastTopAggro(SPELL_CORRUPTING_MAW_DMG, true);
                    DoCastTopAggro(SPELL_DECAY, true);
                    break;
                case EVENT_CONSUMING_SPHERE:
                    me->ClearSpellTargets(SPELL_CONSUMING_SPHERE_FILTER);
                    DoCast(me, SPELL_CONSUMING_SPHERE_FILTER, true);
                    break;
                case EVENT_SIPHON_CORRUPTION:
                    Talk(SAY_WARN_SIPHON_CORRUPTION);
                    me->StopAttack(true);
                    me->CastSpell(me, SPELL_SIPHON_CORRUPTION);
                    break;
                case EVENT_WEIGHT_OF_DARKNESS:
                    DoCast(me, SPELL_WEIGHT_OF_DARKNESS_FILTER, true);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//122477
struct npc_felhounds_fharg : public boss_felhounds_encounters
{
    npc_felhounds_fharg(Creature* creature) : boss_felhounds_encounters(creature) {}

    void Reset() override
    {
        boss_felhounds_encounters::Reset();
    }

    void EnterCombat(Unit* who) override
    {
        boss_felhounds_encounters::EnterCombat(who);

        DoCast(me, SPELL_FHARG_START_LEAP, true);
        me->SetPower(me->getPowerType(), 62);
        events.RescheduleEvent(EVENT_BURNING_MAW, 9000);
    }

    void EnterEvadeMode() override
    {
        boss_felhounds_encounters::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        boss_felhounds_encounters::JustDied(nullptr);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        boss_felhounds_encounters::DamageTaken(attacker, damage, dmgType);
    }

    void DoAction(int32 const actionID) override
    {
        if (darkRes)
            return;

        switch (actionID)
        {
            case ACTION_1: //33
                events.RescheduleEvent(EVENT_DESOLATE_PATH, 500);
                break;
            case ACTION_2: //100
                events.RescheduleEvent(EVENT_ENFLAME_CORRUPTION, 500);
                break;
            case ACTION_3: //66
                if (IsHeroicPlusRaid())
                    events.RescheduleEvent(EVENT_MOLTEN_TOUCH, 500);
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_DESOLATE_PATH_FILTER:
                if (me->GetGUID() != target->GetGUID())
                {
                    Talk(SAY_WARN_DESOLATE_PATH, target->GetGUID());
                    me->CastSpell(target, SPELL_DESOLATE_PATH_MARK, true);
                }
                break;
        }
    }

    void SpellFinishCast(const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_DESOLATE_PATH_CAST)
            me->CastSpell(me, SPELL_DESOLATE_PATH_VISUAL, true);

        if (spell->Id == SPELL_DARK_RECONSTITUTION)
        {
            darkRes = false;
            me->SetReactState(REACT_AGGRESSIVE);
        }
    }

    void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
    {
        if (apply || !me->isInCombat())
            return;

        switch (spellId)
        {
            case SPELL_DESOLATE_PATH_CHANNEL:
                me->CastSpell(me, SPELL_DESOLATE_PATH_CAST);
                break;
            case SPELL_ENFLAME_CORRUPTION:
                me->SetReactState(REACT_AGGRESSIVE, 500);
                break;
            case SPELL_BURNING_MAW_AURA:
                events.CancelEvent(EVENT_BURNING_MAW_DMG);
                break;
        }
    }

    void OnRemoveAuraTarget(Unit* target, uint32 spellId, AuraRemoveMode mode) override
    {
        if (!me->isInCombat() || mode != AURA_REMOVE_BY_EXPIRE)
            return;

        switch (spellId)
        {
            case SPELL_DESOLATE_PATH_MARK:
            {
                Position pos;
                me->SimplePosXYRelocationByAngle(pos, 90.0f, me->GetRelativeAngle(target));
                me->CastSpell(pos, SPELL_DESOLATE_PATH_VISUAL_CHARGE, true);
                me->CastSpell(pos, SPELL_DESOLATE_PATH_AT, true);
                break;
            }
            case SPELL_ENFLAME_CORRUPTION_MARK:
                if (me->GetGUID() != target->GetGUID())
                    me->CastSpell(target, SPELL_ENFLAME_CORRUPTION_DMG, true);
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        boss_felhounds_encounters::UpdateAI(diff);
        events.Update(diff);

        if (CheckHomeDistToEvade(diff, 55.0f, -3248.99f, 10396.73f, -155.46f))
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BURNING_MAW:
                    DoCast(me, SPELL_BURNING_MAW_AURA, true);
                    events.RescheduleEvent(EVENT_BURNING_MAW, 11000);
                    events.RescheduleEvent(EVENT_BURNING_MAW_DMG, 2500);
                    break;
                case EVENT_BURNING_MAW_DMG:
                    DoCastTopAggro(SPELL_SMOULDERING, true);
                    DoCastTopAggro(SPELL_BURNING_MAW_DMG, true);
                    break;
                case EVENT_DESOLATE_PATH:
                    DoCast(me, SPELL_DESOLATE_PATH_CHANNEL, true);
                    DoCast(me, SPELL_DESOLATE_PATH_FILTER, true);
                    break;
                case EVENT_ENFLAME_CORRUPTION:
                    Talk(SAY_WARN_ENFLAME_CORRUPTION);
                    me->StopAttack(true);
                    me->CastSpell(me, SPELL_ENFLAME_CORRUPTION);
                    break;
                case EVENT_MOLTEN_TOUCH:
                {
                    DoCast(me, SPELL_MOLTEN_TOUCH_FILTER, true);

                    if (auto shatug = instance->instance->GetCreature(instance->GetGuidData(NPC_SHATUG)))
                    {
                        Position pos;
                        me->GetNearPosition(pos, 10.0f, me->GetRelativeAngle(shatug));
                        if (auto summon = me->SummonCreature(NPC_MOLTEN_TOUCH, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10.0f, pos.GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 15 * IN_MILLISECONDS))
                        {
                            me->AddDelayedCombat(1800, [this, summon]() -> void
                            {
                                if (summon)
                                    me->CastSpell(summon, SPELL_MOLTEN_TOUCH_CHANNEL, true);
                            });
                        }
                    }
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
    }
};

//244050
class spell_felhounds_destroyers_boon_energy_type : public SpellScript
{
    PrepareSpellScript(spell_felhounds_destroyers_boon_energy_type);

    void HandleCalcEffectMask(uint32 & effMask)
    {
        if (GetCaster()->GetEntry() == NPC_FHARG)
            effMask &= ~(1 << EFFECT_1);
        
        if (GetCaster()->GetEntry() == NPC_SHATUG)
            effMask &= ~(1 << EFFECT_2);
    }

    void Register() override
    {
        DoCalcEffMask += CalcEffectMaskFn(spell_felhounds_destroyers_boon_energy_type::HandleCalcEffectMask);
    }
};

//244107
class spell_felhounds_consuming_sphere : public SpellScript
{
    PrepareSpellScript(spell_felhounds_consuming_sphere);

    void HandleBeforeCast()
    {
        if (!GetCaster())
            return;

        WorldLocation pos;
        GetCaster()->GetNearPosition(pos, 50.0f, GetCaster()->GetRelativeAngle(GetExplTargetDest()));
        GetSpell()->destAtTarget = pos;

        if (GetCaster()->GetMap()->IsMythicRaid())
            GetCaster()->CastSpell(pos, SPELL_CONSUMING_DETONATION_AT, true);
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
            GetSpell()->destTarget = &*caster;
    }

    void Register()
    {
        BeforeCast += SpellCastFn(spell_felhounds_consuming_sphere::HandleBeforeCast);
        OnEffectHit += SpellEffectFn(spell_felhounds_consuming_sphere::HandleOnHit, EFFECT_0, SPELL_EFFECT_CREATE_AREATRIGGER);
    }
};

//244441
class spell_felhounds_desolate_path : public SpellScript
{
    PrepareSpellScript(spell_felhounds_desolate_path);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (auto caster = GetCaster())
        {
            if (caster->getVictim())
                targets.remove(caster->getVictim());

            if (caster->GetMap()->IsMythicRaid())
                Trinity::Containers::RandomResizeList(targets, 5);
            else if (caster->GetMap()->GetPlayersCountExceptGMs() < 14)
                Trinity::Containers::RandomResizeList(targets, 3);
            else if (caster->GetMap()->GetPlayersCountExceptGMs() < 24)
                Trinity::Containers::RandomResizeList(targets, 4);
            else
                Trinity::Containers::RandomResizeList(targets, 5);
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_desolate_path::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//244069
class spell_felhounds_weight_of_darkness : public SpellScript
{
    PrepareSpellScript(spell_felhounds_weight_of_darkness);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (auto caster = GetCaster())
        {
            if (caster->GetMap()->IsMythicRaid() || caster->GetMap()->GetPlayersCountExceptGMs() > 15)
                Trinity::Containers::RandomResizeList(targets, 5);
            else
                Trinity::Containers::RandomResizeList(targets, 3);
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_weight_of_darkness::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//244071
class spell_felhounds_weight_of_darkness_fear_filter : public SpellScript
{
    PrepareSpellScript(spell_felhounds_weight_of_darkness_fear_filter);

    uint8 targetCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.size() >= 3)
            targets.clear();
        else
            targetCount = targets.size();
    }

    void HandleAfterHit()
    {
        if (!GetHitUnit())
            return;

        if (auto aura = GetHitUnit()->GetAura(SPELL_WEIGHT_OF_DARKNESS_FEAR))
            aura->SetDuration(aura->GetDuration() / targetCount);
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_weight_of_darkness_fear_filter::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_weight_of_darkness_fear_filter::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_weight_of_darkness_fear_filter::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
        AfterHit += SpellHitFn(spell_felhounds_weight_of_darkness_fear_filter::HandleAfterHit);
    }
};

//244471, 244578
class spell_felhounds_corruption_filter : public SpellScript
{
    PrepareSpellScript(spell_felhounds_corruption_filter);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (auto caster = GetCaster())
        {
            targets.remove_if([this](WorldObject* object) -> bool
            {
                if (object == nullptr)
                    return true;

                if (object->ToPlayer()->HasAura(SPELL_ENFLAME_CORRUPTION_MARK) || object->ToPlayer()->HasAura(SPELL_SIPHONED_MARK))
                    return true;

                return false;
            });

            if (caster->GetMap()->IsMythicRaid() || caster->GetMap()->GetPlayersCountExceptGMs() > 20)
                Trinity::Containers::RandomResizeList(targets, 3);
            else
                Trinity::Containers::RandomResizeList(targets, 2);
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_corruption_filter::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//244072
class spell_felhounds_molten_touch_filter : public SpellScript
{
    PrepareSpellScript(spell_felhounds_molten_touch_filter);

    std::list<WorldObject*> saveTargets;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        std::list<WorldObject*> tempHealers;
        std::list<WorldObject*> tempDamagers;

        if (auto caster = GetCaster())
        {
            if (!saveTargets.empty())
            {
                targets = saveTargets;
                return;
            }

            for (auto object : targets)
            {
                if (object->ToPlayer()->HasAura(SPELL_MOLTEN_TOUCH_MARK))
                    continue;

                if (object->ToPlayer()->GetSpecializationRole() == ROLES_HEALER)
                    tempHealers.push_back(object);
                else if (object->ToPlayer()->GetSpecializationRole() == ROLES_DPS)
                    tempDamagers.push_back(object);
            }

            uint8 DDCountMax = 1;
            if (caster->GetMap()->IsMythicRaid() || caster->GetMap()->GetPlayersCountExceptGMs() > 20)
                DDCountMax = 2;

            Trinity::Containers::RandomResizeList(tempHealers, 1);
            Trinity::Containers::RandomResizeList(tempDamagers, DDCountMax);

            targets.clear();
            targets.splice(targets.end(), tempHealers);
            targets.splice(targets.end(), tempDamagers);

            saveTargets = targets;
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_molten_touch_filter::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_molten_touch_filter::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_molten_touch_filter::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//245149
class spell_felhounds_shadowscar_filter : public SpellScript
{
    PrepareSpellScript(spell_felhounds_shadowscar_filter);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([this](WorldObject* object) -> bool
        {
            if (object == nullptr || !object->ToPlayer()->HasAura(SPELL_SHADOWTOUCHED))
                return true;

            return false;
        });

        if (targets.size() > 1)
        {
            targets.sort(Trinity::UnitSortDistance(true, GetCaster()));
            targets.resize(1);
        }
    }

    void HandleDummy(SpellEffIndex /*effectIndex*/)
    {
        if (GetCaster() && GetHitUnit())
        {
            GetHitUnit()->CastSpell(GetCaster(), SPELL_SHADOWSCAR_JUMP, true);
            GetCaster()->CastSpell(GetHitUnit(), SPELL_SHADOWSCAR_DMG, true);
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_shadowscar_filter::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_felhounds_shadowscar_filter::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

//244130
class spell_felhounds_consuming_detonation : public SpellScript
{
    PrepareSpellScript(spell_felhounds_consuming_detonation);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        auto target = GetHitUnit();
        if (!caster || !target)
            return;

        float distance = caster->GetExactDist2d(target);
        uint8 pct = 10;
        if (distance < 100.0f)
            pct = 100 - distance;

        SetHitDamage(CalculatePct(GetHitDamage(), pct));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_felhounds_consuming_detonation::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

//244159
class spell_felhounds_consuming_sphere_filter : public SpellScript
{
    PrepareSpellScript(spell_felhounds_consuming_sphere_filter);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (!GetCaster())
            return;

        for (auto object : targets)
            GetCaster()->AddSpellTargets(SPELL_CONSUMING_SPHERE_FILTER, object->GetGUID());

        if (auto instance = GetCaster()->GetInstanceScript())
        {
            if (auto fharg = instance->instance->GetCreature(instance->GetGuidData(NPC_FHARG)))
            {
                if (auto player = fharg->FindNearestPlayer(30.0f, true))
                {
                    targets.clear();
                    targets.push_back(player);
                }
                else
                    Trinity::Containers::RandomResizeList(targets, 1);
            }
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_felhounds_consuming_sphere_filter::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//244054, 244055
class spell_felhounds_touched_proc : public AuraScript
{
    PrepareAuraScript(spell_felhounds_touched_proc);

    void OnProc(AuraEffect const* auraEffect, ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetDamageInfo()->GetSpellInfo();
        if (!spellInfo || !GetTarget() || eventInfo.GetActor()->GetGUID() == GetTarget()->GetGUID())
            return;

        if (GetId() == SPELL_SHADOWTOUCHED)
        {
            if (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE)
                GetTarget()->CastSpell(GetTarget(), SPELL_SHADOWSCAR_FILTER, true);
        }
        else if (GetId() == SPELL_FLAMETOUCHED)
        {
            if (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW)
            {
                if (auto instance = GetCaster()->GetInstanceScript())
                    if (auto shatug = instance->instance->GetCreature(instance->GetGuidData(NPC_SHATUG)))
                    {
                        shatug->CastSpell(GetTarget(), SPELL_BURNING_FLASH, true);
                    }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_felhounds_touched_proc::OnProc, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    }
};

//244072
class spell_felhounds_molten_touch : public AuraScript
{
    PrepareAuraScript(spell_felhounds_molten_touch);

    uint32 updateTeleportTimer = 0;
    uint32 updateTimer = 0;

    void OnApply(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (!GetCaster() || !GetTarget())
            return;

        updateTeleportTimer = 1600;
        updateTimer = 2000;

        if (auto player = GetTarget()->ToPlayer())
        {
            player->SetControlled(true, UNIT_STATE_STUNNED);
            player->GetMotionMaster()->MoveSmoothFlyPath(1, Position(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ() + 10.0f), 5.0f);
            player->SetDisableGravity(true);
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget())
        {
            GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);
            GetTarget()->SetDisableGravity(false);
        }
    }

    void OnUpdate(uint32 diff, AuraEffect* auraEffect)
    {
        if (updateTeleportTimer)
        {
            if (updateTeleportTimer <= diff)
            {
                if (auto target = GetUnitOwner())
                {
                    if (auto instance = GetCaster()->GetInstanceScript())
                        if (auto trig = instance->instance->GetCreature(instance->GetGuidData(NPC_MOLTEN_TOUCH)))
                        {
                            target->CastSpell(trig->GetPositionX() + frand(-1.5f, 1.5f), trig->GetPositionY() + frand(-1.5f, 1.5f), trig->GetPositionZ(), SPELL_MOLTEN_TOUCH_TELEPORT, true);
                        }
                }
                updateTeleportTimer = 0;
            }
            else
                updateTeleportTimer -= diff;
        }

        if (updateTimer)
        {
            if (updateTimer <= diff)
            {
                if (auto target = GetUnitOwner())
                {
                    target->CastSpell(target, SPELL_MOLTEN_TOUCH_STUN, true, nullptr, nullptr, GetCaster()->GetGUID());
                    GetCaster()->CastSpell(target, SPELL_MOLTEN_TOUCH_PERIODIC_DUMMY, true);
                    GetCaster()->CastSpell(target, SPELL_MOLTEN_TOUCH_MARK, true);
                }
                updateTimer = 0;
            }
            else
                updateTimer -= diff;
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_felhounds_molten_touch::OnApply, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_felhounds_molten_touch::OnRemove, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE, AURA_EFFECT_HANDLE_REAL);
        OnEffectUpdate += AuraEffectUpdateFn(spell_felhounds_molten_touch::OnUpdate, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE);
    }
};

//244086
class spell_felhounds_molten_touch_periodic : public AuraScript
{
    PrepareAuraScript(spell_felhounds_molten_touch_periodic);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (GetCaster() && GetUnitOwner())
        {
            GetCaster()->CastSpell(GetUnitOwner(), SPELL_MOLTEN_TOUCH_SINGED, true);
            GetCaster()->CastSpell(GetUnitOwner(), SPELL_MOLTEN_TOUCH_VISUAL, true);
            GetCaster()->CastSpell(GetUnitOwner(), SPELL_MOLTEN_TOUCH_VISUAL_2, true);
            
            if (auto target = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 90.0f, true, -SPELL_MOLTEN_TOUCH_PERIODIC_DUMMY))
            {
                GetCaster()->CastSpell(target, SPELL_MOLTEN_FLARE, true);
                GetUnitOwner()->CastSpell(target, SPELL_MOLTEN_FLARE_VISUAL, true);
            }
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget())
            GetTarget()->RemoveAurasDueToSpell(SPELL_MOLTEN_TOUCH_FILTER);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_felhounds_molten_touch_periodic::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_felhounds_molten_touch_periodic::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

//254429
class spell_felhounds_weight_of_darkness_periodic : public AuraScript
{
    PrepareAuraScript(spell_felhounds_weight_of_darkness_periodic);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (GetUnitOwner())
            GetUnitOwner()->CastSpell(GetUnitOwner(), SPELL_WEIGHT_OF_DARKNESS_MOD_SPEED, true);
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetCaster() && GetTarget())
            GetCaster()->CastSpell(GetTarget(), SPELL_WEIGHT_OF_DARKNESS_FEAR, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_felhounds_weight_of_darkness_periodic::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_felhounds_weight_of_darkness_periodic::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

//244050
class spell_felhounds_destroyers_boon_energize : public AuraScript
{
    PrepareAuraScript(spell_felhounds_destroyers_boon_energize);

    bool specialAbility_33 = true;
    bool specialAbility_66 = false;
    uint8 tickCount = 0;
    uint8 dynMaxCount = 4;
    uint8 powerCount = 0;

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        if (!caster->IsCreature() || GetCaster()->HasAura(SPELL_FOCUSING_POWER))
            return;

        if (auto instance = caster->GetInstanceScript())
        {
            if (auto otherBoss = instance->instance->GetCreature(instance->GetGuidData(caster->GetEntry() == NPC_SHATUG ? NPC_FHARG : NPC_SHATUG)))
            {
                if (GetCaster()->GetExactDist(otherBoss) <= 40.0f)
                {
                    if (!GetCaster()->HasAura(SPELL_SARGERAS_BLESSING))
                        GetCaster()->CastSpell(GetCaster(), SPELL_SARGERAS_BLESSING, true);

                    if (!otherBoss->HasAura(SPELL_SARGERAS_BLESSING))
                        otherBoss->CastSpell(otherBoss, SPELL_SARGERAS_BLESSING, true);
                }
                else
                {
                    GetCaster()->RemoveAurasDueToSpell(SPELL_SARGERAS_BLESSING);
                    otherBoss->RemoveAurasDueToSpell(SPELL_SARGERAS_BLESSING);
                }
            }

            powerCount = caster->GetPower(caster->getPowerType());

            if (caster->GetEntry() == NPC_SHATUG)
            {
                if (tickCount++ == dynMaxCount)
                {
                    tickCount = 0;
                    powerCount += 1;
                    dynMaxCount = dynMaxCount == 4 ? 5 : 4;
                }
            }

            if (powerCount < 100)
            {
                caster->SetPower(caster->getPowerType(), ++powerCount);
            
                if (!specialAbility_33 && powerCount >= 33)
                {
                    specialAbility_33 = true;
                    caster->GetAI()->DoAction(ACTION_1);
                }

                if (!specialAbility_66 && powerCount >= 66 && powerCount <= 69)
                {
                    specialAbility_66 = true;
                    caster->GetAI()->DoAction(ACTION_3);
                }
            }
            else
            {
                caster->SetPower(caster->getPowerType(), 0);
                caster->GetAI()->DoAction(ACTION_2);
                specialAbility_33 = false;
                specialAbility_66 = false;
                dynMaxCount = dynMaxCount == 4 ? 5 : 4;
            }
        }
    }

    void OnApply(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        const_cast<AuraEffect*>(auraEffect)->SetAmount(0);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_felhounds_destroyers_boon_energize::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectApply += AuraEffectApplyFn(spell_felhounds_destroyers_boon_energize::OnApply, EFFECT_3, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

//246057
class spell_felhounds_sargeras_blessing : public AuraScript
{
    PrepareAuraScript(spell_felhounds_sargeras_blessing);

    void OnApply(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (!GetCaster())
            return;

        // Mod DMG apply
        if (auto aurEff = GetCaster()->GetAuraEffect(SPELL_DESTROYER_BOON_ALTER, EFFECT_3))
        {
            if (aurEff->GetAmount() == 0)
                aurEff->ChangeAmount(100);
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (!GetCaster())
            return;

        // Mod DMG Remove
        if (auto aurEff = GetCaster()->GetAuraEffect(SPELL_DESTROYER_BOON_ALTER, EFFECT_3))
        {
            if (aurEff->GetAmount() == 100)
                aurEff->ChangeAmount(0);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_felhounds_sargeras_blessing::OnApply, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_felhounds_sargeras_blessing::OnRemove, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
    }
};

//245098, 251445
class spell_felhounds_decay_or_smouldering : public AuraScript
{
    PrepareAuraScript(spell_felhounds_decay_or_smouldering);

    void OnTick(AuraEffect const* aurEff)
    {
        if (GetCaster() && GetUnitOwner())
        {
            float damage = GetSpellInfo()->GetEffect(EFFECT_1, GetCaster()->GetMap()->GetDifficultyID())->BasePoints * aurEff->GetBase()->GetStackAmount();
            GetCaster()->CastCustomSpell(GetUnitOwner(), GetId() == SPELL_DECAY ? SPELL_DECAY_DMG : SPELL_SMOULDERING_DMG, &damage, nullptr, nullptr, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_felhounds_decay_or_smouldering::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//Entry: 10580 (Custom: 15424)
struct at_felhounds_weight_of_darkness : AreaTriggerAI
{
    at_felhounds_weight_of_darkness(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    uint32 scaleTimer = 200;

    void OnUpdate(uint32 diff) override
    {
        if (scaleTimer)
        {
            if (scaleTimer <= diff)
            {
                if (at->GetRadius() >= 8.0f)
                {
                    scaleTimer = 0;
                    return;
                }
                else
                    scaleTimer = 200;

                at->SetSphereScale(0.1f, 200, false);
            }
            else
                scaleTimer -= diff;
        }
    }
};

//Entry: 10586, 10661 (Custom: 15431)
struct at_felhounds_consuming_sphere : AreaTriggerAI
{
    at_felhounds_consuming_sphere(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    std::vector<AreaTrigger*> windAt;
    std::vector<AreaTrigger*> windAt2;
    uint32 tempId = 0;

    void OnCreate() override
    {
        auto spline = at->GetSplineInfo();
        if (spline.VerticesPoints.empty())
            return;

        windAt.resize(spline.VerticesPoints.size(), nullptr);
        windAt2.resize(spline.VerticesPoints.size(), nullptr);
        Position pos;

        for (uint8 i = 0; i < windAt.size(); ++i)
        {
            pos = { spline.VerticesPoints[i].x, spline.VerticesPoints[i].y, spline.VerticesPoints[i].z };
            auto areaTrigger = new AreaTrigger;
            if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, at->GetCaster(), nullptr, pos, pos, nullptr, ObjectGuid::Empty, 15434))
            {
                delete areaTrigger;
                continue;
            }
            windAt[i] = areaTrigger;
        }

        for (uint8 i = 0; i < windAt2.size(); ++i)
        {
            pos = { spline.VerticesPoints[i].x, spline.VerticesPoints[i].y, spline.VerticesPoints[i].z };
            auto areaTrigger = new AreaTrigger;
            if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, at->GetCaster(), nullptr, pos, pos, nullptr, ObjectGuid::Empty, 15441))
            {
                delete areaTrigger;
                continue;
            }
            windAt2[i] = areaTrigger;
        }
    }

    void OnRemove() override
    {
        ForceUpdate(0, 0, true);
    }

    void OnChangeSplineId(uint32 splineIdNew, uint32 splineIdOld) override
    {
        tempId = splineIdNew;
        ForceUpdate(splineIdNew, splineIdOld);
    }

    void ForceUpdate(uint32 newId, uint32 oldId, bool despawn = false)
    {
        auto caster = at->GetCaster();
        if (!caster)
            return;

        if (caster->m_spell_targets[SPELL_CONSUMING_SPHERE_FILTER].empty())
            return;

        for (auto guid : caster->m_spell_targets[SPELL_CONSUMING_SPHERE_FILTER])
        {
            if (auto player = Player::GetPlayer(*caster, guid))
            {
                if (player->GetDistance(at) < 100.0f)
                {
                    player->SendMovementForce(windAt[despawn ? tempId : oldId]); //Disable ForceMove
                    player->SendMovementForce(windAt2[despawn ? tempId : oldId]); //Disable ForceMove

                    if (player->isAlive() && !despawn)
                    {
                        if (windAt[newId])
                            player->SendMovementForce(windAt[newId], *windAt[newId], 3.0f, 1, true);

                        if (windAt2[newId] && player->GetDistance(*windAt2[newId]) <= 8.0f)
                            player->SendMovementForce(windAt2[newId], *windAt2[newId], 3.0f, 1, true);
                    }
                }
            }
        }
    }
};

void AddSC_boss_felhounds()
{
    RegisterCreatureAI(npc_felhounds_shatug);
    RegisterCreatureAI(npc_felhounds_fharg);
    RegisterSpellScript(spell_felhounds_destroyers_boon_energy_type);
    RegisterSpellScript(spell_felhounds_consuming_sphere);
    RegisterSpellScript(spell_felhounds_desolate_path);
    RegisterSpellScript(spell_felhounds_weight_of_darkness);
    RegisterSpellScript(spell_felhounds_weight_of_darkness_fear_filter);
    RegisterSpellScript(spell_felhounds_corruption_filter);
    RegisterSpellScript(spell_felhounds_molten_touch_filter);
    RegisterSpellScript(spell_felhounds_shadowscar_filter);
    RegisterSpellScript(spell_felhounds_consuming_detonation);
    RegisterSpellScript(spell_felhounds_consuming_sphere_filter);
    RegisterAuraScript(spell_felhounds_touched_proc);
    RegisterAuraScript(spell_felhounds_molten_touch);
    RegisterAuraScript(spell_felhounds_molten_touch_periodic);
    RegisterAuraScript(spell_felhounds_weight_of_darkness_periodic);
    RegisterAuraScript(spell_felhounds_destroyers_boon_energize);
    RegisterAuraScript(spell_felhounds_sargeras_blessing);
    RegisterAuraScript(spell_felhounds_decay_or_smouldering);
    RegisterAreaTriggerAI(at_felhounds_weight_of_darkness);
    RegisterAreaTriggerAI(at_felhounds_consuming_sphere);
}
