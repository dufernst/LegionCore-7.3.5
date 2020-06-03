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

#ifndef BankPackets_h__
#define BankPackets_h__

#include "ItemPackets.h"
#include "Packet.h"
#include "ObjectGuid.h"
#include "WorldSession.h"

namespace WorldPackets
{
    namespace Bank
    {
        class AutoBankItem final : public ClientPacket
        {
        public:
            AutoBankItem(WorldPacket&& packet) : ClientPacket(CMSG_AUTOBANK_ITEM, std::move(packet)) { }

            void Read() override;

            Item::InvUpdate Inv;
            uint8 Bag = 0;
            uint8 Slot = 0;
        };

        class AutoStoreBankItem final : public ClientPacket
        {
        public:
            AutoStoreBankItem(WorldPacket&& packet) : ClientPacket(CMSG_AUTOSTORE_BANK_ITEM, std::move(packet)) { }

            void Read() override;

            Item::InvUpdate Inv;
            uint8 Bag = 0;
            uint8 Slot = 0;
        };

        class BuyBankSlot final : public ClientPacket
        {
        public:
            BuyBankSlot(WorldPacket&& packet) : ClientPacket(CMSG_BUY_BANK_SLOT, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
        };
        
        class AutostoreBankReagent final : public ClientPacket
        {
        public:
            AutostoreBankReagent(WorldPacket&& packet) : ClientPacket(CMSG_AUTOSTORE_BANK_REAGENT, std::move(packet)) { }

            void Read() override;

            Item::InvUpdate Inv;
            uint8 Bag = 0;
            uint8 Slot = 0;
        };

        class BuyReagentBank final : public ClientPacket
        {
        public:
            BuyReagentBank(WorldPacket&& packet) : ClientPacket(CMSG_BUY_REAGENT_BANK, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
        };

        class DepositReagentBank final : public ClientPacket
        {
        public:
            DepositReagentBank(WorldPacket&& packet) : ClientPacket(CMSG_DEPOSIT_REAGENT_BANK, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
        };
    }
}

#endif // BankPackets_h__
