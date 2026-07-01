#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemType.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	// 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// 아이템 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::Etc;

	// 월드에 보일 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;

	// 인벤토리 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	// 최대 스택
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;

	// 총기 데이터 Row (무기가 아닐 경우 None)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponRowName;
};