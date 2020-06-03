//#include "Object.h"
#include "LFGListMgr.h"
#include "GroupMgr.h"
//#include "LFGPackets.h"
#include "ObjectMgr.h"
#include "LfgListPackets.h"
//#include "SocialMgr.h"
#include "LFGList.h"

LFGListEntry::LFGListApplicationEntry::LFGListApplicationEntry(ObjectGuid::LowType playerGuid, LFGListEntry* owner)
{
    ID = sObjectMgr->GetGenerator<HighGuid::LFGObject>()->GenerateLow();
    ApplicationTime = time(nullptr);
    PlayerLowGuid = playerGuid;
    Timeout = ApplicationTime + LFG_LIST_APPLY_FOR_GROUP_TIMEOUT;
    ApplicationStatus = LFGListApplicationStatus::None;
    Listed = true;
    m_Owner = owner;
    Status = LFGListStatus::None;
    RoleMask = 0;
}

Player* LFGListEntry::LFGListApplicationEntry::GetPlayer() const
{
    return sObjectMgr->GetPlayerByLowGUID(PlayerLowGuid);
};

bool LFGListEntry::IsApplied(ObjectGuid::LowType guidLow) const
{
    return ApplicationsContainer.find(guidLow) != ApplicationsContainer.end();
}

bool LFGListEntry::IsApplied(Player* player) const
{
    return IsApplied(player->GetGUIDLow());
}

void LFGListEntry::BroadcastApplicantUpdate(LFGListApplicationEntry const* applicant)
{
    auto applicantList = std::list<LFGListEntry::LFGListApplicationEntry const*>({applicant}); // nyi

    WorldPackets::LfgList::LfgListApplicationUpdate update;
    update.ApplicationTicket.RequesterGuid = ApplicationGroup->GetGUID();
    update.ApplicationTicket.Id = ApplicationGroup->GetGUIDLow();
    update.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
    update.ApplicationTicket.Time = CreationTime;

    update.UnkInt = 6;

    for (auto const& v : applicantList)
    {
        auto player = v->GetPlayer();

        WorldPackets::LfgList::ApplicantInfo info;
        info.ApplicantTicket.RequesterGuid = ObjectGuid::Create<HighGuid::Player>(v->PlayerLowGuid);
        info.ApplicantTicket.Id = v->ID;
        info.ApplicantTicket.Type = WorldPackets::LFG::RideType::LfgListApplicant;
        info.ApplicantTicket.Time = v->ApplicationTime;

        info.ApplicantPartyLeader = ObjectGuid::Create<HighGuid::Player>(v->PlayerLowGuid);
        info.ApplicationStatus = AsUnderlyingType(v->ApplicationStatus);
        info.Comment = v->Comment;
        info.Listed = v->Listed;

        if (player && v->Listed)
        {
            WorldPackets::LfgList::ApplicantMember member;
            member.PlayerGUID = player->GetGUID();
            member.VirtualRealmAddress = GetVirtualRealmAddress();
            member.Level = player->getLevel();
            member.HonorLevel = player->GetHonorLevel();
            member.ItemLevel = sLFGListMgr->GetPlayerItemLevelForActivity(GroupFinderActivityData, player);
            member.PossibleRoleMask = v->RoleMask;
            member.SelectedRoleMask = 0;

            //for (auto const& v : applicants)
            //{
            //    ACStatInfo info;
            //    info.UnkInt4 = v.Role;
            //    info.UnkInt5 = 0;
            //    AcStat.emplace_back(info);
            //}

            info.Member.emplace_back(member);
        }

        update.Applicants.emplace_back(info);
    }

    ApplicationGroup->BroadcastPacket(update.Write(), false);
}

void LFGListEntry::LFGListApplicationEntry::ResetTimeout()
{
    Timeout = time(nullptr) + (ApplicationStatus == LFGListApplicationStatus::Invited ? LFG_LIST_INVITE_TO_GROUP_TIMEOUT : LFG_LIST_APPLY_FOR_GROUP_TIMEOUT);
}

void LFGListEntry::ResetTimeout()
{
    Timeout = time(nullptr) + LFG_LIST_GROUP_TIMEOUT;
    sLFGListMgr->SendLFGListStatusUpdate(this);
}

uint32 LFGListEntry::GetID() const
{
    return ApplicationGroup->GetGUIDLow();
}

bool LFGListEntry::Update(uint32 const diff)
{
    for (auto itr = ApplicationsContainer.begin(); itr != ApplicationsContainer.end();)
    {
        if (!itr->second.Update(diff))
        {
            sLFGListMgr->ChangeApplicantStatus(&itr->second, LFGListApplicationStatus::Timeout);
            itr = ApplicationsContainer.begin();
        }
        else
            ++itr;
    }

    return Timeout > time(nullptr);
}

bool LFGListEntry::LFGListApplicationEntry::Update(uint32 const /*diff*/)
{
    return Timeout > time(nullptr); ///< Bye bye
}

LFGListEntry::LFGListEntry() : GroupFinderActivityData(nullptr), ApplicationGroup(nullptr), HonorLevel(0), QuestID(0), ItemLevel(0), AutoAccept(false)
{
    CreationTime = uint32(time(nullptr));
    Timeout = CreationTime + LFG_LIST_GROUP_TIMEOUT;
}

LFGListEntry::LFGListApplicationEntry* LFGListEntry::GetApplicant(ObjectGuid::LowType id)
{
    return Trinity::Containers::MapGetValuePtr(ApplicationsContainer, id);
}

LFGListEntry::LFGListApplicationEntry * LFGListEntry::GetApplicantByPlayerGUID(ObjectGuid::LowType lowGuid)
{
    for (auto& application : ApplicationsContainer)
        if (application.second.PlayerLowGuid == lowGuid)
            return &application.second;

    return nullptr;
}
