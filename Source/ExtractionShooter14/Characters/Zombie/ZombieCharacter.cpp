#include "ZombieCharacter.h"

#include "../../AI/Controllers/ZombieAIController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 사용할 AIController 지정
	AIControllerClass = AZombieAIController::StaticClass();

	// 월드에 배치되거나 Spawn되면 자동으로 AIController가 붙음
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 현재 체력을 최대 체력으로 초기화
	CurrentHP = MaxHP;

	// 블루프린트에서 설정한 이동속도 적용
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

