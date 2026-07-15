#include "WeaponBase.h"
#include "Character/PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Character/MyPlayerController.h"
#include "Characters/Zombie/ZombieCharacter.h"



void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, CurrentMagazineAmmo);
	DOREPLIFETIME(AWeaponBase, CurrentReserveAmmo);
	DOREPLIFETIME(AWeaponBase, bIsReloading);
	DOREPLIFETIME(AWeaponBase, WeaponDataRow);
	DOREPLIFETIME(AWeaponBase, OwnerCharacter);
}

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SceneRoot);

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
	UE_LOG(LogTemp, Warning, TEXT("ApplyWeaponData : %s"),
		WeaponDataRow.IsNull() ? TEXT("NULL") : TEXT("VALID"));

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
		MeshComponent->SetStaticMesh(CurrentWeaponData.WeaponMesh);
	}

	MeshComponent->SetRelativeLocation(CurrentWeaponData.AttachLocationOffset);
	MeshComponent->SetRelativeRotation(CurrentWeaponData.AttachRotationOffset);
	MeshComponent->SetRelativeScale3D(CurrentWeaponData.AttachScale);

	UE_LOG(LogTemp, Warning, TEXT("Weapon Offset: %s / %s"),
		*CurrentWeaponData.AttachLocationOffset.ToString(),
		*CurrentWeaponData.AttachRotationOffset.ToString());

	CurrentMagazineAmmo = CurrentWeaponData.MagazineSize;
	CurrentReserveAmmo = CurrentWeaponData.MaxAmmo;
}

bool AWeaponBase::CanFire() const
{

	if (OwnerCharacter && OwnerCharacter->IsSprinting())
	{
		return false;
	}

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

	FRotator BulletRotation = CameraRotation;

	BulletRotation.Pitch += FMath::FRandRange(
		-CurrentWeaponData.Spread,
		CurrentWeaponData.Spread);

	BulletRotation.Yaw += FMath::FRandRange(
		-CurrentWeaponData.Spread,
		CurrentWeaponData.Spread);


	const FVector TraceStart = CameraLocation;
	const FVector TraceEnd = TraceStart + (BulletRotation.Vector() * CurrentWeaponData.Range);

	// ī�޶� -> ������ trace
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

	// ������
	FVector HitLocation = bHit ? HitResult.Location : TraceEnd;

	// �ѱ� ��ġ
	FVector MuzzleLocation = GetMuzzleLocation();

	// �ѱ� �� ������ ����
	FVector ShootDirection = (HitLocation - MuzzleLocation).GetSafeNormal();

	// �ѱ� Trace ����
	FVector MuzzleEnd = MuzzleLocation + (ShootDirection * CurrentWeaponData.Range);

	// �ѱ� -> ������ Trace
	FHitResult WeaponHit;

	bool bWeaponHit = GetWorld()->LineTraceSingleByChannel(
		WeaponHit,
		MuzzleLocation,
		MuzzleEnd,
		ECC_Visibility,
		Params
	);

	const FVector TracerEndLocation = bWeaponHit
		? WeaponHit.ImpactPoint
		: MuzzleEnd;

	if (HasAuthority() && bWeaponHit && OwnerCharacter)
	{
		AActor* HitActor = WeaponHit.GetActor();

		if (HitActor)
		{
			const bool bCanShowHitMarker =
				HitActor->IsA<APlayerCharacter>() ||
				HitActor->IsA<AZombieCharacter>();

			if (bCanShowHitMarker)
			{
				float Damage = CurrentWeaponData.Damage;
				bool bCritical = false;

				if (IsHeadShot(WeaponHit))
				{
					Damage *= CurrentWeaponData.HeadShotMultiplier;
					bCritical = true;

					UE_LOG(LogTemp, Warning, TEXT("HeadShot!"));
				}

				const float AppliedDamage = UGameplayStatics::ApplyDamage(
					HitActor,
					Damage,
					OwnerCharacter->GetController(),
					this,
					nullptr
				);

				if (AppliedDamage > 0.0f)
				{
					if (AMyPlayerController* MyPC =
						Cast<AMyPlayerController>(OwnerCharacter->GetController()))
					{
						MyPC->Client_ShowHitMarker(bCritical);
					}
				}
			}
		}
	}

	// ī�޶� Trace
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

	// �ѱ� Trace
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

	// �ѱ� ��ġ
	DrawDebugSphere(
		GetWorld(),
		MuzzleLocation,
		5.f,
		12,
		FColor::Blue,
		false,
		2.f
	);

	if (HasAuthority())
	{
		MulticastPlayFireEffects(
			MuzzleLocation,
			TracerEndLocation
		);
	}

}

void AWeaponBase::Fire()
{
	switch (CurrentWeaponData.FireMode)
	{
	case EFireMode::Single:
		break;

	case EFireMode::Auto:
		break;

	case EFireMode::Burst:
		break;
	}

	if (!CanFire())
	{
		return;
	}

	CurrentMagazineAmmo--;
	//성윤 호출용

	NotifyAmmoChanged();

	LastFireTime = GetWorld()->GetTimeSeconds();

	FireTrace();


	ClientApplyRecoil();

	UE_LOG(LogTemp, Warning, TEXT("Fire! Ammo : %d"), CurrentMagazineAmmo);
}

void AWeaponBase::FinishReload()
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

	bIsReloading = false;

	NotifyAmmoChanged();
	//성윤 호출용

	UE_LOG(LogTemp, Warning, TEXT("Reload! Magazine : %d / Reserve : %d"),
		CurrentMagazineAmmo,
		CurrentReserveAmmo);
}

void AWeaponBase::ServerFire_Implementation()
{
	Fire();
}

void AWeaponBase::ServerReload_Implementation()
{
	Reload();
}

void AWeaponBase::BurstFire()
{
	if (BurstCount >= BurstShotCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurstTimerHandle);
		return;
	}

	Fire();

	BurstCount++;

	GetWorld()->GetTimerManager().SetTimer(
		BurstTimerHandle,
		this,
		&AWeaponBase::BurstFire,
		CurrentWeaponData.FireRate,
		false
	);
}

void AWeaponBase::ServerStartFire_Implementation()
{
	StartFire();
}

void AWeaponBase::ServerStopFire_Implementation()
{
	StopFire();
}

void AWeaponBase::Reload()
{
	if (bIsReloading)
	{
		return;
	}

	if (CurrentMagazineAmmo >= CurrentWeaponData.MagazineSize)
	{
		return;
	}

	if (CurrentReserveAmmo <= 0)
	{
		return;
	}

	bIsReloading = true;

	if (HasAuthority() && OwnerCharacter)
	{
		OwnerCharacter->MulticastPlayReloadMontage(CurrentWeaponData.WeaponType);
	}

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AWeaponBase::FinishReload,
		CurrentWeaponData.ReloadTime,
		false
	);
}

void AWeaponBase::OnRep_CurrentMagazineAmmo()
{
	NotifyAmmoChanged();
	//źâ ź�� �� UI����
}

void AWeaponBase::OnRep_CurrentReserveAmmo()
{
	NotifyAmmoChanged();
	//���� ź�� �� UI����
}

void AWeaponBase::StartFire()
{

	if (CurrentMagazineAmmo <= 0)
	{
		Reload();

		return;
	}

	if (!CanFire())
	{
		return;
	}

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

		BurstCount = 0;

		BurstFire();

		break;
	}
}


void AWeaponBase::StopFire()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BurstTimerHandle);

	BurstCount = 0;

}


void AWeaponBase::ClientApplyRecoil_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ClientApplyRecoil Called"));

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("No OwnerCharacter"));
		return;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Local"));
		return;
	}

	APlayerController* PC =
		Cast<APlayerController>(OwnerCharacter->GetController());

	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("No PC"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Apply Recoil"));

	PC->AddPitchInput(-CurrentWeaponData.VerticalRecoil);

	PC->AddYawInput(
		FMath::FRandRange(
			-CurrentWeaponData.HorizontalRecoil,
			CurrentWeaponData.HorizontalRecoil));
}


void AWeaponBase::MulticastPlayFireEffects_Implementation(
	FVector_NetQuantize MuzzleLocation,
	FVector_NetQuantize TracerEndLocation)
{
	if (!MeshComponent)
	{
		return;
	}

	APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(GetOwner());

	if (!PlayerCharacter)
	{
		PlayerCharacter = OwnerCharacter;
	}

	if (PlayerCharacter)
	{
		PlayerCharacter->PlayAttackMontage(
			CurrentWeaponData.WeaponType
		);
	}

	// �ѱ� ȭ��
	if (CurrentWeaponData.MuzzleFlash)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			CurrentWeaponData.MuzzleFlash,
			MeshComponent,
			TEXT("Muzzle"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	if (CurrentWeaponData.TracerEffect)
{
	UNiagaraComponent* TracerComponent =
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CurrentWeaponData.TracerEffect,
			MuzzleLocation,
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			false,
			ENCPoolMethod::None,
			true
		);

	if (TracerComponent)
	{
		TracerComponent->SetVariablePosition(
			TEXT("User.BeamStart"),
			FVector(MuzzleLocation)
		);

		TracerComponent->SetVariablePosition(
			TEXT("User.BeamEnd"),
			FVector(TracerEndLocation)
		);

		TracerComponent->Activate(true);
	}
}
	// �߻� ����
	if (CurrentWeaponData.FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			CurrentWeaponData.FireSound,
			GetMuzzleLocation()
		);
	}
}

void AWeaponBase::OnRep_WeaponData()
{
	ApplyWeaponData();
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

bool AWeaponBase::IsHeadShot(const FHitResult& HitResult) const
{
	return HitResult.BoneName == TEXT("head");
}

int32 AWeaponBase::GetCurrentMagazineAmmo() const
{
	return CurrentMagazineAmmo;
}

int32 AWeaponBase::GetCurrentReserveAmmo() const
{
	return CurrentReserveAmmo;
}

bool AWeaponBase::IsReloading() const
{
	return bIsReloading;
}

void AWeaponBase::NotifyAmmoChanged()
{
	APlayerCharacter* PlayerCharacter = OwnerCharacter;

	if (!IsValid(PlayerCharacter))
	{
		PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	}

	if (!IsValid(PlayerCharacter))
	{
		return;
	}

	if (!PlayerCharacter->IsLocallyControlled())
	{
		return;
	}

	PlayerCharacter->BP_OnWeaponAmmoChanged(
		CurrentMagazineAmmo,
		CurrentReserveAmmo
	);
}