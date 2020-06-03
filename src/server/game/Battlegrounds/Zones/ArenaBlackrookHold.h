
#ifndef __BATTLEGROUNDBrh_H
#define __BATTLEGROUNDBrh_H

#include "Arena.h"

class ArenaBlackrookHold : public Arena
{
public:
    ArenaBlackrookHold();
    ~ArenaBlackrookHold();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};

#endif
