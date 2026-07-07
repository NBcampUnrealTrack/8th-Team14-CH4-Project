#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_EndWidgetSection.generated.h"

class UES_EndWidgetRow;
class UVerticalBox;
class UTextBlock;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_EndWidgetSection : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetSectionTitle(const FText& NewTitle);
	void AddRowSetRowTitle(const FText& InTitle, const FText& InValue);
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SectionTitleText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> RowVerticalBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UES_EndWidgetRow> RowWidgetClass;
};
