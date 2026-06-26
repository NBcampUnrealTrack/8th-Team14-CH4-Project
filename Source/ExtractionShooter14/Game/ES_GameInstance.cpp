#include "ES_GameInstance.h"

#include "ES_SaveGame.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"

void UES_GameInstance::Init()
{
	Super::Init();
	
	LoadOrCreateSaveGame();
}

void UES_GameInstance::LoadOrCreateSaveGame()
{
	// find file :: name = playerguidlist, 0
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex))
	{
		CachedSaveGame = Cast<UES_SaveGame>(Loaded);
	}
	// no find == no file == create new
	if (!CachedSaveGame)
	{
		// make new and cached pointer
		CachedSaveGame = Cast<UES_SaveGame>(
			UGameplayStatics::CreateSaveGameObject(UES_SaveGame::StaticClass())
		);
		// save file(cachedsavegame, name = playerguidlist, 0)
		UGameplayStatics::SaveGameToSlot(CachedSaveGame, SaveSlotName, SaveUserIndex);
	}
}

bool UES_GameInstance::FindPlayerData(const FGuid& PlayerGuid, FPlayerDatas& OutPlayerData) const
{
	if (!CachedSaveGame || !PlayerGuid.IsValid())
	{
		return false;
	}

	if (const FPlayerDatas* FoundData = CachedSaveGame->PlayerDataMap.Find(PlayerGuid))
	{
		OutPlayerData = *FoundData;
		return true;
	}

	return false;
}

void UES_GameInstance::SavePlayerData(const FGuid& PlayerGuid, const FPlayerDatas& PlayerData)
{
	if (!CachedSaveGame || !PlayerGuid.IsValid())
	{
		return;
	}

	CachedSaveGame->PlayerDataMap.Add(PlayerGuid, PlayerData);

	UGameplayStatics::SaveGameToSlot(CachedSaveGame, SaveSlotName, SaveUserIndex);
}

FPlayerDatas& UES_GameInstance::FindOrAddPlayerData(const FGuid& PlayerGuid)
{
	check(CachedSaveGame);
	check(PlayerGuid.IsValid());

	return CachedSaveGame->PlayerDataMap.FindOrAdd(PlayerGuid);
}