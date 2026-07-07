#include "Title/UI/RowWidget/ES_EndWidgetRow.h"
#include "Components/TextBlock.h"

void UES_EndWidgetRow::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayAnimation(FadeUpAnimation_Tit);
	
	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeUpAnimation_Tit_Finished);

	BindToAnimationFinished(FadeUpAnimation_Tit, Event);
	
}

void UES_EndWidgetRow::NativeDestruct()
{
	Super::NativeDestruct();
	
	UnbindAllFromAnimationFinished(FadeUpAnimation_Tit);
	UnbindAllFromAnimationFinished(FadeUpAnimation_Dir);
}

void UES_EndWidgetRow::SetTitle(const FText& Title, const FText& Value)
{
	InfoCategory->SetText(Title);
	InfoValue->SetText(Value);
}

void UES_EndWidgetRow::OnFadeUpAnimation_Tit_Finished()
{
	
	PlayAnimation(FadeUpAnimation_Dir);
	
	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeUpAnimation_Dir_Finished);

	BindToAnimationFinished(FadeUpAnimation_Dir, Event);
}

void UES_EndWidgetRow::OnFadeUpAnimation_Dir_Finished()
{
	PlayAnimation(FadeUpAnimation_Val);
}