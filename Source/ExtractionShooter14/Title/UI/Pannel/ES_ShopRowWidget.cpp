#include "ES_ShopRowWidget.h"

#include "Character/PlayerCharacterState.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Game/DataManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/SlateBlueprintLibrary.h"

namespace
{
	FText ToStatText(float Value)
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 3;
		return FText::AsNumber(Value, &Options);
	}

	void SetIcon(UImage* Image, UTexture2D* Texture)
	{
		if (Image && Texture) Image->SetBrushFromTexture(Texture);
	}
}

void UES_ShopRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PurchaseButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleClicked);
	PurchaseButton->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleHovered);
	PurchaseButton->OnUnhovered.AddUniqueDynamic(this, &ThisClass::HandleUnhovered);
}

void UES_ShopRowWidget::Setup(const FShopItemDisplay& Display)
{
	ItemName = Display.ItemName;
	ItemIconTexture = Display.ItemImage;
	PurchaseSound = Display.PurchaseSound;

	SetIcon(ItemImage, Display.ItemImage);
	SetIcon(Icon_1, Display.PrimaryIcon);
	SetIcon(Icon_2, Display.SecondaryIcon);

	Icon_1_Value->SetText(ToStatText(Display.PrimaryValue));
	Icon_2_Value->SetText(ToStatText(Display.SecondaryValue));
	Icon_3_Value->SetText(FText::AsNumber(Display.Price));
}

void UES_ShopRowWidget::HandleClicked()
{
	UGameplayStatics::PlaySound2D(this, ClickSound);

	// intent only : server decides with its own tables
	if (APlayerCharacterState* PS = GetOwningPlayerState<APlayerCharacterState>())
	{
		PS->BuyItem(ItemName);
	}
}

void UES_ShopRowWidget::HandleHovered()
{
	if (Hovered) PlayAnimationForward(Hovered);
	UGameplayStatics::PlaySound2D(this, HoverSound);
}

void UES_ShopRowWidget::HandleUnhovered()
{
	if (Hovered) PlayAnimationReverse(Hovered);
}

void UES_ShopRowWidget::PlayPurchaseResult(bool bSuccess)
{
	if (bSuccess)
	{
		UGameplayStatics::PlaySound2D(this, PurchaseSound);
	}
	else if (NotEnoughMoney)
	{
		PlayAnimationForward(NotEnoughMoney);
	}
}

FVector2D UES_ShopRowWidget::GetItemIconViewportPosition() const
{
	const FGeometry& Geometry = ItemImage->GetCachedGeometry();

	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(const_cast<UES_ShopRowWidget*>(this), Geometry, Geometry.GetLocalSize() * 0.5f, PixelPosition, ViewportPosition);
	return ViewportPosition;
}
