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
//#include "MidiStreamTrackIsolatorNode.h"

#include "SfizzSynthNode.h"
//#include "MidiTrackIsolator.h"

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
		DEFINE_INPUT_METASOUND_PARAM(MidiDeviceName, "Midi Device Name", "The name of the midi input device we want to receive with this node")
		//DEFINE_INPUT_METASOUND_PARAM(IncludeConductorTrack, "Include Conductor Track", "Enable to include the conductor track (AKA track 0)");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
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
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiDeviceName))
					//TInputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::IncludeConductorTrack), false)
				),
				FOutputVertexInterface(
					TOutputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::MidiStream))
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
			FStringReadRef MidiDeviceName;
			//FBoolReadRef IncludeConductorTrack;
		};

		struct FOutputs
		{
			FMidiStreamWriteRef MidiStream;
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
				InputData.GetOrCreateDefaultDataReadReference<FString>(Inputs::MidiDeviceNameName, InParams.OperatorSettings)
				//InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::IncludeConductorTrackName, InParams.OperatorSettings)
			};

			FOutputs Outputs
			{
				FMidiStreamWriteRef::CreateNew()
			};

			return MakeUnique<FSfizzSynthMetasoundOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
		}

		FSfizzSynthMetasoundOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, Outputs(MoveTemp(InOutputs))
		{
			Reset(InParams);
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Inputs::EnableName, Inputs.Enabled);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
			InVertexData.BindReadVertex(Inputs::MinTrackIndexName, Inputs.MinTrackIndex);
			InVertexData.BindReadVertex(Inputs::MaxTrackIndexName, Inputs.MaxTrackIndex);
			InVertexData.BindReadVertex(Inputs::MidiDeviceNameName, Inputs.MidiDeviceName);
			//InVertexData.BindReadVertex(Inputs::IncludeConductorTrackName, Inputs.IncludeConductorTrack);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Outputs::MidiStreamName, Outputs.MidiStream);
		}

		void Reset(const FResetParams&)
		{
	
		}


		//destructor
		virtual ~FSfizzSynthMetasoundOperator()
		{
			UE_LOG(LogTemp, Log, TEXT("Sfizz Synth Node Destructor"));

		}


		void Execute()
		{
			//Filter.SetFilterValues(*Inputs.MinTrackIndex, *Inputs.MaxTrackIndex, false);

			Outputs.MidiStream->PrepareBlock();

			if (*Inputs.Enabled)
			{
				//stream current tick?
				//int32 CurrentTick = Inputs.MidiStream->GetClock()->GetCurrentMidiTick();
				//Inputs.MidiStream->Add
				
				PendingMessages.Empty();
				//Filter.Process(*Inputs.MidiStream, *Outputs.MidiStream);
			}
		}
	private:
		FInputs Inputs;
		FOutputs Outputs;

		FDelegateHandle RawEventDelegateHandle;
		int32 TickOffset = 0; //in theory we can start when the stream tick is different from the device tick, we'll see
		TArray<TTuple<int32, FMidiMsg>> PendingMessages;
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