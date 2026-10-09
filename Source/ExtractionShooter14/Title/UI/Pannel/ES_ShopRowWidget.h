

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/Struct/InventoryItemDatas.h"
#include "ES_ShopRowWidget.generated.h"

class APlayerCharacterState;
class UButton;
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class EXTRACTIONSHOOTER14_API UES_ShopRowWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeItemInfo(float Icon1_value, int32 Icon2_value, int32 ItemCost,
		UTexture2D* InItemImage, UTexture2D* Icon1_image, UTexture2D* Icon2_image,
		USoundBase* InPurchaseSound, FName InItemName);
	
	const FVector2D GetItemIconViewportPosition();
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Icon_1_Value;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Icon_2_Value;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Icon_3_Value;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_2;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;
	UPROPERTY()
	int32 ItemCost;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PurchaseButton;
	UPROPERTY()
	TObjectPtr<USoundBase> PurchaseSound;
	
	UPROPERTY()
	FName ItemName;
	UPROPERTY()
	TObjectPtr<APlayerCharacterState> OwningPlayerState;
	UPROPERTY()
	FInventoryItemData InventoryItemData;
};
