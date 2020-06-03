/*
*/

#include "ChatPackets.h"

enum eSpells
{
    SPELL_VOLUNTEER_CITY_GUARD_HORDE = 231163,
    SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE = 231193
};

enum eClasses
{
    WARRIOR = 1,
    PALADIN = 2,
    DK      = 3,
    DRUID   = 4,
    PRIEST  = 5,
    SHAMAN  = 6,
    MAGE    = 7,
    WARLOCK = 8
};

struct npc_volunteer_guard_day : public ScriptedAI
{
    npc_volunteer_guard_day(Creature* creature) : ScriptedAI(creature) {}

    void ReceiveEmote(Player* player, uint32 textEmote) override
    {
        if (textEmote == TEXT_EMOTE_SALUTE)
        {
            if (sGameEventMgr->IsActiveEvent(70))
            {
                if (player->GetTeam() == HORDE)
                {
                    DoCast(player, SPELL_VOLUNTEER_CITY_GUARD_HORDE, false);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                }
                else
                {
                    DoCast(player, SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, false);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                }
            }
        }
    }
};

struct npc_city_invader : public ScriptedAI
{
    npc_city_invader(Creature* creature) : ScriptedAI(creature) {}

    float bp = 0.f;
    float bp0 = 0.f;
    uint32 randomClass = 0;
    EventMap events;

    void EnterEvadeMode() override
    {
        me->DespawnOrUnsummon();
    }

    void IsSummonedBy(Unit* owner) override
    {
        randomClass = urand(0, 7);
        AttackStart(owner);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        switch (randomClass)
        {
        case WARRIOR:
            if (auto victim = me->getVictim())
                DoCast(victim, 22120, true);

            me->SetDisplayId(RAND(65252, 65262));
            SetEquipmentSlots(false, 107367, 0, EQUIP_NO_CHANGE);
            events.RescheduleEvent(EVENT_1, 5000); // charge
            events.RescheduleEvent(EVENT_2, 15000); // Demoralizing Shout
            events.RescheduleEvent(EVENT_3, 7000); // Hamstring
            events.RescheduleEvent(EVENT_4, 3000); // Heroic Strike
            events.RescheduleEvent(EVENT_5, 20000); // Sunder Armor
            events.RescheduleEvent(EVENT_6, 6000); // Molter Strike
            events.RescheduleEvent(EVENT_7, 4000); // Slam
            break;
        case PALADIN:
            me->SetDisplayId(RAND(68104, 68105));
            SetEquipmentSlots(false, 108923, 0, EQUIP_NO_CHANGE);

            DoCast(79976);
            DoCast(79977);
            events.RescheduleEvent(EVENT_19, 1000);
            events.RescheduleEvent(EVENT_20, 2000);
            events.RescheduleEvent(EVENT_21, 5000);
            break;
        case PRIEST:
            me->SetDisplayId(RAND(65430, 17882));
            SetEquipmentSlots(false, 28738, 0, EQUIP_NO_CHANGE);

            DoCast(16592);
            events.RescheduleEvent(EVENT_13, 1000);
            events.RescheduleEvent(EVENT_14, 3500);
            events.RescheduleEvent(EVENT_15, 6000);
            break;
        case SHAMAN:
            me->SetDisplayId(RAND(75302, 65023));
            SetEquipmentSlots(false, 126320, 126334, EQUIP_NO_CHANGE);

            DoCast(19514);
            events.RescheduleEvent(EVENT_8, 2000);
            events.RescheduleEvent(EVENT_9, 7000);
            events.RescheduleEvent(EVENT_10, 5000);
            events.RescheduleEvent(EVENT_11, 10000);
            break;
        case MAGE:
            me->SetDisplayId(RAND(65253, 65441));
            SetEquipmentSlots(false, 126324, 0, EQUIP_NO_CHANGE);

            events.RescheduleEvent(EVENT_16, 1000);
            events.RescheduleEvent(EVENT_17, 2000);
            events.RescheduleEvent(EVENT_18, 10000);
            break;
        case WARLOCK:
            me->SetDisplayId(65255);
            SetEquipmentSlots(false, 115865, 0, EQUIP_NO_CHANGE);

            DoCast(79954);
            events.RescheduleEvent(EVENT_22, 1000);
            events.RescheduleEvent(EVENT_23, 4000);
            break;
        case DRUID:
            me->SetDisplayId(RAND(68114, 65413));
            SetEquipmentSlots(false, 64632, 0, EQUIP_NO_CHANGE);

            DoCast(19030);
            DoCast(me, 79833, true);
            events.RescheduleEvent(EVENT_12, 2000);
            break;
        case DK:
            me->SetDisplayId(29818);
            SetEquipmentSlots(false, 41261, 0, EQUIP_NO_CHANGE);

            DoCast(79891);
            events.RescheduleEvent(EVENT_24, 1000);
            events.RescheduleEvent(EVENT_25, 2000);
            events.RescheduleEvent(EVENT_26, 2500);
            break;
        default: 0;
            break;
        }
    }

    void SendWorldTextToOwner(ObjectGuid guid, uint32 broadcastID)
    {
        BroadcastTextEntry const* bct = sBroadcastTextStore.LookupEntry(broadcastID);
        if (!bct)
            return;

        WorldPackets::Chat::WorldText packet;
        packet.Guid = guid;

        if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
        {
            if (sConditionMgr->IsPlayerMeetingCondition(player, bct->ConditionID))
            {
                packet.Text = DB2Manager::GetBroadcastTextValue(bct, player->GetSession()->GetSessionDbLocaleIndex());
                player->SendDirectMessage(packet.Write());
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        for (auto const& _guid : *me->GetSaveThreatList())
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, _guid))
            {
                if (player->ToPlayer()->GetTeam() == HORDE)
                {
                    switch (me->GetCurrentZoneID())
                    {
                    case 1637:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_1))
                        {
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127100);
                        }
                        break;
                    case 1497:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_3))
                        {
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127135);
                        }
                        break;
                    case 1638:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_2))
                        {
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127134);
                        }
                        break;
                    case 3487:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_4))
                        {
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127136);
                        }
                        break;
                    default: 0;
                        break;
                    }

                    if (me->GetAreaId() == 7333)
                    {
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_5))
                        {
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127137);
                        }
                    }
                }
                else if (player->ToPlayer()->GetTeam() == ALLIANCE)
                {
                    switch (me->GetCurrentZoneID())
                    {
                    case 1519:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_1))
                        {
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127138);
                        }
                        break;
                    case 1537:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_2))
                        {
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127139);
                        }
                        break;
                    case 1657:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_3))
                        {
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127140);
                        }
                        break;
                    case 3557:
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_4))
                        {
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);
                            if (AuraEffect* aurEff5 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_5))
                                if (aurEff5->GetAmount() > 0)
                                    aurEff5->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127141);
                        }
                        break;
                    default: 0;
                        break;
                    }

                    if (me->GetAreaId() == 7332)
                    {
                        if (AuraEffect* aurEff = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_5))
                        {
                            if (AuraEffect* aurEff1 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_1))
                                if (aurEff1->GetAmount() > 0)
                                    aurEff1->SetAmount(0);
                            if (AuraEffect* aurEff3 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_3))
                                if (aurEff3->GetAmount() > 0)
                                    aurEff3->SetAmount(0);
                            if (AuraEffect* aurEff4 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_4))
                                if (aurEff4->GetAmount() > 0)
                                    aurEff4->SetAmount(0);
                            if (AuraEffect* aurEff2 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_ALLIANCE, EFFECT_2))
                                if (aurEff2->GetAmount() > 0)
                                    aurEff2->SetAmount(0);

                            if (AuraEffect* aurEff0 = player->GetAuraEffect(SPELL_VOLUNTEER_CITY_GUARD_HORDE, EFFECT_0))
                            {
                                bp0 += aurEff0->GetAmount() + 1;
                                aurEff0->SetAmount(bp0);
                                bp0 = 0.f;
                            }

                            bp += aurEff->GetAmount() + 1;
                            aurEff->SetAmount(bp);
                            bp = 0.f;
                            SendWorldTextToOwner(_guid, 127142);
                        }
                    }
                }
            }
        }
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
                DoCast(11971);
                events.RescheduleEvent(EVENT_1, 25000);
                break;
            case EVENT_2:
                DoCast(13730);
                events.RescheduleEvent(EVENT_2, 35000);
                break;
            case EVENT_3:
                DoCast(9080);
                events.RescheduleEvent(EVENT_3, 15000);
                break;
            case EVENT_4:
                DoCast(57846);
                events.RescheduleEvent(EVENT_4, 8000);
                break;
            case EVENT_5:
                DoCast(11971);
                events.RescheduleEvent(EVENT_5, 30000);
                break;
            case EVENT_6:
                DoCast(32736);
                events.RescheduleEvent(EVENT_6, 10000);
                break;
            case EVENT_7:
                DoCast(79881);
                events.RescheduleEvent(EVENT_7, 5000);
                break;
            case EVENT_8:
                DoCast(79913);
                events.RescheduleEvent(EVENT_8, 6000);
                break;
            case EVENT_9:
                DoCast(33844);
                events.RescheduleEvent(EVENT_9, 20000);
                break;
            case EVENT_10:
                DoCast(79890);
                events.RescheduleEvent(EVENT_10, 12000);
                break;
            case EVENT_11:
                DoCast(79886);
                events.RescheduleEvent(EVENT_11, 10000);
                break;
            case EVENT_12:
                DoCast(48562);
                events.RescheduleEvent(EVENT_12, 3000);
                break;
            case EVENT_13:
                DoCast(16568);
                events.RescheduleEvent(EVENT_13, 5000);
                break;
            case EVENT_14:
                DoCast(11639);
                events.RescheduleEvent(EVENT_14, 15000);
                break;
            case EVENT_15:
                DoCast(13860);
                events.RescheduleEvent(EVENT_15, 4000);
                break;
            case EVENT_16:
                DoCast(34447);
                events.RescheduleEvent(EVENT_16, 3500);
                break;
            case EVENT_17:
                DoCast(79868);
                events.RescheduleEvent(EVENT_17, 3500);
                break;
            case EVENT_18:
                DoCast(me, 21655, true);
                events.RescheduleEvent(EVENT_18, 10000);
                break;
            case EVENT_19:
                DoCast(me, 79966, true);
                events.RescheduleEvent(EVENT_19, 8000);
                break;
            case EVENT_20:
                DoCast(79964);
                events.RescheduleEvent(EVENT_20, 4000);
                break;
            case EVENT_21:
                DoCast(79970);
                events.RescheduleEvent(EVENT_21, 3000);
                break;
            case EVENT_22:
                if (auto victim = me->getVictim())
                    DoCast(victim, 79930, true);
                events.RescheduleEvent(EVENT_22, 15000);
                break;
            case EVENT_23:
                DoCast(79932);
                events.RescheduleEvent(EVENT_23, 3000);
                break;
            case EVENT_24:
                DoCast(me, 79887, true);
                events.RescheduleEvent(EVENT_24, 10000);
                break;
            case EVENT_25:
                DoCast(79903);
                events.RescheduleEvent(EVENT_25, 4000);
                break;
            case EVENT_26:
                DoCast(79903);
                events.RescheduleEvent(EVENT_26, 2000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 231193, 231163
class spell_volunteer_city_guard : public AuraScript
{
    PrepareAuraScript(spell_volunteer_city_guard);

    uint16 timer;

    bool Load()
    {
        timer = 30000;
        return true;
    }

    void OnUpdate(uint32 diff, AuraEffect* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (timer)
        {
            if (timer <= diff)
            {
                timer = 30000;

                if (aurEff->GetSpellInfo()->Id == 231163)
                    if ((caster->GetCurrentZoneID() == 1637) || (caster->GetCurrentZoneID() == 1497) || (caster->GetCurrentZoneID() == 1638) || (caster->GetCurrentZoneID() == 3487) || (caster->GetCurrentAreaID() == 7333))
                        caster->CastSpell(caster, 231192, true);
                if (aurEff->GetSpellInfo()->Id == 231193)
                    if ((caster->GetCurrentZoneID() == 1519) || (caster->GetCurrentZoneID() == 1537) || (caster->GetCurrentZoneID() == 1657) || (caster->GetCurrentZoneID() == 3557) || (caster->GetCurrentAreaID() == 7332))
                        caster->CastSpell(caster, 231192, true);
            }
            else
                timer -= diff;
        }
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_volunteer_city_guard::OnUpdate, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

void AddSC_VolunteerGuardDay()
{
    RegisterCreatureAI(npc_volunteer_guard_day);
    RegisterCreatureAI(npc_city_invader);
    RegisterAuraScript(spell_volunteer_city_guard);
}