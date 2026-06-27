#include "TestGameMode.h"
#include "PlayerCharacter.h"
#include "MyPlayerController.h"
#include "ExtractionShooter14/Character/PlayerCharacterState.h"
#include "ExtractionShooter14/Game/ES_GameInstanceSubsystem.h"
#include "GameFramework/OnlineReplStructs.h"

ATestGameMode::ATestGameMode()
{
    DefaultPawnClass = APlayerCharacter::StaticClass();

    
    PlayerControllerClass = AMyPlayerController::StaticClass();
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
}

void ATestGameMode::Logout(AController* Exiting)
{
    if (APlayerCharacterState* PS = Exiting->GetPlayerState<APlayerCharacterState>())
    {
        const FString PlayerKey = PS->GetUniqueId()->ToString();

        if (UES_GameInstanceSubsystem* SaveSubsystem =
            GetGameInstance()->GetSubsystem<UES_GameInstanceSubsystem>())
        {
            SaveSubsystem->SavePlayerData(PlayerKey, PS->GetPlayerInfo());
        }
    }
    
    Super::Logout(Exiting);
}
