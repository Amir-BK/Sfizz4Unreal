// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetasoundNodeInterface.h"

#include "HarmonixMetasound/Common.h"

namespace Sfizz4Unreal::SfizzSynthNode
{
	SFIZZ4UNREAL_API const Metasound::FNodeClassName& GetClassName();
	SFIZZ4UNREAL_API int32 GetCurrentMajorVersion();

	//namespace Inputs
	//{
	//	DECLARE_METASOUND_PARAM_ALIAS(Enable);
	//	DECLARE_METASOUND_PARAM_ALIAS(MidiStream);
	//	DECLARE_METASOUND_PARAM_EXTERN(MinTrackIndex);
	//	DECLARE_METASOUND_PARAM_EXTERN(MaxTrackIndex);
	//	DECLARE_METASOUND_PARAM_EXTERN(IncludeConductorTrack);
	//}

	//namespace Outputs
	//{
	//	DECLARE_METASOUND_PARAM_ALIAS(MidiStream);
	//}
}
