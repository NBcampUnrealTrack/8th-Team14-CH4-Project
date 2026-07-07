#include "ES_ResaultWidgetHUD.h"

#include "Pannel/ES_EndWidgetSection.h"

void UES_ResaultWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayAnimation(FadeInAnimation_Com);
	
	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeInAnimation_Com_Finished);

	BindToAnimationFinished(FadeInAnimation_Com, Event);
}

void UES_ResaultWidgetHUD::NativeDestruct()
{
	Super::NativeDestruct();
	
	UnbindAllFromAnimationFinished(FadeInAnimation_Com);
	UnbindAllFromAnimationFinished(FadeInAnimation_Col);
}

void UES_ResaultWidgetHUD::SetPlayerCombatInfo(const FCollectCombatInfo& CombatInfo)
{
	PlayerCombatInfo = CombatInfo;
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
	CombatSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("DamageDealt")), FText::AsNumber(PlayerCombatInfo.DamageDealt));
	CombatSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("DamageTaken")), FText::AsNumber(PlayerCombatInfo.DamageTaken));
	CombatSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("KillCount")), FText::AsNumber(PlayerCombatInfo.KillCount));
}

void UES_ResaultWidgetHUD::AddRowInCollectSection()
{
	CollectSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("LootCount")), FText::AsNumber(PlayerCombatInfo.LootCount));
	CollectSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("SurviveTime")), FText::AsNumber(PlayerCombatInfo.SurviveTime));
	CollectSectionWidget->AddRowSetRowTitle(FText::FromString(TEXT("Score")), FText::AsNumber(PlayerCombatInfo.Score));
}



