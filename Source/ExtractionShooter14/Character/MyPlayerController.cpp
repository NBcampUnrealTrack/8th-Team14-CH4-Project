#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MainPlayerCharacterState.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterState.h"
#include "TestGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Title/UI/ES_LobbyWidgetHUD.h"
#include "Title/UI/ES_ResaultWidgetHUD.h"
#include "Title/UI/ES_OptionWidget.h"
#include "Character/TestGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ESHUDWidget.h"
#include "Components/AudioComponent.h"
#include "Components/Button.h"
#include "Game/ES_AudioSubsystem.h"
#include "Sound/SoundBase.h"

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
    
    // if (IsLocalController())
    // {
    //     if (!IsValid(ResultWidgetClass)) return;
    //     UES_ResaultWidgetHUD* ResultWidgetInstance = CreateWidget<UES_ResaultWidgetHUD>(this, ResultWidgetClass);
    //     if (!IsValid(ResultWidgetInstance)) return;
    //     
    //     if (!IsValid(OptionWidgetClass)) return;
    //     OptionWidgetInstance = CreateWidget<UUserWidget>(this, OptionWidgetClass);
    //     if (!IsValid(OptionWidgetInstance)) return;
    //     
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
    //
    // }
}

void AMyPlayerController::Find_ApplyLobbyCamera()
{
    if (IsLocalController())
    {
        APlayerCharacterState* PlayerCharacterStateES = GetPlayerState<APlayerCharacterState>();
        
        if (!IsValid(PlayerCharacterStateES)) return;
        
        const int32 PlayerSlotIndex = PlayerCharacterStateES->GetLobbyIndex();
        
        TArray<AActor*> FoundCameras;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyMainCamera"), FoundCameras);
        ensureMsgf(FoundCameras.Num() > 0, TEXT("None Camera"));

        for (const AActor* Camera : FoundCameras)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s"), *Camera->GetName());
        }
        
        FoundCameras.Sort([](const AActor& A, const AActor& B)
        {
            return A.GetName() < B.GetName();
        }); // sort by name camera_1 , camera_2 , camera_3
        
        if (!FoundCameras.IsValidIndex(PlayerSlotIndex)) return;
        
        SetAutoManageCamera(false);
        SetViewTargetWithBlend(FoundCameras[PlayerSlotIndex], 0.f);
    }
}

void AMyPlayerController::InitLobbyWidgetandCachedPlayerState()
{
    UWidgetLayoutLibrary::RemoveAllWidgets(this);
    
    APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    ATestGameState* GameStateES = Cast<ATestGameState>(UGameplayStatics::GetGameState(this));
    
    if (!IsValid(PlayerStateES)) return;
    if (!IsValid(GameStateES)) return;
    if (!IsValid(LobbyWidgetClass)) return;
    //if (!IsValid(LobbyWidgetInstance)) return;
    
    LobbyWidgetInstance = CreateWidget<UES_LobbyWidgetHUD>(this, LobbyWidgetClass);
    
    LobbyWidgetInstance->CachedPlayerState(PlayerStateES);
    LobbyWidgetInstance->CachedGameState(GameStateES);
    LobbyWidgetInstance->CachedPlayerController(this);
    
    LobbyWidgetInstance->AddToViewport();
    // INPUT MODE ON AND INPUT KEY
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
    SetInputMode(InputMode);
    bShowMouseCursor = true;

    //PlayLobbyBGM();
    if (UES_AudioSubsystem* Audio = GetGameInstance()->GetSubsystem<UES_AudioSubsystem>())
    {
        Audio->PlayBGM(LobbyBGM);
    }
}

void AMyPlayerController::StopPlayerMovement()
{
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    
    ACharacter* CharacterES = Cast<ACharacter>(GetPawn());
    if (!IsValid(CharacterES)) return;
    CharacterES->GetCharacterMovement()->StopMovementImmediately();
    CharacterES->GetCharacterMovement()->DisableMovement();
    // CharacterES->SetActorRotation(FRotator::ZeroRotator);
    // SetControlRotation(FRotator::ZeroRotator);
}

void AMyPlayerController::StartPlayerMovement()
{
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);

    ACharacter* CharacterES = Cast<ACharacter>(GetPawn());
    if (!IsValid(CharacterES)) return;
    CharacterES->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    // CharacterES->SetActorRotation(FRotator::ZeroRotator);
    // SetControlRotation(FRotator::ZeroRotator);

}

void AMyPlayerController::Client_ApplySpawnRotation_Implementation(FRotator Rot)
{
    if (ACharacter* Ch = Cast<ACharacter>(GetPawn()))
    {
        Ch->SetActorRotation(Rot);
    }

    SetControlRotation(Rot);
}

void AMyPlayerController::SetAutoManageCamera(bool bEnable)
{
    bAutoManageActiveCameraTarget = bEnable;
}

void AMyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    StopPlayerMovement();
    InitLobbyWidgetandCachedPlayerState();
    Find_ApplyLobbyCamera();
}

void AMyPlayerController::Client_WhenMoveToLobbyUI_Implementation()
{
    // SetIgnoreMoveInput(true);
    // SetIgnoreLookInput(true);
    
    // ACharacter* CharacterES = Cast<ACharacter>(GetPawn());
    // CharacterES->GetCharacterMovement()->StopMovementImmediately();
    // CharacterES->GetCharacterMovement()->DisableMovement();
    StopPlayerMovement();
    InitLobbyWidgetandCachedPlayerState();
    Find_ApplyLobbyCamera();
    ClearWidgetStack();
    
    UES_AudioSubsystem* Audio = GetGameInstance()->GetSubsystem<UES_AudioSubsystem>();
    if (Audio)
    {
        Audio->PlayBGM(LobbyBGM);
    }
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
        UE_LOG(LogTemp, Warning, TEXT("Client_WhenMoveToStagePopUpUI Called"));
        //StopBGM(0.5f);

        APlayerCharacterState* PS = GetPlayerState<APlayerCharacterState>();
        if (!IsValid(PS)) return;
        ATestGameState* GameStateES = GetWorld()->GetGameState<ATestGameState>();
        if (!IsValid(GameStateES)) return;
            
        StartPlayerMovement();
        
        UWidgetLayoutLibrary::RemoveAllWidgets(this);
        ClearWidgetStack();
        
        FInputModeGameOnly GameMode;
        SetInputMode(GameMode);
        bShowMouseCursor = false;
    
        SetViewTargetWithBlend(GetPawn(), 0.f);
        SetAutoManageCamera(true);
    
        if (!IsValid(StageWidgetClass)) return;
        StageWidgetInstance = CreateWidget<UESHUDWidget>(this, StageWidgetClass);

        StageWidgetInstance->SetOwningGameState(GameStateES);
        StageWidgetInstance->SetOwningPlayerState(PS);
        
        if (!IsValid(StageWidgetInstance)) return;
        StageWidgetInstance->AddToViewport(0);

        PS->OnHealthChanged.RemoveDynamic(this, &AMyPlayerController::OnPlayerHealthChanged);
        PS->OnHealthChanged.AddDynamic(this, &AMyPlayerController::OnPlayerHealthChanged);
        
        StageWidgetInstance.Get()->BP_UpdatePlayerHP(
            PS->GetCurrentHealth(),
            PS->GetMaxHealth()
        );
        
        UES_AudioSubsystem* Audio = GetGameInstance()->GetSubsystem<UES_AudioSubsystem>();
        if (Audio)
        {
            Audio->PlayBGM(StageBGM);
        }
    }
}

void AMyPlayerController::Client_OpenResultUI_Implementation()
{
    //UWidgetLayoutLibrary::RemoveAllWidgets(this);
    APlayerCharacterState* PS = GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PS)) return;
    if (!IsValid(ResultWidgetClass)) return;
    
    UES_ResaultWidgetHUD* ResultWidgetInstance = CreateWidget<UES_ResaultWidgetHUD>(this, ResultWidgetClass);
    ResultWidgetInstance->SetPlayerCombatInfo(PS->GetPlayerCombatInfo());
    ResultWidgetInstance->SetPlayerNickName(FText::FromString(PS->GetPlayerInfo().PlayerNickName));
    
    ResultWidgetInstance->AddToViewport(BaseUIZOrder + WidgetStack.Num());
    //ResultWidgetInstance->SetFocus();
    // ACharacter* PlayerCharcterES= Cast<ACharacter>(GetPawn());
    // if (!IsValid(PlayerCharcterES)) return;
    // PlayerCharcterES->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    
    FInputModeUIOnly UIOnlyMode;
    SetInputMode(UIOnlyMode);
    bShowMouseCursor = true;
    //Find_ApplyLobbyCamera();
}

void AMyPlayerController::PushWidget(UUserWidget* NewWidget)
{
    if (!IsValid(NewWidget)) return;

    const int32 ZOrder = BaseUIZOrder + WidgetStack.Num();

    NewWidget->AddToViewport(ZOrder);
    WidgetStack.Add(NewWidget);

    UpdateInputMode();
}

void AMyPlayerController::OpenOptionUI()
{
    if (!IsValid(OptionWidgetClass)) return;
    if (IsValid(OptionWidgetInstance)) return;

    OptionWidgetInstance =
        CreateWidget<UES_OptionWidget>(this, OptionWidgetClass);

    if (!IsValid(OptionWidgetInstance)) return;

    PushWidget(OptionWidgetInstance);
}

void AMyPlayerController::CloseOptionUI()
{
    if (!IsValid(OptionWidgetInstance)) return;

    WidgetStack.Remove(OptionWidgetInstance);

    OptionWidgetInstance->RemoveFromParent();
    OptionWidgetInstance = nullptr;

    UpdateInputMode();
}

void AMyPlayerController::OnPressedEscape()
{
    WidgetStack.RemoveAll(
        [](const TObjectPtr<UUserWidget>& Widget)
        {
            return !IsValid(Widget);
        });

    if (WidgetStack.Num() > 0)
    {
        UUserWidget* TopWidget = WidgetStack.Pop();

        if (TopWidget == OptionWidgetInstance)
        {
            OptionWidgetInstance = nullptr;
        }

        if (IsValid(TopWidget))
        {
            TopWidget->RemoveFromParent();
        }

        UpdateInputMode();
        return;
    }

    OpenOptionUI();
}

void AMyPlayerController::FocusWidget(UUserWidget* Widget)
{
    if (!IsValid(Widget)) return;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(Widget->TakeWidget());   // 이 위젯에 포커스
    SetInputMode(InputMode);
    bShowMouseCursor = true;
}

void AMyPlayerController::UpdateInputMode()
{
    APlayerCharacterState* PS =
        GetPlayerState<APlayerCharacterState>();

    if (!IsValid(PS)) return;

    // 스택에 UI가 있으면 가장 위 UI에 포커스
    if (WidgetStack.Num() > 0)
    {
        FocusWidget(WidgetStack.Last());
        return;
    }

    // 스택이 비었을 때 장소별 기본 입력 모드
    switch (PS->GetPlayerStagePlace())
    {
    case EPlayerPlace::Stage:
        {
            FInputModeGameOnly InputMode;
            SetInputMode(InputMode);
            bShowMouseCursor = false;
            break;
        }

    case EPlayerPlace::Lobby:
        {
            FocusWidget(LobbyWidgetInstance);
            break;
        }

    default:
        break;
    }
    
    // APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    // if (!IsValid(PlayerStateES)) return;
    //
    // bool bAnyUIOpen = (WidgetStack.Num() > 0) || IsValid(OptionWidgetInstance);
    // EPlayerPlace Place = PlayerStateES->GetPlayerStagePlace();
    //
    // // GameOnly가 되는 유일한 경우: Stage + UI 전부 닫힘
    // if (Place == EPlayerPlace::Stage && !bAnyUIOpen)
    // {
    //     FInputModeGameOnly GameMode;
    //     SetInputMode(GameMode);
    //     bShowMouseCursor = false;
    // }
    // else if (Place == EPlayerPlace::Lobby && !bAnyUIOpen)
    // {
    //     // 그 외 = UIOnly. 포커스 줄 위젯이 있으면 그것에 포커스
    //     UUserWidget* FocusTarget = nullptr;
    //     if (WidgetStack.Num() > 0)
    //         FocusTarget = WidgetStack.Last();
    //     else if (IsValid(LobbyWidgetInstance))
    //         FocusTarget = LobbyWidgetInstance;
    //
    //     if (FocusTarget)
    //     {
    //         FocusWidget(FocusTarget);   // UIOnly + 포커스
    //     }
    //     else
    //     {
    //         // UI가 없는데 UIOnly인 경우 (Lobby에서 UI 다 닫힘) << ? 소용없는 로직
    //         FInputModeUIOnly UIMode;
    //         SetInputMode(UIMode);
    //         bShowMouseCursor = true;
    //     }
    // }
}

void AMyPlayerController::MakeSettingWidget()
{
    if (!IsValid(SettingWidgetClass)) return;
    SettingWidgetInstance = CreateWidget<UUserWidget>(this, SettingWidgetClass);
    if (!IsValid(SettingWidgetInstance)) return;
    
    UUserWidget* CloseButtonWidget = 
            Cast<UUserWidget>(SettingWidgetInstance->GetWidgetFromName(TEXT("ReturnButtonWidget")));
    
    UButton* CloseButton =
        Cast<UButton>(CloseButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
    
    CloseButton->OnClicked.AddDynamic(this, &ThisClass::OnPressedEscape);
    
    PushWidget(SettingWidgetInstance);
}

void AMyPlayerController::ToggleShopWidget()
{
    if (IsValid(ShopWidgetInstance) &&
        ShopWidgetInstance->IsInViewport())
    {
        CloseShopWidget();
        return;
    }

    if (!IsValid(ShopWidgetClass)) return;

    if (!IsValid(ShopWidgetInstance))
    {
        ShopWidgetInstance =
            CreateWidget<UUserWidget>(this, ShopWidgetClass);
    }

    if (!IsValid(ShopWidgetInstance)) return;

    PushWidget(ShopWidgetInstance);
}

void AMyPlayerController::CloseShopWidget()
{
    if (!IsValid(ShopWidgetInstance)) return;

    WidgetStack.Remove(ShopWidgetInstance);

    ShopWidgetInstance->RemoveFromParent();
    ShopWidgetInstance = nullptr;

    UpdateInputMode();
}

void AMyPlayerController::ClearWidgetStack()
{
    WidgetStack.Reset();
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

void AMyPlayerController::SetCheatItem(FName ItemName, int32 ItemCount)
{
    APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
}

void AMyPlayerController::SetCheatStageItem(FName ItemName, int32 ItemCount)
{
    APlayerCharacterState* PlayerStateES = GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
    FInventoryItemData ItemData;
    ItemData.ItemName = ItemName;
    ItemData.ItemCount = ItemCount;
    PlayerStateES->Server_SetStageItem(ItemData);
}


#pragma endregion 

void AMyPlayerController::OnPlayerHealthChanged(float CurrentHP, float MaxHP)
{
    UE_LOG(LogTemp, Warning, TEXT("HP Changed: %.1f / %.1f"), CurrentHP, MaxHP);

    if (!IsValid(StageWidgetInstance.Get())) return;

    StageWidgetInstance.Get()->BP_UpdatePlayerHP(CurrentHP, MaxHP);
    StageWidgetInstance.Get()->BP_PlayerDamageFlash();
}

void AMyPlayerController::Client_OnReceiveBuyItemResult_Implementation(const FName& BuyItemID, bool Result, int32 InventoryIndex)
{
    OnReceiveBuyItemResult.Broadcast(BuyItemID, Result, InventoryIndex);
}

// void AMyPlayerController::PlayBGM(USoundBase* NewBGM, float Volume)
// {
//     UE_LOG(LogTemp, Warning, TEXT("PlayBGM Called / Local=%d / BGM=%s / Volume=%.2f"),
//         IsLocalController(),
//         *GetNameSafe(NewBGM),
//         Volume
//     );
//
//     if (!IsLocalController())
//     {
//         UE_LOG(LogTemp, Warning, TEXT("PlayBGM Failed: Not Local"));
//         return;
//     }
//
//     if (!IsValid(NewBGM))
//     {
//         UE_LOG(LogTemp, Warning, TEXT("PlayBGM Failed: NewBGM invalid"));
//         return;
//     }
//
//     if (CurrentBGMSound == NewBGM &&
//         IsValid(CurrentBGMComponent) &&
//         CurrentBGMComponent->IsPlaying())
//     {
//         UE_LOG(LogTemp, Warning, TEXT("PlayBGM Skipped: Already playing"));
//         return;
//     }
//
//     if (IsValid(CurrentBGMComponent))
//     {
//         CurrentBGMComponent->Stop();
//         CurrentBGMComponent = nullptr;
//     }
//
//     CurrentBGMComponent = UGameplayStatics::SpawnSound2D(
//         this,
//         NewBGM,
//         Volume,
//         1.0f,
//         0.0f,
//         nullptr,
//         false,
//         false
//     );
//
//     CurrentBGMSound = NewBGM;
//
//     UE_LOG(LogTemp, Warning, TEXT("PlayBGM Spawned / Component=%s"),
//         *GetNameSafe(CurrentBGMComponent)
//     );
// }
// void AMyPlayerController::PlayLobbyBGM()
// {
//     PlayBGM(LobbyBGM, LobbyBGMVolume);
// }

// void AMyPlayerController::StopBGM(float FadeTime)
// {
//     UE_LOG(LogTemp, Warning, TEXT("StopBGM Called / Local=%d / Component=%s"),
//         IsLocalController(),
//         *GetNameSafe(CurrentBGMComponent)
//     );
//
//     if (!IsLocalController())
//     {
//         return;
//     }
//
//     if (IsValid(CurrentBGMComponent))
//     {
//         UE_LOG(LogTemp, Warning, TEXT("StopBGM Success"));
//         CurrentBGMComponent->Stop();
//         CurrentBGMComponent = nullptr;
//     }
//     else
//     {
//         UE_LOG(LogTemp, Warning, TEXT("StopBGM Failed: CurrentBGMComponent invalid"));
//     }
//
//     CurrentBGMSound = nullptr;
// }

// void AMyPlayerController::Client_StopBGM_Implementation()
// {
//     UE_LOG(LogTemp, Warning, TEXT("Client_StopBGM Called"));
//
//     StopBGM(0.0f);
// }
void AMyPlayerController::Client_ShowHitMarker_Implementation(bool bCritical)
{
   BP_ShowHitMarker(bCritical);
}