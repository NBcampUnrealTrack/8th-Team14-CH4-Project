#pragma once

#include "CoreMinimal.h"
#include "TeleportTriggerBox_Base.h"
#include "GameFramework/Actor.h"
#include "TeleportTriggerBox_Timer.generated.h"

class AMyPlayerController;

USTRUCT()
struct FCountdownInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	FTimerHandle TimerHandle;
	UPROPERTY()
	float RemainingTime = 0.f;
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
	
protected:
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|MoveTo")
	// FVector MoveToLocation;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|MoveTo")
	// FRotator MoveToRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|MoveTo")
	float MaxTakesTime = 3.f;
	UFUNCTION()
	void UpdatePlayerCountdown(AMyPlayerController* InPlayer);
	
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	// TObjectPtr<USceneComponent> SceneComponent;
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	// TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	// TObjectPtr<UBoxComponent> BoxComponent;
	
private:
	UPROPERTY()
	TMap<TObjectPtr<AMyPlayerController>, FCountdownInfo> CountdownPlayerListMap;
};
