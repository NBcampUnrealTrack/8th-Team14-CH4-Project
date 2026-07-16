#include "ES_ResaultWidgetHUD.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/MyPlayerController.h"
#include "Character/PlayerCharacterState.h"
#include "Components/TextBlock.h"
#include "Pannel/ES_EndWidgetSection.h"

void UES_ResaultWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	// if (!PlayerCommitInfoDelegateHandle.IsValid())
	// {
	// 	APlayerCharacterState* PlayerStateES = GetOwningPlayer()->GetPlayerState<APlayerCharacterState>();
	// 	if (!IsValid(PlayerStateES)) return;
	// 	PlayerStateES->OnPlayerCombatInfoChanged.AddUObject(this, &ThisClass::)
	// }
	CombatSectionWidget->SetSectionTitle(FText::FromString(TEXT("Combat")));
	CollectSectionWidget->SetSectionTitle(FText::FromString(TEXT("Collection")));
	RankTextBlock->SetText(FText::FromString(FString::Printf(TEXT("# %d"),PlayerCombatInfo.Rank)));
	
	PlayAnimation(FadeInAnimation_Com);
	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeInAnimation_Com_Finished);
	BindToAnimationFinished(FadeInAnimation_Com, Event);
}

void UES_ResaultWidgetHUD::NativeDestruct()
{

	UnbindAllFromAnimationFinished(FadeInAnimation_Com);
	UnbindAllFromAnimationFinished(FadeInAnimation_Col);
	GetWorld()->GetTimerManager().ClearTimer(CombatRowTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(CollectRowTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ReturnToLobbyTimerHandle);
	Super::NativeDestruct();
}

void UES_ResaultWidgetHUD::SetPlayerCombatInfo(const FCollectCombatInfo& CombatInfo)
{
	PlayerCombatInfo = CombatInfo;
}
void UES_ResaultWidgetHUD::SetPlayerNickName(const FText& NickName)
{
	NickNameTextBlock->SetText(NickName);
}
void UES_ResaultWidgetHUD::OnFadeInAnimation_Com_Finished()
{
	AddRowInComBatSection();
	
	PlayAnimation(FadeInAnimation_Col);
	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeInAnimation_Col_Finished);

	BindToAnimationFinished(FadeInAnimation_Col, Event);
}

void UES_ResaultWidgetHUD::OnFadeInAnimation_Col_Finished()
{
	AddRowInCollectSection();
}

void UES_ResaultWidgetHUD::AddRowInComBatSection()
{
	CombatInfoArray =
	{
		// CombatSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("DamageDealt")), FText::AsNumber(PlayerCombatInfo.DamageDealt));
		// CombatSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("DamageTaken")), FText::AsNumber(PlayerCombatInfo.DamageTaken));
		// CombatSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("KillCount")), FText::AsNumber(PlayerCombatInfo.KillCount));
		{ FText::FromString(TEXT("DamageDealt")), FText::AsNumber(PlayerCombatInfo.DamageDealt) },
		{ FText::FromString(TEXT("DamageTaken")), FText::AsNumber(PlayerCombatInfo.DamageTaken) },
		{ FText::FromString(TEXT("KillCount")), FText::AsNumber(PlayerCombatInfo.KillCount) }
	};
	
	CombatInfoArrayIndex = 0;

	GetWorld()->GetTimerManager().SetTimer(
		CombatRowTimerHandle,
		this,
		&ThisClass::AddCombatRowByTime,
		0.35f,
		true
	);
}

void UES_ResaultWidgetHUD::AddRowInCollectSection()
{
	CollectInfoArray =
	{
		// CollectSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("LootCount")), FText::AsNumber(PlayerCombatInfo.LootCount));
		// CollectSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("SurviveTime")), FText::AsNumber(PlayerCombatInfo.SurviveTime));
		// CollectSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("Score")), FText::AsNumber(PlayerCombatInfo.Score));
		{ FText::FromString(TEXT("LootCount")), FText::AsNumber(PlayerCombatInfo.LootCount) },
		{ FText::FromString(TEXT("SurviveTime")), FText::FromString(FString::Printf(TEXT("%.1f"),PlayerCombatInfo.SurviveTime)) },
		{ FText::FromString(TEXT("Score")), FText::AsNumber(PlayerCombatInfo.Score) }
	};	
	
	CollectInfoArrayIndex = 0;

	GetWorld()->GetTimerManager().SetTimer(
		CollectRowTimerHandle,
		this,
		&ThisClass::AddCollectRowByTime,
		0.35f,
		true
	);
}

void UES_ResaultWidgetHUD::AddCombatRowByTime()
{
	if (CombatInfoArrayIndex == 3)
	{
		GetWorld()->GetTimerManager().ClearTimer(CombatRowTimerHandle);
		return;
	}
	CombatSectionWidget->AddRowSetRowTitle(CombatInfoArray[CombatInfoArrayIndex].Key, CombatInfoArray[CombatInfoArrayIndex].Value);
	CombatInfoArrayIndex++;
	
}

void UES_ResaultWidgetHUD::AddCollectRowByTime()
{
	if (CollectInfoArrayIndex == 3)
	{
		PlayAnimation(FadeInAnimation_ReT);
		GetWorld()->GetTimerManager().ClearTimer(CollectRowTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
		ReturnToLobbyTimerHandle,
		[this]()
		{
			if (!IsValid(this)) return;
			PlayAnimation(FadeInAnimation_ReL);
			ReturnToLobbyTimeText->SetText(FText::AsNumber(FMath::Max(ReturnToLobbyTime,0)));
			ReturnToLobbyTime--;
			if (ReturnToLobbyTime == -1 )
			{
				GetWorld()->GetTimerManager().ClearTimer(ReturnToLobbyTimerHandle);
				AMyPlayerController* PlayerES = Cast<AMyPlayerController>(GetOwningPlayer());
				if (!IsValid(PlayerES)) return;
				RemoveFromParent();
				
				PlayerES->Server_OnClickedResultUI();
			}
		},
		1.f,
		true);
		
		return;
	}
	CollectSectionWidget->AddRowSetRowTitle(CollectInfoArray[CollectInfoArrayIndex].Key, CollectInfoArray[CollectInfoArrayIndex].Value);
	CollectInfoArrayIndex++;
}



