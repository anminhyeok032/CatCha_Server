#include "stdafx.h"
#include "AnimationCallbackFunc.h"
#include "AudioManager.h"

CAnimationCallbackFunc::~CAnimationCallbackFunc()
{
}

CSoundCallbackFunc::CSoundCallbackFunc(CAudioManager* audio_manager, const std::string& track_name) : audio_manager_(audio_manager), track_name_(track_name)
{
	if (!audio_manager->IsTrack(track_name))
		audio_manager->AddTrack(track_name);
}

void CSoundCallbackFunc::Util()
{
	audio_manager_->PlayTrack(track_name_);
}