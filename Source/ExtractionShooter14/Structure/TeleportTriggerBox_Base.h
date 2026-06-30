#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportTriggerBox_Base.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class EXTRACTIONSHOOTER14_API ATeleportTriggerBox_Base : public AActor
{
	GENERATED_BODY()

public:
	ATeleportTriggerBox_Base();
	
	UFUNCTION()
	virtual void MoveToLocationBeginOverlap
	(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	virtual void MoveToLocationEndOverlap
	(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|MoveTo")
	FVector MoveToLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|MoveTo")
	FRotator MoveToRotation = FRotator::ZeroRotator;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> BoxComponent;
};
