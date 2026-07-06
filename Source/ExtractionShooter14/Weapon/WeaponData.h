#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"

#include "WeaponType.h"
#include "FireMode.h"

#include "WeaponData.generated.h"

class USkeletalMesh;

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 무기 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::Rifle;

	// 총기 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* WeaponMesh = nullptr;

	// 발사 방식
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFireMode FireMode = EFireMode::Auto;

	// 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.f;

	// 탄창
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineSize = 30;

	// 최대 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = 120;

	// 연사 속도(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.1f;

	// 재장전 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 2.f;

	// 사거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 10000.f;

	// 탄퍼짐
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Spread = 0.f;

	// 수직 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalRecoil = 1.f;

	// 수평 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalRecoil = 0.5f;

	// 총구 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotMultiplier = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> FireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UNiagaraSystem> MuzzleFlash = nullptr;
};