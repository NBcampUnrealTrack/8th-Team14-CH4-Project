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
#include "Components/CapsuleComponent.h"
#include "MainPlayerCharacterState.h"
#include "ESHUDWidget.h"
#include "PlayerCharacterState.h"
#include "Weapon/WeaponBase.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponType.h"
#include "Weapon/WeaponBase.h"


APlayerCharacter::APlayerCharacter()
{
 	
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    if (GetMesh())
    {
        GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
        GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
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

    bUseControllerRotationYaw = true;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
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

    //ApplyWeaponVisuals();

    if (HasAuthority())
    {
        const FDataTableRowHandle* InitialWeapon =
            WeaponDataMap.Find(CurrentWeaponType);

        if (InitialWeapon && !InitialWeapon->IsNull())
        {
            EquipWeaponByRowNameInternal(InitialWeapon->RowName);
        }
    }
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
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &APlayerCharacter::StopSprint);
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
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerCharacter::StartFire);

            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopFire);

            EnhancedInputComponent->BindAction(
                FireAction,
                ETriggerEvent::Canceled,
                this,
                &APlayerCharacter::StopFire
            );
        }

        if (ReloadAction)
        {
            EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &APlayerCharacter::Reload);
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

    UpdateAimPitch();
}

void APlayerCharacter::StartSprint(const FInputActionValue& Value)
{
    if (!IsLocallyControlled())
    {
        return;
    }

    bWantsToSprint = true;

    if (bIsAiming || bIsSprinting)
    {
        return;
    }

    StopFire();

    bIsSprinting = true;

    if (UCharacterMovementComponent* MovementComp =
        GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = SprintSpeed;
    }

    if (!HasAuthority())
    {
        ServerStartSprint();
    }
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    if (!IsLocallyControlled())
    {
        return;
    }

    bWantsToSprint = false;

    if (!bIsSprinting)
    {
        return;
    }

    bIsSprinting = false;

    if (UCharacterMovementComponent* MovementComp =
        GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = WalkSpeed;
    }

    if (!HasAuthority())
    {
        ServerStopSprint();
    }
}

void APlayerCharacter::OpenOptionUI(const FInputActionValue& Value)
{
    AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(GetController());
    if (!IsValid(PlayerControllerES)) return;
    
    // if (PlayerControllerES->IsOpenOptionUI)
    // {
    //     PlayerControllerES->CloseOptionUI();
    // }
    // else
    // {
        PlayerControllerES->OpenOptionUI();
    //}
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    
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

        MeshComp->SetVisibility(false, true);
    }

    if (IsValid(CurrentWeapon))
    {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // 복제 Actor인 무기의 실제 제거는 서버에서만
    if (HasAuthority() && IsValid(CurrentWeapon))
    {
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

    
    if (IsLocallyControlled())  
    {
        AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(GetController());
        if (!IsValid(PlayerControllerES)) return;
        PlayerControllerES->Client_OpenResultUI();
        
        //Destroy();
    }
    

 
}

void APlayerCharacter::ServerStartSprint_Implementation()
{
    if (bIsSprinting)
    {
        return;
    }

    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }

    bIsSprinting = true;

    if (UCharacterMovementComponent* MovementComp =
        GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = SprintSpeed;
    }
}

void APlayerCharacter::ServerStopSprint_Implementation()
{
    if (!bIsSprinting)
    {
        return;
    }

    bIsSprinting = false;

    if (UCharacterMovementComponent* MovementComp =
        GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = WalkSpeed;
    }
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerCharacter, CurrentWeaponType);

    DOREPLIFETIME_CONDITION(
        APlayerCharacter,
        bIsSprinting,
        COND_SkipOwner
    );
    //DOREPLIFETIME(APlayerCharacter, bIsDead);
    DOREPLIFETIME(APlayerCharacter, CurrentWeapon);
    DOREPLIFETIME(APlayerCharacter, CurrentWeaponRowName);
}

void APlayerCharacter::StartAim()
{
    if (!IsLocallyControlled())
    {
        return;
    }

    bIsAiming = true;

    if (bIsSprinting)
    {
        bIsSprinting = false;

        if (UCharacterMovementComponent* MovementComp =
            GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = WalkSpeed;
        }

        if (!HasAuthority())
        {
            ServerStopSprint();
        }
    }

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
    if (!IsLocallyControlled())
    {
        return;
    }

    bIsAiming = false;

    if (bWantsToSprint && !bIsSprinting)
    {
        bIsSprinting = true;

        if (UCharacterMovementComponent* MovementComp =
            GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = SprintSpeed;
        }

        if (!HasAuthority())
        {
            ServerStartSprint();
        }
    }

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

//void APlayerCharacter::EquipWeapon(EWeaponType NewWeaponType)
//{
//    if (CurrentWeaponType == NewWeaponType && CurrentWeapon)
//    {
//        return;
//    }
//
//    if (bIsAiming)
//    {
//        CancelAimInstant();
//    }
//
//    if (HasAuthority())
//    {
//        EquipWeaponInternal(NewWeaponType);
//    }
//    else
//    {
//        ServerSetWeaponType(NewWeaponType);
//    }
//}

//void APlayerCharacter::ServerSetWeaponType_Implementation(EWeaponType NewWeaponType)
//{
//    EquipWeaponInternal(NewWeaponType);
//
//    if (IsLocallyControlled())
//    {
//        BP_OnWeaponChanged(GetCurrentWeaponDataRow());
//    }
//}

//void APlayerCharacter::OnRep_CurrentWeaponType()
//{
//    //ApplyWeaponVisuals();
//    //성윤 클라이언트에서 CurrentWeaponType 복제 완료 시 장비 UI와 3D 프리뷰를 갱신하기 위한 이벤트
//    if (IsLocallyControlled())
//    {
//        BP_OnWeaponChanged(GetCurrentWeaponDataRow());
//    }
//}

void APlayerCharacter::MulticastPlayEquipMontage_Implementation(EWeaponType NewWeaponType)
{
    UAnimMontage* MontageToPlay = nullptr;

    switch (NewWeaponType)
    {
    case EWeaponType::Pistol:
        MontageToPlay = PistolEquipMontage;
        break;

    case EWeaponType::Rifle:
        MontageToPlay = RifleEquipMontage;
        break;

    case EWeaponType::Shotgun:
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

void APlayerCharacter::StartFire()
{

    if (!IsLocallyControlled()) return;

    ServerStartFire();
}

void APlayerCharacter::StopFire()
{
    if (!IsLocallyControlled()) return;

    ServerStopFire();
}

void APlayerCharacter::ServerFire_Implementation()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->ServerFire();
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
        
       // FOnMontageEnded EndDelegate;
       // EndDelegate.BindUObject(this, &APlayerCharacter::OnDeathMontageEnded);
       // GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, DeathMontage);
    }

    if (!HasAuthority())
    {
        return;
    }

    APlayerCharacterState* PS = GetPlayerState<APlayerCharacterState>();

    if (!PS)
    {
        return;
    }

    const int32 Gold = PS->GetPlayerStageGold();

    UE_LOG(LogTemp, Warning, TEXT("Player Gold : %d"), Gold);

    if (CoinClass && Gold > 0)
    {
        FVector SpawnLocation = GetActorLocation();
        SpawnLocation.Z += 30.f;

        ACoinBase::SpawnCoins(
            GetWorld(),
            CoinClass,
            SpawnLocation,
            Gold
        );

        PS->SetPlayerStageGold(0);
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

//float  APlayerCharacter::TakeDamage
//UGameplayStatics::ApplyDamage
float APlayerCharacter::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser
)
{ // maybe server?
    const float ActualDamage = Super::TakeDamage(
        DamageAmount,
        DamageEvent,
        EventInstigator,
        DamageCauser
    );
    
    APlayerCharacterState* PS = GetPlayerState<APlayerCharacterState>();
    if (PS)
    {
        PS->TakenDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
    }

    return ActualDamage;
}


//bool APlayerCharacter::EquipWeaponActor(EWeaponType NewWeaponType)
//{
//    const FDataTableRowHandle* FoundWeaponData =
//        WeaponDataMap.Find(NewWeaponType);
//
//    if (!FoundWeaponData || FoundWeaponData->IsNull())
//    {
//        return false;
//    }
//
//    return EquipWeaponActorFromRow(*FoundWeaponData);
//}


//성윤 WeaponType에 맞는 DT_Weapon RowHandle을 반환하는 함수
FDataTableRowHandle APlayerCharacter::GetWeaponDataRowByType(EWeaponType WeaponType) const
{
    const FDataTableRowHandle* FoundWeaponData = WeaponDataMap.Find(WeaponType);

    if (!FoundWeaponData || FoundWeaponData->IsNull())
    {
        return FDataTableRowHandle();
    }

    return *FoundWeaponData;
}

//성윤 현재 장착 중인 무기의 DT_Weapon RowHandle을 UI/장비 프리뷰에서 가져가기 위한 함수
FDataTableRowHandle APlayerCharacter::GetCurrentWeaponDataRow() const
{
    EWeaponType WeaponType{};
    FDataTableRowHandle WeaponData;

    if (FindWeaponByRowName(
        CurrentWeaponRowName,
        WeaponType,
        WeaponData
    ))
    {
        return WeaponData;
    }

    return FDataTableRowHandle();
}

//성윤 현재 장착 중인 Weapon Actor를 반환해서 장비 프리뷰가 실제 무기까지 함께 캡처할 수 있게 하는 함수
AWeaponBase* APlayerCharacter::GetCurrentWeaponActor() const
{
    return CurrentWeapon;
}

void APlayerCharacter::Reload()
{
    if (!IsLocallyControlled()) return;

    if (CurrentWeapon)
    {
        CurrentWeapon->ServerReload();
    }
}

//void APlayerCharacter::EquipWeaponInternal(EWeaponType NewWeaponType)
//{
//    if (CurrentWeaponType == NewWeaponType && CurrentWeapon)
//    {
//        return;
//    }
//
//    bIsAiming = false;
//
//    if (!EquipWeaponActor(NewWeaponType))
//    {
//        return;
//    }
//
//    CurrentWeaponType = NewWeaponType;
//
//    MulticastPlayEquipMontage(NewWeaponType);
//}

void APlayerCharacter::ServerStartFire_Implementation()
{

    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

void APlayerCharacter::ServerStopFire_Implementation()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

void APlayerCharacter::UpdateAimPitch()
{
    if (!Controller)
    {
        return;
    }

    const float NormalizedPitch =
        FRotator::NormalizeAxis(
            Controller->GetControlRotation().Pitch
        );

   
    AimPitch = FMath::Clamp(
        NormalizedPitch,
        -70.0f,
        70.0f
    );
}

bool APlayerCharacter::FindWeaponByRowName(
    FName WeaponRowName,
    EWeaponType& OutWeaponType,
    FDataTableRowHandle& OutWeaponData
) const
{
    if (WeaponRowName.IsNone())
    {
        return false;
    }

    for (const TPair<EWeaponType, FDataTableRowHandle>& Pair : WeaponDataMap)
    {
        const FDataTableRowHandle& RowHandle = Pair.Value;

        if (RowHandle.IsNull() || RowHandle.RowName != WeaponRowName)
        {
            continue;
        }
        if (!RowHandle.DataTable ||
            !RowHandle.DataTable->FindRowUnchecked(WeaponRowName))
        {
            return false;
        }

        OutWeaponType = Pair.Key;
        OutWeaponData = RowHandle;
        return true;
    }

    return false;
}

void APlayerCharacter::EquipWeaponByRowName(FName WeaponRowName)
{
    if (WeaponRowName.IsNone())
    {
        return;
    }

    if (HasAuthority())
    {
        EquipWeaponByRowNameInternal(WeaponRowName);
    }
    else
    {
        ServerEquipWeaponByRowName(WeaponRowName);
    }
}

void APlayerCharacter::ServerEquipWeaponByRowName_Implementation(
    FName WeaponRowName
)
{
    EquipWeaponByRowNameInternal(WeaponRowName);
}

void APlayerCharacter::EquipWeaponByRowNameInternal(FName WeaponRowName)
{
    if (!HasAuthority())
    {
        return;
    }

    if (WeaponRowName.IsNone())
    {
        return;
    }

    if (CurrentWeaponRowName == WeaponRowName &&
        IsValid(CurrentWeapon))
    {
        return;
    }

    EWeaponType NewWeaponType;
    FDataTableRowHandle WeaponData;

    if (!FindWeaponByRowName(
        WeaponRowName,
        NewWeaponType,
        WeaponData))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Weapon RowName not found: %s"),
            *WeaponRowName.ToString()
        );

        return;
    }

    if (bIsAiming)
    {
        CancelAimInstant();
    }

   
    PendingWeaponRowName = WeaponRowName;

    
    MulticastPlayEquipMontage(NewWeaponType);
}

bool APlayerCharacter::EquipWeaponActorFromRow(
    const FDataTableRowHandle& WeaponData
)
{
    if (!HasAuthority() ||
        !WeaponBaseClass ||
        WeaponData.IsNull())
    {
        return false;
    }

    if (IsValid(CurrentWeapon))
    {
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

    CurrentWeapon = GetWorld()->SpawnActorDeferred<AWeaponBase>(
        WeaponBaseClass,
        FTransform::Identity,
        this,
        this,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (!CurrentWeapon)
    {
        return false;
    }

    // FinishSpawning 전에 RowHandle 전달
    CurrentWeapon->WeaponDataRow = WeaponData;

    UGameplayStatics::FinishSpawningActor(
        CurrentWeapon,
        FTransform::Identity
    );

    CurrentWeapon->SetOwner(this);
    CurrentWeapon->SetOwnerCharacter(this);

    CurrentWeapon->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("WeaponSocket")
    );

    return true;
}

void APlayerCharacter::OnRep_CurrentWeaponRowName()
{
    if (!IsLocallyControlled())
    {
        return;
    }

    EWeaponType WeaponType{};
    FDataTableRowHandle WeaponData;

    if (FindWeaponByRowName(
        CurrentWeaponRowName,
        WeaponType,
        WeaponData
    ))
    {
        BP_OnWeaponChanged(WeaponData);
    }
}

void APlayerCharacter::EquipDefaultWeapon()
{
    const FDataTableRowHandle DefaultWeaponData =
        GetWeaponDataRowByType(EWeaponType::Pistol);

    if (DefaultWeaponData.IsNull())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Default pistol weapon data is not configured.")
        );
        return;
    }

    EquipWeaponByRowName(DefaultWeaponData.RowName);
}

void APlayerCharacter::MulticastPlayReloadMontage_Implementation(EWeaponType WeaponType)
{
    if (!GetMesh())
    {
        return;
    }

    UAnimInstance* AnimInstance =
        GetMesh()->GetAnimInstance();

    if (!AnimInstance)
    {
        return;
    }

    UAnimMontage* MontageToPlay = nullptr;

    switch (WeaponType)
    {
    case EWeaponType::Rifle:
        MontageToPlay = RifleReloadMontage;
        break;

    case EWeaponType::Pistol:
        MontageToPlay = PistolReloadMontage;
        break;

    case EWeaponType::Shotgun:
        // 전용 몽타주를 만들기 전까지 Rifle 재사용
        MontageToPlay = RifleReloadMontage;
        break;

    default:
        break;
    }

    if (!MontageToPlay)
    {
        return;
    }

    AnimInstance->Montage_Play(MontageToPlay);
}

void APlayerCharacter::PlayAttackMontage(
    EWeaponType WeaponType)
{
    if (!GetMesh())
    {
        return;
    }

    UAnimInstance* AnimInstance =
        GetMesh()->GetAnimInstance();

    if (!AnimInstance)
    {
        return;
    }

    UAnimMontage* MontageToPlay = nullptr;

    switch (WeaponType)
    {
    case EWeaponType::Rifle:
        MontageToPlay = RifleAttackMontage;
        break;

    case EWeaponType::Pistol:
        MontageToPlay = PistolAttackMontage;
        break;

    default:
        break;
    }

    if (!MontageToPlay)
    {
        return;
    }

    AnimInstance->Montage_Play(MontageToPlay);
}

void APlayerCharacter::CommitPendingWeaponChange()
{
    if (!HasAuthority() ||
        PendingWeaponRowName.IsNone())
    {
        return;
    }

    const FName WeaponRowName = PendingWeaponRowName;

    EWeaponType NewWeaponType;
    FDataTableRowHandle WeaponData;

    if (!FindWeaponByRowName(
        WeaponRowName,
        NewWeaponType,
        WeaponData))
    {
        PendingWeaponRowName = NAME_None;
        return;
    }

    if (!EquipWeaponActorFromRow(WeaponData))
    {
        PendingWeaponRowName = NAME_None;
        return;
    }

    CurrentWeaponType = NewWeaponType;
    CurrentWeaponRowName = WeaponRowName;

    
    PendingWeaponRowName = NAME_None;

    
    if (IsLocallyControlled())
    {
        BP_OnWeaponChanged(WeaponData);
    }
}

void APlayerCharacter::HandleChangeWeaponNotify()
{
    
    if (HasAuthority())
    {
        CommitPendingWeaponChange();
        return;
    }

   
    if (IsLocallyControlled())
    {
        ServerHandleChangeWeaponNotify();
    }
}

void APlayerCharacter::ServerHandleChangeWeaponNotify_Implementation()
{
    CommitPendingWeaponChange();
}

