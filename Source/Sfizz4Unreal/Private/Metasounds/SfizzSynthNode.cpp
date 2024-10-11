// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"

#include "HarmonixMetasound/DataTypes/MidiStream.h"
#include "HarmonixMetasound/DataTypes/MusicTransport.h"
#include "HarmonixDsp/AudioUtility.h"

#include "HarmonixMetasound/MidiOps/StuckNoteGuard.h"
#include "Sfizz.h"
#include <vector>
//#include "MidiStreamTrackIsolatorNode.h"

#include "SfizzSynthNode.h"
//#include "MidiTrackIsolator.h"

DEFINE_LOG_CATEGORY_STATIC(LogSfizzSamplerNode, Log, All);

#define LOCTEXT_NAMESPACE "Sfizz4Unreal_SfizzSyntNode"

namespace Sfizz4Unreal::SfizzSynthNode
{
	using namespace Metasound;
	using namespace HarmonixMetasound;

	const FNodeClassName& GetClassName()
	{
		static FNodeClassName ClassName
		{
			"Sfizz",
			"SfizzSynth",
			""
		};
		return ClassName;
	}

	int32 GetCurrentMajorVersion()
	{
		return 1;
	}

	namespace Inputs
	{
		DEFINE_INPUT_METASOUND_PARAM(Enable, "Enable", "Enable");
		DEFINE_INPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
		DEFINE_INPUT_METASOUND_PARAM(MinTrackIndex, "Track Index", "Track");
		DEFINE_INPUT_METASOUND_PARAM(MaxTrackIndex, "Channel Index", "Channel");
		DEFINE_INPUT_METASOUND_PARAM(SfzLibPath, "Sfz Lib Path", "Absolute path to the Sfz lib, passed to the Sfizz synth")
		DEFINE_INPUT_METASOUND_PARAM(ScalaFilePath, "Scala File Path", "Optional path to Scala file")
		//DEFINE_INPUT_METASOUND_PARAM(IncludeConductorTrack, "Include Conductor Track", "Enable to include the conductor track (AKA track 0)");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(AudioOutLeft, "Audio Out Left", "Left output of SFizz Synth");
		DEFINE_OUTPUT_METASOUND_PARAM(AudioOutRight, "Audio Out Right", "Right output of Sfizz Synth");
	}

	class FSfizzSynthMetasoundOperator final : public TExecutableOperator<FSfizzSynthMetasoundOperator>
	{
	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetClassName();
					Info.MajorVersion = 1;
					Info.MinorVersion = 0;
					Info.DisplayName = INVTEXT("Sfizz Sampler");
					Info.Description = INVTEXT("Renders MIDI Stream to audio data using Sfizz");
					Info.Author = PluginAuthor;
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();
					Info.CategoryHierarchy = { INVTEXT("Sfizz"), NodeCategories::Music };
					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Enable), true),
					TInputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiStream)),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MinTrackIndex), 0),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MaxTrackIndex), 0),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::SfzLibPath)),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::ScalaFilePath))
	
				),
				FOutputVertexInterface(
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::AudioOutLeft)),
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::AudioOutRight))
				)
			);

			return Interface;
		}

		struct FInputs
		{
			FBoolReadRef Enabled;
			FMidiStreamReadRef MidiStream;
			FInt32ReadRef MinTrackIndex;
			FInt32ReadRef MaxTrackIndex;
			FStringReadRef SfzLibPath;
			FStringReadRef ScalaFilePath;
		};


		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs
			{
				InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::EnableName, InParams.OperatorSettings),
				InputData.GetOrConstructDataReadReference<FMidiStream>(Inputs::MidiStreamName),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MinTrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MaxTrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FString>(Inputs::SfzLibPathName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FString>(Inputs::ScalaFilePathName, InParams.OperatorSettings)
			};

			// outputs
			FOutputVertexInterface OutputInterface;


			return MakeUnique<FSfizzSynthMetasoundOperator>(InParams, MoveTemp(Inputs));
		}

		FSfizzSynthMetasoundOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs)
			: Inputs(MoveTemp(InInputs))
			, SampleRate(InParams.OperatorSettings.GetSampleRate())
			, AudioOutLeft(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
			, AudioOutRight(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
		{
			Reset(InParams);
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Inputs::EnableName, Inputs.Enabled);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
			InVertexData.BindReadVertex(Inputs::MinTrackIndexName, Inputs.MinTrackIndex);
			InVertexData.BindReadVertex(Inputs::MaxTrackIndexName, Inputs.MaxTrackIndex);
			InVertexData.BindReadVertex(Inputs::SfzLibPathName, Inputs.SfzLibPath);
			InVertexData.BindReadVertex(Inputs::ScalaFilePathName, Inputs.ScalaFilePath);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindWriteVertex(Outputs::AudioOutLeftName, AudioOutLeft);
			InVertexData.BindWriteVertex(Outputs::AudioOutRightName, AudioOutRight);
		}

		void Reset(const FResetParams&)
		{

		}


		//destructor
		virtual ~FSfizzSynthMetasoundOperator()
		{
			UE_LOG(LogSfizzSamplerNode, VeryVerbose, TEXT("Sfizz Synth Node Destructor"));
			if (SfizzSynth)
			{
				sfizz_free(SfizzSynth);
				SfizzSynth = nullptr;
				DeinterleavedBuffer[0] = nullptr;
				DeinterleavedBuffer[1] = nullptr;
				DecodedAudioDataBuffer.clear();
			}

		}
			




		void HandleMidiMessage(FMidiVoiceId InVoiceId, int8 InStatus, int8 InData1, int8 InData2, int32 InEventTick, int32 InCurrentTick, float InMsOffset)
		{
			using namespace Harmonix::Midi::Constants;
			int8 InChannel = InStatus & 0xF;
			FScopeLock Lock(&sNoteActionCritSec);
			switch (InStatus & 0xF0)
			{
			case GNoteOff:
				sfizz_send_note_off(SfizzSynth, 0, InData1, InData2);
				NoteStatus[InData1].KeyedOn = false;
				break;
			case GNoteOn:
				sfizz_send_note_on(SfizzSynth, 0, InData1, InData2);
				NoteStatus[InData1].KeyedOn = true;
				break;
			case GPolyPres:

		
				break;
			case GChanPres:
				sfizz_send_channel_aftertouch(SfizzSynth, 0, InData1);
				break;
			case GControl:
				sfizz_send_cc(SfizzSynth, 0, InData1, InData2);
				break;
			case GPitch:
				sfizz_send_pitch_wheel(SfizzSynth, 0, InData1);
				break;
			}
		}


		void Execute()
		{
			const int32 BlockSizeFrames = AudioOutLeft->Num();
			PendingNoteActions.Empty();
			
			//if (*Inputs.SfzLibPath != LibPath)
			{
			
				if (!bSfizzContextCreated || (*Inputs.SfzLibPath != LibPath))
				{
					if (bSfizzContextCreated)
					{
						UE_LOG(LogTemp, Warning, TEXT("Sfizz Context already created, freeing resources"));
						sfizz_free(SfizzSynth);
						SfizzSynth = nullptr;
						DeinterleavedBuffer[0] = nullptr;
						DeinterleavedBuffer[1] = nullptr;
						DecodedAudioDataBuffer.clear();
					}
					
					LibPath = *Inputs.SfzLibPath;
					SfizzSynth = sfizz_create_synth();
					sfizz_set_preload_size(SfizzSynth, 1024 * 1024 * 1024); // 1GB (default is 64MB
	
					//sfizz_enable_logging(SfizzSynth, "Sfizz");
					sfizz_set_sample_rate(SfizzSynth, SampleRate);
					sfizz_set_samples_per_block(SfizzSynth, BlockSizeFrames);
					//DecodedAudioDataBuffer.resize(2 * BlockSizeFrames);
					DeinterleavedBuffer.resize(2);
					DeinterleavedBuffer[0] = AudioOutLeft->GetData();
					DeinterleavedBuffer[1] = AudioOutRight->GetData();
					bSfizzContextCreated = true;
					FString RemoveQuotes = LibPath;
					RemoveQuotes.TrimQuotesInline();

					bSuccessLoadSFZFile = sfizz_load_file(SfizzSynth, TCHAR_TO_ANSI(*RemoveQuotes));
					UE_CLOG(!bSuccessLoadSFZFile, LogTemp, Warning, TEXT("SFZ file failed to load, Synth not initialized."));

					//set number of voices to 8
					sfizz_set_num_voices(SfizzSynth, 16);

					CurrentTrackNumber = *Inputs.MinTrackIndex;
					CurrentChannelNumber = *Inputs.MaxTrackIndex;

					//if scala path is not empty load the scala file
					if (!Inputs.ScalaFilePath->IsEmpty())
					{
						ScalaPath = *Inputs.ScalaFilePath;
						ScalaPath.TrimQuotesInline();
						bool bSuccessLoadingScalaFile = sfizz_load_scala_file(SfizzSynth, TCHAR_TO_ANSI(*ScalaPath));
					}
				}

			}

			
			if (!bSuccessLoadSFZFile)
			{
				return;
			}

			
			StuckNoteGuard.UnstickNotes(*Inputs.MidiStream, [this](const FMidiStreamEvent& Event)
				{
					//NoteOff(Event.GetVoiceId(), Event.MidiMessage.GetStdData1(), Event.MidiMessage.GetStdChannel());
					sfizz_send_note_off(SfizzSynth, 0, Event.MidiMessage.GetStdData1(), 0);
				});
			


			// create an iterator for midi events in the block
			const TArray<FMidiStreamEvent>& MidiEvents = Inputs.MidiStream->GetEventsInBlock();
			auto MidiEventIterator = MidiEvents.begin();

			// create an iterator for the midi clock 
			const TSharedPtr<const FMidiClock, ESPMode::NotThreadSafe> MidiClock = Inputs.MidiStream->GetClock();

			int32 FramesRequired = 1;
			//while (FramesRequired > 0)
			{
				while (MidiEventIterator != MidiEvents.end())
				{

					{
						const FMidiMsg& MidiMessage = (*MidiEventIterator).MidiMessage;
						if (MidiMessage.IsStd()  && (*MidiEventIterator).TrackIndex == CurrentTrackNumber)
						{
							
							HandleMidiMessage(
								(*MidiEventIterator).GetVoiceId(),
								MidiMessage.GetStdStatus(),
								MidiMessage.GetStdData1(),
								MidiMessage.GetStdData2(),
								(*MidiEventIterator).AuthoredMidiTick,
								(*MidiEventIterator).CurrentMidiTick,
								0.0f);
						}
						else if (MidiMessage.IsAllNotesOff())
						{
							//AllNotesOff();
							UE_LOG(LogTemp, Warning, TEXT("All Notes Off"));
							for (size_t i = 0; i < Harmonix::Midi::Constants::GMaxNumNotes; i++)
							{
								sfizz_send_note_off(SfizzSynth, 0, i, 0);
								NoteStatus[i].KeyedOn = false;
							}
	
						}
						else if (MidiMessage.IsAllNotesKill())
						{
							UE_LOG(LogTemp, Warning, TEXT("All Notes Kill"));
							for (size_t i = 0; i < Harmonix::Midi::Constants::GMaxNumNotes; i++)
							{
								sfizz_send_note_off(SfizzSynth, 0, i, 0);
								NoteStatus[i].KeyedOn = false;
							}
						}
						++MidiEventIterator;
					}

				}
			}

			if (MidiClock.IsValid())
			{
				const float ClockSpeed = MidiClock->GetSpeedAtBlockSampleFrame(0);
				//SetSpeed(ClockSpeed, !(*ClockSpeedAffectsPitchInPin));
				const float ClockTempo = MidiClock->GetTempoAtBlockSampleFrame(0);
				sfizz_send_bpm_tempo(SfizzSynth, 0, ClockTempo);
				//SetTempo(ClockTempo);
				//const float Beat = MidiClock->GetQuarterNoteIncludingCountIn();
				//SetBeat(Beat);
			}


			FScopeLock Lock(&SfizzCritSec);

			sfizz_render_block(SfizzSynth, DeinterleavedBuffer.data(), 2, BlockSizeFrames);


		}
	private:
		FInputs Inputs;
	//	FOutputs Outputs;


		struct FPendingNoteAction
		{
			int8  MidiNote = 0;
			int8  Velocity = 0;
			int32 EventTick = 0;
			int32 TriggerTick = 0;
			float OffsetMs = 0.0f;
			int32 FrameOffset = 0;
			FMidiVoiceId VoiceId;
		};

		struct FMIDINoteStatus
		{
			// is the key pressed down?
			bool KeyedOn = false;

			// is there any sound coming out of this note? (release could mean key off but voices active)
			int32 NumActiveVoices = 0;
		};


		//stuff copied from the fusion sampler...
		FCriticalSection sNoteActionCritSec;
		FCriticalSection SfizzCritSec;
		FCriticalSection sNoteStatusCritSec;
		static const int8 kNoteIgnore = -1;
		static const int8 kNoteOff = 0;
		static const int32 kMaxLayersPerNote = 128;
		Harmonix::Midi::Ops::FStuckNoteGuard StuckNoteGuard;

		FSampleRate SampleRate;

		//** DATA
		int32 FramesPerBlock = 0;
		int32 CurrentTrackNumber = 0;
		int32 CurrentChannelNumber = 0;
		bool MadeAudioLastFrame = false;

		TArray<FPendingNoteAction> PendingNoteActions;
		FMIDINoteStatus NoteStatus[Harmonix::Midi::Constants::GMaxNumNotes];

		//pitch bend

		// on range [-1, 1]
		//TLinearRamper<float> PitchBendRamper;

		// extra pitch bend in semitones
		float ExtraPitchBend = 0.0f;
		float PitchBendFactor = 1.0f;

		float FineTuneCents = 0.0f;
		
		//sfizz stuff
		sfizz_synth_t* SfizzSynth;

		std::vector<float>   DecodedAudioDataBuffer;
		std::vector<float*>  DeinterleavedBuffer;
	
		bool bSuccessLoadSFZFile = false;
		bool bSfizzContextCreated = false;
		FString LibPath;
		FString CurrentLibPath;
		FString ScalaPath;


		
		FAudioBufferWriteRef AudioOutLeft;
		FAudioBufferWriteRef AudioOutRight;
		//unDAWMetasounds::TrackIsolatorOP::FMidiTrackIsolator Filter;

	

	};

	class FSfizzSynthNode final : public FNodeFacade
	{
	public:
		explicit FSfizzSynthNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FSfizzSynthMetasoundOperator>())
		{}
		virtual ~FSfizzSynthNode() override = default;
	};

	METASOUND_REGISTER_NODE(FSfizzSynthNode)
}

#undef LOCTEXT_NAMESPACE // "HarmonixMetaSound"