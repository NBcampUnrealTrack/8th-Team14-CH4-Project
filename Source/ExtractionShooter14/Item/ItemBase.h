#pragma once

#include "CoreMinimal.h"
#include "ItemData.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ItemBase.generated.h"

class UStaticMeshComponent;

UCLASS()
class EXTRACTIONSHOOTER14_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	void ApplyItemData();

protected:

	FItemData CurrentItemData;

public:

	// 아이템 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MeshComponent;

	// DataTable Row
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FDataTableRowHandle ItemDataRow;
};