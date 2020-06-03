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

#ifndef SystemPackets_h__
#define SystemPackets_h__

#include "Packet.h"

namespace WorldPackets
{
    namespace System
    {
        class FeatureSystemStatus final : public ServerPacket
        {
        public:
            struct SavedThrottleObjectState
            {
                uint32 MaxTries = 0;
                uint32 PerMilliseconds = 0;
                uint32 TryCount = 0;
                uint32 LastResetTimeBeforeNow = 0;
            };

            struct EuropaTicketConfig
            {
                SavedThrottleObjectState ThrottleState;
                bool TicketsEnabled = false;
                bool BugsEnabled = false;
                bool ComplaintsEnabled = false;
                bool SuggestionsEnabled = false;
            };

            struct SessionAlertConfig
            {
                int32 Delay = 0;
                int32 Period = 0;
                int32 DisplayTime = 0;
            };

            struct SocialQueueConfig
            {
                float DelayDuration = 0.0f;
                float PlayerFriendValue = 0.0f;
                float PlayerGuildValue = 0.0f;
                float PlayerMultiplier = 0.0f;
                float QueueMultiplier = 0.0f;
                float ThrottleDecayTime = 0.0f;
                float ThrottleDfBestPriority = 0.0f;
                float ThrottleDfMaxItemLevel = 0.0f;
                float ThrottleInitialThreshold = 0.0f;
                float ThrottleLfgListIlvlScalingAbove = 0.0f;
                float ThrottleLfgListIlvlScalingBelow = 0.0f;
                float ThrottleLfgListPriorityAbove = 0.0f;
                float ThrottleLfgListPriorityBelow = 0.0f;
                float ThrottleLfgListPriorityDefault = 0.0f;
                float ThrottleMinThreshold = 0.0f;
                float ThrottlePrioritySpike = 0.0f;
                float ThrottlePvPHonorThreshold = 0.0f;
                float ThrottlePvPPriorityLow = 0.0f;
                float ThrottlePvPPriorityNormal = 0.0f;
                float ThrottleRfIlvlScalingAbove = 0.0f;
                float ThrottleRfPriorityAbove = 0.0f;
                float ToastDuration = 0.0f;
                bool ToastsDisabled = false;
            };

            FeatureSystemStatus() : ServerPacket(SMSG_FEATURE_SYSTEM_STATUS, 48) { }

            WorldPacket const* Write() override;

            Optional<SessionAlertConfig> SessionAlert;
            Optional<EuropaTicketConfig> EuropaTicketSystemStatus;
            Optional<std::vector<uint8>> RaceClassExpansionLevels;
            SocialQueueConfig QuickJoinConfig;
            uint64 TokenBalanceAmount = 0;
            uint32 BpayStoreProductDeliveryDelay = 0;
            uint32 CfgRealmID = 0;
            uint32 ScrollOfResurrectionMaxRequestsPerDay = 0;
            uint32 ScrollOfResurrectionRequestsRemaining = 0;
            int32 CfgRealmRecID = 0;
            int32 TokenPollTimeSeconds = 0;
            int32 TokenRedeemIndex = 0;
            int32 TwitterPostThrottleCooldown = 0;
            int32 TwitterPostThrottleLimit = 0;
            uint8 ComplaintStatus = 0;
            bool BpayStoreAvailable = false;
            bool BpayStoreDisabledByParentalControls = false;
            bool BpayStoreEnabled = false;
            bool BrowserEnabled = false;
            bool CharUndeleteEnabled = false;
            bool CommerceSystemEnabled = false;
            bool CompetitiveModeEnabled = false;
            bool ItemRestorationButtonEnabled = false;
            bool KioskModeEnabled = false;
            bool LiveRegionAccountCopyEnabled = false;
            bool LiveRegionCharacterCopyEnabled = false;
            bool LiveRegionCharacterListEnabled = false;
            bool NPETutorialsEnabled = false;
            bool RecruitAFriendSendingEnabled = false;
            bool RestrictedAccount = false;
            bool ScrollOfResurrectionEnabled = false;
            bool TokenBalanceEnabled = false;
            bool TutorialsEnabled = false;
            bool TwitterEnabled = false;
            bool Unk67 = false;
            bool VoiceEnabled = false;
            bool WillKickFromWorld = false;
        };

        class FeatureSystemStatusGlueScreen final : public ServerPacket
        {
        public:
            FeatureSystemStatusGlueScreen() : ServerPacket(SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN, 1) { }

            WorldPacket const* Write() override;

            int64 TokenBalanceAmount = 0;
            int32 TokenPollTimeSeconds = 0;
            int32 TokenRedeemIndex = 0;
            uint32 BpayStoreProductDeliveryDelay = 0;
            uint32 UnkInt1 = 0;
            uint32 UnkInt2 = 0;
            uint32 UnkInt3 = 0;
            uint32 UnkInt4 = 0;
            bool BpayStoreAvailable = false;
            bool BpayStoreDisabledByParentalControls = false;
            bool CharUndeleteEnabled = false;
            bool BpayStoreEnabled = false;
            bool CommerceSystemEnabled = false;
            bool Unk14 = false;
            bool WillKickFromWorld = false;
            bool IsExpansionPreorderInStore = false;
            bool KioskModeEnabled = false;
            bool CompetitiveModeEnabled = false;
            bool TrialBoostEnabled = false;
            bool TokenBalanceEnabled = false;
            bool LiveRegionCharacterListEnabled = false;
            bool LiveRegionCharacterCopyEnabled = false;
            bool LiveRegionAccountCopyEnabled = false;
        };

        class MOTD final : public ServerPacket
        {
        public:
            MOTD() : ServerPacket(SMSG_MOTD) { }

            WorldPacket const* Write() override;

            StringVector const* Text = nullptr;
        };

        class SetTimeZoneInformation final : public ServerPacket
        {
        public:
            SetTimeZoneInformation() : ServerPacket(SMSG_SET_TIME_ZONE_INFORMATION) { }

            WorldPacket const* Write() override;

            std::string ServerTimeTZ;
            std::string GameTimeTZ;
        };
    }
}

#endif // SystemPackets_h__
