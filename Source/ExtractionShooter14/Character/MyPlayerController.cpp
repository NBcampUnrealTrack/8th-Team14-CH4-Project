#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

AMyPlayerController::AMyPlayerController()
    :IsOpenOptionUI(false)
{
    
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
    
    if (IsLocalController())
    {
        if (!IsValid(ResultWidgetClass)) return;
        ResultWidgetInstance = CreateWidget<UUserWidget>(this, ResultWidgetClass);
        if (!IsValid(ResultWidgetInstance)) return;
        
        if (!IsValid(OptionWidgetClass)) return;
        OptionWidgetInstance = CreateWidget<UUserWidget>(this, OptionWidgetClass);
        if (!IsValid(OptionWidgetInstance)) return;
    }
}

void AMyPlayerController::OpenOptionUI()
{
    if (!IsValid(OptionWidgetInstance)) return;
    OptionWidgetInstance->AddToViewport(1);
    IsOpenOptionUI = true;
    FInputModeGameAndUI GameUIMode;
    SetInputMode(GameUIMode);
    bShowMouseCursor = true;
}

void AMyPlayerController::CloseOptionUI()
{
    if (!IsValid(OptionWidgetInstance)) return;
    OptionWidgetInstance->RemoveFromParent();
    IsOpenOptionUI = false;
    FInputModeGameOnly GameMode;
    SetInputMode(GameMode);
    bShowMouseCursor = false;
    
}

void AMyPlayerController::Client_OpenResultUI_Implementation()
{
    if (!IsValid(ResultWidgetInstance)) return;
    ResultWidgetInstance->AddToViewport();
}
