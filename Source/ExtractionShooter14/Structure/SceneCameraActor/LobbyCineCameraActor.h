#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyCineCameraActor.generated.h"
class UCineCameraComponent;

UCLASS()
class EXTRACTIONSHOOTER14_API ALobbyCineCameraActor : public AActor
{
	GENERATED_BODY()

public:
	ALobbyCineCameraActor();

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCineCameraComponent> CineCameraComponent;
};
