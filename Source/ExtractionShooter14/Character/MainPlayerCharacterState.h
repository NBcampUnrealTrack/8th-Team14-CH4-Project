// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PlayerCharacterState.h"
#include "MainPlayerCharacterState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged,
	float, CurrentHealth,
	float, MaxHealth
);

UCLASS()
class EXTRACTIONSHOOTER14_API AMainPlayerCharacterState : public APlayerCharacterState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UPROPERTY(BlueprintAssignable, Category = "Player|Health")
	FOnHealthChanged OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void ApplyDamage(float DamageAmount);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Player|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Health")
	float MaxHealth = 100.0f;

protected:

	UFUNCTION()
	void OnRep_Health();


};
