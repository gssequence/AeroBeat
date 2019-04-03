using AeroBeatTools.Interop;
using AeroBeatTools.Mvvm;
using Codeplex.Data;
using StatefulModel;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace AeroBeatTools.Models
{
    public class Config : BindableModel
    {
        private static readonly string _configDirectory = "ABFiles/Settings";
        private static readonly string _configPath = "ABFiles/Settings/Config.json";

        private static Config _current;
        public static Config Current => _current;

        private static int defaultAudioDeviceIndex;
        public static int DefaultAudioDeviceIndex => defaultAudioDeviceIndex;

        public static IEnumerable<PortAudioDevice> AudioDevices { get; private set; } = PortAudioDevice.GetDevices(out defaultAudioDeviceIndex);

        static Config()
        {
            if (File.Exists(_configPath))
                _current = load();
            else
            {
                _current = new Config();
                _current.Save();
            }
        }

        private static Config load()
        {
            string json;
            using (var sr = new StreamReader(_configPath, Encoding.UTF8))
                json = sr.ReadToEnd();
            dynamic obj = DynamicJson.Parse(json);
            var ret = new Config();
            if (obj.config())
            {
                var c = obj.config;
                if (c.resolution_width())
                    ret._resolutionWidth = (int)c.resolution_width;
                if (c.resolution_height())
                    ret._resolutionHeight = (int)c.resolution_height;
                if (c.fullscreen())
                    ret._fullscreen = c.fullscreen;
                if (c.vsync())
                    ret._vSync = c.vsync;
                if (c.scene_filter())
                    ret._sceneFilter = c.scene_filter;
                if (c.scan_bms_at_launch())
                    ret._scanBMSAtLaunch = c.scan_bms_at_launch;
                if (c.disable_portaudio())
                    ret._disablePortAudio = c.disable_portaudio;
                if (c.use_default_audio_device())
                    ret._useDefaultAudioDevice = c.use_default_audio_device;
                if (c.audio_device_index())
                    ret._audioDeviceIndex = (int)c.audio_device_index;
                if (c.minimize_mute())
                    ret._minimizeMute = c.minimize_mute;
                if (c.bms_directories())
                    foreach (string item in c.bms_directories)
                        ret.BMSDirectories.Add(item);
            }
            return ret;
        }

        public void Save()
        {
            var obj = new
            {
                config = new
                {
                    resolution_width = ResolutionWidth,
                    resolution_height = ResolutionHeight,
                    fullscreen = Fullscreen,
                    vsync = VSync,
                    scene_filter = SceneFilter,
                    scan_bms_at_launch = ScanBMSAtLaunch,
                    disable_portaudio = DisablePortAudio,
                    use_default_audio_device = UseDefaultAudioDevice,
                    audio_device_index = AudioDeviceIndex,
                    minimize_mute = MinimizeMute,
                    bms_directories = BMSDirectories
                }
            };
            var json = DynamicJson.Serialize(obj);
            if (!Directory.Exists(_configDirectory))
                Directory.CreateDirectory(_configDirectory);
            using (var sw = new StreamWriter(_configPath, false, Encoding.UTF8))
                sw.Write(json);
        }

        public void AddBMSDirectory(string path)
        {
            BMSDirectories.Add(path);
        }

        private int _resolutionWidth = 1280;
        public int ResolutionWidth
        {
            get { return _resolutionWidth; }
            set { SetProperty(ref _resolutionWidth, value); }
        }

        private int _resolutionHeight = 720;
        public int ResolutionHeight
        {
            get { return _resolutionHeight; }
            set { SetProperty(ref _resolutionHeight, value); }
        }

        private bool _fullscreen = false;
        public bool Fullscreen
        {
            get { return _fullscreen; }
            set { SetProperty(ref _fullscreen, value); }
        }

        private bool _vSync = false;
        public bool VSync
        {
            get { return _vSync; }
            set { SetProperty(ref _vSync, value); }
        }

        private bool _sceneFilter = false;
        public bool SceneFilter
        {
            get { return _sceneFilter; }
            set { SetProperty(ref _sceneFilter, value); }
        }

        private bool _scanBMSAtLaunch = true;
        public bool ScanBMSAtLaunch
        {
            get { return _scanBMSAtLaunch; }
            set { SetProperty(ref _scanBMSAtLaunch, value); }
        }

        private bool _disablePortAudio = false;
        public bool DisablePortAudio
        {
            get { return _disablePortAudio; }
            set { SetProperty(ref _disablePortAudio, value); }
        }

        private bool _useDefaultAudioDevice = true;
        public bool UseDefaultAudioDevice
        {
            get { return _useDefaultAudioDevice; }
            set { SetProperty(ref _useDefaultAudioDevice, value); }
        }

        private int _audioDeviceIndex = 0;
        public int AudioDeviceIndex
        {
            get { return _audioDeviceIndex; }
            set { SetProperty(ref _audioDeviceIndex, value); }
        }

        private bool _minimizeMute = true;
        public bool MinimizeMute
        {
            get { return _minimizeMute; }
            set { SetProperty(ref _minimizeMute, value); }
        }


        public ObservableSynchronizedCollection<string> BMSDirectories { get; private set; } = new ObservableSynchronizedCollection<string>();
    }
}
