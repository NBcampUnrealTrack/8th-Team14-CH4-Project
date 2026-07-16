// ES_AudioSubsystem.cpp
#include "ES_AudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

void UES_AudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	///Script/Engine.SoundMix'/Game/LJW/Title/UI/Sound/SM_MainMix.SM_MainMix'
	MainSoundMix = LoadObject<USoundMix>(nullptr,
		TEXT("/Game/LJW/Title/UI/Sound/SM_MainMix.SM_MainMix"));
	///Script/Engine.SoundClass'/Game/LJW/Title/UI/Sound/SC_BGM.SC_BGM'
	BGMSoundClass = LoadObject<USoundClass>(nullptr,
		TEXT("/Game/LJW/Title/UI/Sound/SC_BGM.SC_BGM"));
	///Script/Engine.SoundClass'/Game/LJW/Title/UI/Sound/SC_UI.SC_UI'
	UISoundClass = LoadObject<USoundClass>(nullptr,
		TEXT("/Game/LJW/Title/UI/Sound/SC_UI.SC_UI"));
	///Script/Engine.SoundClass'/Game/LJW/Title/UI/Sound/SC_Master.SC_Master'
	MasterSoundClass = LoadObject<USoundClass>(nullptr,
		TEXT("/Game/LJW/Title/UI/Sound/SC_Master.SC_Master"));
	
	// Sound Mix를 한 번만 Push해서 활성화 (이후엔 값만 갱신)
	if (MainSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MainSoundMix);
	}
}

void UES_AudioSubsystem::PlayBGM(USoundBase* NewBGM)
{
	if (!NewBGM)
	{
		StopBGM();
		return;
	}

	// 같은 곡이면 무시 (중복 재생 방지)
	if (NewBGM == CurrentBGMSound && IsValid(CurrentBGMComponent))
	{
		return;
	}

	// 기존 BGM 정지
	StopBGM();

	// 새 BGM 재생 (레벨 전환 시 끊기도록 persist=false)
	CurrentBGMComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(), NewBGM, 1.0f, 1.0f, 0.0f, nullptr,
		false,   // bPersistAcrossLevelTransition=false → 레벨 전환 시 끊김 (요구사항 1)
		false);

	CurrentBGMSound = NewBGM;
}

void UES_AudioSubsystem::StopBGM()
{
	if (IsValid(CurrentBGMComponent))
	{
		CurrentBGMComponent->Stop();
	}
	CurrentBGMComponent = nullptr;
	CurrentBGMSound = nullptr;   // ← 중요: 기억 초기화
}

void UES_AudioSubsystem::SetBGMVolume(float NewVolume)
{
	BGMVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	if (MainSoundMix && BGMSoundClass)
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MainSoundMix);
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(), MainSoundMix, BGMSoundClass, BGMVolume, 1.0f, 0.0f);
	}
}

void UES_AudioSubsystem::SetUIVolume(float NewVolume)
{
	UIVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	if (MainSoundMix && UISoundClass)  // ← SC_Master 참조
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MainSoundMix);
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(), MainSoundMix, UISoundClass,
			UIVolume, 1.0f, 0.0f);
	}
}

void UES_AudioSubsystem::SetMasterVolume(float NewVolume)
{
	MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	if (MainSoundMix && MasterSoundClass)  // ← SC_Master 참조
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MainSoundMix);
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(), MainSoundMix, MasterSoundClass,
			MasterVolume, 1.0f, 0.0f);
	}
}
