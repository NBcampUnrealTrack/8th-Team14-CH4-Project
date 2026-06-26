// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGameMode.h"
#include "PlayerCharacter.h"
#include "MyPlayerController.h"

ATestGameMode::ATestGameMode()
{
    DefaultPawnClass = APlayerCharacter::StaticClass();

    
    PlayerControllerClass = AMyPlayerController::StaticClass();
}
