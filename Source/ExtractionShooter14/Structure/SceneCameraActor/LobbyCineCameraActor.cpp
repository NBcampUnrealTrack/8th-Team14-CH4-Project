#include "LobbyCineCameraActor.h"
#include "CineCameraComponent.h"

ALobbyCineCameraActor::ALobbyCineCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	
	CineCameraComponent = CreateDefaultSubobject<UCineCameraComponent>("CineCameraComponent");
	CineCameraComponent->SetupAttachment(GetRootComponent());
	
}
