#include "TeleportTriggerBox_Timer.h"
#include "Character/MyPlayerController.h"
#include "Character/PlayerCharacterState.h"
#include "NiagaraComponent.h"
#include "Character/TestGameMode.h"
#include "Character/TestGameState.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ATeleportTriggerBox_Timer::ATeleportTriggerBox_Timer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SphereComponent->SetSphereRadius(5000.f);
	
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->SetRelativeLocation(FVector(0.f,0.f,-800.f));
	NiagaraComponent->SetAutoActivate(true);
}

void ATeleportTriggerBox_Timer::MoveToLocationBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// only run on server
	if (!HasAuthority()) return;
	
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!IsValid(PlayerPawn)) return;
	AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(PlayerPawn->GetController());
	if (!IsValid(PlayerControllerES)) return;
	APlayerCharacterState* PlayerCharacterStateES = PlayerControllerES->GetPlayerState<APlayerCharacterState>();
	if (!IsValid(PlayerCharacterStateES)) return;
	ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>();
	if (!IsValid(GS)) return;
	
	// map에 등록되있으면 새로 타이머등록안함
	if (CountdownPlayerListMap.Contains(PlayerControllerES)) return;
	// player is dead > return
	if (PlayerCharacterStateES->GetbIsDead()) return;
	
	FCountdownInfo Info;
	Info.PlayerState = PlayerCharacterStateES;
	FCountdownInfo& CountRefInfo = CountdownPlayerListMap.Add(PlayerControllerES, Info);
	
	PlayerCharacterStateES->OnPlayerbIsDeadChanged.AddUniqueDynamic(this,	&ThisClass::OnPlayerDeadChanged);
	
	UE_LOG(LogTemp, Warning, TEXT("Player is going to Move In %.fs."), MaxTakesTime);
	
	const double StartServerTime = GS->GetServerWorldTimeSeconds();
	PlayerControllerES->Client_ShowEscapeProgressWidget(StartServerTime, MaxTakesTime);
	
	GetWorldTimerManager().SetTimer(
		CountRefInfo.TimerHandle,
		
		FTimerDelegate::CreateWeakLambda(
			(this),
			[this, PlayerControllerES]
			{
				FinishPlayerCountdown(PlayerControllerES);
			}),
		MaxTakesTime,
		false
	);
}

void ATeleportTriggerBox_Timer::MoveToLocationEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// only run on server
	if (!HasAuthority()) return;
	
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!IsValid(PlayerPawn)) return;
	AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(PlayerPawn->GetController());
	if (!IsValid(PlayerControllerES)) return;
	
	CancelPlayerCountdown(PlayerControllerES);
}

void ATeleportTriggerBox_Timer::CancelAllPlayerCountdowns()
{
	if (!HasAuthority()) return;

	TArray<TObjectPtr<AMyPlayerController>> Controllers;
	CountdownPlayerListMap.GetKeys(Controllers);

	for (AMyPlayerController* Controller : Controllers)
	{
		CancelPlayerCountdown(Controller);
	}
}

void ATeleportTriggerBox_Timer::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority()) return;

	ATestGameMode* GameModeES =	Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameModeES)) return;

	GameModeES->OnStageEnded.AddUObject(this, &ThisClass::CancelAllPlayerCountdowns);
	// disconnect does not reliably fire EndOverlap with a valid controller
	GameModeES->OnPlayerLoggedOut.AddUObject(this, &ThisClass::CancelPlayerCountdown);
	
}

void ATeleportTriggerBox_Timer::FinishPlayerCountdown(AMyPlayerController* InPlayer)
{
	if (!HasAuthority()) return;
	if (!IsValid(InPlayer)) return;
	if (!CountdownPlayerListMap.Contains(InPlayer)) return;

	ATestGameMode* GameModeES = Cast<ATestGameMode>(InPlayer->GetWorld()->GetAuthGameMode());

	CancelPlayerCountdown(InPlayer); // cleartimer, unbinddelegate, remove map
	if (!IsValid(GameModeES)) return;

	GameModeES->SetPlayerLobbyStartPoint(InPlayer);
}

void ATeleportTriggerBox_Timer::OnPlayerDeadChanged(APlayerCharacterState* InPlayerState, uint8 bIsDead)
{
	if (!HasAuthority()) return;
	if (!bIsDead) return; // true == pass
	if (!IsValid(InPlayerState)) return;
	
	AMyPlayerController* PlayerController =	Cast<AMyPlayerController>(InPlayerState->GetPlayerController());
	if (!IsValid(PlayerController)) return;
	
	CancelPlayerCountdown(PlayerController);
}

void ATeleportTriggerBox_Timer::CancelPlayerCountdown(AMyPlayerController* InPlayer)
{
	if (!IsValid(InPlayer)) return;
	FCountdownInfo* CountInfo =	CountdownPlayerListMap.Find(InPlayer);
	if (!CountInfo)	return;
	
	InPlayer->Client_CloseEscapeProgressWidget();
	GetWorldTimerManager().ClearTimer(CountInfo->TimerHandle); // clear timer
	
	APlayerCharacterState* PlayerState =
		CountInfo->PlayerState.Get();
	
	if (IsValid(PlayerState))
	{
		PlayerState->OnPlayerbIsDeadChanged.RemoveDynamic(this, &ThisClass::OnPlayerDeadChanged); // clear bindfunction
	}
	CountdownPlayerListMap.Remove(InPlayer); // clear maplist
}

void ATeleportTriggerBox_Timer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (ATestGameMode* GameModeES =	Cast<ATestGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameModeES->OnStageEnded.RemoveAll(this);
			GameModeES->OnPlayerLoggedOut.RemoveAll(this);
		}

		CancelAllPlayerCountdowns();
	}
	Super::EndPlay(EndPlayReason);
}