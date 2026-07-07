#include "ES_EndWidgetSection.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
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
	RowVerticalBox->AddChild(RowWidget);
}
