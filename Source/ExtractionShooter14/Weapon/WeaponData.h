#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "WeaponType.h"
#include "FireMode.h"

#include "WeaponData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 무기 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::Rifle;

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
};