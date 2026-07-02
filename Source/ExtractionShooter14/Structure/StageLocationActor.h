#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageLocationActor.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API AStageLocationActor : public AActor
{
	GENERATED_BODY()

public:
	AStageLocationActor();

protected:
	virtual void BeginPlay() override;
};
