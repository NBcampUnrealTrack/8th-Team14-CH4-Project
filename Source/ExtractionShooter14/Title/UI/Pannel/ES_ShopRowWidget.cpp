#include "ES_ShopRowWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UES_ShopRowWidget::InitializeItemInfo(float Icon1_value, int32 Icon2_value, int32 InItemCost, UTexture2D* InItemImage,
                                           UTexture2D* Icon1_image, UTexture2D* Icon2_image, USoundBase* InPurchaseSound, FName InItemName)
{
	Icon_1_Value->SetText(FText::AsNumber(Icon1_value));
	Icon_2_Value->SetText(FText::AsNumber(Icon2_value));
	Icon_3_Value->SetText(FText::AsNumber(InItemCost));
	ItemCost = InItemCost;
	
	ItemImage->SetBrushFromTexture(InItemImage);
	Icon_1->SetBrushFromTexture(Icon1_image);
	Icon_2->SetBrushFromTexture(Icon2_image);
	
	PurchaseSound = InPurchaseSound;
	ItemName = InItemName;
}

const FVector2D UES_ShopRowWidget::GetItemIconViewportPosition()
{
	const FGeometry Geometry = ItemImage->GetCachedGeometry();
	FVector2D Geometry2D = Geometry.GetLocalSize();
	Geometry2D = Geometry2D * 0.5;
	
	FVector2D PixelPosition;
	FVector2D ViewPosition;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geometry, Geometry2D,
		PixelPosition, ViewPosition);
	
	return ViewPosition;
}
