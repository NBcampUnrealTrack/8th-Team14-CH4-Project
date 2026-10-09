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
#include "Game/ES_SessionSubsystem.h"
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
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ServerTest] Dedicated=%d NetMode=%d Map=%s"),
        IsRunningDedicatedServer(),
        static_cast<int32>(GetNetMode()),
        *GetWorld()->GetMapName()
    );
    
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AStageLocationActor::StaticClass(),
        StageSpawnLocationPoints);
    
}

void ATestGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
    FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    
    if (LobbyPlayerStartPointMap.Num() >= MaxInGamePlayerCount)
    {
        ErrorMessage = FString::Printf(TEXT("Max Player Joined in this Game. %d/%d"),LobbyPlayerStartPointMap.Num(), MaxInGamePlayerCount);
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
    const FString PlayerKey = PS->GetUniqueId()->ToString();;
    
    UES_GameInstanceSubsystem* SaveSubsystem =
        GetGameInstance()->GetSubsystem<UES_GameInstanceSubsystem>();

    FPlayerDatas GetPlayerData;

    if (SaveSubsystem->FindPlayerData(PlayerKey, GetPlayerData))
    {
        // 기존 데이터 복원
        PS->ApplySavedDataToPlayer(GetPlayerData);
        
        UE_LOG(LogTemp, Warning, TEXT("[Apply] Inventory Num=%d"),
        PS->GetPlayerInfo().ItemStorage.Num());
                
        for (const auto& ItemName: PS->GetPlayerInfo().ItemStorage)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Apply] Inventory Name=%s"),*ItemName.ItemName.ToString());
        }
    }
    else
    {
        static int32 NewPlayerCount = 0;
        FPlayerDatas NewPlayerData;
        NewPlayerData.PlayerNickName += FString::Printf(TEXT("%2d"),++NewPlayerCount);

        SaveSubsystem->SavePlayerData(PlayerKey, NewPlayerData);
        PS->ApplySavedDataToPlayer(NewPlayerData);
        PS->InitializeInventoryMaxSlot(12);
        
        UE_LOG(LogTemp, Warning, TEXT("[Apply Newbie] Inventory Num=%d"),
        PS->GetPlayerInfo().ItemStorage.Num());
                
        for (const auto& ItemName: PS->GetPlayerInfo().ItemStorage)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Apply Newbie] Inventory Name=%s"),*ItemName.ItemName.ToString());
        }
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

        LobbyPlayerStartPoints.Sort(
            [](const AActor& A, const AActor& B)
            {
                return A.GetName() < B.GetName();
            }
        );

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
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Exiting->GetPawn());
    if (IsValid(PlayerCharacter)) PlayerCharacter->WeaponDelete();
    
    APlayerCharacterState* PS =
    Exiting
    ? Exiting->GetPlayerState<APlayerCharacterState>()
    : nullptr;

    if (IsValid(PS))
    {
        PS->SetPlayerStagePlace(EPlayerPlace::None);
        
        if (PS->GetUniqueId().IsValid())
        {
            const FString PlayerKey =
                PS->GetUniqueId()->ToString();

            if (UES_GameInstanceSubsystem* SaveSubsystem =
                GetGameInstance()
                ->GetSubsystem<UES_GameInstanceSubsystem>())
            {
                UE_LOG(LogTemp, Warning, TEXT("[Save] Inventory Num=%d"),
                PS->GetPlayerInfo().ItemStorage.Num());
                
                for (const auto& ItemName: PS->GetPlayerInfo().ItemStorage)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[Save] Inventory Name=%s"),*ItemName.ItemName.ToString());
                }
                
                SaveSubsystem->SavePlayerData(
                    PlayerKey,
                    PS->GetPlayerInfo()
                );
            }
        }
    }

    APlayerController* ExitingPlayerController = Cast<APlayerController>(Exiting);

    if (IsValid(ExitingPlayerController))
    {
        LobbyPlayerStartPointMap.Remove(ExitingPlayerController);
    }

    ATestGameState* GS = GetGameState<ATestGameState>();

    if (IsValid(GS))
    {
        if (IsValid(PS))
        {
            GS->RemoveLobbyPlayerStates(PS);
        }

        const int32 RemainingPlayerCount = LobbyPlayerStartPointMap.Num();
        
        GS->SetPlayerInGameCount_GS(RemainingPlayerCount);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Logout] RemainingPlayers=%d"),
            RemainingPlayerCount
        );

        if (RemainingPlayerCount == 0)
        {
            StartNone();
        }
    }

    Super::Logout(Exiting);
}

void ATestGameMode::HandleAllPlayerisReady()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    int32 PlayerCount = GS->GetLobbyPlayerStates().Num();
    
    const FPhaseTimerData& Data = GS->GetGameStatePhaseTimerData();
    
    if (Data.GameStateEnum == EGameState::LobbyWaiting)
    {
            if (IsAllPlayersReady())
            {
                GS->SetGameStatePhaseTimerData(EGameState::LobbyWaiting, 0.f); // > SetGamestatePhaaseTimerData ?
            
                // 5초뒤 move
            
            }
    }
    
    if (Data.GameStateEnum == EGameState::Playing)
    {
        float PassedTime = GS->GetServerWorldTimeSeconds() - Data.PhaseStartServerTime;
        float Ratio = PassedTime / Data.PhaseDuration;
        // 난입허가 Playing시작후 0.3비율까지 ex 100sec in 33sec can join
        if (Ratio < 0.2f)
        { // inplaying free to restart in 0.2f
                MoveToStage();
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
    
    return true;
}

void ATestGameMode::StartNone()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS))
    {
        return;
    }

    // gamemode timer clear
    GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);

    // initialize force ready
    bForced = false;

    // end rank timer
    StopRankUpdateTimer();

    // gs state convert
    GS->SetGameStatePhaseTimerData(
        EGameState::None,
        0.0f
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[StartNone] Server entered empty state.")
    );
}

void ATestGameMode::StartLobby()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    if (GS->GetGameStatePhaseTimerData().GameStateEnum == EGameState::Playing)
    {
        return;
    }
    
    if (LobbyPlayerStartPointMap.IsEmpty())
    {
        StartNone();
        return;
    }
    GS->SetGameStatePhaseTimerData(EGameState::LobbyWaiting, LobbyDuration);
    StartPhaseTimer();
    StopRankUpdateTimer();
}

void ATestGameMode::StartStage()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    
    GS->SetGameStatePhaseTimerData(EGameState::Playing, PlayDuration);
    
    CalculatePlayerRank();
    
    StartPhaseTimer();
    StartRankUpdateTimer();
}

void ATestGameMode::MoveToStage()
{
        
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
        
            if (PlayerCharacterStateES->GetPlayerStagePlace() == EPlayerPlace::Lobby)
            {
                if (PlayerCharacterStateES->IsPlayerReady() == true)
                {
                    UE_LOG(LogTemp, Warning, TEXT("BeforeStage PC=%s Pawn=%s State=%s"),
                    *GetNameSafe(PlayerControllerES),
                    *GetNameSafe(PlayerControllerES->GetPawn()),
                    *PlayerControllerES->GetStateName().ToString());
                    
                    PlayerCharacterStateES->InitializeCombatScore();
                    PlayerCharacterStateES->SetIsParticipatedInThisRound(true);
                    PlayerCharacterStateES->SetPlayerStagePlace(EPlayerPlace::Stage);
                    PlayerCharacterStateES->StartSurviveTime();
                    APlayerCharacter* PC = Cast<APlayerCharacter>(PlayerPawn);
                    if (IsValid(PC))
                    {
                        PC->EquipDefaultWeapon();
                    }
                    PlayerPawn->TeleportTo
                    (
                        StageSpawnLocationPoints[StartIndex]->GetActorLocation(),
                        StageSpawnLocationPoints[StartIndex]->GetActorRotation()
                        );
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

void ATestGameMode::StartPhaseTimer()
{

    GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);

    GetWorldTimerManager().SetTimer(
        CheckRemainLobbyTimeHandle,
        this,
        &ThisClass::CheckRemainTime,
        0.5f,
        true
    );
}

bool ATestGameMode::CheckAllPlayerStatePlace(EPlayerPlace InPlayerPlace)
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!GS) return false;
    
    int32 count = 0;
    const TArray<APlayerCharacterState*>& PlayerStates =
        GS->GetLobbyPlayerStates();
    
    if (PlayerStates.IsEmpty() == true)
    {
        return false;
    }
    
    for (auto PlayerStateES : PlayerStates)
    {
        if (PlayerStateES->GetPlayerStagePlace() == InPlayerPlace)
        {
            count++;
        }
    }
    
    
    if (count >= PlayerStates.Num())
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
    
    if (Phase == EGameState::None)
    {
        GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);
        bForced = false;
    }
    else if (Phase == EGameState::LobbyWaiting)
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
            bForced = false;
            MoveToStage();
            
            if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby)) // all player non ready
            {
                StartLobby();
            }
            else
            {
                StartStage();
            }
        }
    }
    else if (Phase == EGameState::Playing)
    {
        if (RemainTime <= 0.f)
        {
            GetWorldTimerManager().ClearTimer(CheckRemainLobbyTimeHandle);
            OnStageEnded.Broadcast();
            
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
    
    // final result for escape ResultUI
    PlayerStateES->UpdateSurviveTime();
    PlayerStateES->CommitCombatInfo();

    PlayerStateES->ApplyPlayerStageInventoryToPlayerStorage();

    // ======================= init playerstates ========================
    PlayerStateES->ClearPlayerStageInventoryGold();
    PlayerStateES->SetPlayerStagePlace(EPlayerPlace::Lobby);
    PlayerStateES->InitiateStats();
    // ======================= init playerstates ========================
    
    // ======================= Player Move ===========================
    TargetController->StopPlayerMovement();
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    ACharacter* CharacterES = Cast<ACharacter>(TargetController->GetPawn());
    FRotator Rotation = MovePoint.Rotator();
    Rotation.Pitch = 0.f;
    Rotation.Roll = 0.f;
    TargetController->GetPawn()->TeleportTo(MovePoint.GetLocation(), Rotation);
    // rotation
    TargetController->SetControlRotation(Rotation);
    CharacterES->SetActorRotation(Rotation);
    TargetController->Client_ApplySpawnRotation(Rotation);
    // ======================= Player Move ===========================
    
    // ======================= Player UI ===========================
    TargetController->Client_WhenMoveToLobbyUI();
    TargetController->Client_OpenResultUI();
    // ======================= Player UI ===========================
    
    if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
    {
        GameStateES->SetGameStatePhaseTimerData(EGameState::LobbyWaiting, LobbyDuration);
        GetWorldTimerManager().SetTimerForNextTick(
        this, &ThisClass::ClearObjects);
        
        StartLobby();
    }
}

void ATestGameMode::RespawnPlayerAtLobbyStartPoint(AMyPlayerController* TargetController)
{
    if (!IsValid(TargetController)) return;
    ATestGameState* GameStateES = GetGameState<ATestGameState>();
    if (!IsValid(GameStateES)) return;
    APlayerCharacterState* PlayerStateES = TargetController->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
    APlayerCharacter* OldPawn = Cast<APlayerCharacter>(TargetController->GetPawn());
    if (!IsValid(OldPawn)) return;
    OldPawn->Destroy();
    OldPawn->WeaponDelete();
    
    InitializePlayerStats(PlayerStateES);
    TargetController->StopPlayerMovement();
    
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    RestartPlayerAtTransform(TargetController, MovePoint);
    TargetController->Client_WhenMoveToLobbyUI();
    if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
    {

        GameStateES->SetGameStatePhaseTimerData(EGameState::LobbyWaiting, LobbyDuration);
        GetWorldTimerManager().SetTimerForNextTick(
      this, &ThisClass::ClearObjects);
        StartLobby();
    }
}

void ATestGameMode::TimeOutReSetPlayer(AMyPlayerController* TargetController)
{
    if (!IsValid(TargetController)) return;
    ATestGameState* GameStateES = GetGameState<ATestGameState>();
    if (!IsValid(GameStateES)) return;
    APlayerCharacterState* PlayerStateES = TargetController->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
    // final result for timeout ResultUI
    if (!PlayerStateES->GetbIsDead())
    {
        PlayerStateES->UpdateSurviveTime();
    }
    PlayerStateES->CommitCombatInfo();
    
    APlayerCharacter* OldPawn = Cast<APlayerCharacter>(TargetController->GetPawn());
    if (!IsValid(OldPawn)) return;
    OldPawn->Destroy();
    OldPawn->WeaponDelete();
    
    InitializePlayerStats(PlayerStateES);
    TargetController->StopPlayerMovement();

    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    RestartPlayerAtTransform(TargetController, MovePoint);
    TargetController->Client_WhenMoveToLobbyUI();
    TargetController->Client_OpenResultUI();
    if (CheckAllPlayerStatePlace(EPlayerPlace::Lobby))
    {

        GameStateES->SetGameStatePhaseTimerData(EGameState::LobbyWaiting, LobbyDuration);
        GetWorldTimerManager().SetTimerForNextTick(
        this, &ThisClass::ClearObjects);
        
        StartLobby();
    }
}

void ATestGameMode::InitializePlayerStats(APlayerCharacterState* InPlayerState)
{
    if (!IsValid(InPlayerState)) return;
    // ================================ Place
    InPlayerState->SetPlayerStagePlace(EPlayerPlace::Lobby);
    InPlayerState->InitiateStats();
    InPlayerState->ClearPlayerStageInventoryGold();
    InPlayerState->ClearPlayerStageInventory();
    InPlayerState->ClearPlayerinfoInventory();
}

void ATestGameMode::ClearObjects()
{
    if (!HasAuthority())
    {
        return;
    }

    TArray<AActor*> FoundCoins;
    UGameplayStatics::GetAllActorsWithTag(this, TEXT("StageCoin"), FoundCoins);

    for (AActor* Coin : FoundCoins)
    {
        if (IsValid(Coin))
        {
            Coin->Destroy();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[Stage] Removed %d coins."), FoundCoins.Num());
}

#pragma region Rank
void ATestGameMode::StartRankUpdateTimer()
{
    GetWorldTimerManager().ClearTimer(RankUpdateTimerHandle);
    GetWorldTimerManager().SetTimer(
        RankUpdateTimerHandle,
        this,
        &ThisClass::CalculatePlayerRank,
        RankUpdateInterval,
        true
    );
}

void ATestGameMode::StopRankUpdateTimer()
{
    GetWorldTimerManager().ClearTimer(RankUpdateTimerHandle);
}

// score changes every tick (survive time), rank changes only sometimes
// > calculate every tick, replicate only players whose rank changed
void ATestGameMode::CalculatePlayerRank()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    if (!IsValid(GS)) return;
    if (GS->GetGameStatePhaseTimerData().GameStateEnum != EGameState::Playing) return;

    // 1. collect this round's players + refresh score
    TArray<APlayerCharacterState*> RankedPlayers;
    for (APlayerCharacterState* State : GS->GetLobbyPlayerStates())
    {
        if (!IsValid(State) || !State->GetIsParticipatedInThisRound()) continue;

        // alive in stage = survive time keeps going (dead / escaped = frozen)
        if (State->GetPlayerStagePlace() == EPlayerPlace::Stage && !State->GetbIsDead())
        {
            State->UpdateSurviveTime();
        }
        RankedPlayers.Add(State);
    }

    // 2. sort by latest(working) score, not last replicated score
    RankedPlayers.Sort(
        [](const APlayerCharacterState& A, const APlayerCharacterState& B)
        {
            return A.GetPlayerWorkingCombatInfo().Score > B.GetPlayerWorkingCombatInfo().Score;
        });

    // 3. same score = same rank (1, 1, 3), only changed rank is replicated
    int32 CurrentRank = 1;
    for (int32 i = 0; i < RankedPlayers.Num(); ++i)
    {
        const int32 Score = RankedPlayers[i]->GetPlayerWorkingCombatInfo().Score;
        if (i > 0 && Score < RankedPlayers[i - 1]->GetPlayerWorkingCombatInfo().Score)
        {
            CurrentRank = i + 1;
        }

        RankedPlayers[i]->SetPlayerCombatInfo_Rank(CurrentRank);
    }
}
#pragma endregion
