/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#ifndef _WARDEN_WIN_H
#define _WARDEN_WIN_H

#include <map>
#include "ARC4.h"
#include "BigNumber.h"
#include "ByteBuffer.h"
#include "Warden.h"

class WorldSession;
class Warden;

class WardenWin : public Warden
{
    public:
        WardenWin(WorldSession* session);
        ~WardenWin() {}

        void InitializeModule();
        void InitializeMPQCheckFunc(ByteBuffer& buff);
        void InitializeLuaCheckFunc(ByteBuffer& buff);
        void InitializeTimeCheckFunc(ByteBuffer& buff);

        void HandleHashResult(ByteBuffer &buff) override;
        void HandleHashResultSpecial(ByteBuffer &buff) override;
        void HandleModuleFailed() override;
        void RequestBaseData() override;
        void SendExtendedData() override;

        void HandleData(ByteBuffer &buff) override;
        void HandleChecks(ByteBuffer &buff);

        void HandleExtendedData(ByteBuffer &buff) override;
        void HandleStringData(ByteBuffer &buff) override;
        void HandleFailedSync(uint32 clientSeqIndex);

        // temp
        void ActivateModule() override;

        void BuildBaseChecksRequest(ByteBuffer &buff);
        void BuildSequenceHeader(ByteBuffer &buff);
        void AddCheckData(uint16 id, ByteBuffer &buff, ByteBuffer &stringBuf, uint8 &index);

        template<class T>
        void WriteAddress(ByteBuffer & buff, T address);

    private:
        std::map<std::string, std::vector<uint16>> _baseChecksList;
        std::vector<uint16> _currentChecks;

        uint32 _sequencePacketIndex;
};

#endif
