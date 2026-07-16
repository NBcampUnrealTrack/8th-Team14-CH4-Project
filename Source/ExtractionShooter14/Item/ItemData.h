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

	// ������ �̸�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::Etc;

	// ���忡 ���� �޽�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;

	// �κ��丮 ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	// �ִ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;

	// �ѱ� ������ Row (���Ⱑ �ƴ� ��� None)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponRowName;

	// ���� �Ǹ� ���� ����(�ʿ� ������ ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 0;

	// 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealAmount = 0.f;
};