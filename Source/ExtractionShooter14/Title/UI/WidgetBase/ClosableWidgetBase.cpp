#include "ClosableWidgetBase.h"
#include "Character/MyPlayerController.h"

void UClosableWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

FReply UClosableWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
			{
				MyPC->OnPressedEscape();   //
			}
			return FReply::Handled();
		}

		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}
}
