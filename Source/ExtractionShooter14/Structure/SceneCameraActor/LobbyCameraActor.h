#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyCameraActor.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API ALobbyCameraActor : public AActor
{
	GENERATED_BODY()
public:
	ALobbyCameraActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Camera")
	TObjectPtr<USceneComponent> SceneComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Camera")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent2D;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Camera")
	TObjectPtr<UTextureRenderTarget2D> LobbyRenderTarget;
};
