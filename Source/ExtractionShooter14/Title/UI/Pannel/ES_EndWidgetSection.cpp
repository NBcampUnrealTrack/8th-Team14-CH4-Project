#include "ES_EndWidgetSection.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/PanelSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Title/UI/RowWidget/ES_EndWidgetRow.h"

void UES_EndWidgetSection::SetSectionTitle(const FText& NewTitle)
{
	SectionTitleText->SetText(NewTitle);
}

void UES_EndWidgetSection::AddRowSetRowTitle(const FText& InTitle, const FText& InValue)
{
	if (!IsValid(RowWidgetClass)) return;
	UES_EndWidgetRow* RowWidget = CreateWidget<UES_EndWidgetRow>(GetOwningPlayer(), RowWidgetClass);
	if (!IsValid(RowWidget)) return;
	
	RowWidget->SetTitle(InTitle, InValue);
	UVerticalBoxSlot* CustomSlot = RowVerticalBox->AddChildToVerticalBox(RowWidget);
	CustomSlot->SetPadding(FMargin(5.f,0.f));
	CustomSlot->SetVerticalAlignment(VAlign_Center);
	CustomSlot->SetHorizontalAlignment(HAlign_Center);
	CustomSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
}
