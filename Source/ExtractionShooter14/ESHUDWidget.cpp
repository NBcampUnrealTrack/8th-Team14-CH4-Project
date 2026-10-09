#include "ESHUDWidget.h"

#include "Character/PlayerCharacterState.h"
#include "Character/TestGameState.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Title/UI/Pannel/EscapeProgressWidget.h"
#include "Kismet/GameplayStatics.h"

void UESHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!OwningPlayerState) return;
	
	if (!PlayerRankDelegateHandle.IsValid())
	{
		PlayerRankDelegateHandle = 
			OwningPlayerState->OnPlayerRankChanged.AddUObject
				(this, &ThisClass::HandlePlayerRank);
		HandlePlayerRank();
	}
	
	HandleStageTime();
}

void UESHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
	
	if (!OwningPlayerState) return;
	
	 if (PlayerRankDelegateHandle.IsValid())
	 {
	 	OwningPlayerState->OnPlayerRankChanged.Remove(PlayerRankDelegateHandle);
	 	PlayerRankDelegateHandle.Reset();
	 }
}

void UESHUDWidget::HandleStageTime()
{
	UpdateStageTimerText();

	GetWorld()->GetTimerManager().SetTimer(
		StageTimerHandle,
		this,
		&ThisClass::UpdateStageTimerText,
		0.2f,
		true
	);
}

void UESHUDWidget::HandlePlayerRank()
{
	if (!OwningPlayerState) return;
	
	CurrentRank->SetText(FText::AsNumber(OwningPlayerState->GetMyRank()));
}

void UESHUDWidget::UpdateStageTimerText()
{
	float RemainTime = OwningGameState->GetRemainTime(); // 100seconds, 50seconds
	int32 TotalSeconds = FMath::CeilToInt(RemainTime); // float to int

	int32 Minutes =	TotalSeconds / 60; // 100seconds / 60 = 1
	int32 Seconds =	TotalSeconds % 60; // 100seconds % 60 = 40

	StageTime->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds)));
	
	ApplyTextColorAnimationByTime(RemainTime);
}

void UESHUDWidget::SetOwningGameState(ATestGameState* InGameState)
{
	OwningGameState = InGameState;
}

void UESHUDWidget::SetOwningPlayerState(APlayerCharacterState* InPlayerCharacterState)
{
	OwningPlayerState = InPlayerCharacterState;
}

void UESHUDWidget::ApplyTextColorAnimationByTime(float InRemainTime)
{
	if (!IsValid(OwningGameState)) return;
	
	const FPhaseTimerData& TimerData =
		OwningGameState->GetGameStatePhaseTimerData();

	if (TimerData.PhaseDuration <= 0.f) return;

	const EGameState GamePhase = TimerData.GameStateEnum;

	const float PassedTime =
		OwningGameState->GetServerWorldTimeSeconds()
		- TimerData.PhaseStartServerTime;

	const float Ratio = PassedTime / TimerData.PhaseDuration;
	
	if (GamePhase == EGameState::Playing)
	{
		if (Ratio < 0.2f)
		{
			StageTime->SetColorAndOpacity(FColor::Green);
		}
		else if (Ratio >= 0.2f)
		{
			if (InRemainTime <= 30.f)
			{
				StageTime->SetColorAndOpacity(FColor::Red);
			}
			else
			{
				StageTime->SetColorAndOpacity(FColor::White);
			}
		}
	}
}

void UESHUDWidget::CreateProgressWidget(UEscapeProgressWidget* InEPWidget)
{
	if (!IsValid(StageCanvasPanel)) return;
	if (!IsValid(InEPWidget)) return;
	
	UCanvasPanelSlot* CanvasSlot = StageCanvasPanel->AddChildToCanvas(InEPWidget);
	if (!IsValid(CanvasSlot)) return;
	
	CanvasSlot->SetAnchors(FAnchors(0.5f, 1.f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.f, -300.f));
	CanvasSlot->SetAutoSize(true);
}

