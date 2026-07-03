#include "KDH_TestGameMode.h"
#include "Characters/KDH_PlayerController.h"
#include "KDH_TestGameState.h"
#include "KDH_TestPlayerState.h"

AKDH_TestGameMode::AKDH_TestGameMode()
{
	PlayerControllerClass = AKDH_PlayerController::StaticClass();
	GameStateClass = AKDH_TestGameState::StaticClass();
	PlayerStateClass = AKDH_TestPlayerState::StaticClass();
}