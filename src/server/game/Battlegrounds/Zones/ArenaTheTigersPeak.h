
#ifndef __BATTLEGROUNDTTP_H
#define __BATTLEGROUNDTTP_H

#include "Arena.h"


class ArenaTheTigersPeak : public Arena
{
public:
    ArenaTheTigersPeak();
    ~ArenaTheTigersPeak();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};

#endif
