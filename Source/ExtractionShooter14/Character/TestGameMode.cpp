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

ATestGameMode::ATestGameMode()
{
    DefaultPawnClass = APlayerCharacter::StaticClass();

    
    PlayerControllerClass = AMyPlayerController::StaticClass();
}

void ATestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    RemainingTime = WaitingTime;
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

bool ATestGameMode::CheckAllPlayerisReady()
{
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
