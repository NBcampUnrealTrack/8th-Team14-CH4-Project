#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_FlyIconWidget.generated.h"

class UImage;
class USizeBox;
class UTexture2D;
class UWidgetAnimation;

// purchase effect : icon flies in an arc from the shop row to the inventory slot, then removes itself
// position is set on its UCanvasPanelSlot, so it must be added to a canvas panel
UCLASS(Abstract)
class EXTRACTIONSHOOTER14_API UES_FlyIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void StartFly(UTexture2D* Icon, const FVector2D& InStartPosition, const FVector2D& InEndPosition);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FlyIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> FlyIconSize;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> StartAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "FlyIcon")
	float Duration = 0.6f;

	// arc peak height (pixels)
	UPROPERTY(EditDefaultsOnly, Category = "FlyIcon")
	float Height = 80.f;

	// shrink starts at this progress
	UPROPERTY(EditDefaultsOnly, Category = "FlyIcon")
	float ShrinkStartAlpha = 0.6f;

private:
	FVector2D StartPosition = FVector2D::ZeroVector;
	FVector2D EndPosition = FVector2D::ZeroVector;
	float ElapsedTime = 0.f;
	bool bIsFlying = false;
};
