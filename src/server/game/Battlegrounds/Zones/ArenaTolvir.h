
#ifndef __BATTLEGROUNDTV_H
#define __BATTLEGROUNDTV_H

#include "Arena.h"

class ArenaTolvir : public Arena
{
public:
    ArenaTolvir();
    ~ArenaTolvir();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};
#endif
