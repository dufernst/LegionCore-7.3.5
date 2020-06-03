#include "zulaman.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_SIPHON  = 2,
    SAY_BOLTS   = 3,
    SAY_PET     = 4,
    SAY_DEATH   = 5,
};

enum Spells
{
    SPELL_SPIRIT_BOLTS              = 43383,
    SPELL_DRAIN_POWER               = 44131,
    SPELL_SIPHON_SOUL               = 43501,

    
    SPELL_FLASH_HEAL                = 43575,
    SPELL_DISPEL_MAGIC              = 43577,

    SPELL_FIREBOLT                  = 43584,

    SPELL_PSYCHIC_WAIL              = 43590,

    SPELL_VENOM_SPIT                = 43579,

    // Druid
    SPELL_DR_THORNS                 = 43420,
    SPELL_DR_LIFEBLOOM              = 43421,
    SPELL_DR_MOONFIRE               = 43545,

    // Hunter
    SPELL_HU_EXPLOSIVE_TRAP         = 43444,
    SPELL_HU_FREEZING_TRAP          = 43447,
    SPELL_HU_SNAKE_TRAP             = 43449,

    // Mage
    SPELL_MG_FIREBALL               = 41383,
    SPELL_MG_FROST_NOVA             = 43426,
    SPELL_MG_ICE_LANCE              = 43427,
    SPELL_MG_FROSTBOLT              = 43428,

    // Paladin
    SPELL_PA_CONSECRATION           = 43429,
    SPELL_PA_AVENGING_WRATH         = 43430,
    SPELL_PA_HOLY_LIGHT             = 43451,

    // Priest
    SPELL_PR_HEAL                   = 41372,
    SPELL_PR_MIND_BLAST             = 41374,
    SPELL_PR_SW_DEATH               = 41375,
    SPELL_PR_PSYCHIC_SCREAM         = 43432,
    SPELL_PR_MIND_CONTROL           = 43550,
    SPELL_PR_PAIN_SUPP              = 44416,

    // Rogue
    SPELL_RO_BLIND                  = 43433,
    SPELL_RO_SLICE_DICE             = 43457,
    SPELL_RO_WOUND_POISON           = 43461,

    // Shaman
    SPELL_SH_CHAIN_LIGHT            = 43435,
    SPELL_SH_FIRE_NOVA              = 43436,
    SPELL_SH_HEALING_WAVE           = 43548,

    // Warlock
    SPELL_WL_CURSE_OF_DOOM          = 43439,
    SPELL_WL_RAIN_OF_FIRE           = 43440,
    SPELL_WL_UNSTABLE_AFFL          = 43522,
    SPELL_WL_UNSTABLE_AFFL_DISPEL   = 43523,

    // Warrior
    SPELL_WR_MORTAL_STRIKE          = 43441,
    SPELL_WR_WHIRLWIND              = 43442,
    SPELL_WR_SPELL_REFLECT          = 43443
};

enum Events
{
    EVENT_SPIRIT_BOLTS          = 1,
    EVENT_SIPHON_SOUL           = 2,
    EVENT_DRAIN_POWER           = 3,
    EVENT_PLAYER_ABILITY        = 4,
    EVENT_PLAYER_ABILITY_OFF    = 5,
};

enum Adds
{
    NPC_ALYSON_ANTILE   = 24240,
    NPC_SLITHER         = 24242, 
    NPC_GAZAKROTH       = 24244, 
    NPC_DARKHEART       = 24246,
};

const Position addPos[2] = 
{
    {109.43f, 922.57f, 33.90f, 1.57f},
    {126.40f, 922.78f, 33.90f, 1.57f}
};

enum AbilityTarget
{
    ABILITY_TARGET_SELF = 0,
    ABILITY_TARGET_VICTIM = 1,
    ABILITY_TARGET_ENEMY = 2,
    ABILITY_TARGET_HEAL = 3,
    ABILITY_TARGET_BUFF = 4,
    ABILITY_TARGET_SPECIAL = 5
};

struct PlayerAbilityStruct
{
    uint32 spell;
    AbilityTarget target;
    uint32 cooldown;
};

static PlayerAbilityStruct PlayerAbility[][3] =
{
    // 1 warrior
    {{SPELL_WR_SPELL_REFLECT, ABILITY_TARGET_SELF, 10000},
    {SPELL_WR_WHIRLWIND, ABILITY_TARGET_SELF, 10000},
    {SPELL_WR_MORTAL_STRIKE, ABILITY_TARGET_VICTIM, 6000}},
    // 2 paladin
    {{SPELL_PA_CONSECRATION, ABILITY_TARGET_SELF, 10000},
    {SPELL_PA_HOLY_LIGHT, ABILITY_TARGET_HEAL, 10000},
    {SPELL_PA_AVENGING_WRATH, ABILITY_TARGET_SELF, 10000}},
    // 3 hunter
    {{SPELL_HU_EXPLOSIVE_TRAP, ABILITY_TARGET_SELF, 10000},
    {SPELL_HU_FREEZING_TRAP, ABILITY_TARGET_SELF, 10000},
    {SPELL_HU_SNAKE_TRAP, ABILITY_TARGET_SELF, 10000}},
    // 4 rogue
    {{SPELL_RO_WOUND_POISON, ABILITY_TARGET_VICTIM, 3000},
    {SPELL_RO_SLICE_DICE, ABILITY_TARGET_SELF, 10000},
    {SPELL_RO_BLIND, ABILITY_TARGET_ENEMY, 10000}},
    // 5 priest
    {{SPELL_PR_PAIN_SUPP, ABILITY_TARGET_HEAL, 10000},
    {SPELL_PR_HEAL, ABILITY_TARGET_HEAL, 10000},
    {SPELL_PR_PSYCHIC_SCREAM, ABILITY_TARGET_SELF, 10000}},
    // 5* shadow priest
    {{SPELL_PR_MIND_CONTROL, ABILITY_TARGET_ENEMY, 15000},
    {SPELL_PR_MIND_BLAST, ABILITY_TARGET_ENEMY, 5000},
    {SPELL_PR_SW_DEATH, ABILITY_TARGET_ENEMY, 10000}},
    // 7 shaman
    {{SPELL_SH_FIRE_NOVA, ABILITY_TARGET_SELF, 10000},
    {SPELL_SH_HEALING_WAVE, ABILITY_TARGET_HEAL, 10000},
    {SPELL_SH_CHAIN_LIGHT, ABILITY_TARGET_ENEMY, 8000}},
    // 8 mage
    {{SPELL_MG_FIREBALL, ABILITY_TARGET_ENEMY, 5000},
    {SPELL_MG_FROSTBOLT, ABILITY_TARGET_ENEMY, 5000},
    {SPELL_MG_ICE_LANCE, ABILITY_TARGET_SPECIAL, 2000}},
    // 9 warlock
    {{SPELL_WL_CURSE_OF_DOOM, ABILITY_TARGET_ENEMY, 10000},
    {SPELL_WL_RAIN_OF_FIRE, ABILITY_TARGET_ENEMY, 10000},
    {SPELL_WL_UNSTABLE_AFFL, ABILITY_TARGET_ENEMY, 10000}},
    // 11 druid
    {{SPELL_DR_LIFEBLOOM, ABILITY_TARGET_HEAL, 10000},
    {SPELL_DR_THORNS, ABILITY_TARGET_SELF, 10000},
    {SPELL_DR_MOONFIRE, ABILITY_TARGET_ENEMY, 8000}}
};

class boss_hex_lord_malacrass : public CreatureScript
{
    public:

        boss_hex_lord_malacrass() : CreatureScript("boss_hex_lord_malacrass") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<boss_hex_lord_malacrassAI>(pCreature);
        }

        struct boss_hex_lord_malacrassAI : public BossAI
        {
            boss_hex_lord_malacrassAI(Creature* pCreature) : BossAI(pCreature, DATA_HEX_LORD_MALACRASS)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            uint32 PlayerClass;

            void Reset()
            {
                _Reset();

                switch (urand(0, 5))
                {
                    case 0:
                        me->SummonCreature(NPC_ALYSON_ANTILE, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_SLITHER, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 1:
                        me->SummonCreature(NPC_ALYSON_ANTILE, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_GAZAKROTH, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 2:
                        me->SummonCreature(NPC_ALYSON_ANTILE, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_DARKHEART, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 3:
                        me->SummonCreature(NPC_GAZAKROTH, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_SLITHER, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 4:
                        me->SummonCreature(NPC_GAZAKROTH, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_DARKHEART, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 5:
                        me->SummonCreature(NPC_SLITHER, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_DARKHEART, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                }       

                me->SetVirtualItem(0, 46916);
                me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.RescheduleEvent(EVENT_SPIRIT_BOLTS, 10000);
                events.RescheduleEvent(EVENT_DRAIN_POWER, 6000);
                Talk(SAY_AGGRO);
                DoZoneInCombat();
                instance->SetBossState(DATA_HEX_LORD_MALACRASS, IN_PROGRESS);
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_KILL);
            }
            
            void SummonedCreatureDies(Creature* summon, Unit* killer)
            {
                Talk(SAY_PET);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_DRAIN_POWER:
                            DoCast(me, SPELL_DRAIN_POWER, true);
                            events.RescheduleEvent(EVENT_DRAIN_POWER, 40000);
                            break;
                        case EVENT_SPIRIT_BOLTS:
                            Talk(SAY_BOLTS);
                            DoCast(me, SPELL_SPIRIT_BOLTS);
                            events.RescheduleEvent(EVENT_SPIRIT_BOLTS, 45000);
                            events.RescheduleEvent(EVENT_SIPHON_SOUL, 10000);
                            break;
                        case EVENT_SIPHON_SOUL:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                PlayerClass = pTarget->getClass() - 1;
                                if (PlayerClass == CLASS_DRUID-1)
                                    PlayerClass = CLASS_DRUID;
                                else if (PlayerClass == CLASS_PRIEST-1 && pTarget->HasSpell(15473))
                                    PlayerClass = CLASS_PRIEST; // shadow priest

                                DoCast(pTarget, SPELL_SIPHON_SOUL);
                                me->ClearUnitState(UNIT_STATE_CASTING);
                                events.RescheduleEvent(EVENT_PLAYER_ABILITY, urand(5000, 8000));
                                events.RescheduleEvent(EVENT_PLAYER_ABILITY_OFF, 30000);
                            }
                            break;
                        case EVENT_PLAYER_ABILITY:
                            UseAbility();
                            events.RescheduleEvent(EVENT_PLAYER_ABILITY, urand(8000, 10000));
                            break;
                        case EVENT_PLAYER_ABILITY_OFF:
                            events.CancelEvent(EVENT_PLAYER_ABILITY);
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }

            void UseAbility()
            {
                uint8 random = urand(0, 2);
                Unit* target = NULL;
                switch (PlayerAbility[PlayerClass][random].target)
                {
                    case ABILITY_TARGET_SELF:
                        target = me;
                        break;
                    case ABILITY_TARGET_VICTIM:
                        target = me->getVictim();
                        break;
                    case ABILITY_TARGET_ENEMY:
                    default:
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true);
                        break;
                    case ABILITY_TARGET_HEAL:
                        target = DoSelectLowestHpFriendly(50, 0);
                        break;
                    case ABILITY_TARGET_BUFF:
                        {
                            std::list<Creature*> templist = DoFindFriendlyMissingBuff(50, PlayerAbility[PlayerClass][random].spell);
                            if (!templist.empty())
                                target = *(templist.begin());
                        }
                        break;
                }
                if (target)
                    DoCast(target, PlayerAbility[PlayerClass][random].spell, false);
            }
        };
};

class npc_alyson_antille : public CreatureScript
{
    public:

        npc_alyson_antille() : CreatureScript("npc_alyson_antille") {}
         
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_alyson_antilleAI>(pCreature);
        }

        struct npc_alyson_antilleAI : public ScriptedAI
        {
            npc_alyson_antilleAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
            }

            uint32 flashheal_timer;
            uint32 dispelmagic_timer;

            void Reset()
            {
                flashheal_timer = 2500;
                dispelmagic_timer = 10000;
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (flashheal_timer <= diff)
                {
                    Unit* target = DoSelectLowestHpFriendly(99, 30000);
                    if (target)
                    {
                        if (target->IsWithinDistInMap(me, 50))
                            DoCast(target, SPELL_FLASH_HEAL, false);
                        else
                        {
                            // bugged
                            //me->GetMotionMaster()->Clear();
                            //me->GetMotionMaster()->MoveChase(target, 20);
                        }
                    }
                    else
                    {
                        if (urand(0, 1))
                            target = DoSelectLowestHpFriendly(50, 0);
                        else
                            target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                        if (target)
                            DoCast(target, SPELL_DISPEL_MAGIC, false);
                    }
                    flashheal_timer = 2500;
                } else flashheal_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };
};
class npc_gazakroth : public CreatureScript
{
    public:

        npc_gazakroth() : CreatureScript("npc_gazakroth") {}
         
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_gazakrothAI>(pCreature);
        }

        struct npc_gazakrothAI : public ScriptedAI
        {
            npc_gazakrothAI(Creature* pCreature) : ScriptedAI(pCreature)  {}

            uint32 firebolt_timer;

            void Reset()
            {
                firebolt_timer = 2000;
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (firebolt_timer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_FIREBOLT, false);
                    firebolt_timer = 700;
                } else firebolt_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };
};

class npc_darkheart : public CreatureScript
{
    public:

        npc_darkheart() : CreatureScript("npc_darkheart") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_darkheartAI>(pCreature);
        }

        struct npc_darkheartAI : public ScriptedAI
        {
            npc_darkheartAI(Creature* pCreature) : ScriptedAI(pCreature)  {}

            uint32 psychicwail_timer;

            void Reset()
            {
                psychicwail_timer = 8000;
            }
            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (psychicwail_timer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_PSYCHIC_WAIL, false);
                    psychicwail_timer = 12000;
                } else psychicwail_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };
};



class npc_slither : public CreatureScript
{
    public:

        npc_slither() : CreatureScript("npc_slither") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_slitherAI>(pCreature);
        }

        struct npc_slitherAI : public ScriptedAI
        {
            npc_slitherAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            uint32 venomspit_timer;

            void Reset()
            {
                venomspit_timer = 5000;
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (venomspit_timer <= diff)
                {
                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        DoCast(victim, SPELL_VENOM_SPIT, false);
                    venomspit_timer = 2500;
                } else venomspit_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };
};

class spell_hexlord_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_hexlord_unstable_affliction() : SpellScriptLoader("spell_hexlord_unstable_affliction") { }

        class spell_hexlord_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hexlord_unstable_affliction_AuraScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WL_UNSTABLE_AFFL_DISPEL))
                    return false;
                return true;
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(dispelInfo->GetDispeller(), SPELL_WL_UNSTABLE_AFFL_DISPEL, true, NULL, GetEffect(EFFECT_0));
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_hexlord_unstable_affliction_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hexlord_unstable_affliction_AuraScript();
        }
};

void AddSC_boss_hex_lord_malacrass()
{
    new boss_hex_lord_malacrass();
    new npc_gazakroth();
    new npc_darkheart();
    new npc_slither();
    new npc_alyson_antille();
    new spell_hexlord_unstable_affliction();
}

