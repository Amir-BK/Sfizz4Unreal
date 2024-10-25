// Fill out your copyright notice in the Description page of Project Settings.


#include "SfizzSynthComponent.h"


// Sets default values


USfizzSynthComponent::~USfizzSynthComponent()
{
	if (SfizzSynth != nullptr)
	{
		sfizz_free(SfizzSynth);
		SfizzSynth = nullptr;
		DeinterleavedBuffer[0] = nullptr;
		DeinterleavedBuffer[1] = nullptr;
		DecodedAudioDataBuffer.clear();

	}
}

void USfizzSynthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);


	sfizz_free(SfizzSynth);
	DeinterleavedBuffer[0] = nullptr;
	DeinterleavedBuffer[1] = nullptr;
	DecodedAudioDataBuffer.clear();

}


bool USfizzSynthComponent::Init(int32& SampleRate)
{
	NumChannels = 2;
	SfizzSynth = sfizz_create_synth();
	sfizz_set_sample_rate(SfizzSynth, SampleRate);
	sfizz_set_samples_per_block(SfizzSynth, 1024);
	DecodedAudioDataBuffer.resize(1024);
	DeinterleavedBuffer.resize(2);
	DeinterleavedBuffer[0] = DecodedAudioDataBuffer.data();
	DeinterleavedBuffer[1] = DecodedAudioDataBuffer.data() + 512;
	//sfizz_set_num_voices(synth, 64);
	FString PathToSanitize = SfzAssetPath;
	PathToSanitize.TrimQuotesInline();
		bool bSuccessLoadSFZFile = sfizz_load_file(SfizzSynth, TCHAR_TO_ANSI(*PathToSanitize));
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

	//we give SFZ a pointer to the head of the deinterleaved array
	sfizz_render_block(SfizzSynth, DeinterleavedBuffer.data(), NumChannels, NumSamples / NumChannels);
	
	//unreal audio system expects interleaved audio for multichannel
	Audio::BufferInterleave2ChannelFast(DeinterleavedBuffer[0], DeinterleavedBuffer[1], OutAudio, NumSamples / 2);

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
