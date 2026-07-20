#include "CoinBase.h"
#include "Components/SphereComponent.h"
#include "Character/PlayerCharacter.h"
#include "Character/PlayerCharacterState.h"

ACoinBase::ACoinBase()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    SetReplicateMovement(true);

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    SphereComponent->SetupAttachment(RootComponent);

    SphereComponent->SetSphereRadius(60.f);

    // �߿�
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SphereComponent->SetGenerateOverlapEvents(true);

    SphereComponent->OnComponentBeginOverlap.AddDynamic(
        this,
        &ACoinBase::OnSphereBeginOverlap
    );
}

void ACoinBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning,
        TEXT("[%s] Coin BeginPlay : %s"),
        HasAuthority() ? TEXT("Server") : TEXT("Client"),
        *GetName());

    StartZ = GetActorLocation().Z;
    GroundZ = StartZ;

    float Angle = FMath::FRandRange(0.f, 360.f);

    FVector Direction = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)),
        FMath::Sin(FMath::DegreesToRadians(Angle)),
        0.f
    );

    LaunchVelocity = Direction * LaunchPower;
    LaunchVelocity.Z = UpPower;
}

void ACoinBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AddActorLocalRotation(FRotator(0, RotateSpeed * DeltaTime, 0));

    // 1. ���ư��� ��
    if (bLaunching)
    {
        LaunchVelocity.Z -= Gravity * DeltaTime;

        FVector Location = GetActorLocation();
        Location += LaunchVelocity * DeltaTime;

        // �ٴ� ����
        if (Location.Z <= GroundZ)
        {
            Location.Z = GroundZ;

            bLaunching = false;
            bFloating = true;

            RunningTime = 0.f;
            StartZ = GroundZ;
        }

        SetActorLocation(Location);
        return;
    }

    // 2. �սǵս�
    if (bFloating)
    {
        RunningTime += DeltaTime;

        FVector Location = GetActorLocation();

        Location.Z =
            StartZ +
            FMath::Sin(RunningTime * FloatSpeed) * FloatHeight;

        SetActorLocation(Location);
    }
}

void ACoinBase::OnSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    UE_LOG(LogTemp, Warning,
        TEXT("[%s] Coin Overlap Actor : %s"),
        HasAuthority() ? TEXT("Server") : TEXT("Client"),
        *OtherActor->GetName());


    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Overlap but Not Server"));
        return;
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (!Player)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Overlap Actor is not Player"));
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("Player Pick Coin : %s"),
        *Player->GetName());

    APlayerCharacterState* PS = Player->GetPlayerState<APlayerCharacterState>();

    if (!PS)
    {
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("Add Gold : %d"),
        CoinValue);
    
    PS->AddPlayerStageGold(CoinValue);
    PS->AddPlayerCombatInfo_LootCount(1);
    UE_LOG(LogTemp, Warning,
        TEXT("Coin Destroy"));

    Destroy();
}

void ACoinBase::SpawnCoins(
    UWorld* World,
    TSubclassOf<ACoinBase> CoinClass,
    const FVector& SpawnLocation,
    int32 TotalGold)
{
    if (!World || !CoinClass)
    {
        return;
    }

    int32 RemainGold = TotalGold;

    while (RemainGold > 0)
    {
        const int32 DropGold = FMath::Min(50, RemainGold);

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ACoinBase* Coin = World->SpawnActor<ACoinBase>(
            CoinClass,
            SpawnLocation,
            FRotator::ZeroRotator,
            Params);

        if (Coin)
        {
            UE_LOG(LogTemp, Warning, TEXT("Coin Spawn Success"));
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("Coin Spawn Failed"));
        }

        if (Coin)
        {
            Coin->SetCoinValue(DropGold);
        }

        RemainGold -= DropGold;
    }
}

void ACoinBase::SetCoinValue(int32 NewValue)
{
    CoinValue = NewValue;
}
