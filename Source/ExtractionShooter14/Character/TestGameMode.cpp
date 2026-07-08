#include "TestGameMode.h"
#include "PlayerCharacter.h"
#include "MyPlayerController.h"
#include "TestGameState.h"
#include "ExtractionShooter14/Character/PlayerCharacterState.h"
#include "ExtractionShooter14/Game/ES_GameInstanceSubsystem.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformProcess.h"
#include "Structure/StageLocationActor.h"
#include "Algo/RandomShuffle.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

ATestGameMode::ATestGameMode()
    :bForced(false)
{
    DefaultPawnClass = APlayerCharacter::StaticClass();
    PlayerControllerClass = AMyPlayerController::StaticClass();
    
    
}

void ATestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AStageLocationActor::StaticClass(),
        StageSpawnLocationPoints);
    
}

void ATestGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
    FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    
    if (InGamePlayerCount >= MaxInGamePlayerCount)
    {
        ErrorMessage = FString::Printf(TEXT("Max Player Joined in this Game. %d/%d"), InGamePlayerCount, MaxInGamePlayerCount);
        return;
    }
    
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (!IsValid(PS))
            continue;

        if (PS->GetUniqueId() == UniqueId)
        {
            ErrorMessage = TEXT("Already Connected");
            return;
        }
    }
    
}

void ATestGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    APlayerCharacterState* PS =
    NewPlayer->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PS)) return;
    
    PS->SetPlayerStagePlace(EPlayerPlace::Lobby);
    
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("UniqueId Valid: %d"),
        PS->GetUniqueId().IsValid()
    );

    if (PS->GetUniqueId().IsValid())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PlayerKey = %s"),
            *PS->GetUniqueId()->ToString()
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GameMode PID = %d"), FPlatformProcess::GetCurrentProcessId());
    // const FString PlayerKey =
    //     PS->GetUniqueId().ToString();
    const FString PlayerKey = PS->GetUniqueId()->ToString();;
    
    UES_GameInstanceSubsystem* SaveSubsystem =
        GetGameInstance()->GetSubsystem<UES_GameInstanceSubsystem>();

    FPlayerDatas GetPlayerData;

    if (SaveSubsystem->FindPlayerData(PlayerKey, GetPlayerData))
    {
        // 기존 데이터 복원
        PS->ApplySavedDataToPlayer(GetPlayerData);
    }
    else
    {
        static int32 NewPlayerCount = 0;
        FPlayerDatas NewPlayerData;
        NewPlayerData.PlayerNickName += FString::Printf(TEXT("%2d"),++NewPlayerCount);

        SaveSubsystem->SavePlayerData(PlayerKey, NewPlayerData);
        PS->ApplySavedDataToPlayer(NewPlayerData);
    }
    
    UGameplayStatics::GetAllActorsOfClass(
    GetWorld(),
    APlayerStart::StaticClass(),
    LobbyPlayerStartPoints
    );

    
    const int32 Index = GetAvailableStartIndex();
    if (Index != INDEX_NONE)
    {
        // add player startpoint
        LobbyPlayerStartPointMap.Add(NewPlayer, Index);
        PS->SetLobbyIndex(Index);
        
        APawn* PlayerPawn = NewPlayer->GetPawn();
        if (!IsValid(PlayerPawn)) return;
        PlayerPawn->TeleportTo(
            LobbyPlayerStartPoints[Index]->GetActorLocation(),
            LobbyPlayerStartPoints[Index]->GetActorRotation()
            );
        
        ATestGameState* GameState_Test = GetGameState<ATestGameState>();
        if (!IsValid(GameState_Test)) return;
        GameState_Test->SetPlayerInGameCount_GS(LobbyPlayerStartPointMap.Num());
        GameState_Test->AddLobbyPlayerStates(PS);
    }
    
    // timer and switch enumgamestate
    StartLobby();
}

void ATestGameMode::Logout(AController* Exiting)
{
    APlayerCharacterState* PS = Exiting->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PS)) return;
    PS->SetPlayerStagePlace(EPlayerPlace::None);
    
    const FString PlayerKey = PS->GetUniqueId()->ToString();

    if (UES_GameInstanceSubsystem* SaveSubsystem =
        GetGameInstance()->GetSubsystem<UES_GameInstanceSubsystem>())
    {
        SaveSubsystem->SavePlayerData(PlayerKey, PS->GetPlayerInfo());
    }


    APlayerController* PlayerControllerES = Cast<APlayerController>(Exiting);
    if (!IsValid(PlayerControllerES)) return;
    
    LobbyPlayerStartPointMap.Remove(PlayerControllerES);
    ATestGameState* GameState_Test = GetGameState<ATestGameState>();
    
    if (!IsValid(GameState_Test)) return;
    GameState_Test->SetPlayerInGameCount_GS(LobbyPlayerStartPointMap.Num());
    GameState_Test->RemoveLobbyPlayerStates(PS);
    
    Super::Logout(Exiting);
}
//
// AActor* ATestGameMode::ChoosePlayerStart_Implementation(AController* Player)
// {
//     return LobbyPlayerStartPoints[InGamePlayerCount++];
// }

int32 ATestGameMode::GetInGamePlayerCount() const
{
    return InGamePlayerCount;
}

void ATestGameMode::SetInGamePlayerCount(int32 InNewPlayerCount)
{
    InGamePlayerCount = InNewPlayerCount;
}

int32 ATestGameMode::GetMaxInGamePlayerCount() const
{
    return MaxInGamePlayerCount;
}

void ATestGameMode::SetMaxInGamePlayerCount(int32 InNewMaxPlayerCount)
{
    MaxInGamePlayerCount = InNewMaxPlayerCount;
}

void ATestGameMode::HandleAllPlayerisReady()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    int32 PlayerCount = GS->GetLobbyPlayerStates().Num();
    
    if (GS->GetGameStatePhaseTimerData().GameStateEnum == EGameState::LobbyWaiting)
    {
        // if (PlayerCount >= MaxInGamePlayerCount)
        // {
            if (IsAllPlayersReady())
            {
                FPhaseTimerData PhaseTimerData;
                PhaseTimerData.GameStateEnum = EGameState::LobbyWaiting;
                PhaseTimerData.PhaseStartServerTime = GS->GetServerWorldTimeSeconds();
                PhaseTimerData.PhaseDuration = 0.f; // 5.f
                GS->SetGameStatePhaseTimerData(PhaseTimerData); // > SetGamestatePhaaseTimerData ?
            
                // 5초뒤 move
            
            }
        //}
    }
    
    if (GS->GetGameStatePhaseTimerData().GameStateEnum == EGameState::Playing)
    {
        float PassedTime = GS->GetServerWorldTimeSeconds() - GS->GetGameStatePhaseTimerData().PhaseStartServerTime;
        float Ratio = PassedTime / GS->GetGameStatePhaseTimerData().PhaseDuration;
        // 난입허가 Playing시작후 0.3비율까지 ex 100sec in 33sec can join
        if (Ratio < 0.2f)
        { // inplaying free to restart in 0.2f
            // if (IsAllPlayersReady())
            // {
                MoveToStage();
            //}
        }
    }
}

bool ATestGameMode::IsAllPlayersReady()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS))
    {
        return false;
    }

    const auto& LobbyPlayers = GS->GetLobbyPlayerStates();
    
    if (LobbyPlayers.Num() == 0)
    {
        return false;
    }
    
    for (APlayerCharacterState* PS : LobbyPlayers)    
    {
        if (!IsValid(PS) || !PS->IsPlayerReady())
        {
            return false;
        }
    }
    
    // all ready =================================
    // for (APlayerCharacterState* PS : LobbyPlayers)
    // {
    //     PS->SetPlayerReady(false);
    // }
    return true;
}

void ATestGameMode::StartLobby()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    if (GS->GetGameStatePhaseTimerData().GameStateEnum == EGameState::Playing)
    {
        return;
    }
    
    FPhaseTimerData SetLobbyTimerData;
    SetLobbyTimerData.GameStateEnum = EGameState::LobbyWaiting;
    SetLobbyTimerData.PhaseStartServerTime = GS->GetServerWorldTimeSeconds();
    SetLobbyTimerData.PhaseDuration = LobbyDuration;
    
    StartPhaseTimer(SetLobbyTimerData);
}

void ATestGameMode::StartStage()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;

    FPhaseTimerData SetStageTimerData;
    SetStageTimerData.GameStateEnum = EGameState::Playing;
    SetStageTimerData.PhaseStartServerTime = GS->GetServerWorldTimeSeconds();
    SetStageTimerData.PhaseDuration = PlayDuration;
    GS->CalculatePlayerRank();
    StartPhaseTimer(SetStageTimerData);
}

void ATestGameMode::MoveToStage()
{
    // if (IsAllPlayersReady())
    // {
            //gs >> gamestate::playing
        
        Algo::RandomShuffle(StageSpawnLocationPoints);

        for (const TPair<TWeakObjectPtr<APlayerController>, int32>& Pair : LobbyPlayerStartPointMap)
        {
            AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(Pair.Key.Get());
            if (!IsValid(PlayerControllerES)) continue;
            APlayerCharacterState* PlayerCharacterStateES = PlayerControllerES->GetPlayerState<APlayerCharacterState>();
            if (!IsValid(PlayerCharacterStateES)) continue;
            APawn* PlayerPawn = PlayerControllerES->GetPawn();
            if (!IsValid(PlayerPawn)) continue;
            ATestGameState* GameStateES = GetGameState<ATestGameState>();
            if (!IsValid(GameStateES)) return;
            
            const int32 StartIndex = Pair.Value;
        
        
            //PlayerControllerES->SetAutoManageCamera(true);
            if (PlayerCharacterStateES->GetPlayerStagePlace() == EPlayerPlace::Lobby)
            {
                if (PlayerCharacterStateES->IsPlayerReady() == true)
                {
                    UE_LOG(LogTemp, Warning, TEXT("BeforeStage PC=%s Pawn=%s State=%s"),
                    *GetNameSafe(PlayerControllerES),
                    *GetNameSafe(PlayerControllerES->GetPawn()),
                    *PlayerControllerES->GetStateName().ToString());
                    
                    PlayerCharacterStateES->InitializeCombatScore();
                    PlayerCharacterStateES->SetPlayerStagePlace(EPlayerPlace::Stage);
                    PlayerPawn->TeleportTo
                    (
                        StageSpawnLocationPoints[StartIndex]->GetActorLocation(),
                        StageSpawnLocationPoints[StartIndex]->GetActorRotation()
                        );
                    GameStateES->CalculatePlayerRank();
                    FRotator PlayerRotation = StageSpawnLocationPoints[StartIndex]->GetActorRotation();
                    PlayerRotation.Pitch = 0.0f;
                    PlayerRotation.Roll = 0.0f;
        
                    PlayerControllerES->SetControlRotation(PlayerRotation);
                    PlayerPawn->SetActorRotation(PlayerRotation);
                    PlayerControllerES->Client_ApplySpawnRotation(PlayerRotation);
                    PlayerControllerES->StartPlayerMovement();
                    PlayerControllerES->Client_WhenMoveToStagePopUpUI();
                }
            }
        }
    
        // timer and switch enumgamestate
   // }
}

void ATestGameMode::ForceReadyPlayers(bool bIsready)
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    
    for (auto PlayerStates : GS->GetLobbyPlayerStates())
    {
        PlayerStates->SetPlayerReady(bIsready);
    }
}

void ATestGameMode::StartPhaseTimer(FPhaseTimerData Data)
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!GS) return;

    GS->SetGameStatePhaseTimerData(Data);

    GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);

    GetWorldTimerManager().SetTimer(
        CheckRemainLobbyTimeHandle,
        this,
        &ThisClass::CheckRemainTime,
        1.0f,
        true
    );
}

bool ATestGameMode::CheckAllPlayerStatePlace(EPlayerPlace InPlayerPlace)
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!GS) return false;
    
    int32 count = 0;
    for (auto PlayerStates : GS->GetLobbyPlayerStates())
    {
        if (PlayerStates->GetPlayerStagePlace() == InPlayerPlace)
        {
            count++;
        }
    }
    
    if (count >= GS->GetLobbyPlayerStates().Num())
    {
        return true;
    }
    return false;
}

void ATestGameMode::CheckRemainTime()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!GS) return;
    
    const EGameState Phase = GS->GetGameStatePhaseTimerData().GameStateEnum;
    const float RemainTime = GS->GetRemainTime();
    
    if (Phase == EGameState::LobbyWaiting)
    {
        if (!IsAllPlayersReady() && RemainTime <= 5.f && !bForced)
        {
            bForced = true;
            ForceReadyPlayers(true);
            // todo:: readybutton inactivate
        }

        if (RemainTime <= 0.f)
        {
            GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);
            //bIsForced = false;
            //ForceReadyPlayers(false);
            bForced = false;
            MoveToStage();
            //GS->CalculatePlayerRank();
            
            if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
            {
                StartLobby();
            }
            else
            {
                StartStage();
            }
        }
        
        return;
    }
    
    if (Phase == EGameState::Playing)
    {
        if (RemainTime <= 0.f)
        {
            GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);

            for (auto PlayerStates : GS->GetLobbyPlayerStates())
            {
                if (PlayerStates->GetPlayerStagePlace() == EPlayerPlace::Stage)
                {
                    AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(PlayerStates->GetOwningController());
                    if (!IsValid(PlayerControllerES)) continue;
                    
                    TimeOutReSetPlayer(PlayerControllerES);
                }
            }
        }
    }
}

int32 ATestGameMode::GetAvailableStartIndex() const
{
    TSet<int32> UsedIndices;

    for (const auto& Pair : LobbyPlayerStartPointMap)
    { // <playercontroller, int32> 
        
        UsedIndices.Add(Pair.Value);
    }

    for (int32 i = 0; i < LobbyPlayerStartPoints.Num(); ++i)
    {
        if (!UsedIndices.Contains(i))
        {
            return i;
        }
    }

    return INDEX_NONE;
}

FTransform ATestGameMode::GetPlayerLobbyStartPoint(AMyPlayerController* TargetController)
{
    FTransform PlayerLobbyStartPoint = LobbyPlayerStartPoints[LobbyPlayerStartPointMap[TargetController]]->GetActorTransform();
    return PlayerLobbyStartPoint;
}

void ATestGameMode::SetPlayerLobbyStartPoint(AMyPlayerController* TargetController)
{
    APlayerCharacterState* PlayerStateES = TargetController->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    ATestGameState* GameStateES = GetGameState<ATestGameState>();
    if (!IsValid(GameStateES)) return;
    
    PlayerStateES->ApplyPlayerStageInventoryToPlayerStorage();
    InitializePlayerStats(PlayerStateES);
    TargetController->StopPlayerMovement();
    
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    ACharacter* CharacterES = Cast<ACharacter>(TargetController->GetPawn());
    //CharacterES->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    // CharacterES->GetCharacterMovement()->DisableMovement();
    FRotator Rotation = MovePoint.Rotator();
    Rotation.Pitch = 0.f;
    Rotation.Roll = 0.f;
    // TargetController->SetControlRotation(Rotation);
    // CharacterES->SetActorRotation(Rotation);
    TargetController->GetPawn()->TeleportTo(MovePoint.GetLocation(), Rotation);
    // rotation
    TargetController->SetControlRotation(Rotation);
    CharacterES->SetActorRotation(Rotation);
    TargetController->Client_ApplySpawnRotation(Rotation);
    // UI
    TargetController->Client_WhenMoveToLobbyUI();
    TargetController->Client_OpenResultUI();
    
    if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
    {
        FPhaseTimerData SetLobbyTimerData;
        SetLobbyTimerData.GameStateEnum = EGameState::LobbyWaiting;
        SetLobbyTimerData.PhaseStartServerTime = GameStateES->GetServerWorldTimeSeconds();
        SetLobbyTimerData.PhaseDuration = LobbyDuration;
        StartPhaseTimer(SetLobbyTimerData);
        //StartLobby();
    }
}

void ATestGameMode::RespawnPlayerAtLobbyStartPoint(AMyPlayerController* TargetController)
{
    if (!IsValid(TargetController)) return;
    ATestGameState* GameStateES = GetGameState<ATestGameState>();
    if (!IsValid(GameStateES)) return;
    APlayerCharacterState* PlayerStateES = TargetController->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
    APawn* OldPawn = TargetController->GetPawn();
    if (!IsValid(OldPawn)) return;
    OldPawn->Destroy();

    InitializePlayerStats(PlayerStateES);
    TargetController->StopPlayerMovement();
    
    //InitiatePlayerStats(TargetController);
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    RestartPlayerAtTransform(TargetController, MovePoint);
    TargetController->Client_WhenMoveToLobbyUI();
    //TargetController->Client_OpenResultUI();
    if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
    {
        FPhaseTimerData SetLobbyTimerData;
        SetLobbyTimerData.GameStateEnum = EGameState::LobbyWaiting;
        SetLobbyTimerData.PhaseStartServerTime = GameStateES->GetServerWorldTimeSeconds();
        SetLobbyTimerData.PhaseDuration = LobbyDuration;
        StartPhaseTimer(SetLobbyTimerData);
        //StartLobby();
    }
}

void ATestGameMode::TimeOutReSetPlayer(AMyPlayerController* TargetController)
{
    if (!IsValid(TargetController)) return;
    ATestGameState* GameStateES = GetGameState<ATestGameState>();
    if (!IsValid(GameStateES)) return;
    APlayerCharacterState* PlayerStateES = TargetController->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
    APawn* OldPawn = TargetController->GetPawn();
    if (!IsValid(OldPawn)) return;
    OldPawn->Destroy();

    InitializePlayerStats(PlayerStateES);
    TargetController->StopPlayerMovement();

    //InitiatePlayerStats(TargetController);
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    RestartPlayerAtTransform(TargetController, MovePoint);
    TargetController->Client_WhenMoveToLobbyUI();
    TargetController->Client_OpenResultUI();
    if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
    {
        FPhaseTimerData SetLobbyTimerData;
        SetLobbyTimerData.GameStateEnum = EGameState::LobbyWaiting;
        SetLobbyTimerData.PhaseStartServerTime = GameStateES->GetServerWorldTimeSeconds();
        SetLobbyTimerData.PhaseDuration = LobbyDuration;
        StartPhaseTimer(SetLobbyTimerData);
        //StartLobby();
    }
}

void ATestGameMode::InitializePlayerStats(APlayerCharacterState* InPlayerState)
{
    if (!IsValid(InPlayerState)) return;
    // ================================ Place
    InPlayerState->SetPlayerStagePlace(EPlayerPlace::Lobby);
    
    InPlayerState->InitiateStats();
    InPlayerState->ClearPlayerStageInventory();
}

