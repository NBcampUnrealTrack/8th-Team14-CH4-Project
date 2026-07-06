#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_LobbyWidgetHUD.generated.h"

class AMyPlayerController;
class ATestGameState;
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
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
	UFUNCTION(BlueprintCallable)
	void OnClickedReadyButton();
	
	UFUNCTION()
	void SetLobbyCountPlayer(const int32 InNewPlayer);
	
	UFUNCTION()
	void RefreshPlayerInfo();
	
	UFUNCTION()
	void RefreshPlayerUserIcon();
	UFUNCTION()
	void AddUserIcon();
	
	void ApplyTextColorAnimation(float LeftTime);
	
	void CachedPlayerState(APlayerCharacterState* InPlayerState);
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
