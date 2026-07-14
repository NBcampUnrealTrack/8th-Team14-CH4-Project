#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"

#include "WeaponType.h"
#include "FireMode.h"

#include "WeaponData.generated.h"

class UStaticMesh;

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::Rifle;

	// �ѱ� �޽�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attach")
	FVector AttachLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attach")
	FRotator AttachRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attach")
	FVector AttachScale = FVector(1.0f, 1.0f, 1.0f);

	// �߻� ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFireMode FireMode = EFireMode::Auto;

	// ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.f;

	// źâ
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineSize = 30;

	// �ִ� ź��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = 120;

	// ���� �ӵ�(��)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.1f;

	// ������ �ð�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 2.f;

	// ��Ÿ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 10000.f;

	// ź����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Spread = 0.f;

	// ���� �ݵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalRecoil = 1.f;

	// ���� �ݵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalRecoil = 0.5f;

	// �ѱ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotMultiplier = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> FireSound = nullptr;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Effects"
	)
	TObjectPtr<UNiagaraSystem> MuzzleFlash = nullptr;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Effects"
	)
	TObjectPtr<UNiagaraSystem> TracerEffect = nullptr;
	
};