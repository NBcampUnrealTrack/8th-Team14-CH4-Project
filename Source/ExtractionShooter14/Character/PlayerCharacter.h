// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacterAnimInstance.h"
#include "InputActionValue.h"
#include "Weapon/WeaponType.h"
#include "Item/CoinBase.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AWeaponBase;

UCLASS()
class EXTRACTIONSHOOTER14_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APlayerCharacter();

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECharacterMovementDirection LastInputDirection;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeaponByRowName(FName WeaponRowName);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetCurrentWeaponRowName() const
	{
		return CurrentWeaponRowName;
	} 

protected:

	//bool EquipWeaponActor(EWeaponType NewWeaponType);

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	
	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

	UFUNCTION(Server, Reliable)
	void ServerStartFire();

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	AWeaponBase* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> WeaponBaseClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TMap<EWeaponType, FDataTableRowHandle> WeaponDataMap;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf<class ACoinBase> CoinClass;
public:	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;
	
	void BindDieFunctionWithPlayerStateDelegate();
	
	UFUNCTION()
	void OnbIsDeadChanged(uint8 changedbisdead);
	
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION(BlueprintCallable, Category = "Death")
	void Die();

	//성윤 현재 장착 중인 무기의 DataTable RowHandle을 UI/프리뷰 블루프린트에서 읽기 위한 함수
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FDataTableRowHandle GetCurrentWeaponDataRow() const;

	//성윤 무기가 변경됐을 때 장비 UI와 3D 프리뷰를 갱신하기 위해 블루프린트로 알려주는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_OnWeaponChanged(FDataTableRowHandle NewWeaponDataRow);

	//성윤 현재 장착 중인 Weapon Actor를 장비 프리뷰 SceneCapture에서 ShowOnly 대상으로 등록하기 위한 함수
	UFUNCTION(BlueprintPure, Category = "Weapon")
	AWeaponBase* GetCurrentWeaponActor() const;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;
	bool GetbIsDead() const;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float AimPitch = 0.0f;

	void UpdateAimPitch();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipDefaultWeapon();
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;

	UFUNCTION(Server, Reliable)
	void ServerFire();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float TestDamage = 10.0f;*/

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRange = 10000.0f;*/

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
	void StartFire();
	void StopFire();
	void Reload();
	
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

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	EWeaponType CurrentWeaponType = EWeaponType::Pistol;

	
	/* 

	/*UFUNCTION(Server, Reliable)
	void ServerSetWeaponType(EWeaponType NewWeaponType);*/

	/*UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(EWeaponType NewWeaponType);*/

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void ApplyWeaponVisuals();

	//성윤 WeaponType을 실제 DT_Weapon RowHandle로 변환하는 내부용 함수
	FDataTableRowHandle GetWeaponDataRowByType(EWeaponType WeaponType) const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayEquipMontage(EWeaponType NewWeaponType);

	void CancelAimInstant();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Montage")
	UAnimMontage* PistolEquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Montage")
	UAnimMontage* RifleEquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	UAnimMontage* StageSpawnMontage;

	/*void EquipWeaponInternal(EWeaponType NewWeaponType);*/
	


	// UPROPERTY(Replicated, BlueprintReadOnly, Category = "Death")
	// bool bIsDead = false;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDie();

	// UFUNCTION(BlueprintCallable, Category = "Death")
	// void CheckDeath();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeaponByRowName(FName WeaponRowName);

	void EquipWeaponByRowNameInternal(FName WeaponRowName);

	bool FindWeaponByRowName(
		FName WeaponRowName,
		EWeaponType& OutWeaponType,
		FDataTableRowHandle& OutWeaponData
	) const;

	bool EquipWeaponActorFromRow(
		const FDataTableRowHandle& WeaponData
	);

	UFUNCTION()
	void OnRep_CurrentWeaponRowName();

	UPROPERTY(
		ReplicatedUsing = OnRep_CurrentWeaponRowName,
		BlueprintReadOnly,
		Category = "Weapon"
	)
	FName CurrentWeaponRowName = NAME_None;
};
