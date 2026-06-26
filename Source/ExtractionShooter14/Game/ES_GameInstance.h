#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Struct/PlayerDatas.h"
#include "ES_GameInstance.generated.h"

class UES_SaveGame;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_GameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;
public:
	UPROPERTY()
	UES_SaveGame* CachedSaveGame;

public:
	void LoadOrCreateSaveGame();

	bool FindPlayerData(const FGuid& PlayerGuid, FPlayerDatas& OutPlayerData) const;

	void SavePlayerData(const FGuid& PlayerGuid, const FPlayerDatas& PlayerData);

	FPlayerDatas& FindOrAddPlayerData(const FGuid& PlayerGuid);

private:
	const FString SaveSlotName = TEXT("PlayerGuidList");
	const int32 SaveUserIndex = 0;
};
