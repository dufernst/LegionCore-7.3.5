/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "QuestPackets.h"

void WorldPackets::Quest::QuestGiverStatusQuery::Read()
{
    _worldPacket >> QuestGiverGUID;
}

WorldPacket const* WorldPackets::Quest::QuestGiverStatus::Write()
{
    _worldPacket << QuestGiver.Guid;
    _worldPacket << QuestGiver.Status;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestGiverStatusMultiple::Write()
{
    _worldPacket << static_cast<int32>(QuestGiver.size());
    for (QuestGiverInfo const& questGiver : QuestGiver)
    {
        _worldPacket << questGiver.Guid;
        _worldPacket << questGiver.Status;
    }

    return &_worldPacket;
}

void WorldPackets::Quest::QuestGiverHello::Read()
{
    _worldPacket >> QuestGiverGUID;
}

void WorldPackets::Quest::QueryQuestInfo::Read()
{
    _worldPacket >> QuestID;
    _worldPacket >> QuestGiver;
}

void WorldPackets::Quest::QueryTreasurePicker::Read()
{
    _worldPacket >> QuestID;
    _worldPacket >> QuestRewardID;
}

WorldPacket const* WorldPackets::Quest::QueryQuestRewardResponse::Write()
{
    _worldPacket << QuestID;
    _worldPacket << QuestRewardID;
    _worldPacket << static_cast<uint32>(Items.size());
    _worldPacket << static_cast<uint32>(Currencys.size());
    _worldPacket << Money;

    for (auto const& v : Currencys)
    {
        _worldPacket << v.CurrencyID;
        _worldPacket << v.Amount;
    }

    for (auto const& v : Items)
    {
        _worldPacket << v.Item;
        _worldPacket << v.ItemCount;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QueryQuestInfoResponse::Write()
{
    _worldPacket << QuestID;

    _worldPacket.WriteBit(Allow);
    _worldPacket.FlushBits();

    if (Allow)
    {
        _worldPacket << Info.QuestID;
        _worldPacket << Info.QuestType;
        _worldPacket << Info.QuestLevel;
        _worldPacket << Info.QuestMaxScalingLevel;
        _worldPacket << Info.QuestPackageID;
        _worldPacket << Info.QuestMinLevel;
        _worldPacket << Info.QuestSortID;
        _worldPacket << Info.QuestInfoID;
        _worldPacket << Info.SuggestedGroupNum;
        _worldPacket << Info.RewardNextQuest;
        _worldPacket << Info.RewardXPDifficulty;
        _worldPacket << Info.RewardXPMultiplier;
        _worldPacket << Info.RewardMoney;
        _worldPacket << Info.RewardMoneyDifficulty;
        _worldPacket << Info.RewardMoneyMultiplier;
        _worldPacket << Info.RewardBonusMoney;
        _worldPacket.append(Info.RewardDisplaySpell, QUEST_REWARD_DISPLAY_SPELL_COUNT);
        _worldPacket << Info.RewardSpell;
        _worldPacket << Info.RewardHonor;
        _worldPacket << Info.RewardHonorMultiplier;
        _worldPacket << Info.RewardArtifactXP;
        _worldPacket << Info.RewardArtifactXPMultiplier;
        _worldPacket << Info.RewardArtifactCategoryID;
        _worldPacket << Info.StartItem;
        _worldPacket << Info.Flags;
        _worldPacket << Info.FlagsEx;

        for (uint32 i = 0; i < QUEST_ITEM_COUNT; ++i)
        {
            _worldPacket << Info.RewardItems[i];
            _worldPacket << Info.RewardAmount[i];
            _worldPacket << Info.ItemDrop[i];
            _worldPacket << Info.ItemDropQuantity[i];
        }

        for (auto & unfilteredChoiceItem : Info.UnfilteredChoiceItems)
        {
            _worldPacket << unfilteredChoiceItem.ItemID;
            _worldPacket << unfilteredChoiceItem.Quantity;
            _worldPacket << unfilteredChoiceItem.DisplayID;
        }

        _worldPacket << Info.POIContinent;
        _worldPacket << Info.POIx;
        _worldPacket << Info.POIy;
        _worldPacket << Info.POIPriority;

        _worldPacket << Info.RewardTitle;
        _worldPacket << Info.RewardArenaPoints;
        _worldPacket << Info.RewardSkillLineID;
        _worldPacket << Info.RewardNumSkillUps;

        _worldPacket << Info.PortraitGiver;
        _worldPacket << Info.PortraitTurnIn;

        for (uint32 i = 0; i < QUEST_REWARD_REPUTATIONS_COUNT; ++i)
        {
            _worldPacket << Info.RewardFactionID[i];
            _worldPacket << Info.RewardFactionValue[i];
            _worldPacket << Info.RewardFactionOverride[i];
            _worldPacket << Info.RewardFactionCapIn[i];
        }

        _worldPacket << Info.RewardFactionFlags;

        for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
        {
            _worldPacket << Info.RewardCurrencyID[i];
            _worldPacket << Info.RewardCurrencyQty[i];
        }

        _worldPacket << Info.AcceptedSoundKitID;
        _worldPacket << Info.CompleteSoundKitID;

        _worldPacket << Info.AreaGroupID;
        _worldPacket << Info.TimeAllowed;

        _worldPacket << static_cast<int32>(Info.Objectives.size());
        _worldPacket << Info.AllowableRaces;
        _worldPacket << Info.QuestRewardID;
        _worldPacket << Info.Expansion;

        _worldPacket.WriteBits(Info.LogTitle.size(), 9);
        _worldPacket.WriteBits(Info.LogDescription.size(), 12);
        _worldPacket.WriteBits(Info.QuestDescription.size(), 12);
        _worldPacket.WriteBits(Info.AreaDescription.size(), 9);
        _worldPacket.WriteBits(Info.PortraitGiverText.size(), 10);
        _worldPacket.WriteBits(Info.PortraitGiverName.size(), 8);
        _worldPacket.WriteBits(Info.PortraitTurnInText.size(), 10);
        _worldPacket.WriteBits(Info.PortraitTurnInName.size(), 8);
        _worldPacket.WriteBits(Info.QuestCompletionLog.size(), 11);
        _worldPacket.FlushBits();

        for (QuestObjective const& questObjective : Info.Objectives)
        {
            _worldPacket << questObjective.ID;
            _worldPacket << questObjective.Type;
            _worldPacket << questObjective.StorageIndex;
            _worldPacket << questObjective.ObjectID;
            _worldPacket << questObjective.Amount;
            _worldPacket << questObjective.Flags;
            _worldPacket << questObjective.Flags2;
            _worldPacket << questObjective.TaskStep;

            _worldPacket << static_cast<int32>(questObjective.VisualEffects.size());
            for (int32 visualEffect : questObjective.VisualEffects)
                _worldPacket << visualEffect;

            _worldPacket.WriteString(questObjective.Description, 8);
        }

        _worldPacket.WriteString(Info.LogTitle);
        _worldPacket.WriteString(Info.LogDescription);
        _worldPacket.WriteString(Info.QuestDescription);
        _worldPacket.WriteString(Info.AreaDescription);
        _worldPacket.WriteString(Info.PortraitGiverText);
        _worldPacket.WriteString(Info.PortraitGiverName);
        _worldPacket.WriteString(Info.PortraitTurnInText);
        _worldPacket.WriteString(Info.PortraitTurnInName);
        _worldPacket.WriteString(Info.QuestCompletionLog);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestUpdateAddCredit::Write()
{
    _worldPacket << VictimGUID;
    _worldPacket << QuestID;
    _worldPacket << ObjectID;
    _worldPacket << Count;
    _worldPacket << Required;
    _worldPacket << ObjectiveType;

    return &_worldPacket;
};

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Quest::QuestRewards const& questRewards)
{
    data << questRewards.ChoiceItemCount;

    for (auto choiceItem : questRewards.ChoiceItems)
    {
        data << choiceItem.ItemID;
        data << choiceItem.Quantity;
    }

    data << questRewards.ItemCount;

    for (uint32 i = 0; i < QUEST_ITEM_COUNT; ++i)
    {
        data << questRewards.ItemID[i];
        data << questRewards.ItemQty[i];
    }

    data << questRewards.Money;
    data << questRewards.XP;
    data << questRewards.ArtifactXP;
    data << questRewards.ArtifactCategoryID;
    data << questRewards.Honor;
    data << questRewards.Title;
    data << questRewards.FactionFlags;

    for (uint32 i = 0; i < QUEST_REWARD_REPUTATIONS_COUNT; ++i)
    {
        data << questRewards.FactionID[i];
        data << questRewards.FactionValue[i];
        data << questRewards.FactionOverride[i];
        data << questRewards.FactionCapIn[i];
    }

    for (auto spellCompletionDisplayID : questRewards.SpellCompletionDisplayID)
        data << spellCompletionDisplayID;

    data << questRewards.SpellCompletionID;

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        data << questRewards.CurrencyID[i];
        data << questRewards.CurrencyQty[i];
    }

    data << questRewards.SkillLineID;
    data << questRewards.NumSkillUps;
    data << questRewards.RewardID;
    data.WriteBit(questRewards.IsBoostSpell);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Quest::QuestGiverOfferReward const& offer)
{
    data << offer.QuestGiverGUID;
    data << offer.QuestGiverCreatureID;
    data << offer.QuestID;
    data << offer.QuestFlags[0];
    data << offer.QuestFlags[1];
    data << offer.SuggestedPartyMembers;

    data << int32(offer.Emotes.size());
    for (WorldPackets::Quest::QuestDescEmote const& emote : offer.Emotes)
    {
        data << emote.Type;
        data << emote.Delay;
    }

    data.WriteBit(offer.AutoLaunched);
    data.FlushBits();

    data << offer.Rewards;

    return data;
}

WorldPacket const* WorldPackets::Quest::QuestGiverOfferRewardMessage::Write()
{
    _worldPacket << QuestData;
    _worldPacket << QuestPackageID;
    _worldPacket << PortraitGiver;
    _worldPacket << PortraitTurnIn;

    _worldPacket.WriteBits(QuestTitle.size(), 9);
    _worldPacket.WriteBits(RewardText.size(), 12);
    _worldPacket.WriteBits(PortraitGiverText.size(), 10);
    _worldPacket.WriteBits(PortraitGiverName.size(), 8);
    _worldPacket.WriteBits(PortraitTurnInText.size(), 10);
    _worldPacket.WriteBits(PortraitTurnInName.size(), 8);

    _worldPacket.WriteString(QuestTitle);
    _worldPacket.WriteString(RewardText);
    _worldPacket.WriteString(PortraitGiverText);
    _worldPacket.WriteString(PortraitGiverName);
    _worldPacket.WriteString(PortraitTurnInText);
    _worldPacket.WriteString(PortraitTurnInName);

    return &_worldPacket;
};

void WorldPackets::Quest::QuestGiverChooseReward::Read()
{
    _worldPacket >> QuestGiverGUID;
    _worldPacket >> QuestID;
    _worldPacket >> ItemChoiceID;
}

WorldPacket const* WorldPackets::Quest::QuestGiverQuestComplete::Write()
{
    _worldPacket << int32(QuestID);
    _worldPacket << int32(XPReward);
    _worldPacket << int64(MoneyReward);
    _worldPacket << int32(SkillLineIDReward);
    _worldPacket << int32(NumSkillUpsReward);

    _worldPacket.WriteBit(UseQuestReward);
    _worldPacket.WriteBit(LaunchGossip);
    _worldPacket.WriteBit(LaunchQuest);
    _worldPacket.WriteBit(HideChatMessage);
    _worldPacket.FlushBits();

    _worldPacket << ItemReward;

    return &_worldPacket;
}

void WorldPackets::Quest::QuestGiverCompleteQuest::Read()
{
    _worldPacket >> QuestGiverGUID;
    _worldPacket >> QuestID;
    FromScript = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Quest::QuestGiverQuestDetails::Write()
{
    _worldPacket << QuestGiverGUID;
    _worldPacket << InformUnit;
    _worldPacket << QuestID;
    _worldPacket << QuestPackageID;
    _worldPacket << PortraitGiver;
    _worldPacket << PortraitTurnIn;
    _worldPacket << QuestFlags[0];
    _worldPacket << QuestFlags[1];
    _worldPacket << SuggestedPartyMembers;
    _worldPacket << static_cast<int32>(LearnSpells.size());
    _worldPacket << static_cast<int32>(DescEmotes.size());
    _worldPacket << static_cast<int32>(Objectives.size());
    _worldPacket << QuestStartItemID;

    for (int32 const& spell : LearnSpells)
        _worldPacket << spell;

    for (QuestDescEmote const& emote : DescEmotes)
    {
        _worldPacket << emote.Type;
        _worldPacket << emote.Delay;
    }

    for (QuestObjectiveSimple const& obj : Objectives)
    {
        _worldPacket << obj.ID;
        _worldPacket << obj.ObjectID;
        _worldPacket << obj.Amount;
        _worldPacket << obj.Type;
    }

    _worldPacket.WriteBits(QuestTitle.size(), 9);
    _worldPacket.WriteBits(DescriptionText.size(), 12);
    _worldPacket.WriteBits(LogDescription.size(), 12);
    _worldPacket.WriteBits(PortraitGiverText.size(), 10);
    _worldPacket.WriteBits(PortraitGiverName.size(), 8);
    _worldPacket.WriteBits(PortraitTurnInText.size(), 10);
    _worldPacket.WriteBits(PortraitTurnInName.size(), 8);
    _worldPacket.WriteBit(AutoLaunched);
    _worldPacket.WriteBit(StartCheat);
    _worldPacket.WriteBit(DisplayPopup);
    _worldPacket.FlushBits();

    _worldPacket << Rewards;
    _worldPacket.WriteString(QuestTitle);
    _worldPacket.WriteString(DescriptionText);
    _worldPacket.WriteString(LogDescription);
    _worldPacket.WriteString(PortraitGiverText);
    _worldPacket.WriteString(PortraitGiverName);
    _worldPacket.WriteString(PortraitTurnInText);
    _worldPacket.WriteString(PortraitTurnInName);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestGiverRequestItems::Write()
{
    _worldPacket << QuestGiverGUID;
    _worldPacket << QuestGiverCreatureID;
    _worldPacket << QuestID;
    _worldPacket << CompEmoteDelay;
    _worldPacket << CompEmoteType;
    _worldPacket << QuestFlags[0];
    _worldPacket << QuestFlags[1];
    _worldPacket << SuggestPartyMembers;
    _worldPacket << MoneyToGet;
    _worldPacket << static_cast<int32>(Collect.size());
    _worldPacket << static_cast<int32>(Currency.size());
    _worldPacket << StatusFlags;

    for (QuestObjectiveCollect const& obj : Collect)
    {
        _worldPacket << obj.ObjectID;
        _worldPacket << obj.Amount;
        _worldPacket << obj.Flags;
    }

    for (QuestCurrency const& cur : Currency)
    {
        _worldPacket << cur.CurrencyID;
        _worldPacket << cur.Amount;
    }

    _worldPacket.WriteBit(AutoLaunched);
    _worldPacket.FlushBits();

    _worldPacket.WriteBits(QuestTitle.size(), 9);
    _worldPacket.WriteBits(CompletionText.size(), 12);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(QuestTitle);
    _worldPacket.WriteString(CompletionText);

    return &_worldPacket;
}

void WorldPackets::Quest::QuestGiverRequestReward::Read()
{
    _worldPacket >> QuestGiverGUID;
    _worldPacket >> QuestID;
}

void WorldPackets::Quest::QuestGiverQueryQuest::Read()
{
    _worldPacket >> QuestGiverGUID;
    _worldPacket >> QuestID;
    RespondToGiver = _worldPacket.ReadBit();
}

void WorldPackets::Quest::QuestGiverAcceptQuest::Read()
{
    _worldPacket >> QuestGiverGUID;
    _worldPacket >> QuestID;
    StartCheat = _worldPacket.ReadBit();
}

void WorldPackets::Quest::QuestLogRemoveQuest::Read()
{
    _worldPacket >> Entry;
}

WorldPackets::Quest::GossipTextData::GossipTextData(uint32 questID, uint32 questType, uint32 questLevel, int32 questMaxScalingLevel, uint32 questFlags, uint32 questFlagsEx, bool repeatable, std::string questTitle):
    QuestID(questID), QuestType(questType), QuestLevel(questLevel), QuestFlags(questFlags), QuestFlagsEx(questFlagsEx), QuestMaxScalingLevel(questMaxScalingLevel), Repeatable(repeatable), QuestTitle(questTitle) { }

WorldPacket const* WorldPackets::Quest::QuestGiverQuestList::Write()
{
    _worldPacket << QuestGiverGUID;
    _worldPacket << GreetEmoteDelay;
    _worldPacket << GreetEmoteType;
    _worldPacket << static_cast<uint32>(GossipTexts.size());
    _worldPacket.WriteBits(Greeting.size(), 11);
    _worldPacket.FlushBits();

    for (GossipTextData const& gossip : GossipTexts)
    {
        _worldPacket << gossip.QuestID;
        _worldPacket << gossip.QuestType;
        _worldPacket << gossip.QuestLevel;
        _worldPacket << int32(gossip.QuestMaxScalingLevel);
        _worldPacket << gossip.QuestFlags;
        _worldPacket << gossip.QuestFlagsEx;
        _worldPacket.WriteBit(gossip.Repeatable);
        _worldPacket.WriteString(gossip.QuestTitle, 9);
    }

    _worldPacket.WriteString(Greeting);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestUpdateComplete::Write()
{
    _worldPacket << int32(QuestID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestUpdateCompleteBySpell::Write()
{
    _worldPacket << int32(QuestID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestConfirmAcceptResponse::Write()
{
    _worldPacket << uint32(QuestID);
    _worldPacket << InitiatedBy;

    _worldPacket.WriteBits(QuestTitle.size(), 10);
    _worldPacket.WriteString(QuestTitle);

    return &_worldPacket;
}

void WorldPackets::Quest::QuestConfirmAccept::Read()
{
    _worldPacket >> QuestID;
}

WorldPacket const* WorldPackets::Quest::QuestPushResultResponse::Write()
{
    _worldPacket << SenderGUID;
    _worldPacket << uint8(Result);

    return &_worldPacket;
}

void WorldPackets::Quest::QuestPushResult::Read()
{
    _worldPacket >> SenderGUID;
    _worldPacket >> QuestID;
    _worldPacket >> Result;
}

WorldPacket const* WorldPackets::Quest::QuestGiverInvalidQuest::Write()
{
    _worldPacket << Reason;
    _worldPacket << int32(ContributionRewardID);

    _worldPacket.WriteBit(SendErrorMessage);
    _worldPacket.WriteBits(ReasonText.length(), 9);
    _worldPacket.WriteString(ReasonText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestUpdateFailedTimer::Write()
{
    _worldPacket << QuestID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestForceRemoved::Write()
{
    _worldPacket << QuestID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::IsQuestCompleteResponse::Write()
{
    _worldPacket << QuestID;
    _worldPacket.WriteBit(Complete);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestGiverQuestFailed::Write()
{
    _worldPacket << QuestID;
    _worldPacket << Reason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestUpdateAddCreditSimple::Write()
{
    _worldPacket << QuestID;
    _worldPacket << ObjectID;
    _worldPacket << ObjectiveType;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::DailyQuestsReset::Write()
{
    _worldPacket << Count;

    return &_worldPacket;
}

void WorldPackets::Quest::PushQuestToParty::Read()
{
    _worldPacket >> QuestID;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Quest::PlayerChoiceResponseRewardEntry const& playerChoiceResponseRewardEntry)
{
    data << playerChoiceResponseRewardEntry.Item;
    data << int32(playerChoiceResponseRewardEntry.Quantity);
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Quest::PlayerChoiceResponseReward const& playerChoiceResponseReward)
{
    data << int32(playerChoiceResponseReward.TitleID);
    data << int32(playerChoiceResponseReward.PackageID);
    data << int32(playerChoiceResponseReward.SkillLineID);
    data << uint32(playerChoiceResponseReward.SkillPointCount);
    data << uint32(playerChoiceResponseReward.ArenaPointCount);
    data << uint32(playerChoiceResponseReward.HonorPointCount);
    data << uint64(playerChoiceResponseReward.Money);
    data << uint32(playerChoiceResponseReward.Xp);
    data << uint32(playerChoiceResponseReward.Items.size());
    data << uint32(playerChoiceResponseReward.Currencies.size());
    data << uint32(playerChoiceResponseReward.Factions.size());
    data << uint32(playerChoiceResponseReward.ItemChoices.size());

    for (auto const& item : playerChoiceResponseReward.Items)
        data << item;

    for (auto const& currency : playerChoiceResponseReward.Currencies)
        data << currency;

    for (auto const& faction : playerChoiceResponseReward.Factions)
        data << faction;

    for (auto const& itemChoice : playerChoiceResponseReward.ItemChoices)
        data << itemChoice;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Quest::PlayerChoiceResponse const& playerChoiceResponse)
{
    data << int32(playerChoiceResponse.ResponseID);
    data << int32(playerChoiceResponse.ChoiceArtFileID);

    data.WriteBits(playerChoiceResponse.Answer.length(), 9);
    data.WriteBits(playerChoiceResponse.Header.length(), 9);
    data.WriteBits(playerChoiceResponse.Description.length(), 11);
    data.WriteBits(playerChoiceResponse.Confirmation.length(), 7);
    data.WriteBit(playerChoiceResponse.Reward.is_initialized());
    data.FlushBits();

    if (playerChoiceResponse.Reward)
        data << *playerChoiceResponse.Reward;

    data.WriteString(playerChoiceResponse.Answer);
    data.WriteString(playerChoiceResponse.Header);
    data.WriteString(playerChoiceResponse.Description);
    data.WriteString(playerChoiceResponse.Confirmation);
    return data;
}

WorldPacket const* WorldPackets::Quest::DisplayPlayerChoice::Write()
{
    _worldPacket << int32(ChoiceID);
    _worldPacket << uint32(Responses.size());
    _worldPacket << SenderGUID;
    _worldPacket << int32(UiTextureKitID);
    _worldPacket.WriteBits(Question.length(), 8);
    _worldPacket.WriteBit(CloseChoiceFrame);
    _worldPacket.WriteBit(HideWarboardHeader);
    _worldPacket.FlushBits();

    for (auto const& response : Responses)
        _worldPacket << response;

    _worldPacket.WriteString(Question);
    return &_worldPacket;
}

void WorldPackets::Quest::AdventureJournalOpenQuest::Read()
{
    _worldPacket >> AdventureJournalID;
}

void WorldPackets::Quest::AdventureJournalStartQuest::Read()
{
    _worldPacket >> QuestID;
}

void WorldPackets::Quest::QueryAdventureMapPOI::Read()
{
    _worldPacket >> AdventureMapPOIID;
}

WorldPacket const* WorldPackets::Quest::QuestUpdateAddPvPCredit::Write()
{
    _worldPacket << QuestID;
    _worldPacket << Count;

    return &_worldPacket;
}

WorldPackets::Quest::WorldQuestUpdateInfo::WorldQuestUpdateInfo(int32 lastUpdate, uint32 areaPoiId, uint32 timer, int32 variableID, int32 value) : LastUpdate(lastUpdate), AreaPoiID(areaPoiId), Timer(timer), VariableID(variableID), Value(value)
{
}

WorldPacket const* WorldPackets::Quest::WorldQuestUpdate::Write()
{
    _worldPacket << static_cast<uint32>(WorldQuestUpdates.size());
    for (WorldQuestUpdateInfo const& worldQuestUpdate : WorldQuestUpdates)
    {
        _worldPacket << worldQuestUpdate.LastUpdate;
        _worldPacket << worldQuestUpdate.AreaPoiID;
        _worldPacket << worldQuestUpdate.Timer;
        _worldPacket << worldQuestUpdate.VariableID;
        _worldPacket << worldQuestUpdate.Value;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::AreaPoiUpdate::Write()
{
    _worldPacket << static_cast<uint32>(Pois.size());
    for (auto const& v : Pois)
    {
        _worldPacket << v.LastUpdate;
        _worldPacket << v.AreaPoiID;
        _worldPacket << v.Timer;
        _worldPacket << v.VariableID;
        _worldPacket << v.Value;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QueryAdventureMapPOIResponse::Write()
{
    _worldPacket << AdventureMapPOIID;
    _worldPacket.WriteBit(Result);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::DisplayQuestPopup::Write()
{
    _worldPacket << QuestID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestSpawnTrackingUpdate::Write()
{
    _worldPacket << static_cast<uint32>(TrackingUpdates.size());
    for (auto const& v : TrackingUpdates)
    {
        _worldPacket << v.QuestID;
        _worldPacket << v.UnkInt;
        _worldPacket.WriteBit(v.UnkBit);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Quest::QuestGiverQuestTurnInFailure::Write()
{
    _worldPacket.WriteString(String, 9);

    return &_worldPacket;
}
