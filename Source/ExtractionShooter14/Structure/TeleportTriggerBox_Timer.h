#pragma once

#include "CoreMinimal.h"
#include "TeleportTriggerBox_Base.h"
#include "GameFramework/Actor.h"
#include "TeleportTriggerBox_Timer.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class AMyPlayerController;
class APlayerCharacterState;

USTRUCT()
struct FCountdownInfo
{
	GENERATED_BODY()
	
	FTimerHandle TimerHandle;
	UPROPERTY()
	TObjectPtr<APlayerCharacterState> PlayerState = nullptr;
};

UCLASS()
class EXTRACTIONSHOOTER14_API ATeleportTriggerBox_Timer : public ATeleportTriggerBox_Base
{
	GENERATED_BODY()

public:
	ATeleportTriggerBox_Timer();

	virtual void MoveToLocationBeginOverlap
	(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult & SweepResult) override;

	virtual void MoveToLocationEndOverlap
	(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	void CancelAllPlayerCountdowns(); // stage time ended > all player move >> list clear
	virtual void BeginPlay() override;
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraComponent> NiagaraComponent;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|MoveTo")
	float MaxTakesTime = 5.f;
	UFUNCTION() // one-shot at MaxTakesTime (same clock as client progress bar)
	void FinishPlayerCountdown(AMyPlayerController* InPlayer);
	
	UFUNCTION() // when player dead > clear timer
	void OnPlayerDeadChanged(APlayerCharacterState* InPlayerState,	uint8 bIsDead);

	void CancelPlayerCountdown(AMyPlayerController* InPlayer);
private:
	UPROPERTY()
	TMap<TObjectPtr<AMyPlayerController>, FCountdownInfo> CountdownPlayerListMap;
};
