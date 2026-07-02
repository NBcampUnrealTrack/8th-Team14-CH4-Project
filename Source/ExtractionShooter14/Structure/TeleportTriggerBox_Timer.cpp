#include "TeleportTriggerBox_Timer.h"
#include "Character/MyPlayerController.h"
// #include "Components/StaticMeshComponent.h"
// #include "Components/BoxComponent.h"
#include "Character/TestGameMode.h"
#include "Kismet/KismetSystemLibrary.h"

ATeleportTriggerBox_Timer::ATeleportTriggerBox_Timer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	// SetRootComponent(SceneComponent);
	//
	// StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	// StaticMeshComponent->SetupAttachment(GetRootComponent());
	//
	// BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	// BoxComponent->SetupAttachment(StaticMeshComponent);
	//
	// BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::MoveToLocationBeginOverlap);
	// BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::MoveToLocationEndOverlap);
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
	
	// map에 등록되있으면 새로 타이머등록안함
	if (CountdownPlayerListMap.Contains(PlayerControllerES)) return;
	
	FCountdownInfo Info;
	Info.RemainingTime = MaxTakesTime;
	CountdownPlayerListMap.Add(PlayerControllerES, Info);
	
	FCountdownInfo& CountRefInfo = CountdownPlayerListMap[PlayerControllerES];
	// ingame ui > PlayerControllerES->ClinetRPC_ShowCountWidget(TakesTime);
	UE_LOG(LogTemp, Warning, TEXT("Player is going to Move In 3s."))
	
	GetWorldTimerManager().SetTimer(
		CountRefInfo.TimerHandle,
		
		FTimerDelegate::CreateWeakLambda(
			(this),
			[this, PlayerControllerES]
			{
				UpdatePlayerCountdown(PlayerControllerES);
			}),
		1.0f,
		true
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
	
	FCountdownInfo* Info =
	CountdownPlayerListMap.Find(PlayerControllerES);

	if (!Info)	return;
	
	GetWorldTimerManager().ClearTimer(Info->TimerHandle);
	// ingame ui > PlayerControllerES->ClinetRPC_CloseCountWidget(TakesTime);
	
	CountdownPlayerListMap.Remove(PlayerControllerES);
}

void ATeleportTriggerBox_Timer::UpdatePlayerCountdown(AMyPlayerController* InPlayer)
{
	if (!IsValid(InPlayer)) return;
	
	FCountdownInfo* Info =
		CountdownPlayerListMap.Find(InPlayer);

	if (!Info)	return;
	Info->RemainingTime--;
	
	UE_LOG(LogTemp, Warning, TEXT("PlayerName: %s >> Move In %.fs"),*InPlayer->GetName(), Info->RemainingTime);
	
	if (InPlayer)
	{
		//ClientRPC for update UI timeseconds;
		//InPlayer->Client_ShowCountdown(Info->RemainingTime);
	}
	

	//FTransform SetPlayerTransform = GameModeES->GetPlayerLobbyStartPoint(InPlayer);
	if (Info->RemainingTime <= 0)
	{
		ATestGameMode* GameModeES = Cast<ATestGameMode>(InPlayer->GetWorld()->GetAuthGameMode());
		if (!IsValid(GameModeES)) return;
		GetWorldTimerManager().ClearTimer(Info->TimerHandle);
		// if (APawn* Pawn = InPlayer->GetPawn())
		// {
		GameModeES->SetPlayerLobbyStartPoint(InPlayer);
			//Pawn->TeleportTo(SetPlayerTransform.GetLocation(), SetPlayerTransform.Rotator()+FRotator(0.f,180.f, 0.f));
		//InPlayer->Client_OpenResultUI();
		// }
		
		CountdownPlayerListMap.Remove(InPlayer);
	}
}
