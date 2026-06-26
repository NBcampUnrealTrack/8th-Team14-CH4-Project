// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h" 
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    
    Character = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    
    if (Character == nullptr)
    {
        Character = Cast<APlayerCharacter>(TryGetPawnOwner());
    }

    
    if (Character == nullptr) return;

   
    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.f; 
    Speed = Velocity.Size();

    
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        bIsInAir = MovementComp->IsFalling();
    }
}