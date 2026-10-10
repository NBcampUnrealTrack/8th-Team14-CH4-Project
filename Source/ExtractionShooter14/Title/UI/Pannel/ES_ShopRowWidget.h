#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_ShopRowWidget.generated.h"

struct FShopItemDisplay;
class UButton;
class UImage;
class UTextBlock;
class UTexture2D;
class USoundBase;
class UWidgetAnimation;

// one shop row : shows a FShopItemDisplay and sends the buy request
// sends only the row name, price and validation are on the server (APlayerCharacterState::BuyItem)
// Abstract : layout lives in WBP_ShopRowWidget (BindWidget), the bare class has no widget tree
UCLASS(Abstract)
class EXTRACTIONSHOOTER14_API UES_ShopRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FShopItemDisplay& Display);

	// result from server (APlayerController::OnReceiveBuyItemResult)
	void PlayPurchaseResult(bool bSuccess);

	FName GetItemName() const { return ItemName; }

	// fly icon start point / image
	UFUNCTION(BlueprintPure, Category = "Shop")
	FVector2D GetItemIconViewportPosition() const;

	UFUNCTION(BlueprintPure, Category = "Shop")
	UTexture2D* GetItemIconTexture() const { return ItemIconTexture; }

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleHovered();

	UFUNCTION()
	void HandleUnhovered();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PurchaseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	// primary value (weapon : Damage / use item : EffectAmount)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Icon_1_Value;

	// secondary value (weapon : MagazineSize / use item : UseTime)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Icon_2_Value;

	// price
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Icon_3_Value;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Hovered;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> NotEnoughMoney;

	UPROPERTY(EditDefaultsOnly, Category = "Shop|Sound")
	TObjectPtr<USoundBase> ClickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Shop|Sound")
	TObjectPtr<USoundBase> HoverSound;

private:
	FName ItemName;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ItemIconTexture;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> PurchaseSound;
};
