#include "ES_GameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "ExtractionShooter14/Game/ES_SaveGame.h"

void UES_GameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	LoadOrCreateSaveGame();
}

void UES_GameInstanceSubsystem::LoadOrCreateSaveGame()
{
	// find file :: name = playerguidlist, 0
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex))
	{
		CachedSaveGame = Cast<UES_SaveGame>(Loaded);
	}
	// no savefile find == no file == create new
	if (!CachedSaveGame)
	{
		// make new and cached pointer
		CachedSaveGame = Cast<UES_SaveGame>(
			UGameplayStatics::CreateSaveGameObject(UES_SaveGame::StaticClass())
		);
		// save file(cachedsavegame, name = playerguidlist, 0) PC = 0, XBox는 달라질수도
		UGameplayStatics::SaveGameToSlot(CachedSaveGame, SaveSlotName, SaveUserIndex);
	}
}

// input = PlayerGuid, output = OutPlayerData
bool UES_GameInstanceSubsystem::FindPlayerData(const FString& PlayerGuid, FPlayerDatas& OutPlayerData) const
{
	if (!CachedSaveGame)
	{
		return false;
	}

	if (const FPlayerDatas* FoundData = CachedSaveGame->PlayerDataMap.Find(PlayerGuid))
	{
		// return Data
		OutPlayerData = *FoundData;
		return true;
	}

	return false;
}

// when logout save data
void UES_GameInstanceSubsystem::SavePlayerData(const FString& PlayerGuid, const FPlayerDatas& PlayerData)
{
	if (!CachedSaveGame)
	{
		return;
	}
	
	UE_LOG(
	LogTemp,
	Warning,
	TEXT("Saving PlayerData: %s"),
	*PlayerGuid
);
	
	CachedSaveGame->PlayerDataMap.Add(PlayerGuid, PlayerData);

	UGameplayStatics::SaveGameToSlot(CachedSaveGame, SaveSlotName, SaveUserIndex);
}

FPlayerDatas& UES_GameInstanceSubsystem::FindOrAddPlayerData(const FString& PlayerGuid)
{
	check(CachedSaveGame);
	//check(PlayerGuid.IsValid());

	return CachedSaveGame->PlayerDataMap.FindOrAdd(PlayerGuid);
}
