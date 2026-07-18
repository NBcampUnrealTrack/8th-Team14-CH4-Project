#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemType.h"
#include "ConsumeItem.generated.h"

USTRUCT(BlueprintType)
struct FConsumeItem : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Info")
	EItemType ItemType = EItemType::Etc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Icon")
	UTexture2D* Icon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Info")
	int32 CountAmount = 1;

	// ���� �Ǹ� ���� ����(�ʿ� ������ ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Info")
	int32 MarketPrice = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Info")
	float EffectAmount = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Icon")
	TObjectPtr<UTexture2D> UIIcon_1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Icon")
	TObjectPtr<UTexture2D> UIIcon_2;
};
