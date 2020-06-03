
//World boss

enum eSpells
{
    SPELL_ARC_NOVA                  = 136338,
    SPELL_STATIC_SHIELD             = 136341,
    SPELL_STATIC_SHIELD_DMG         = 136343,
    SPELL_LIGHTNING_TETHER          = 136339,
    SPELL_STORMCLOUD                = 136340,

    SPELL_THROW_SPEAR               = 137660,
    SPELL_SUMMON_ESSENCE_OF_STORM   = 137883,
    SPELL_SIPHON_ESSENCE            = 137889,
    SPELL_FIXATE_DEMON_CREATOR      = 130357,
    SPELL_FIXATE_DEMON_CREATOR_2    = 101199,
    SPELL_COMPLETE_QUEST            = 137887,
};

enum eEvents
{
    EVENT_ARC_NOVA           = 1,
    EVENT_LIGHTNING_TETHER   = 2,
    EVENT_STORMCLOUD         = 3,
    EVENT_RE_ATTACK          = 4,
};

//69099
class boss_nalak : public CreatureScript
{
public:
    boss_nalak() : CreatureScript("boss_nalak") { }

    struct boss_nalakAI : public ScriptedAI
    {
        boss_nalakAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
        }

        EventMap events;
        bool attack_ready;

        void Reset() override
        {
            events.Reset();
            me->SetReactState(REACT_DEFENSIVE);
            attack_ready = true;
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    me->SetVisible(true);
                    Talk(1);
                    break;
                default:
                    break;
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_THROW_SPEAR)
            {
                events.RescheduleEvent(EVENT_5, 1 * IN_MILLISECONDS);
                me->SetReactState(REACT_PASSIVE);
                me->SetTarget(caster->GetGUID());
            }
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(me->GetTargetUnit());
            summoned->GetMotionMaster()->MoveChase(me->GetTargetUnit());
            summoned->AddAura(SPELL_SIPHON_ESSENCE, summoned);

        }

        void EnterCombat(Unit* unit) override
        {
            DoZoneInCombat(me, 75.0f);
            DoCast(me, SPELL_STATIC_SHIELD, true);
            events.RescheduleEvent(EVENT_ARC_NOVA,         42000);
          //events.RescheduleEvent(EVENT_LIGHTNING_TETHER, 35000); not work
            events.RescheduleEvent(EVENT_STORMCLOUD,       24000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            /*if (me->getVictim() && attack_ready)
                if (!me->IsWithinMeleeRange(me->getVictim()))
                    me->GetMotionMaster()->MoveCharge(me->getVictim()->GetPositionX(), me->getVictim()->GetPositionY(), me->getVictim()->GetPositionZ() + 18.0f, 15.0f);*/

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ARC_NOVA:
                    attack_ready = false;
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    DoCastAOE(SPELL_ARC_NOVA);
                    events.DelayEvents(3300);
                    events.RescheduleEvent(EVENT_RE_ATTACK, 3000);
                    events.RescheduleEvent(EVENT_ARC_NOVA, 42000);
                    break;
                case EVENT_LIGHTNING_TETHER:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_LIGHTNING_TETHER);
                    events.RescheduleEvent(EVENT_LIGHTNING_TETHER, 35000);
                    break;
                case EVENT_STORMCLOUD:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_STORMCLOUD);
                    events.RescheduleEvent(EVENT_STORMCLOUD,  24000);
                    break;
                case EVENT_RE_ATTACK:
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 75.0f);
                    attack_ready = true;
                    break;
                case EVENT_5:
                    events.RescheduleEvent(EVENT_6, 2 * IN_MILLISECONDS);
                    Talk(2);
                    break;
                case EVENT_6:
                    events.RescheduleEvent(EVENT_7, 2 * IN_MILLISECONDS);
                    Talk(0);
                    //DoCast(SPELL_SUMMON_ESSENCE_OF_STORM);
                    me->SummonCreature(69739, 7112.963f, 5168.82f, 120.6497f, me->GetOrientation());
                    break;
                case EVENT_7:
                    me->SetVisible(false);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_nalakAI(creature);
    }
};

//< 69739
class npc_essence_of_storm : public CreatureScript
{
public:
    npc_essence_of_storm() : CreatureScript("npc_essence_of_storm") { }

    struct npc_essence_of_stormAI : public ScriptedAI
    {
        npc_essence_of_stormAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
        }

        void Reset() override
        {
            if (Player* player = me->GetTargetUnit()->ToPlayer())
            {
                me->AddAura(SPELL_SIPHON_ESSENCE, me);
                me->AddAura(SPELL_FIXATE_DEMON_CREATOR_2, player);
            }
        }

        void EnterCombat(Unit* /*unit*/) override
        { }

        void JustDied(Unit* /*killer*/) override
        {
            if (Creature* nalak = me->FindNearestCreature(69099, 150.0f))
                nalak->AI()->DoAction(ACTION_1);

            if (Player* player = me->GetTargetUnit()->ToPlayer())
                me->CastSpell(player, SPELL_COMPLETE_QUEST);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (!me->isInCombat() && !me->GetTargetUnit())
            {
                me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                if (Creature* nalak = me->FindNearestCreature(69099, 150.0f))
                    nalak->AI()->DoAction(ACTION_1);
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_essence_of_stormAI(creature);
    }
};

void AddSC_boss_nalak()
{
    new boss_nalak();
    new npc_essence_of_storm();
}
