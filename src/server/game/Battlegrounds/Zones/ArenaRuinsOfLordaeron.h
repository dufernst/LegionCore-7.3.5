
#ifndef __BATTLEGROUNDRl_H
#define __BATTLEGROUNDRl_H

#include "Arena.h"

class ArenaRuinsOfLordaeron : public Arena
{
public:
    ArenaRuinsOfLordaeron();
    ~ArenaRuinsOfLordaeron();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};

#endif
