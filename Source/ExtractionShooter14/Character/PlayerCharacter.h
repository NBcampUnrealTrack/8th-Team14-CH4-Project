// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacterAnimInstance.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol  UMETA(DisplayName = "Pistol"),
	Rifle   UMETA(DisplayName = "Rifle"),
	Shotgun UMETA(DisplayName = "Shotgun")
};

UCLASS()
class EXTRACTIONSHOOTER14_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APlayerCharacter();

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECharacterMovementDirection LastInputDirection;

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	
	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

public:	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* OptionAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 600.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	bool bIsAiming = false;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	void StartAim();
	void StopAim();
	
	void OpenOptionUI(const FInputActionValue& Value);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim|Camera")
	float AimArmLength = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim|Camera")
	FVector AimSocketOffset = FVector(0.0f, 45.0f, 15.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim|Camera")
	float AimCameraInterpSpeed = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim|Camera")
	float DefaultArmLength = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim|Camera")
	FVector DefaultSocketOffset = FVector::ZeroVector;

	UFUNCTION()
	void UpdateAimCamera(float Alpha);

	UFUNCTION()
	void FinishAimCamera();

	FTimeline AimCameraTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim|Camera")
	UCurveFloat* AimCameraCurve = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponType, BlueprintReadOnly, Category = "Weapon")
	EWeaponType CurrentWeaponType = EWeaponType::Pistol;

	UFUNCTION()
	void OnRep_CurrentWeaponType();

	UFUNCTION(Server, Reliable)
	void ServerSetWeaponType(EWeaponType NewWeaponType);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(EWeaponType NewWeaponType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void ApplyWeaponVisuals();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayEquipMontage(EWeaponType NewWeaponType);

	void CancelAimInstant();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Montage")
	UAnimMontage* PistolEquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Montage")
	UAnimMontage* RifleEquipMontage;
};
