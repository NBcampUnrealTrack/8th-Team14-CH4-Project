// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ES_TitlePlayerController.generated.h"

class USoundBase;
class UAudioComponent;

UCLASS()
class EXTRACTIONSHOOTER14_API AES_TitlePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<USoundBase> TitleBGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	float TitleBGMVolume = 0.3f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> TitleBGMComponent;
};