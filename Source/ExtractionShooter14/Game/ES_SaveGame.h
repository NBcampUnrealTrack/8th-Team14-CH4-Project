#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Struct/PlayerDatas.h"
#include "ES_SaveGame.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API UES_SaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TMap<FString, FPlayerDatas> PlayerDataMap;
};
