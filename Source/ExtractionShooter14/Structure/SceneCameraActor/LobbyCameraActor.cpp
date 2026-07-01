#include "LobbyCameraActor.h"
#include "Components/SceneCaptureComponent2D.h"

ALobbyCameraActor::ALobbyCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent2D->SetupAttachment(GetRootComponent());
	
	SceneCaptureComponent2D->TextureTarget = LobbyRenderTarget;
	SceneCaptureComponent2D->ProjectionType = ECameraProjectionMode::Perspective;
	SceneCaptureComponent2D->bCaptureEveryFrame = true;
}

void ALobbyCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}


