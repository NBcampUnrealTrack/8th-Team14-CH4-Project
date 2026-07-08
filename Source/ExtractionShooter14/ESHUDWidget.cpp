#include "ESHUDWidget.h"

#include "Character/PlayerCharacterState.h"
#include "Character/TestGameState.h"
#include "Components/TextBlock.h"

#include "Kismet/GameplayStatics.h"

void UESHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!OwningPlayerState) return;
	
	if (!PlayerRankDelegateHandle.IsValid())
	{
		PlayerRankDelegateHandle = 
			OwningPlayerState->OnPlayerCombatInfoChanged.AddUObject
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
	 	OwningPlayerState->OnPlayerCombatInfoChanged.Remove(PlayerRankDelegateHandle);
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
	
	const FCollectCombatInfo& CombatInfo = OwningPlayerState->GetPlayerCombatInfo();
	
	CurrentRank->SetText(FText::AsNumber(CombatInfo.Rank));
}

void UESHUDWidget::UpdateStageTimerText()
{
	float RemainTime = OwningGameState->GetRemainTime(); // 100seconds, 50seconds
	int32 TotalSeconds = FMath::CeilToInt(RemainTime); // float to int

	int32 Minutes =	TotalSeconds / 60; // 100seconds / 60 = 1
	int32 Seconds =	TotalSeconds % 60; // 100seconds % 60 = 40

	StageTime->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds)));
}

void UESHUDWidget::SetOwningGameState(ATestGameState* InGameState)
{
	OwningGameState = InGameState;
}

void UESHUDWidget::SetOwningPlayerState(APlayerCharacterState* InPlayerCharacterState)
{
	OwningPlayerState = InPlayerCharacterState;
}
