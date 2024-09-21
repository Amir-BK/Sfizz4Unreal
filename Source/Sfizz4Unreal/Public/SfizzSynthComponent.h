// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SynthComponent.h"
#include "Sfizz.h"
#include <vector>
#include "SfizzSynthComponent.generated.h"

UCLASS(ClassGroup = Synth, meta = (BlueprintSpawnableComponent))
class SFIZZ4UNREAL_API USfizzSynthComponent : public USynthComponent
{
	GENERATED_BODY()

public:	

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called when synth is created
	virtual bool Init(int32& SampleRate) override;

	// Called to generate more audio
	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override;

	UFUNCTION(BlueprintCallable)
	void SendNoteOn(int32 NoteNumber, int32 Velocity);

	UFUNCTION(BlueprintCallable)
	void SendNoteOff(int32 NoteNumber, int32 Velocity);

	~USfizzSynthComponent();

	//the local path for the SFZ sample bank, if you intend to use it in a packaged game make sure this is a local path and the the SFZ file and its samples are packaged
	UPROPERTY(EditAnywhere, Category = "Sfizz")
	FString SfzAssetPath;

protected:
	sfizz_synth_t* SfizzSynth;

	std::vector<float>   DecodedAudioDataBuffer;
	std::vector<float*>  DeinterleavedBuffer;



};
