
#ifndef _sCharService
#define _sCharService

class CharacterService
{
	CharacterService() = default;
	~CharacterService() = default;
	
public:
    void SetRename(Player* player);
    void ChangeFaction(Player* player);
    void ChangeRace(Player* player);
    void Customize(Player* player);
    void Boost(Player* player);
    void RestoreDeletedCharacter(WorldSession* session);

	static CharacterService* instance();
};

#define sCharacterService CharacterService::instance()

#endif