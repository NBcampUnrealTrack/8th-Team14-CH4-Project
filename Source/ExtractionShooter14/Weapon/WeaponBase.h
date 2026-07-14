#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include "WeaponData.h"

#include "WeaponBase.generated.h"

class UStaticMeshMeshComponent;
class USceneComponent;
class APlayerCharacter;
class UNiagaraSystem;


UCLASS()
class EXTRACTIONSHOOTER14_API AWeaponBase : public AActor
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	void ApplyWeaponData();

	bool CanFire() const;

	FTimerHandle FireTimerHandle;

	FTimerHandle ReloadTimerHandle;

	void FireTrace();

	void Fire();


	void FinishReload();

	void NotifyAmmoChanged();

	UPROPERTY(Replicated)
	bool bIsReloading = false;

	bool bIsFiring = false;

	UPROPERTY()
	int32 BurstCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 BurstShotCount = 3;

	FTimerHandle BurstTimerHandle;

	void BurstFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFireEffects(
		FVector_NetQuantize MuzzleLocation,
		FVector_NetQuantize TraceEnd);

public:
	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(Server, Reliable)
	void ServerStartFire();

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	void Reload();

	UFUNCTION()
	void OnRep_CurrentMagazineAmmo();

	UFUNCTION()
	void OnRep_CurrentReserveAmmo();


	void StartFire();

	void StopFire();

	UFUNCTION(Client, Reliable)
	void ClientApplyRecoil();
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY()
	FWeaponData CurrentWeaponData;

	// �ѱ� �޽�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MeshComponent;

	// ���������� ���̺� �ο����
	UPROPERTY(ReplicatedUsing = OnRep_WeaponData, EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FDataTableRowHandle WeaponDataRow;

	UFUNCTION()
	void OnRep_WeaponData();

	// ���� źâ ź��
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMagazineAmmo, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentMagazineAmmo = 0;

	// ���� ���� ź��
	UPROPERTY(ReplicatedUsing = OnRep_CurrentReserveAmmo, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentReserveAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> TracerEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> ImpactEffect = nullptr;

	float LastFireTime = -100.f;

	void SetOwnerCharacter(class APlayerCharacter* NewOwner);

	FVector GetMuzzleLocation() const;

	FRotator GetMuzzleRotation() const;

	bool IsHeadShot(const FHitResult& HitResult) const;

protected:

	UPROPERTY(Replicated)
	class APlayerCharacter* OwnerCharacter = nullptr;

public:

	int32 GetCurrentMagazineAmmo() const;

	int32 GetCurrentReserveAmmo() const;

	bool IsReloading() const;
};
