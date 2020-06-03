#ifndef __ArenaBladesEdge_H
#define __ArenaBladesEdge_H

#include "Arena.h"

class ArenaBladesEdge : public Arena
{
public:
    ArenaBladesEdge();
    ~ArenaBladesEdge();

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
};

#endif
