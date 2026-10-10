#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoinBase.generated.h"

class UStaticMeshComponent;

// drop-only currency (not a shop / inventory item, so no DataTable row)
UCLASS()
class EXTRACTIONSHOOTER14_API ACoinBase : public AActor
{
    GENERATED_BODY()

public:
    ACoinBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere)
    class USphereComponent* SphereComponent;

    UFUNCTION()
    void OnSphereBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UPROPERTY(EditAnywhere, Category = "Coin|Effect")
    float RotateSpeed = 180.f;
    UPROPERTY(EditAnywhere, Category = "Coin|Effect")
    float FloatHeight = 20.f;
    UPROPERTY(EditAnywhere, Category = "Coin|Effect")
    float FloatSpeed = 3.f;
    UPROPERTY(EditAnywhere, Category = "Coin|Effect")
    float LaunchPower = 250.f;
    UPROPERTY(EditAnywhere, Category = "Coin|Effect")
    float UpPower = 350.f;
    UPROPERTY(EditAnywhere, Category = "Coin|Effect")
    float Gravity = 980.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
    int32 CoinValue = 1;

private:
    float RunningTime = 0.f;

    float StartZ = 0.f;

    bool bLaunching = true;

    bool bFloating = false;

    FVector LaunchVelocity;

    float GroundZ = 0.f;

public:
    static void SpawnCoins(
        UWorld* World,
        TSubclassOf<ACoinBase> CoinClass,
        const FVector& SpawnLocation,
        int32 TotalGold
    );

    void SetCoinValue(int32 NewValue);
};