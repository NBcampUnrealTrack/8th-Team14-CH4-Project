#include "ItemBase.h"
#include "ItemData.h"

#include "Components/StaticMeshComponent.h"
#include "Game/DataManagerSubsystem.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = MeshComponent;
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyItemData();
}

void AItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyItemData();
}

void AItemBase::ApplyItemData()
{
	if (ItemDataRow.IsNull())
	{
		return;
	}

	const FItemData* ItemData = ItemDataRow.GetRow<FItemData>(TEXT("ApplyItemData"));

	if (ItemData == nullptr)
	{
		return;
	}

	CurrentItemData = *ItemData;

	// look comes from the linked performance table (DT_Weapon / DT_UseItem)
	if (UStaticMesh* WorldMesh = UDataManagerSubsystem::FindWorldMesh(this, *ItemData))
	{
		MeshComponent->SetStaticMesh(WorldMesh);
	}
}