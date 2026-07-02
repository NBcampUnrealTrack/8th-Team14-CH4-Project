// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UES_LobbyWidgetHUD;
class UInputMappingContext;
class UUserWidget;

UCLASS()
class EXTRACTIONSHOOTER14_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void OnUnPossess() override;
	virtual void OnPossess(APawn* InPawn) override;
protected:
	AMyPlayerController();
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
public:
	void Find_ApplyLobbyCamera();
	void InitLobbyWidgetandCachedPlayerState();
	
	UFUNCTION(BlueprintCallable)
	void SetAutoManageCamera(bool bEnable);
	
	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(Client, Reliable)
	void Client_WhenMoveToStagePopUpUI();
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Result")
	TSubclassOf<UUserWidget> ResultWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Result")
	TObjectPtr<UUserWidget> ResultWidgetInstance;
	
	UFUNCTION(Client, Reliable)
	void Client_OpenResultUI();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Option")
	TSubclassOf<UUserWidget> OptionWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Option")
	TObjectPtr<UUserWidget> OptionWidgetInstance;
	
	UPROPERTY()
	uint8 IsOpenOptionUI : 1;
	UFUNCTION()
	void OpenOptionUI();
	UFUNCTION()
	void CloseOptionUI();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Lobby")
	TSubclassOf<UES_LobbyWidgetHUD> LobbyWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Lobby")
	TObjectPtr<UES_LobbyWidgetHUD> LobbyWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Stage")
	TSubclassOf<UUserWidget> StageWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Stage")
	TObjectPtr<UUserWidget> StageWidgetInstance;
};
