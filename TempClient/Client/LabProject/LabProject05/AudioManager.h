#pragma once

class CAudioTrack
{

	WAVEFORMATEXTENSIBLE wave_format_;

	IXAudio2SourceVoice* ix_source_voice_;

	XAUDIO2_BUFFER buffer_;


public:
	CAudioTrack() {}
	CAudioTrack(IXAudio2* ix_audio, const std::string& file_name);
	~CAudioTrack();

	void Play();
	void Pause() { ix_source_voice_->Stop(); }
	void Stop() { ix_source_voice_->Stop(); ix_source_voice_->FlushSourceBuffers(); }
};

class CAudioManager
{
	static const std::string file_path_;

	IXAudio2* ix_audio_;
	IXAudio2MasteringVoice* ix_mastering_voice_;

	std::unordered_map<std::string, CAudioTrack> audio_tracks_;

public:
	CAudioManager();
	~CAudioManager();

	bool IsTrack(const std::string& track_name) 
	{ 
		if (audio_tracks_.empty())
			return false;
		return audio_tracks_.count(track_name); 
	}

	void AddTrack(const std::string& track_name);

	void PlayTrack(const std::string& track_name);

};

