#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClosableWidgetBase.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API UClosableWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
};
