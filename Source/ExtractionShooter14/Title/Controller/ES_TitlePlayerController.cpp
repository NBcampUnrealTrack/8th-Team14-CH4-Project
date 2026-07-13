// Fill out your copyright notice in the Description page of Project Settings.

#include "ES_TitlePlayerController.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void AES_TitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (!IsValid(TitleBGM))
	{
		return;
	}

	TitleBGMComponent = UGameplayStatics::SpawnSound2D(
		this,
		TitleBGM,
		TitleBGMVolume,
		1.0f,
		0.0f,
		nullptr,
		false,
		true
	);
}