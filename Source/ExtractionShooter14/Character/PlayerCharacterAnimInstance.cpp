// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    
    Character = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (Character == nullptr)
    {
        Character = Cast<APlayerCharacter>(TryGetPawnOwner());
    }

    if (Character == nullptr) return;

    
    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();

    
    FRotator AimRotation = Character->GetBaseAimRotation();

    

    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetBaseAimRotation());

    
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        bIsInAir = MovementComp->IsFalling();
    }

    if (Direction >= -45.0f && Direction <= 45.0f)
    {
        CustomCardinalDirection = ECharacterMovementDirection::Forward;
    }
    else if (Direction > 45.0f && Direction <= 135.0f)
    {
        CustomCardinalDirection = ECharacterMovementDirection::Right;
    }
    else if (Direction >= -135.0f && Direction < -45.0f)
    {
        CustomCardinalDirection = ECharacterMovementDirection::Left;
    }
    else
    {
        CustomCardinalDirection = ECharacterMovementDirection::Backward;
    }
}