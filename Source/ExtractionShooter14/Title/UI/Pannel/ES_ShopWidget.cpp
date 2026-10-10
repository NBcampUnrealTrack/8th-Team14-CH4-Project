#include "ES_ShopWidget.h"

#include "ES_ShopRowWidget.h"
#include "ES_FlyIconWidget.h"
#include "Character/MyPlayerController.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Game/DataManagerSubsystem.h"

void UES_ShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildRows();

	if (AMyPlayerController* PC = GetOwningPlayer<AMyPlayerController>())
	{
		PC->OnReceiveBuyItemResult.AddUniqueDynamic(this, &ThisClass::HandleBuyItemResult);
	}

	if (OpenUpAnimation_HUD) PlayAnimationForward(OpenUpAnimation_HUD);
}

void UES_ShopWidget::NativeDestruct()
{
	// remove only this widget's binding
	if (AMyPlayerController* PC = GetOwningPlayer<AMyPlayerController>())
	{
		PC->OnReceiveBuyItemResult.RemoveDynamic(this, &ThisClass::HandleBuyItemResult);
	}

	Super::NativeDestruct();
}

void UES_ShopWidget::BuildRows()
{
	ShopItemListScrollBox->ClearChildren();
	RowWidgets.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	const UDataManagerSubsystem* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManagerSubsystem>() : nullptr;
	const UDataTable* ShopItemTable = DataManager ? DataManager->GetShopItemTable() : nullptr;
	if (!ShopItemTable) return;
	if (!ShopRowWidgetClass || ShopRowWidgetClass->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogTemp, Error, TEXT("[Shop] ShopRowWidgetClass must be a widget blueprint (WBP_ShopRowWidget)"));
		return;
	}

	for (const FName& ItemName : ShopItemTable->GetRowNames())
	{
		// false : no price row for its ItemType -> not sold
		FShopItemDisplay Display;
		if (!DataManager->GetShopItemDisplay(ItemName, Display)) continue;

		UES_ShopRowWidget* Row = CreateWidget<UES_ShopRowWidget>(GetOwningPlayer(), ShopRowWidgetClass);
		if (!Row) continue;

		Row->Setup(Display);

		if (UScrollBoxSlot* RowSlot = Cast<UScrollBoxSlot>(ShopItemListScrollBox->AddChild(Row)))
		{
			RowSlot->SetPadding(RowPadding);
			RowSlot->SetHorizontalAlignment(HAlign_Center);
			RowSlot->SetVerticalAlignment(VAlign_Center);
		}

		RowWidgets.Add(ItemName, Row);
	}
}

void UES_ShopWidget::HandleBuyItemResult(FName ItemName, bool Result, int32 StageInventoryIndex)
{
	UES_ShopRowWidget* Row = RowWidgets.FindRef(ItemName);
	if (!Row) return;

	Row->PlayPurchaseResult(Result);

	if (Result)
	{
		PlayFlyIcon(Row, StageInventoryIndex);
	}
}

void UES_ShopWidget::SetFlyIconTarget(UUserWidget* InInventoryWidget, UCanvasPanel* InEffectCanvas)
{
	InventoryWidget = InInventoryWidget;
	EffectCanvas = InEffectCanvas;
}

void UES_ShopWidget::PlayFlyIcon(const UES_ShopRowWidget* Row, int32 StageInventoryIndex)
{
	if (!FlyIconClass || !EffectCanvas.IsValid()) return;

	FVector2D EndPosition;
	if (!GetInventorySlotViewportPosition(StageInventoryIndex, EndPosition)) return;

	UES_FlyIconWidget* FlyIcon = CreateWidget<UES_FlyIconWidget>(GetOwningPlayer(), FlyIconClass);
	if (!FlyIcon) return;

	EffectCanvas->AddChildToCanvas(FlyIcon);
	FlyIcon->StartFly(Row->GetItemIconTexture(), Row->GetItemIconViewportPosition(), EndPosition);
}

bool UES_ShopWidget::GetInventorySlotViewportPosition(int32 SlotIndex, FVector2D& OutPosition) const
{
	// slots are the children of InventoryGridPanel in index order (same order WBP_InventoryGrid fills InventorySlots)
	const UUserWidget* InventoryGrid = InventoryWidget.IsValid() ? Cast<UUserWidget>(InventoryWidget->GetWidgetFromName(TEXT("WBP_InventoryGrid"))) : nullptr;
	const UPanelWidget* GridPanel = InventoryGrid ? Cast<UPanelWidget>(InventoryGrid->GetWidgetFromName(TEXT("InventoryGridPanel"))) : nullptr;
	const UWidget* SlotWidget = GridPanel ? GridPanel->GetChildAt(SlotIndex) : nullptr;
	if (!SlotWidget) return false;

	const FGeometry& Geometry = SlotWidget->GetCachedGeometry();
	FVector2D PixelPosition;
	USlateBlueprintLibrary::LocalToViewport(const_cast<UES_ShopWidget*>(this), Geometry, Geometry.GetLocalSize() * 0.5f, PixelPosition, OutPosition);
	return true;
}
