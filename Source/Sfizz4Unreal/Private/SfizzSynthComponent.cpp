// Fill out your copyright notice in the Description page of Project Settings.


#include "SfizzSynthComponent.h"


// Sets default values


USfizzSynthComponent::~USfizzSynthComponent()
{
	if (SfizzSynth != nullptr)
	{
		sfizz_free(SfizzSynth);
		SfizzSynth = nullptr;
		DecodedChannelsStartPtr[0] = nullptr;
		DecodedChannelsStartPtr[1] = nullptr;
		DecodedAudioDataBuffer.clear();

	}
}

void USfizzSynthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);


	sfizz_free(SfizzSynth);
	DecodedChannelsStartPtr[0] = nullptr;
	DecodedChannelsStartPtr[1] = nullptr;
	DecodedAudioDataBuffer.clear();

}


bool USfizzSynthComponent::Init(int32& SampleRate)
{
	NumChannels = 2;
	SfizzSynth = sfizz_create_synth();
	sfizz_set_sample_rate(SfizzSynth, SampleRate);
	sfizz_set_samples_per_block(SfizzSynth, 1024);
	DecodedAudioDataBuffer.resize(1024);
	DecodedChannelsStartPtr.resize(2);
	DecodedChannelsStartPtr[0] = DecodedAudioDataBuffer.data();
	DecodedChannelsStartPtr[1] = DecodedAudioDataBuffer.data() + 512;
	//sfizz_set_num_voices(synth, 64);
	FString PathToSanitize = SfzAssetPath;
	PathToSanitize.TrimQuotesInline();
	auto LoadPath = TCHAR_TO_ANSI(*PathToSanitize);
	bool bSuccessLoadSFZFile = sfizz_load_file(SfizzSynth, LoadPath);
	UE_CLOG(!bSuccessLoadSFZFile, LogTemp, Warning, TEXT("SFZ file failed to load, Synth not initialized."));

	//if couldn't init, free synth to avoid memory leak 
	if (!bSuccessLoadSFZFile) {
		sfizz_free(SfizzSynth);
		SfizzSynth = nullptr;
	}
	return bSuccessLoadSFZFile;
}

int32 USfizzSynthComponent::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{

	if (!SfizzSynth)
	{
		return 0;
	}

	sfizz_render_block(SfizzSynth, DecodedChannelsStartPtr.data(), 2, 512);
	
	Audio::BufferInterleave2ChannelFast(DecodedChannelsStartPtr[0], DecodedChannelsStartPtr[1], OutAudio, NumSamples / 2);

	return NumSamples;
}

void USfizzSynthComponent::SendNoteOn(int32 NoteNumber, int32 Velocity)
{
	sfizz_send_note_on(SfizzSynth, 0, NoteNumber, Velocity);
}

void USfizzSynthComponent::SendNoteOff(int32 NoteNumber, int32 Velocity)
{
	sfizz_send_note_off(SfizzSynth, 0, NoteNumber, Velocity);
}
