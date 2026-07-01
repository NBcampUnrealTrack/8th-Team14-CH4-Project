#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Title/UI/ES_LobbyWidgetHUD.h"

AMyPlayerController::AMyPlayerController()
    :IsOpenOptionUI(false)
{
    //APlayerController의 기본값은 = true
    //의미하는바 = 매프레임 or possess할때 viewtarget을 possess하고있는 pawn으로 자동관리
    //한마디로 pawn의 시점을 강제하는 기능
    bAutoManageActiveCameraTarget = false;
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
        
        if (!IsValid(LobbyWidgetClass)) return;
        LobbyWidgetInstance = CreateWidget<UES_LobbyWidgetHUD>(this, LobbyWidgetClass);
        if (!IsValid(LobbyWidgetInstance)) return;
        LobbyWidgetInstance->AddToViewport();
       
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsWithTag(
            GetWorld(),
            FName("LobbyCamera"),
            FoundActors
        );
        
        Find_ApplyLobbyCamera();
        
        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void AMyPlayerController::Find_ApplyLobbyCamera()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyMainCamera"), FoundActors);
    ensureMsgf(FoundActors.Num() > 0, TEXT("None Camera"));
    
    SetViewTargetWithBlend(FoundActors[0], 0.f);
}

void AMyPlayerController::SetAutoManageCamera(bool bEnable)
{
    bAutoManageActiveCameraTarget = bEnable;
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
