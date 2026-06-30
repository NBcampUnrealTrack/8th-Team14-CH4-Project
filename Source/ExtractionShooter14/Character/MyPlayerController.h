// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

UCLASS()
class EXTRACTIONSHOOTER14_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	AMyPlayerController();
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
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
};
