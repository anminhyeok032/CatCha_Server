#include "stdafx.h"
#include "AudioManager.h"

const std::string CAudioManager::file_path_ = "../Resource/Sound/";

CAudioManager::CAudioManager()
{
	HRESULT comResult;

	comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	comResult = XAudio2Create(&ix_audio_);

	comResult = ix_audio_->CreateMasteringVoice(&ix_mastering_voice_);
}

CAudioManager::~CAudioManager()
{
	ix_mastering_voice_->DestroyVoice();
	ix_audio_->Release();
}

void CAudioManager::AddTrack(const std::string& track_name)
{
	if (IsTrack(track_name)) return;

	std::string file_name = file_path_ + track_name + ".wav";

	CAudioTrack t(ix_audio_, file_name);

	audio_tracks_.insert(std::make_pair(track_name, t));

}

void CAudioManager::PlayTrack(const std::string& track_name)
{
	audio_tracks_[track_name].Play();
}

CAudioTrack::CAudioTrack(IXAudio2* ix_audio, const std::string& file_name)
{
	auto data = ::LoadWavFile(file_name, wave_format_);

	if (FAILED(ix_audio->CreateSourceVoice(&ix_source_voice_, &wave_format_.Format)))
		std::string a{ "fail" };

	buffer_.AudioBytes = data.size;
	buffer_.pAudioData = data.data;
	buffer_.Flags = XAUDIO2_END_OF_STREAM;

	//ix_source_voice_->SubmitSourceBuffer(&buffer_);
}

CAudioTrack::~CAudioTrack()
{
	//if(ix_source_voice_) ix_source_voice_->DestroyVoice();
}

void CAudioTrack::Play()
{
	XAUDIO2_VOICE_STATE state;
	ix_source_voice_->GetState(&state);
	if (state.BuffersQueued < 1)
	{
		ix_source_voice_->SubmitSourceBuffer(&buffer_);
		ix_source_voice_->Start();
	}

}