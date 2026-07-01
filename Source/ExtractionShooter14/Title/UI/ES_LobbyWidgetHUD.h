#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_LobbyWidgetHUD.generated.h"

class APlayerCharacterState;
class UTextBlock;
class UES_LobbyUserIconWidget;
class UHorizontalBox;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_LobbyWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable)
	void OnClickedReadyButton();
	
	UFUNCTION()
	void SetLobbyCountPlayer(const int32 InNewPlayer);
	UFUNCTION()
	void AddUserIcon();
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LobbyCountPlayer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PlayerIconHorizontal;
	UPROPERTY(EditDefaultsOnly, blueprintReadOnly, Category = "Lobby|Icon")
	TSubclassOf<UES_LobbyUserIconWidget> UserIconClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Icon")
	TObjectPtr<UES_LobbyUserIconWidget> UserIconInstance;
	
	UPROPERTY()
	TObjectPtr<APlayerCharacterState> OwningPlayerState;
};
