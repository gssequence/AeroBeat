#pragma once

#include "Config.h"

class SoundManager
{
public:
	class Sound
	{
		friend SoundManager;

	private:
		bool _usePortAudio;
		bool _fail = true;
		bool _loop;
		bool _playing = false;
		int _sampleRate;
		int _handle;
		int _channels;
		int _bitDepth;
		int _length;
		int _index = 0;
		void* _ptr;
		double _denominator;
		double _volume = 1.0;
		double _master_volume = 1.0;
		bool _mute = false;

		Sound(std::wstring path, bool usePortAudio, int sampleRate, bool loop = false);

		double _get(int i);

	public:
		virtual ~Sound();

		bool fail() { return _fail; }
		bool loop() { return _loop; }

		void play();
		void stop();
		bool is_playing();
		double duration();
		void volume(double v);
		void master_volume(double v);
		void mute(bool v);
		void write(float* ptr, unsigned long frames, int channels);
	};

private:
	bool _disablePortAudio, _mute = false;
	PaStream* _stream;
	std::vector<std::weak_ptr<SoundManager::Sound>> _sounds;
	int _sampleRate;

	void register_sound(std::shared_ptr<SoundManager::Sound> sound);

public:
	SoundManager(Config& config);
	virtual ~SoundManager();

	std::shared_ptr<Sound> load(std::wstring path, bool loop = false);
	void clear_sounds();
	void mute(bool v);
};
