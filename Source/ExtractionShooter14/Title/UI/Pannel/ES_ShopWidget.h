#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Layout/Margin.h"
#include "ES_ShopWidget.generated.h"

class UCanvasPanel;
class UScrollBox;
class UES_FlyIconWidget;
class UES_ShopRowWidget;
class UWidgetAnimation;

// shop list : one row per DT_ShopItem row that has a price (DataManager decides)
// purchase result comes back from the server through AMyPlayerController::OnReceiveBuyItemResult
UCLASS(Abstract)
class EXTRACTIONSHOOTER14_API UES_ShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// where the purchase fly icon lands (called by WBP_ShopInven)
	//   InventoryWidget : WBP_Inventory (slot = InventoryGridPanel child of its WBP_InventoryGrid)
	//   EffectCanvas    : full screen canvas the fly icon is drawn on
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetFlyIconTarget(UUserWidget* InInventoryWidget, UCanvasPanel* InEffectCanvas);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BuildRows();
	void PlayFlyIcon(const UES_ShopRowWidget* Row, int32 StageInventoryIndex);
	bool GetInventorySlotViewportPosition(int32 SlotIndex, FVector2D& OutPosition) const;

	UFUNCTION()
	void HandleBuyItemResult(FName ItemName, bool Result, int32 StageInventoryIndex);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ShopItemListScrollBox;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OpenUpAnimation_HUD;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UES_ShopRowWidget> ShopRowWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	FMargin RowPadding = FMargin(20.f, 15.f);

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UES_FlyIconWidget> FlyIconClass;

private:
	// item name -> row (find the row a server result belongs to)
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UES_ShopRowWidget>> RowWidgets;

	TWeakObjectPtr<UUserWidget> InventoryWidget;
	TWeakObjectPtr<UCanvasPanel> EffectCanvas;
};
