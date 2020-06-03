
#ifndef __BATTLEGROUNDVsh_H
#define __BATTLEGROUNDVsh_H

#include "Arena.h"

class ArenaAshamanesFall : public Arena
{
public:
    ArenaAshamanesFall();
    ~ArenaAshamanesFall();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};

#endif
