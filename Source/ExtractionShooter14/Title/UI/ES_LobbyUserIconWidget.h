#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_LobbyUserIconWidget.generated.h"

class APlayerCharacterState;
class UImage;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_LobbyUserIconWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void RefreshReadyIcon(uint8 bisready);
	
	void CachedPlayerCharacterState(APlayerCharacterState* PlayerCharacterState);
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> UserIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ReadyIcon;
	
	UPROPERTY()
	TObjectPtr<APlayerCharacterState> OwningPlayerState;
};
