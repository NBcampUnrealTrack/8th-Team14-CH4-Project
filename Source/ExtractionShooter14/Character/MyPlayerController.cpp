#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MainPlayerCharacterState.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterState.h"
#include "TestGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Title/UI/ES_LobbyWidgetHUD.h"
#include "Blueprint/WidgetLayoutLibrary.h"

class APlayerCharacter;

AMyPlayerController::AMyPlayerController()
    //:IsOpenOptionUI(false)
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
            for (const UInputMappingContext* MappingContext : MappingContextList)
            {
                if (IsValid(MappingContext))
                {
                    Subsystem->AddMappingContext(MappingContext, 0);
                }
            }
            // if (DefaultMappingContext)
            // {
            //     Subsystem->AddMappingContext(DefaultMappingContext, 0);
            // }
        }
    }
    
    if (IsLocalController())
    {
        if (!IsValid(ResultWidgetClass)) return;
        ResultWidgetInstance = CreateWidget<UUserWidget>(this, ResultWidgetClass);
        if (!IsValid(ResultWidgetInstance)) return;
        
        // if (!IsValid(OptionWidgetClass)) return;
        // OptionWidgetInstance = CreateWidget<UUserWidget>(this, OptionWidgetClass);
        // if (!IsValid(OptionWidgetInstance)) return;
        
    //     if (!IsValid(LobbyWidgetClass)) return;
    //     LobbyWidgetInstance = CreateWidget<UES_LobbyWidgetHUD>(this, LobbyWidgetClass);
    //     if (!IsValid(LobbyWidgetInstance)) return;
    //     LobbyWidgetInstance->AddToViewport();
    //     
    //     Find_ApplyLobbyCamera();
    //     
    // // INPUT MODE ON AND INPUT KEY
    //     FInputModeUIOnly InputMode;
    //     InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
    //     SetInputMode(InputMode);
    //     bShowMouseCursor = true;

    }
}

void AMyPlayerController::Find_ApplyLobbyCamera()
{
    SetAutoManageCamera(false);
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyMainCamera"), FoundActors);
    ensureMsgf(FoundActors.Num() > 0, TEXT("None Camera"));
    
    SetViewTargetWithBlend(FoundActors[0], 0.f);

}

void AMyPlayerController::InitLobbyWidgetandCachedPlayerState()
{
    UWidgetLayoutLibrary::RemoveAllWidgets(this);

    APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    
    if (!IsValid(PlayerStateES)) return;
    if (!IsValid(LobbyWidgetClass)) return;
    //if (!IsValid(LobbyWidgetInstance)) return;
    
    LobbyWidgetInstance = CreateWidget<UES_LobbyWidgetHUD>(this, LobbyWidgetClass);
    
    LobbyWidgetInstance->CachedPlayerState(PlayerStateES);
    LobbyWidgetInstance->CachedPlayerController(this);
    
    LobbyWidgetInstance->AddToViewport();
    // INPUT MODE ON AND INPUT KEY
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
    SetInputMode(InputMode);
    bShowMouseCursor = true;

}

void AMyPlayerController::SetAutoManageCamera(bool bEnable)
{
    bAutoManageActiveCameraTarget = bEnable;
}

void AMyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    InitLobbyWidgetandCachedPlayerState();
    Find_ApplyLobbyCamera();
}

void AMyPlayerController::Client_WhenMoveToLobbyUI_Implementation()
{
    InitLobbyWidgetandCachedPlayerState();
    Find_ApplyLobbyCamera();
}

void AMyPlayerController::Server_OnClickedResultUI_Implementation()
{// Die -> Goto Lobby and Client_WhenMoveTo // Live -> Client_WhenMoveTo
    if (HasAuthority())
    {
        ATestGameMode* GameModeES = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
        APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
        if (!IsValid(GameModeES)) return;
        if (!IsValid(PlayerStateES)) return;
        //if (!IsValid(// PLYAER CAHCHEDD)) return;
        if (PlayerStateES->GetbIsDead())
        {
            //PlayerStateES->InitiateStats();
            GameModeES->RespawnPlayerAtLobbyStartPoint(this);
        }
    }
}

void AMyPlayerController::Server_MoveToLobbyLocation_Implementation()
{
    ATestGameMode* GameModeES = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
    if (!IsValid(GameModeES)) return;
    
    GameModeES->SetPlayerLobbyStartPoint(this);
}

void AMyPlayerController::Client_WhenMoveToStagePopUpUI_Implementation()
{   //  ======================= when stage moved // play animontage ====================== //
    if (IsLocalController())
    {
        UWidgetLayoutLibrary::RemoveAllWidgets(this);
        
        FInputModeGameOnly GameMode;
        SetInputMode(GameMode);
        bShowMouseCursor = false;
    
        SetViewTargetWithBlend(GetPawn(), 0.f);
        SetAutoManageCamera(true);
    
        if (!IsValid(StageWidgetClass)) return;
        StageWidgetInstance = CreateWidget<UUserWidget>(this, StageWidgetClass);
        if (!IsValid(StageWidgetInstance)) return;
        StageWidgetInstance->AddToViewport();

    }
}

void AMyPlayerController::Client_OpenResultUI_Implementation()
{
    //UWidgetLayoutLibrary::RemoveAllWidgets(this);
    
    if (!IsValid(ResultWidgetInstance)) return;
    ResultWidgetInstance->AddToViewport(1);
    
    FInputModeUIOnly UIOnlyMode;
    SetInputMode(UIOnlyMode);
    bShowMouseCursor = true;
    //Find_ApplyLobbyCamera();
}

void AMyPlayerController::OpenOptionUI()
{

    if (!IsValid(OptionWidgetClass)) return;
    if (!IsValid(OptionWidgetInstance))
    {
        OptionWidgetInstance = CreateWidget<UUserWidget>(this, OptionWidgetClass);
        OptionWidgetInstance->AddToViewport(3);
    }
    else
    {
        OptionWidgetInstance->RemoveFromParent();
        OptionWidgetInstance=nullptr;
    }
}

#pragma region Cheat 

void AMyPlayerController::SetCheatCoin(int32 InCheatCoin)
{
    APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    PlayerStateES->Server_SetGold(InCheatCoin);
}

void AMyPlayerController::SetCheatStageCoin(int32 InCheatCoin)
{
    APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    PlayerStateES->Server_SetStageGold(InCheatCoin);
}


#pragma endregion 
