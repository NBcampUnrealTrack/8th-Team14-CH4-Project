// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerCharacterState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"

void AMainPlayerCharacterState::OnRep_Health()
{
}

void AMainPlayerCharacterState::ApplyDamage(float DamageAmount)
{
    if (!HasAuthority()) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
        {
            PlayerCharacter->Die();
        }
    }
}

void AMainPlayerCharacterState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMainPlayerCharacterState, CurrentHealth);
}
