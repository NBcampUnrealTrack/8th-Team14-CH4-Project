#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetBase/ClosableWidgetBase.h"
#include "ES_LobbyWidgetHUD.generated.h"

class AMyPlayerController;
class ATestGameState;
class APlayerCharacterState;
class UTextBlock;
class UES_LobbyUserIconWidget;
class UHorizontalBox;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_LobbyWidgetHUD : public UClosableWidgetBase
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable)
	void OnClickedReadyButton();
	UFUNCTION(BlueprintCallable)
	void OnClickedShopButton();
	
	UFUNCTION()
	void SetLobbyCountPlayer(const int32 InNewPlayer);
	
	UFUNCTION()
	void RefreshPlayerInfo();
	
	UFUNCTION()
	void RefreshPlayerUserIcon();
	
	void ApplyTextColorAnimation(float LeftTime);
	
	void CachedPlayerState(APlayerCharacterState* InPlayerState);
	void CachedGameState(ATestGameState* InTestGameState);
	void CachedPlayerController(AMyPlayerController* InPlayerController);
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LobbyCountPlayer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNickName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LobbyTimerText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PlayerIconHorizontal;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> LobbyTotalGold;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> ShopButtonWidget;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> ReadyButtonWidget;
	
	UPROPERTY(EditDefaultsOnly, blueprintReadOnly, Category = "Lobby|Icon")
	TSubclassOf<UES_LobbyUserIconWidget> UserIconClass;
	
	UPROPERTY()
	TObjectPtr<APlayerCharacterState> OwningPlayerState;
	UPROPERTY()
	TObjectPtr<AMyPlayerController> OwningPlayerController;
	UPROPERTY()
	TObjectPtr<ATestGameState> OwningGameState;
	
	FDelegateHandle PlayerStateChangedHandle;
	FDelegateHandle PlayerinfoChangedHandle;
	FDelegateHandle PhaseTimerChangedHandle;
	
	FTimerHandle PhaseTimerHandle;
	void HandlePhaseTimerChanged();
	void UpdateTimerText();
};
