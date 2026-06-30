

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right")
};

UCLASS()
class EXTRACTIONSHOOTER14_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    
    virtual void NativeInitializeAnimation() override;

protected:
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PreviousSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bWasMovingLastFrame = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bShouldPlayStop = false;



    
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    class APlayerCharacter* Character;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    ECharacterMovementDirection CustomCardinalDirection;
};
