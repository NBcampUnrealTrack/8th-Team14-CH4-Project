#pragma once

#include "CoreMinimal.h"
#include "Struct/PlayerDatas.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ES_GameInstanceSubsystem.generated.h"

class UES_SaveGame;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_GameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool FindPlayerData(const FString& PlayerGuid, FPlayerDatas& OutData) const;
	void SavePlayerData(const FString& PlayerGuid, const FPlayerDatas& Data);
	FPlayerDatas& FindOrAddPlayerData(const FString& PlayerGuid);

private:
	UPROPERTY()
	TObjectPtr<UES_SaveGame> CachedSaveGame;

	UPROPERTY()
	FString SaveSlotName = TEXT("PlayerGuidList");

	UPROPERTY()
	int32 SaveUserIndex = 0;

	void LoadOrCreateSaveGame();
};
