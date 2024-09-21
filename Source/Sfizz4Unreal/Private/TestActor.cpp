// Fill out your copyright notice in the Description page of Project Settings.


#include "TestActor.h"


// Sets default values



void USfizzSynthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	sfizz_free(synth);
	DecodedChannelsStartPtr[0] = nullptr;
	DecodedChannelsStartPtr[1] = nullptr;
	DecodedAudioDataBuffer.clear();

}


bool USfizzSynthComponent::Init(int32& SampleRate)
{
	NumChannels = 2;
	synth = sfizz_create_synth();
	sfizz_set_sample_rate(synth, SampleRate);
	sfizz_set_samples_per_block(synth, 1024);
	DecodedAudioDataBuffer.resize(1024);
	DecodedChannelsStartPtr.resize(2);
	DecodedChannelsStartPtr[0] = DecodedAudioDataBuffer.data();
	DecodedChannelsStartPtr[1] = DecodedAudioDataBuffer.data() + 512;
	//sfizz_set_num_voices(synth, 64);
	auto LoadPath = ("E:\\UsersFolders\\Downloads\\Harpsichord\\Choir\\Concert_Harp\\Concert Harp-1.sfz");
	bool bSuccessLoadSFZFile = sfizz_load_file(synth, LoadPath);
	UE_LOG(LogTemp, Log, TEXT("SFZ file loaded: %d"), bSuccessLoadSFZFile);

	return true;
}

int32 USfizzSynthComponent::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{

	sfizz_render_block(synth, DecodedChannelsStartPtr.data(), 2, 512);
	


	for (int32 Sample = 0; Sample < NumSamples / 2; ++Sample)
	{
		OutAudio[Sample * 2] = DecodedChannelsStartPtr[0][Sample];
		OutAudio[Sample * 2 + 1] = DecodedChannelsStartPtr[1][Sample];

	}

	return NumSamples;
}

void USfizzSynthComponent::SendNoteOn(int32 NoteNumber, int32 Velocity)
{
	sfizz_send_note_on(synth, 0, NoteNumber, Velocity);
}
