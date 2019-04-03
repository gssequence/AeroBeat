#include "stdafx.h"
#include "util.h"
#include "SoundManager.h"

namespace global_impl
{
	namespace portaudio
	{
		std::mutex data_mutex;
		int channels = 2;

		int PortAudioCallback(const void* input, void* output, unsigned long framesPerBuf, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
		{
			std::lock_guard<std::mutex> lock(data_mutex);
			float* out = static_cast<float*>(output);
			
			for (unsigned long i = 0; i < framesPerBuf * channels; i++)
				out[i] = 0;

			auto& sounds = *static_cast<std::vector<std::weak_ptr<SoundManager::Sound>>*>(userData);
			sounds.erase(std::remove_if(sounds.begin(), sounds.end(), [&](std::weak_ptr<SoundManager::Sound>& a)
			{
				if (auto ptr = a.lock())
				{
					ptr->write(out, framesPerBuf, channels);
					return false;
				}
				return true;
			}), sounds.end());

			return 0;
		}
	}
}

SoundManager::SoundManager(Config& config)
{
	_disablePortAudio = config.disablePortAudio();
	if (!_disablePortAudio)
	{
		auto err = Pa_Initialize();
		if (err != paNoError)
		{
			_disablePortAudio = true;
			return;
		}

		PaDeviceIndex device;
		if (config.useDefaultAudioDevice())
		{
			device = Pa_GetDefaultOutputDevice();
			if (device == paNoDevice)
			{
				Pa_Terminate();
				_disablePortAudio = true;
				return;
			}
		}
		else
		{
			auto count = Pa_GetDeviceCount();
			auto deviceIndex = config.audioDeviceIndex();
			if (deviceIndex < 0 || count < 0 || deviceIndex >= count)
			{
				Pa_Terminate();
				_disablePortAudio = true;
				return;
			}
			device = deviceIndex;
		}

		auto pDeviceInfo = Pa_GetDeviceInfo(device);

		PaStreamParameters params;
		params.device = device;
		params.channelCount = min(pDeviceInfo->maxOutputChannels, 2);
		params.sampleFormat = paFloat32;
		params.suggestedLatency = pDeviceInfo->defaultLowOutputLatency;
		params.hostApiSpecificStreamInfo = NULL;

		_sampleRate = util::round(pDeviceInfo->defaultSampleRate);
		global_impl::portaudio::channels = params.channelCount;

		err = Pa_OpenStream(&_stream, NULL, &params, pDeviceInfo->defaultSampleRate, paFramesPerBufferUnspecified, paNoFlag, global_impl::portaudio::PortAudioCallback, &_sounds);
		if (err != paNoError)
		{
			Pa_Terminate();
			_disablePortAudio = true;
			return;
		}

		Pa_StartStream(_stream);
	}
}

SoundManager::~SoundManager()
{
	if (!_disablePortAudio)
	{
		Pa_StopStream(_stream);
		Pa_CloseStream(_stream);
		Pa_Terminate();
	}
}

std::shared_ptr<SoundManager::Sound> SoundManager::load(std::wstring path, bool loop)
{
	if (!_disablePortAudio)
	{
		auto ret = std::shared_ptr<Sound>(new Sound(path, true, _sampleRate, loop));
		if (!ret->fail())
			register_sound(ret);
		return ret;
	}
	else
	{
		auto ret = std::shared_ptr<Sound>(new Sound(path, false, 0, loop));
		if (!ret->fail())
			register_sound(ret);
		return ret;
	}
}

void SoundManager::register_sound(std::shared_ptr<SoundManager::Sound> sound)
{
	std::lock_guard<std::mutex> lock(global_impl::portaudio::data_mutex);
	sound->mute(_mute);
	_sounds.push_back(sound);
}

void SoundManager::clear_sounds()
{
	std::lock_guard<std::mutex> lock(global_impl::portaudio::data_mutex);
	_sounds.clear();
}

void SoundManager::mute(bool v)
{
	if (_mute != v)
	{
		_mute = v;
		std::lock_guard<std::mutex> lock(global_impl::portaudio::data_mutex);
		for (auto a : _sounds)
		{
			if (auto p = a.lock())
				p->mute(v);
		}
	}
}

#pragma region Sound

SoundManager::Sound::Sound(std::wstring path, bool usePortAudio, int sampleRate, bool loop)
{
	_usePortAudio = usePortAudio;
	_loop = loop;
	_sampleRate = sampleRate;

	fs::path temppath = path;
	if (!temppath.has_extension())
		temppath.replace_extension(L".wav");
	if (util::tolower(temppath.extension()) == L".wav")
	{
		if (!fs::exists(temppath))
			temppath = temppath.replace_extension(L".ogg");
	}
	else if (util::tolower(temppath.extension()) == L".ogg")
	{
		if (!fs::exists(temppath))
			temppath = temppath.replace_extension(L".wav");
	}
	if (!fs::exists(temppath)) return;
	path = temppath;

	if (usePortAudio)
	{
		auto h = dx::LoadSoftSound(path.c_str());
		if (h == -1) return;
		int channels, bits, freq;
		if (dx::GetSoftSoundFormat(h, &channels, &bits, &freq) == -1)
		{
			dx::DeleteSoftSound(h);
			return;
		}
		if (freq == sampleRate)
		{
			_handle = h;
			_channels = channels;
			_bitDepth = bits;
			_length = dx::GetSoftSoundSampleNum(h);
			_ptr = dx::GetSoftSoundDataImage(_handle);
			_denominator = (double)(1 << (bits - 1));
		}
		else
		{
			// サンプリング周波数変換
			auto srcnum = dx::GetSoftSoundSampleNum(h);
			auto dstnum = (int)(srcnum * ((double)sampleRate / freq));
			auto a = dx::MakeSoftSound(h, dstnum);
			if (a == -1)
			{
				dx::DeleteSoftSound(h);
				return;
			}
			if (dx::WritePitchShiftSoftSoundData(h, a) == -1)
			{
				dx::DeleteSoftSound(a);
				dx::DeleteSoftSound(h);
				return;
			}
			dx::DeleteSoftSound(h);
			_handle = a;
			_channels = channels;
			_bitDepth = bits;
			_length = dx::GetSoftSoundSampleNum(a);
			_ptr = dx::GetSoftSoundDataImage(a);
			_denominator = (double)(1 << (bits - 1));
		}

		// メモリをコピー
		auto src = _ptr;
		auto size = _length * _channels;
		if (bits == 8)
		{
			_ptr = new unsigned char[size];
			std::memcpy(_ptr, src, sizeof(unsigned char) * size);
		}
		else if (bits == 16)
		{
			_ptr = new short[size];
			std::memcpy(_ptr, src, sizeof(short) * size);
		}
		dx::DeleteSoftSound(_handle);
		_fail = false;
	}
	else
	{
		_handle = dx::LoadSoundMem(path.c_str());
		if (_handle == -1) return;
		_fail = false;
	}
}

SoundManager::Sound::~Sound()
{
	if (!fail())
	{
		if (_usePortAudio)
			delete[] _ptr;
		else
			dx::DeleteSoundMem(_handle);
	}
}

double SoundManager::Sound::_get(int i)
{
	if (_bitDepth == 8) return (static_cast<unsigned char*>(_ptr)[i] - 128) / _denominator;
	if (_bitDepth == 16) return static_cast<short*>(_ptr)[i] / _denominator;
	return 0;
}

void SoundManager::Sound::play()
{
	if (!fail())
	{
		if (_usePortAudio)
		{
			_playing = true;
			_index = 0;
		}
		else
		{
			dx::PlaySoundMem(_handle, _loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
		}
	}
}

void SoundManager::Sound::stop()
{
	if (_usePortAudio)
		_playing = false;
	else
		dx::StopSoundMem(_handle);
}

bool SoundManager::Sound::is_playing()
{
	if (fail()) return false;
	if (_usePortAudio) return _playing;
	return dx::CheckSoundMem(_handle) == 1;
}

double SoundManager::Sound::duration()
{
	if (fail()) return 0;
	if (_usePortAudio) return (double)_length / _sampleRate;
	return dx::GetSoundTotalTime(_handle) / 1000.;
}

void SoundManager::Sound::volume(double v)
{
	if (!fail())
	{
		_volume = util::clip(v, 0., 1.);
		if (!_usePortAudio)
			dx::ChangeVolumeSoundMem(util::round(_volume * _master_volume * (_mute ? 0 : 1) * 255), _handle);
	}
}

void SoundManager::Sound::master_volume(double v)
{
	if (!fail())
	{
		_master_volume = util::clip(v, 0., 1.);
		if (!_usePortAudio)
			dx::ChangeVolumeSoundMem(util::round(_volume * _master_volume * (_mute ? 0 : 1) * 255), _handle);
	}
}

void SoundManager::Sound::mute(bool v)
{
	if (!fail())
	{
		_mute = v;
		if (!_usePortAudio)
			dx::ChangeVolumeSoundMem(util::round(_volume * _master_volume * (_mute ? 0 : 1) * 255), _handle);
	}
}

void SoundManager::Sound::write(float* ptr, unsigned long frames, int channels)
{
	if (fail() || !_playing) return;

	if (channels == _channels)
	{
		for (unsigned long i = 0; i < frames; i++)
		{
			for (int j = 0; j < channels; j++)
			{
				if (_index >= _length * _channels)
				{
					_index = 0;
					if (!_loop)
					{
						_playing = false;
						return;
					}
				}
				*ptr++ += static_cast<float>(_get(_index++) * _volume * _master_volume * (_mute ? 0 : 1));
			}
		}
	}
	else if (channels < _channels)
	{
		for (unsigned long i = 0; i < frames; i++)
		{
			if (_index >= _length * _channels)
			{
				_index = 0;
				if (!_loop)
				{
					_playing = false;
					return;
				}
			}
			*ptr++ += (static_cast<float>(_get(_index++) * _volume * _master_volume * (_mute ? 0 : 1)) + static_cast<float>(_get(_index++) * _volume * _master_volume * (_mute ? 0 : 1))) / 2;
		}
	}
	else
	{
		for (unsigned long i = 0; i < frames; i++)
		{
			if (_index >= _length * _channels)
			{
				_index = 0;
				if (!_loop)
				{
					_playing = false;
					return;
				}
			}
			auto value = static_cast<float>(_get(_index++) * _volume * _master_volume * (_mute ? 0 : 1));
			*ptr++ += value;
			*ptr++ += value;
		}
	}
}

#pragma endregion
