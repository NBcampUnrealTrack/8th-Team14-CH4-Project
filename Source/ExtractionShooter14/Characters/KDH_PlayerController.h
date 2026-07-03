#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KDH_PlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class EXTRACTIONSHOOTER14_API AKDH_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AKDH_PlayerController();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
};
