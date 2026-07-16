// ES_AudioSubsystem.h
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ES_AudioSubsystem.generated.h"

class USoundBase;
class USoundClass;
class USoundMix;
class UAudioComponent;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_AudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 곡 교체 (기존 것 멈추고 새로 재생)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayBGM(USoundBase* NewBGM);
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopBGM();
	// BGM 볼륨 조절 (0.0 ~ 1.0), 어디서 호출해도 모든 BGM에 적용
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetBGMVolume(float NewVolume);
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetUIVolume(float NewVolume);
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetBGMVolume() const { return BGMVolume; }
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetUIVolume() const { return UIVolume; }
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetMasterVolume() const { return MasterVolume; }
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 에디터에서 지정: 위에서 만든 SM_MainMix, SC_BGM
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundMix> MainSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundClass> BGMSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundClass> UISoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundClass> MasterSoundClass;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentBGMComponent;

	UPROPERTY()
	TObjectPtr<USoundBase> CurrentBGMSound;

	float BGMVolume = 0.5f;
	
	float MasterVolume = 0.5f;
	
	float UIVolume = 0.5f;
};