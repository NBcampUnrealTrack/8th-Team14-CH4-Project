// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
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

APlayerCharacter::APlayerCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = false;

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
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
    }

}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
       
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(ForwardDirection, MovementVector.X);

        
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(RightDirection, MovementVector.Y);
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

    DOREPLIFETIME(APlayerCharacter, bIsSprinting);
}