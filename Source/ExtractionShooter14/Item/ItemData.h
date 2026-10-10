#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemType.h"
#include "ItemData.generated.h"

// DT_ShopItem row : how a shop item is shown (name, icons, sound)
// price and performance live in its own table, picked by ItemType, found by DataRowName
//   Weapon  -> DT_Weapon
//   UseItem -> DT_UseItem
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Info")
	FText DisplayName;

	// item category (also picks the table DataRowName points into)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Info")
	EItemType ItemType = EItemType::Weapon;

	// inventory / shop image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Icon")
	UTexture2D* Icon = nullptr;

	// row in DT_Weapon or DT_UseItem (decided by ItemType)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Link")
	FName DataRowName;

	// shop row stat icons (primary / secondary value)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Icon")
	TObjectPtr<UTexture2D> UIIcon_1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Icon")
	TObjectPtr<UTexture2D> UIIcon_2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Sound")
	TObjectPtr<USoundBase> PurchaseSound;
};
