#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include "WeaponData.h"

#include "WeaponBase.generated.h"

class USkeletalMeshComponent;

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

	void FireTrace();

	void Fire();

	UPROPERTY(Replicated)
	bool bIsReloading = false;

public:

	void Reload();

	void StartFire();

	void StopFire();

public:
	UPROPERTY()
	FWeaponData CurrentWeaponData;

	// 총기 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* MeshComponent;

	// 웹폰데이터 테이블 로우네임
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FDataTableRowHandle WeaponDataRow;

	// 현재 탄창 탄약
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentMagazineAmmo = 0;

	// 현재 예비 탄약
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentReserveAmmo = 0;

	float LastFireTime = -100.f;

	void SetOwnerCharacter(class APlayerCharacter* NewOwner);

	FVector GetMuzzleLocation() const;

	FRotator GetMuzzleRotation() const;

protected:
	UPROPERTY()
	class APlayerCharacter* OwnerCharacter = nullptr;
};
