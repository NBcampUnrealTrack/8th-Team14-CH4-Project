// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerCharacterState.h"
#include "PlayerCharacterState.h"


APlayerCharacter::APlayerCharacter()
{
 	
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    if (GetMesh())
    {
        GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
        GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

        
        GetMesh()->SetupAttachment(RootComponent);
    }

    
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;      
    CameraBoom->bUsePawnControlRotation = true; 
    
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; 

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }

    bIsSprinting = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (CameraBoom)
    {
        DefaultArmLength = CameraBoom->TargetArmLength;
        DefaultSocketOffset = CameraBoom->SocketOffset;

        if (AimCameraCurve)
        {
            FOnTimelineFloat TimelineProgress;
            TimelineProgress.BindUFunction(this, FName("UpdateAimCamera"));
            AimCameraTimeline.AddInterpFloat(AimCameraCurve, TimelineProgress);

            FOnTimelineEvent TimelineFinished;
            TimelineFinished.BindUFunction(this, FName("FinishAimCamera"));
            AimCameraTimeline.SetTimelineFinishedFunc(TimelineFinished);

            AimCameraTimeline.SetLooping(false);
        }
    }

    ApplyWeaponVisuals();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        }

        
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
        }

        if (JumpAction)
        {
            
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        
        if (SprintAction)
        {
            
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
            
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
        }
        
        if (OptionAction)
        {
            EnhancedInputComponent->BindAction(OptionAction, ETriggerEvent::Started, this, &ThisClass::OpenOptionUI);
        }
        
        if (AimAction)
        {
            EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerCharacter::StartAim);

            EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopAim);
        }

        if (FireAction)
        {
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerCharacter::Fire);
        }
    }

}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        
        AddMovementInput(ForwardDirection, MovementVector.X);
        AddMovementInput(RightDirection, MovementVector.Y);
    }

    if (FMath::Abs(MovementVector.X) > 0.1f || FMath::Abs(MovementVector.Y) > 0.1f)
    {
        if (FMath::Abs(MovementVector.X) >= FMath::Abs(MovementVector.Y))
        {
            LastInputDirection = MovementVector.X > 0.f
                ? ECharacterMovementDirection::Forward
                : ECharacterMovementDirection::Backward;
        }
        else
        {
            LastInputDirection = MovementVector.Y > 0.f
                ? ECharacterMovementDirection::Right
                : ECharacterMovementDirection::Left;
        }
    }
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        
        AddControllerYawInput(LookAxisVector.X);

        
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void APlayerCharacter::StartSprint(const FInputActionValue& Value)
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = 600.f;
    }

    if (IsLocallyControlled())
    {
        ServerStartSprint();
    }
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = 300.f;
    }

    if (IsLocallyControlled())
    {
        ServerStopSprint();
    }
}

void APlayerCharacter::OpenOptionUI(const FInputActionValue& Value)
{
    AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(GetController());
    if (!IsValid(PlayerControllerES)) return;
    
    if (PlayerControllerES->IsOpenOptionUI)
    {
        PlayerControllerES->CloseOptionUI();
    }
    else
    {
        PlayerControllerES->OpenOptionUI();
    }
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (InputComponent)
    {
        SetupPlayerInputComponent(InputComponent);
    }
    
    BindDieFunctionWithPlayerStateDelegate();
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    BindDieFunctionWithPlayerStateDelegate();
}

void APlayerCharacter::BindDieFunctionWithPlayerStateDelegate()
{
    APlayerCharacterState* PlayerCharacterStateES = Cast<APlayerCharacterState>(GetPlayerState());
    if (!IsValid(PlayerCharacterStateES)) return;
    
    PlayerCharacterStateES->OnPlayerbIsDeadChanged.RemoveDynamic(this, &ThisClass::OnbIsDeadChanged);
    PlayerCharacterStateES->OnPlayerbIsDeadChanged.AddDynamic(this, &ThisClass::OnbIsDeadChanged);
}

void APlayerCharacter::OnbIsDeadChanged(uint8 changedbisdead)
{
    if (changedbisdead)
    {
        Die();
    }
}

void APlayerCharacter::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // run on server and local client
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->bPauseAnims = true;
        MeshComp->SetComponentTickEnabled(false);
    }
    if (HasAuthority())
    { // only do it in server
        AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(GetController());
        if (!IsValid(PlayerControllerES)) return;
        PlayerControllerES->Client_OpenResultUI();
        Destroy();
    }
}

void APlayerCharacter::ServerStartSprint_Implementation()
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        
        MovementComp->MaxWalkSpeed = 600.f;
        bIsSprinting = true;
    }
}


void APlayerCharacter::ServerStopSprint_Implementation()
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        
        MovementComp->MaxWalkSpeed = 300.f;
        bIsSprinting = false;
    }
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerCharacter, CurrentWeaponType);
    DOREPLIFETIME(APlayerCharacter, bIsSprinting);
    //DOREPLIFETIME(APlayerCharacter, bIsDead);
}

void APlayerCharacter::StartAim()
{
    bIsAiming = true;

    SetActorTickEnabled(true);

    if (AimCameraCurve)
    {
        AimCameraTimeline.Play();
    }
    else
    {
        UpdateAimCamera(1.0f);
        SetActorTickEnabled(false);
    }
}

void APlayerCharacter::StopAim()
{
    bIsAiming = false;

    SetActorTickEnabled(true);

    if (AimCameraCurve)
    {
        AimCameraTimeline.Reverse();
    }
    else
    {
        UpdateAimCamera(0.0f);
        SetActorTickEnabled(false);
    }
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AimCameraTimeline.TickTimeline(DeltaSeconds);
}

void APlayerCharacter::UpdateAimCamera(float Alpha)
{
    if (!CameraBoom)
    {
        return;
    }

    CameraBoom->TargetArmLength = FMath::Lerp(
        DefaultArmLength,
        AimArmLength,
        Alpha
    );

    CameraBoom->SocketOffset = FMath::Lerp(
        DefaultSocketOffset,
        AimSocketOffset,
        Alpha
    );
}

void APlayerCharacter::FinishAimCamera()
{
    SetActorTickEnabled(false);
}

void APlayerCharacter::EquipWeapon(EWeaponTypes NewWeaponType)
{
    if (CurrentWeaponType == NewWeaponType) return;

    if (bIsAiming)
    {
        CancelAimInstant();
    }

    if (HasAuthority())
    {
        ServerSetWeaponType_Implementation(NewWeaponType);
    }
    else
    {
        ServerSetWeaponType(NewWeaponType);
    }
}

void APlayerCharacter::ServerSetWeaponType_Implementation(EWeaponTypes NewWeaponType)
{
    if (CurrentWeaponType == NewWeaponType) return;

    bIsAiming = false;

    CurrentWeaponType = NewWeaponType;
    ApplyWeaponVisuals();

    MulticastPlayEquipMontage(NewWeaponType);
}

void APlayerCharacter::OnRep_CurrentWeaponType()
{
    ApplyWeaponVisuals();
}

void APlayerCharacter::MulticastPlayEquipMontage_Implementation(EWeaponTypes NewWeaponType)
{
    UAnimMontage* MontageToPlay = nullptr;

    switch (NewWeaponType)
    {
    case EWeaponTypes::Pistol:
        MontageToPlay = PistolEquipMontage;
        break;

    case EWeaponTypes::Rifle:
        MontageToPlay = RifleEquipMontage;
        break;

    case EWeaponTypes::Shotgun:
        MontageToPlay = RifleEquipMontage;
        break;
    }

    if (MontageToPlay && GetMesh() && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay);
    }
}

void APlayerCharacter::CancelAimInstant()
{
    bIsAiming = false;

    AimCameraTimeline.Stop();
    UpdateAimCamera(0.0f);
    SetActorTickEnabled(false);
}

void APlayerCharacter::Fire()
{
    if (!IsLocallyControlled()) return;

    ServerFire();
}

void APlayerCharacter::ServerFire_Implementation()
{
    AController* OwnerController = GetController();
    if (!OwnerController) return;

    FVector ViewLocation;
    FRotator ViewRotation;
    OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

    const FVector TraceStart = ViewLocation;
    const FVector TraceEnd = TraceStart + ViewRotation.Vector() * FireRange;

    FHitResult HitResult;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    DrawDebugLine(
        GetWorld(),
        TraceStart,
        TraceEnd,
        FColor::Red,
        false,
        2.0f,
        0,
        1.0f
    );

    if (bHit && HitResult.GetActor())
    {
        UGameplayStatics::ApplyDamage(
            HitResult.GetActor(),
            TestDamage,
            OwnerController,
            this,
            UDamageType::StaticClass()
        );
    }
}

void APlayerCharacter::Die()
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->DisableMovement();
        MovementComp->StopMovementImmediately();
    }
    //
    // if (IsLocallyControlled())
    // {
    //     if (AController* MyController = GetController())
    //     {
    //         DisableInput(Cast<APlayerController>(MyController));
    //     }
    // }
    
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (DeathMontage && GetMesh() && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);
        
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::OnDeathMontageEnded);
        GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, DeathMontage);
    }
    
    //if (!HasAuthority()) return;
    // if (bIsDead) return;
    //
    // bIsDead = true; // Server's bIsDead is Changed ?
    //MulticastDie();
    
    // SetLifeSpan(0.f);
    //
    // AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(GetController());
    // if (!IsValid(PlayerControllerES)) return;
    // PlayerControllerES->Client_OpenResultUI();
    //PlayerControllerES->Server_HandleMoveToLobbyCaseBybIsDead();
}

void APlayerCharacter::MulticastDie_Implementation()
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->DisableMovement();
        MovementComp->StopMovementImmediately();
    }

    if (AController* MyController = GetController())
    {
        DisableInput(Cast<APlayerController>(MyController));
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (DeathMontage && GetMesh() && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);
        
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::OnDeathMontageEnded);
        GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, DeathMontage);
    }
}

// void APlayerCharacter::CheckDeath()
// {
//     if (!HasAuthority()) return;
//     if (bIsDead) return;
//
//     APlayerCharacterState* PS = GetPlayerState<APlayerCharacterState>();
//     if (!PS) return;
//
//     if (PS->GetCurrentHealth() <= 0.0f)
//     {
//         
//         Die();
//     }
// }