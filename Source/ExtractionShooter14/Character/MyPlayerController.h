#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBuyItemResult,
	FName, ItemName,
	bool, Result
);

struct FInventoryItemData;
class UES_ResaultWidgetHUD;
class UES_LobbyWidgetHUD;
class UInputMappingContext;
class UUserWidget;
class APlayerCharacterState;
class UUserWidget;
class UESHUDWidget;
class USoundBase;
class UAudioComponent;

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
	void Client_OnReceiveBuyItemResult(const FName& BuyItemID, bool Result);
	
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void PlayLobbyBGM();
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void StopBGM(float FadeTime = 0.5f);
	UFUNCTION(Client, Reliable)
	void Client_StopBGM();

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
	TSubclassOf<UUserWidget> OptionWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Option")
	TObjectPtr<UUserWidget> OptionWidgetInstance;
	
	// UPROPERTY()
	// uint8 IsOpenOptionUI : 1;
	UFUNCTION(BlueprintCallable)
	void OpenOptionUI();
	// UFUNCTION()
	// void CloseOptionUI();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Lobby")
	TSubclassOf<UES_LobbyWidgetHUD> LobbyWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Lobby")
	TObjectPtr<UES_LobbyWidgetHUD> LobbyWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Stage")
	TSubclassOf<UESHUDWidget> StageWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Stage")
	TObjectPtr<UESHUDWidget> StageWidgetInstance;
	
	
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
	void PlayBGM(USoundBase* NewBGM, float Volume = 0.3f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<USoundBase> LobbyBGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	float LobbyBGMVolume = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	float BGMFadeTime = 0.5f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CurrentBGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentBGMSound;
};
