#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnBuyItemResult,
	FName, ItemName,
	bool, Result,
	int32, StageInventoryIndex
);

struct FInventoryItemData;
class UES_ResaultWidgetHUD;
class UES_LobbyWidgetHUD;
class UES_OptionWidget;
class UInputMappingContext;
class UUserWidget;
class APlayerCharacterState;
class UESHUDWidget;
class USoundBase;
class UAudioComponent;
class UEscapeProgressWidget;

UCLASS()
class EXTRACTIONSHOOTER14_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnBuyItemResult OnReceiveBuyItemResult;
protected:
	AMyPlayerController();
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> MappingContextList;
public:
	void Find_ApplyLobbyCamera();
	void InitLobbyWidgetandCachedPlayerState();
	
	void StopPlayerMovement();
	void StartPlayerMovement();
	UFUNCTION(Client, Reliable)
	void Client_ApplySpawnRotation(FRotator Rot);
	
	UFUNCTION(BlueprintCallable)
	void SetAutoManageCamera(bool bEnable);
	
	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(Client, Reliable)
	void Client_WhenMoveToStagePopUpUI();
	UFUNCTION(Client, Reliable)
	void Client_WhenMoveToLobbyUI();
	UFUNCTION(Server, Reliable)
	void Server_MoveToLobbyLocation();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_OnClickedResultUI();
	UFUNCTION()
	void OnPlayerHealthChanged(float CurrentHP, float MaxHP);
	
	UFUNCTION(Client, Reliable)
	void Client_OnReceiveBuyItemResult(const FName& BuyItemID, bool Result, int32 InventoryIndex);
	
	// UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	// void PlayLobbyBGM();
	// UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	// void StopBGM(float FadeTime = 0.5f);
	// UFUNCTION(Client, Reliable)
	// void Client_StopBGM();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "HUD")
	void Client_ShowHitMarker(bool bCritical);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void BP_ShowHitMarker(bool bCritical);
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Result")
	TSubclassOf<UES_ResaultWidgetHUD> ResultWidgetClass;
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Result")
	// TObjectPtr<UUserWidget> ResultWidgetInstance;
	
	UFUNCTION(Client, Reliable)
	void Client_OpenResultUI();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Option")
	TSubclassOf<UES_OptionWidget> OptionWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Option")
	TObjectPtr<UES_OptionWidget> OptionWidgetInstance;

	//UES_OptionWidget
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> WidgetStack;
	UFUNCTION(BlueprintCallable)
	void PushWidget(UUserWidget* NewWidget);
	UFUNCTION()
	void OpenOptionUI();
	UFUNCTION()
	void CloseOptionUI();
	UFUNCTION(BlueprintCallable)
	void OnPressedEscape();
	void FocusWidget(UUserWidget* Widget);
	void UpdateInputMode();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Lobby")
	TSubclassOf<UES_LobbyWidgetHUD> LobbyWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Lobby")
	TObjectPtr<UES_LobbyWidgetHUD> LobbyWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Stage")
	TSubclassOf<UESHUDWidget> StageWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Stage")
	TObjectPtr<UESHUDWidget> StageWidgetInstance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Stage")
	TSubclassOf<UEscapeProgressWidget> EscapeProgressWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Stage")
	TObjectPtr<UEscapeProgressWidget> EscapeProgressWidgetInstance;
	UFUNCTION(Client, Reliable)
	void Client_ShowEscapeProgressWidget(float StartTime, float Duration);
	UFUNCTION(Client, Reliable)
	void Client_CloseEscapeProgressWidget();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> SettingWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> SettingWidgetInstance;
	void MakeSettingWidget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> ShopWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> ShopWidgetInstance;
	void ToggleShopWidget();
	void CloseShopWidget();
	
	void ClearWidgetStack();
	int32 BaseUIZOrder = 1;
#pragma region Cheat
	UFUNCTION(Exec)
	void SetCheatCoin(int32 InCheatCoin);
	UFUNCTION(Exec)
	void SetCheatStageCoin(int32 InCheatCoin);
	
	UFUNCTION(Exec)
	void SetCheatItem(FName ItemName, int32 ItemCount);
	UFUNCTION(Exec)
	void SetCheatStageItem(FName ItemName, int32 ItemCount);
#pragma endregion 

protected:
	// void PlayBGM(USoundBase* NewBGM, float Volume = 0.3f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<USoundBase> LobbyBGM;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<USoundBase> StageBGM;
	//
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	// float LobbyBGMVolume = 0.3f;
	//
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	// float BGMFadeTime = 0.5f;
	//
	// UPROPERTY(Transient)
	// TObjectPtr<UAudioComponent> CurrentBGMComponent;
	//
	// UPROPERTY(Transient)
	// TObjectPtr<USoundBase> CurrentBGMSound;
};
