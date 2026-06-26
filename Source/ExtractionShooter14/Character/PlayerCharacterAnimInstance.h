

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class EXTRACTIONSHOOTER14_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    
    virtual void NativeInitializeAnimation() override;

protected:
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float Speed;

    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    class APlayerCharacter* Character;
};
