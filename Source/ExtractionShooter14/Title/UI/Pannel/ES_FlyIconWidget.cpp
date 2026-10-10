#include "ES_FlyIconWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

void UES_FlyIconWidget::StartFly(UTexture2D* Icon, const FVector2D& InStartPosition, const FVector2D& InEndPosition)
{
	if (Icon) FlyIconImage->SetBrushFromTexture(Icon);

	StartPosition = InStartPosition;
	EndPosition = InEndPosition;
	ElapsedTime = 0.f;
	bIsFlying = true;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(StartPosition);
	}

	if (StartAnimation) PlayAnimationForward(StartAnimation);
}

void UES_FlyIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsFlying) return;

	ElapsedTime += InDeltaTime;
	const float RawAlpha = FMath::Clamp(Duration > 0.f ? ElapsedTime / Duration : 1.f, 0.f, 1.f);
	const float Alpha = FMath::InterpEaseOut(0.f, 1.f, RawAlpha, 2.f);

	// straight line + sine arc on top
	FVector2D Position = FMath::Lerp(StartPosition, EndPosition, Alpha);
	Position.Y -= FMath::Sin(PI * RawAlpha) * Height;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetPosition(Position);
	}

	if (RawAlpha >= ShrinkStartAlpha)
	{
		FlyIconSize->SetRenderScale(FVector2D(1.f + ShrinkStartAlpha - RawAlpha));
	}

	if (Alpha >= 1.f)
	{
		bIsFlying = false;
		RemoveFromParent();
	}
}
