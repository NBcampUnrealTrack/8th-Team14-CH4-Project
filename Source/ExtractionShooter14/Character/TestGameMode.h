#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	ATestGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
