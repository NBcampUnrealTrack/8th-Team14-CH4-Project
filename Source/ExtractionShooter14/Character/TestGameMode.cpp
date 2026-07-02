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
        StageSpawnLocationActors);
    
}

void ATestGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    APlayerCharacterState* PS =
    NewPlayer->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PS)) return;
    
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
        LobbyPlayerStartPointMap.Add(NewPlayer, Index);
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
    
    
}

void ATestGameMode::Logout(AController* Exiting)
{
    APlayerCharacterState* PS = Exiting->GetPlayerState<APlayerCharacterState>();
    if (!IsValid(PS)) return;

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
    if (IsAllPlayersReady())
    {
        //gs >> gamestate::playing
        
        Algo::RandomShuffle(StageSpawnLocationActors);
        
        // ATestGameState* GS = GetGameState<ATestGameState>();
        // if (!IsValid(GS)) return;
        //
        // for ( int i = 0; i < GS->GetLobbyPlayerStates().Num(); ++i)
        // {
        //     APlayerCharacterState* PlayerStates =  GS->GetLobbyPlayerStates()[i];
        //     AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(PlayerStates->GetPlayerController());
        //     ACharacter* PlayerCharacterES = Cast<ACharacter>(PlayerStates->GetPawn());
        //     if (!IsValid(PlayerControllerES)) continue;
        //     if (!IsValid(PlayerCharacterES)) continue;
        //     
        //     PlayerCharacterES->TeleportTo(
        //     StageSpawnLocationActors[i]->GetActorLocation(),
        //     StageSpawnLocationActors[i]->GetActorRotation()
        //         );
        //     
        //     PlayerControllerES->Client_WhenMoveToStagePopUpUI();
        //}
        for (const TPair<TWeakObjectPtr<APlayerController>, int32>& Pair : LobbyPlayerStartPointMap)
        {
            AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(Pair.Key.Get());
            const int32 StartIndex = Pair.Value;
            
            if (!IsValid(PlayerControllerES)) continue;
            
            APawn* PlayerPawn = PlayerControllerES->GetPawn();
            if (!IsValid(PlayerPawn)) continue;
            
            PlayerControllerES->SetAutoManageCamera(true);
            
            UE_LOG(LogTemp, Warning, TEXT("BeforeStage PC=%s Pawn=%s State=%s"),
            *GetNameSafe(PlayerControllerES),
            *GetNameSafe(PlayerControllerES->GetPawn()),
            *PlayerControllerES->GetStateName().ToString());
            
            PlayerPawn->TeleportTo
            (
                StageSpawnLocationActors[StartIndex]->GetActorLocation(),
                StageSpawnLocationActors[StartIndex]->GetActorRotation()
                );
            PlayerControllerES->Client_WhenMoveToStagePopUpUI();
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
    for (APlayerCharacterState* PS : LobbyPlayers)
    {
        PS->SetPlayerReady(false);
    }
    return true;
}

int32 ATestGameMode::GetAvailableStartIndex() const
{
    TSet<int32> UsedIndices;

    for (const auto& Pair : LobbyPlayerStartPointMap)
    {
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
