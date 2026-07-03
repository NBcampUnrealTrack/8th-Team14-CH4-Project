#include "WeaponBase.h"
#include "Character/PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, CurrentMagazineAmmo);
	DOREPLIFETIME(AWeaponBase, CurrentReserveAmmo);
	DOREPLIFETIME(AWeaponBase, bIsReloading);
}

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	RootComponent = MeshComponent;

	bReplicates = true;

	SetReplicateMovement(true);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyWeaponData();
}

void AWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyWeaponData();
}

void AWeaponBase::ApplyWeaponData()
{
	if (WeaponDataRow.IsNull())
	{
		return;
	}

	const FWeaponData* WeaponData = WeaponDataRow.GetRow<FWeaponData>(TEXT("Weapon"));

	if (WeaponData == nullptr)
	{
		return;
	}

	CurrentWeaponData = *WeaponData;

	if (CurrentWeaponData.WeaponMesh)
	{
		MeshComponent->SetSkeletalMesh(CurrentWeaponData.WeaponMesh);
	}

	CurrentMagazineAmmo = CurrentWeaponData.MagazineSize;
	CurrentReserveAmmo = CurrentWeaponData.MaxAmmo;
}

bool AWeaponBase::CanFire() const
{
	if (CurrentMagazineAmmo <= 0)
	{
		return false;
	}

	if (bIsReloading)
	{
		return false;
	}

	if (!GetWorld())
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastFireTime < CurrentWeaponData.FireRate)
	{
		return false;
	}

	return true;
}

void AWeaponBase::FireTrace()
{
	if (!OwnerCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());

	if (!PC)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceStart = CameraLocation;
	const FVector TraceEnd = TraceStart + (CameraRotation.Vector() * CurrentWeaponData.Range);

	// 카메라 -> 조준점 trace
	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	// 조준점
	FVector HitLocation = bHit ? HitResult.Location : TraceEnd;

	// 총구 위치
	FVector MuzzleLocation = GetMuzzleLocation();

	// 총구 → 조준점 방향
	FVector ShootDirection = (HitLocation - MuzzleLocation).GetSafeNormal();

	// 총구 Trace 끝점
	FVector MuzzleEnd = MuzzleLocation + (ShootDirection * CurrentWeaponData.Range);

	// 총구 -> 조준점 Trace
	FHitResult WeaponHit;

	bool bWeaponHit = GetWorld()->LineTraceSingleByChannel(
		WeaponHit,
		MuzzleLocation,
		MuzzleEnd,
		ECC_Visibility,
		Params
	);

	// 카메라 Trace
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		HitLocation,
		FColor::Red,
		false,
		2.f,
		0,
		1.f
	);

	// 총구 Trace
	DrawDebugLine(
		GetWorld(),
		MuzzleLocation,
		bWeaponHit ? WeaponHit.Location : MuzzleEnd,
		FColor::Yellow,
		false,
		2.f,
		0,
		1.f
	);

	// 총구 위치
	DrawDebugSphere(
		GetWorld(),
		MuzzleLocation,
		5.f,
		12,
		FColor::Blue,
		false,
		2.f
	);
}

void AWeaponBase::Fire()
{
	if (!CanFire())
	{
		return;
	}

	CurrentMagazineAmmo--;

	LastFireTime = GetWorld()->GetTimeSeconds();

	FireTrace();

	UE_LOG(LogTemp, Warning, TEXT("Fire! Ammo : %d"), CurrentMagazineAmmo);
}

void AWeaponBase::Reload()
{
	if (CurrentMagazineAmmo >= CurrentWeaponData.MagazineSize)
	{
		return;
	}

	if (CurrentReserveAmmo <= 0)
	{
		return;
	}

	int32 NeedAmmo = CurrentWeaponData.MagazineSize - CurrentMagazineAmmo;

	int32 ReloadAmmo = FMath::Min(NeedAmmo, CurrentReserveAmmo);

	CurrentMagazineAmmo += ReloadAmmo;

	CurrentReserveAmmo -= ReloadAmmo;

	UE_LOG(LogTemp, Warning, TEXT("Reload! Magazine : %d / Reserve : %d"),
		CurrentMagazineAmmo,
		CurrentReserveAmmo);
}

void AWeaponBase::StartFire()
{
	switch (CurrentWeaponData.FireMode)
	{
	case EFireMode::Single:

		Fire();

		break;

	case EFireMode::Auto:

		Fire();

		GetWorld()->GetTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&AWeaponBase::Fire,
			CurrentWeaponData.FireRate,
			true
		);

		break;

	case EFireMode::Burst:

		Fire();

		break;
	}
}


void AWeaponBase::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
}

void AWeaponBase::SetOwnerCharacter(APlayerCharacter* NewOwner)
{
	OwnerCharacter = NewOwner;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
	return MeshComponent->GetSocketLocation(CurrentWeaponData.MuzzleSocketName);
}

FRotator AWeaponBase::GetMuzzleRotation() const
{
	return MeshComponent->GetSocketRotation(CurrentWeaponData.MuzzleSocketName);
}
