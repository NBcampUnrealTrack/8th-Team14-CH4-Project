#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_LobbyUserIconWidget.generated.h"

class UImage;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_LobbyUserIconWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> UserIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ReadyIcon;
};
