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

    if (Character == nullptr)
    {
        Character = Cast<APlayerCharacter>(TryGetPawnOwner());
    }

    if (Character == nullptr) return;

    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.f;

    PreviousSpeed = Speed;
    Speed = Velocity.Size();

    AnimSpeed = FMath::FInterpTo(
        AnimSpeed,
        Speed,
        DeltaSeconds,
        SpeedInterpRate
    );

    if (Speed > 10.0f)
    {
        LocomotionDirection = Velocity.GetSafeNormal();
    }

    const bool bIsMovingNow = Speed > 10.0f;
    bShouldPlayStop = bWasMovingLastFrame && !bIsMovingNow;
    bWasMovingLastFrame = bIsMovingNow;

    Direction = UKismetAnimationLibrary::CalculateDirection(
        Velocity,
        Character->GetActorRotation()
    );

    LocomotionAngle = Direction;

    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        bIsInAir = MovementComp->IsFalling();
    }

    CustomCardinalDirection = Character->LastInputDirection;
}

