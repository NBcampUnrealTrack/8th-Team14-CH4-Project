#include "EscapeProgressWidget.h"
#include "Character/TestGameState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEscapeProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	DurationTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), DurationTime)));
	
	PlayAnimation(FadeInAnimation);
	PlayAnimation(TextAnimation,0.f, 0);
	
	GetWorld()->GetTimerManager().SetTimer(
	EscapeTimerHandle,
	this,
	&ThisClass::UpdateEscapeProgressBar_Text,
	0.05f,
	true
	);
}

void UEscapeProgressWidget::NativeDestruct()
{
	UnbindAllFromAnimationFinished(FadeInAnimation);
	GetWorld()->GetTimerManager().ClearTimer(EscapeTimerHandle);
	
	Super::NativeDestruct();
}

void UEscapeProgressWidget::InitializePrivateProperties(ATestGameState* GS, float InStartServerTime, float InDurationTime)
{
	if (!IsValid(GS)) return;
	
	OwningGameState = GS;
	DurationTime = InDurationTime;
	OverLappedTime = InStartServerTime;
}

void UEscapeProgressWidget::CloseEscapeProgressWidget()
{
	
	PlayAnimation(FadeInAnimation,0.f, 1, EUMGSequencePlayMode::Reverse);
	
	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeInAnimation_Finished);
	BindToAnimationFinished(FadeInAnimation, Event);
}

void UEscapeProgressWidget::OnFadeInAnimation_Finished()
{
	GetWorld()->GetTimerManager().ClearTimer(EscapeTimerHandle);
	this->RemoveFromParent();
}

void UEscapeProgressWidget::UpdateEscapeProgressBar_Text()
{
	if (!IsValid(OwningGameState)) return;
	
	const float CurrentServerTime = OwningGameState->GetServerWorldTimeSeconds();
	const float PassedTime = FMath::Clamp(CurrentServerTime - OverLappedTime, 0.f, DurationTime);
	const float Ratio =	PassedTime / DurationTime;
	
	EscapeProgressBar->SetPercent(Ratio);
	StartTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), PassedTime)));
}
