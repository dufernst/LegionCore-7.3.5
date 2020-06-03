#include "scarlet_monastery.h"
#include "LFGMgr.h"
#include "Group.h"
#include "QuestData.h"

uint32 RandomLaugh[] = {11965, 11975, 11976};

enum Entry
{
    HH_MOUNTED                  = 23682,
    HH_DISMOUNTED               = 23800,  // unhorsed?? wtf type of engrish was that?
    HEAD                        = 23775,
    PULSING_PUMPKIN             = 23694,
    PUMPKIN_FIEND               = 23545,
    HELPER                      = 23686,
    WISP_INVIS                  = 24034
};

enum Spells
{
    SPELL_CLEAVE                = 42587,
    SPELL_CONFLAGRATION         = 42380,       //Phase 2, can't find real spell(Dim Fire?)
 // SPELL_CONFL_SPEED           = 22587,       //8% increase speed, value 22587 from SPELL_CONFLAGRATION mains that spell?
    SPELL_SUMMON_PUMPKIN        = 42394,

    SPELL_WHIRLWIND             = 43116,
    SPELL_IMMUNE                = 42556,
    SPELL_BODY_REGEN            = 42403,
    SPELL_CONFUSE               = 43105,

    SPELL_FLYING_HEAD           = 42399,       //visual flying head
    SPELL_HEAD                  = 42413,       //visual buff, "head"
    SPELL_HEAD_IS_DEAD          = 42428,       //at killing head, Phase 3

    SPELL_PUMPKIN_AURA          = 42280,
    SPELL_PUMPKIN_AURA_GREEN    = 42294,
    SPELL_SQUASH_SOUL           = 42514,
    SPELL_SPROUTING             = 42281,
    SPELL_SPROUT_BODY           = 42285,

    //Effects
    SPELL_RHYME_BIG             = 42909,
 // SPELL_RHYME_SMALL           = 42910,
    SPELL_HEAD_SPEAKS           = 43129,
    SPELL_HEAD_LANDS            = 42400,
    SPELL_BODY_FLAME            = 42074,
    SPELL_HEAD_FLAME            = 42971,
 // SPELL_ENRAGE_VISUAL         = 42438,       // he uses this spell?
    SPELL_WISP_BLUE             = 42821,
    SPELL_WISP_FLIGHT_PORT      = 42818,
 // SPELL_WISP_INVIS            = 42823,
    SPELL_SMOKE                 = 42355,
    SPELL_DEATH                 = 42566       //not correct spell
};

struct mob_wisp_invis : public ScriptedAI
{
    mob_wisp_invis(Creature* creature) : ScriptedAI(creature)
    {
        Creaturetype = delay = spell = spell2 = 0;
    }

    uint32 Creaturetype;
    uint32 delay;
    uint32 spell;
    uint32 spell2;

    void SetType(uint32 _type)
    {
        switch (Creaturetype = _type)
        {
        case 1:
            spell = SPELL_PUMPKIN_AURA_GREEN;
            break;
        case 2:
            delay = 15000;
            spell = SPELL_BODY_FLAME;
            spell2 = SPELL_DEATH;
            break;
        case 3:
            delay = 15000;
            spell = SPELL_SMOKE;
            break;
        case 4:
            delay = 7000;
            spell2 = SPELL_WISP_BLUE;
            break;
        }
        if (spell)
            DoCast(me, spell, false);
    }

    void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_WISP_FLIGHT_PORT && Creaturetype == 4)
            me->SetDisplayId(2027);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || Creaturetype != 1 || !who->isTargetableForAttack())
            return;

        if (me->IsWithinDist(who, 0.1f, false) && !who->HasAura(SPELL_SQUASH_SOUL))
            DoCast(who, SPELL_SQUASH_SOUL, false);
    }

    void UpdateAI(uint32 diff)
    {
        if (delay)
        {
            if (delay <= diff)
            {
                me->RemoveAurasDueToSpell(SPELL_SMOKE);

                if (spell2)
                    DoCast(me, spell2, false);

                delay = 0;
            }
            else
                delay -= diff;
        }
    }
};

struct mob_head : public ScriptedAI
{
    mob_head(Creature* creature) : ScriptedAI(creature) {}

    ObjectGuid bodyGUID;

    uint32 Phase;
    uint32 laugh;
    uint32 wait;

    bool withbody;
    bool die;

    void Reset() override
    {
        Phase = 0;
        bodyGUID.Clear();
        die = false;
        withbody = true;
        wait = 1000;
        laugh = urand(15000, 30000);
    }

    void Disappear()
    {
        if (withbody)
            return;

        if (bodyGUID)
        {
            if (auto body = Unit::GetCreature((*me), bodyGUID))
            {
                withbody = true;
                me->RemoveAllAuras();
                body->RemoveAurasDueToSpell(SPELL_IMMUNE);
                DoCast(body, SPELL_FLYING_HEAD, false);
                me->SetFullHealth();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->GetMotionMaster()->MoveIdle();

                if (auto boss = me->FindNearestCreature(23682, 100.0f, true))
                    boss->AI()->DoAction(ACTION_1);
            }
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_2)
        {
            Phase = Phase;
            Disappear();
        }
    }

    void DamageTaken(Unit* /*done_by*/, uint32 &damage, DamageEffectType dmgType) override
    {
        if (withbody)
            return;

        switch (Phase)
        {
        case 1:
            if (me->HealthBelowPctDamaged(67, damage))
                Disappear();
            break;
        case 2:
            if (me->HealthBelowPctDamaged(34, damage))
                Disappear();
            break;
        case 3:
            if (damage >= me->GetHealth())
            {
                die = true;
                withbody = true;
                wait = 300;
                damage = me->GetHealth() - me->CountPctFromMaxHealth(1);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->StopMoving();
                DoCast(me, SPELL_HEAD_IS_DEAD, false);
            }
            break;
        }
    }

    void JustDied(Unit* /*who*/) override
    {
        if (auto body = Unit::GetUnit(*me, bodyGUID))
            body->Kill(body);
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (!withbody)
            return;

        if (spell->Id == SPELL_FLYING_HEAD)
        {
            if (Phase < 3)
                ++Phase;
            else
                Phase = 3;

            withbody = false;

            if (!bodyGUID)
                bodyGUID = caster->GetGUID();

            me->RemoveAllAuras();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_HEAD_LANDS, true);
            DoCast(SPELL_HEAD);
            me->GetMotionMaster()->Clear(false);

            if (auto victim = caster->getVictim())
                me->GetMotionMaster()->MoveFleeing(victim);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!me->HasAura(SPELL_HEAD) && !withbody)
            me->AddAura(SPELL_HEAD, me);

        if (!withbody)
        {
            if (wait <= diff)
            {
                wait = 1000;

                if (auto victim = me->getVictim())
                {
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveFleeing(victim);
                }
            }
            else
                wait -= diff;

            if (laugh <= diff)
            {
                laugh = urand(15000, 30000);
                DoPlaySoundToSet(me, RandomLaugh[urand(0, 2)]);
            }
            else
                laugh -= diff;
        }
        else
        {
            if (die)
            {
                if (wait <= diff)
                {
                    die = false;

                    me->Kill(me);
                }
                else
                    wait -= diff;
            }
        }
    }
};

struct boss_headless_horseman : public ScriptedAI
{
    boss_headless_horseman(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    ObjectGuid headGUID;
    ObjectGuid playerGUID;

    uint32 Phase;
    uint32 countPhase;
    uint32 id;
    uint32 say_timer1;
    uint32 say_timer2;
    uint32 say_timer3;
    uint32 say_timer4;

    uint32 conflagrate;
    uint32 summonadds;
    uint32 cleave;
    uint32 regen;
    uint32 whirlwind;
    uint32 laugh;
    uint32 burn;

    bool withhead;
    bool returned;
    bool burned;

    void Reset() override
    {
        Phase = 1;
        countPhase = 0;
        conflagrate = 15000;
        summonadds = 0;
        laugh = urand(16000, 20000);
        cleave = 2000;
        regen = 1000;
        burn = 6000;
        say_timer1 = 0;
        say_timer2 = 0;
        say_timer3 = 0;
        say_timer4 = 0;

        withhead = true;
        returned = true;
        burned = false;
        DoCast(SPELL_HEAD);

        if (headGUID)
        {
            if (auto Head = Unit::GetCreature(*me, headGUID))
                Head->DespawnOrUnsummon();

            headGUID.Clear();
        }
    }

    void MovementInform(uint32 type, uint32 i) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (i == 3)
        {
            Phase = 1;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetWalk(true);
            me->SetCanFly(false);
            me->SetDisableGravity(false);

            if (auto player = me->FindNearestPlayer(15.0f, true))
                AttackStart(player);
        }

        ++id;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance)
            instance->SetData(DATA_HORSEMAN_EVENT, IN_PROGRESS);

        DoZoneInCombat();
    }

    void EnterEvadeMode() override
    {
        me->RemoveAllAuras();
        Phase = 1;
        ScriptedAI::EnterEvadeMode();
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (withhead && Phase != 0)
            ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->StopMoving();
        Talk(4);
        Position pos;

        if (auto flame = me->SummonCreature(HELPER, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60000))
            flame->CastSpell(flame, SPELL_BODY_FLAME, false);

        if (instance)
            instance->SetData(DATA_HORSEMAN_EVENT, DONE);

        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        if (!players.isEmpty())
        {
            Player* pPlayer = players.begin()->getSource();
            if (pPlayer && pPlayer->GetGroup())
                sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 285);
        }
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (withhead)
            return;

        if (spell->Id == SPELL_FLYING_HEAD)
        {
            if (Phase < 3)
                ++Phase;
            else
                Phase = 3;

            summonadds = 15000;
            withhead = true;
            me->RemoveAllAuras();
            me->SetFullHealth();
            Talk(1);
            DoCast(SPELL_HEAD);
            caster->GetMotionMaster()->Clear(false);
            caster->GetMotionMaster()->MoveFollow(me, 6, float(urand(0, 5)));

            std::list<HostileReference*>::const_iterator itr;
            for (itr = caster->getThreatManager().getThreatList().begin(); itr != caster->getThreatManager().getThreatList().end(); ++itr)
                if (auto unit = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                    me->AddThreat(unit, caster->getThreatManager().getThreat(unit));
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            if (!returned)
                returned = true;
        }

        if (action == ACTION_2)
        {
            me->SetVisible(false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
            me->SetSpeed(MOVE_WALK, 5.0f, true);
            me->SetHomePosition(1086.2392f, 627.4357f, -0.054f, 0.0199f);
            me->AddDelayedEvent(4000, [=]() -> void { DoAction(ACTION_3); });

            DoCast(me, SPELL_RHYME_BIG, false);
            say_timer1 = 100;
            say_timer2 = 2100;
            say_timer3 = 4100;
            say_timer4 = 7100;
            id = 0;
            Phase = 0;
        }

        if (action == ACTION_3)
        {
            me->SetVisible(true);
            me->GetMotionMaster()->MovePath(23682, false);
            Talk(0);
        }
    }

    void DamageTaken(Unit* /*done_by*/, uint32 &damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth() && countPhase != 4)
            damage = 0;

        if (me->HealthBelowPct(2) && withhead)
        {
            withhead = false;
            returned = false;
            me->RemoveAllAuras();
            countPhase++;

            if (!headGUID)
                headGUID = DoSpawnCreature(HEAD, float(rand() % 6), float(rand() % 6), 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0)->GetGUID();

            if (auto head = Unit::GetUnit(*me, headGUID))
            {
                head->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->InterruptNonMeleeSpells(false);
                DoCast(me, SPELL_IMMUNE, true);
                DoCast(me, SPELL_BODY_REGEN, true);
                DoCast(head, SPELL_FLYING_HEAD, true);
                DoCast(me, SPELL_CONFUSE, false);
                whirlwind = urand(4000, 8000);
                regen = 0;
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (withhead)
        {
            switch (Phase)
            {
            case 0:
            {
                if (say_timer1)
                {
                    if (say_timer1 <= diff)
                    {
                        say_timer1 = 0;

                        if (auto find = me->FindNearestPlayer(100.0f, false))
                            playerGUID = find->GetGUID();

                        if (auto player = Unit::GetPlayer(*me, playerGUID))
                        {
                            sCreatureTextMgr->SendChat(me, 5, player->GetGUID(), CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                            player->HandleEmoteCommand(ANIM_EMOTE_SHOUT);
                        }
                    }
                    else
                        say_timer1 -= diff;
                }
                if (say_timer2)
                {
                    if (say_timer2 <= diff)
                    {
                        say_timer2 = 0;

                        if (auto player = Unit::GetPlayer(*me, playerGUID))
                        {
                            sCreatureTextMgr->SendChat(me, 6, player->GetGUID(), CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                            player->HandleEmoteCommand(ANIM_EMOTE_SHOUT);
                        }
                    }
                    else
                        say_timer2 -= diff;
                }

                if (say_timer3)
                {
                    if (say_timer3 <= diff)
                    {
                        say_timer3 = 0;

                        if (auto player = Unit::GetPlayer(*me, playerGUID))
                        {
                            sCreatureTextMgr->SendChat(me, 7, player->GetGUID(), CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                            player->HandleEmoteCommand(ANIM_EMOTE_SHOUT);
                        }
                    }
                    else
                        say_timer3 -= diff;
                }

                if (say_timer4)
                {
                    if (say_timer4 <= diff)
                    {
                        say_timer4 = 0;

                        if (auto player = Unit::GetPlayer(*me, playerGUID))
                        {
                            sCreatureTextMgr->SendChat(me, 8, player->GetGUID(), CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                            player->HandleEmoteCommand(ANIM_EMOTE_SHOUT);
                        }

                        DoCast(me, SPELL_RHYME_BIG, false);
                    }
                    else
                        say_timer4 -= diff;
                }
            }
            break;
            case 1:
                if (burn <= diff)
                {
                    if (!burned)
                    {
                        me->SummonCreature(HELPER, 1105.11f, 610.36f, -0.43f, 0, TEMPSUMMON_TIMED_DESPAWN, 17000);
                        burned = true;
                    }
                }
                else
                    burn -= diff;
                break;
            case 2:
                if (conflagrate <= diff)
                {
                    if (auto player = me->FindNearestPlayer(30.0f, true))
                        DoCast(player, SPELL_CONFLAGRATION, false);

                    conflagrate = urand(10000, 16000);
                }
                else
                    conflagrate -= diff;
                break;
            case 3:
                if (summonadds)
                {
                    if (summonadds <= diff)
                    {
                        me->InterruptNonMeleeSpells(false);
                        DoCast(me, SPELL_SUMMON_PUMPKIN, false);
                        Talk(3);
                        summonadds = urand(25000, 35000);
                    }
                    else
                        summonadds -= diff;
                }
                break;
            }

            if (laugh <= diff)
            {
                laugh = urand(15000, 30000);
                DoPlaySoundToSet(me, RandomLaugh[urand(0, 2)]);
            }
            else
                laugh -= diff;

            if (UpdateVictim())
            {
                DoMeleeAttackIfReady();
                if (cleave <= diff)
                {
                    if (auto victim = me->getVictim())
                        DoCast(victim, SPELL_CLEAVE, false);

                    cleave = urand(2000, 6000);
                }
                else
                    cleave -= diff;
            }
        }
        else
        {
            if (regen <= diff)
            {
                regen = 1000;

                if (me->IsFullHealth() && !returned)
                {
                    if (Phase > 1)
                        --Phase;
                    else
                        Phase = 1;

                    if (auto Head = Unit::GetCreature((*me), headGUID))
                        Head->AI()->DoAction(ACTION_2);
                    return;
                }
            }
            else
                regen -= diff;

            if (whirlwind <= diff)
            {
                whirlwind = urand(4000, 8000);
                if (urand(0, 1))
                {
                    me->RemoveAurasDueToSpell(SPELL_CONFUSE);
                    DoCast(me, SPELL_WHIRLWIND, true);
                    DoCast(me, SPELL_CONFUSE, false);
                }
                else
                    me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
            }
            else
                whirlwind -= diff;
        }
    }
};

struct mob_pulsing_pumpkin : public ScriptedAI
{
    mob_pulsing_pumpkin(Creature* creature) : ScriptedAI(creature) {}

    bool sprouted;
    ObjectGuid debuffGUID;

    void Reset() override
    {
        float x, y, z;
        me->GetPosition(x, y, z);
        me->SetPosition(x, y, z + 0.35f, 0.0f);
        Despawn();

        if (auto debuff = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 14500))
        {
            debuff->SetDisplayId(me->GetDisplayId());
            debuff->CastSpell(debuff, SPELL_PUMPKIN_AURA_GREEN, false);
            debuffGUID = debuff->GetGUID();
        }

        sprouted = false;
        DoCast(me, SPELL_PUMPKIN_AURA, true);
        DoCast(me, SPELL_SPROUTING, false);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_SPROUTING)
        {
            sprouted = true;
            me->RemoveAllAuras();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
            DoCast(me, SPELL_SPROUT_BODY, true);
            me->UpdateEntry(PUMPKIN_FIEND);

            if (auto victim = me->getVictim())
                DoStartMovement(victim);
        }
    }

    void Despawn()
    {
        if (!debuffGUID)
            return;

        if (auto debuff = Unit::GetUnit(*me, debuffGUID))
        {
            debuff->SetVisible(false);
            debuffGUID.Clear();
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!sprouted)
            Despawn();
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || !me->IsValidAttackTarget(who) || me->getVictim())
            return;

        me->AddThreat(who, 0.0f);

        if (sprouted)
            DoStartMovement(who);
    }

    void UpdateAI(uint32 /*diff*/)
    {
        if (sprouted && UpdateVictim())
            DoMeleeAttackIfReady();
    }
};

class go_loosely_turned_soil : public GameObjectScript
{
public:
    go_loosely_turned_soil() : GameObjectScript("go_loosely_turned_soil") {}

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (auto group = player->GetGroup())
            if (group->isLFGGroup())
                return false;

        InstanceScript* instance = player->GetInstanceScript();
        if (!instance)
            return true;

        if (instance->GetData(DATA_HORSEMAN_EVENT) != NOT_STARTED || instance->GetGuidData(NPC_HORSEMAN))
            return true;

        instance->SetData(DATA_HORSEMAN_EVENT, IN_PROGRESS);
        player->AreaExploredOrEventHappens(11405);

        if (auto horseman = go->SummonCreature(HH_MOUNTED, 1089.398f, 612.1663f, -0.7859635f, 0.1787923f, TEMPSUMMON_MANUAL_DESPAWN, 0))
            horseman->AI()->DoAction(ACTION_2);

        return true;
    }
};

void AddSC_boss_headless_horseman()
{
    RegisterCreatureAI(boss_headless_horseman);
    RegisterCreatureAI(mob_head);
    RegisterCreatureAI(mob_pulsing_pumpkin);
    RegisterCreatureAI(mob_wisp_invis);
    new go_loosely_turned_soil();
}
