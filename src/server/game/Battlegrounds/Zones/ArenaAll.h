
#ifndef __BATTLEGROUNDAA_H
#define __BATTLEGROUNDAA_H

class Battleground;

class ArenaAll : public Battleground
{
public:
    ArenaAll();
    ~ArenaAll();

    void AddPlayer(Player* player) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;

    void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;
    bool SetupBattleground() override;
    void HandleKillPlayer(Player* player, Player* killer) override;
};

#endif
