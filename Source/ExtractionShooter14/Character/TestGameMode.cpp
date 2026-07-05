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
{
    DefaultPawnClass = APlayerCharacter::StaticClass();

    
    PlayerControllerClass = AMyPlayerController::StaticClass();
}

void ATestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    RemainingTime = WaitingTime;
    
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AStageLocationActor::StaticClass(),
        StageSpawnLocationPoints);
    
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
    
    
    // if (PlayerCount >= MaxInGamePlayerCount)
    // {
        if (IsAllPlayersReady())
        {
            //gs >> gamestate::playing
        
            Algo::RandomShuffle(StageSpawnLocationPoints);

            for (const TPair<TWeakObjectPtr<APlayerController>, int32>& Pair : LobbyPlayerStartPointMap)
            {
                AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(Pair.Key.Get());
                APlayerCharacterState* PlayerCharacterStateES = PlayerControllerES->GetPlayerState<APlayerCharacterState>();
                APawn* PlayerPawn = PlayerControllerES->GetPawn();
            
                const int32 StartIndex = Pair.Value;
            
                if (!IsValid(PlayerControllerES)) continue;
                if (!IsValid(PlayerCharacterStateES)) continue;
                if (!IsValid(PlayerPawn)) continue;
            
                //PlayerControllerES->SetAutoManageCamera(true);
            
                UE_LOG(LogTemp, Warning, TEXT("BeforeStage PC=%s Pawn=%s State=%s"),
                *GetNameSafe(PlayerControllerES),
                *GetNameSafe(PlayerControllerES->GetPawn()),
                *PlayerControllerES->GetStateName().ToString());
            
                PlayerCharacterStateES->SetPlayerStagePlace(EPlayerPlace::Stage);
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
            
                PlayerControllerES->Client_WhenMoveToStagePopUpUI();
                

            }
            
            // timer and switch enumgamestate
            StartStage();
        }
    //}
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
    for (APlayerCharacterState* PS : LobbyPlayers)
    {
        PS->SetPlayerReady(false);
    }
    return true;
}

void ATestGameMode::StartLobby()
{
    ATestGameState* GS = GetGameState<ATestGameState>();
    
    FPhaseTimerData SetLobbyTimerData;
    SetLobbyTimerData.GameStateEnum = EGameState::Waiting;
    SetLobbyTimerData.PhaseStartServerTime = GS->GetServerWorldTimeSeconds();
    SetLobbyTimerData.PhaseDuration = 10.f;
    
    GS->SetGameStatePhaseTimerData(SetLobbyTimerData);
}

void ATestGameMode::StartStage()
{
    ATestGameState* GS = GetGameState<ATestGameState>();

    FPhaseTimerData SetLobbyTimerData;
    SetLobbyTimerData.GameStateEnum = EGameState::Playing;
    SetLobbyTimerData.PhaseStartServerTime = GS->GetServerWorldTimeSeconds();
    SetLobbyTimerData.PhaseDuration = 20.f;
    
    GS->SetGameStatePhaseTimerData(SetLobbyTimerData);
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
    
    PlayerStateES->SetPlayerStagePlace(EPlayerPlace::Lobby);
    PlayerStateES->ApplyPlayerStageInventoryToPlayerStorage();
    InitializePlayerStats(PlayerStateES);
    
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    ACharacter* CharacterES = Cast<ACharacter>(TargetController->GetPawn());
    CharacterES->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    // CharacterES->GetCharacterMovement()->DisableMovement();
    FRotator Rotation = MovePoint.Rotator();
    Rotation.Pitch = 0.f;
    Rotation.Roll = 0.f;
    // TargetController->SetControlRotation(Rotation);
    // CharacterES->SetActorRotation(Rotation);
    TargetController->GetPawn()->TeleportTo(MovePoint.GetLocation(), Rotation);
    
    TargetController->SetControlRotation(Rotation);
    CharacterES->SetActorRotation(Rotation);
    TargetController->Client_ApplySpawnRotation(Rotation);
    
    TargetController->Client_WhenMoveToLobbyUI();
    TargetController->Client_OpenResultUI();
}

void ATestGameMode::RespawnPlayerAtLobbyStartPoint(AMyPlayerController* TargetController)
{
    if (!IsValid(TargetController)) return;
    
    APlayerCharacterState* PlayerStateES = TargetController->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PlayerStateES)) return;
    
    PlayerStateES->SetPlayerStagePlace(EPlayerPlace::Lobby);
    APawn* OldPawn = TargetController->GetPawn();
    if (!IsValid(OldPawn)) return;
    OldPawn->Destroy();

    InitializePlayerStats(PlayerStateES);

    //InitiatePlayerStats(TargetController);
    FTransform MovePoint = GetPlayerLobbyStartPoint(TargetController);
    RestartPlayerAtTransform(TargetController, MovePoint);
    TargetController->Client_WhenMoveToLobbyUI();
    //TargetController->Client_OpenResultUI();
}

void ATestGameMode::InitializePlayerStats(APlayerCharacterState* InPlayerState)
{
    if (!IsValid(InPlayerState)) return;
    InPlayerState->InitiateStats();
    // ================================ Place
    InPlayerState->SetPlayerStagePlace(EPlayerPlace::Lobby);
    
    InPlayerState->ClearPlayerStageInventory();
}

