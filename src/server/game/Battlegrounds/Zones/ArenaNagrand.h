#ifndef __BATTLEGROUNDNA_H
#define __BATTLEGROUNDNA_H

#include "Arena.h"

class ArenaNagrandArena : public Arena
{
public:
    ArenaNagrandArena();
    ~ArenaNagrandArena();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};

#endif
