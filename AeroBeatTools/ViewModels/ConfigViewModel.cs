using AeroBeatTools.Interop;
using AeroBeatTools.Models;
using AeroBeatTools.Mvvm;
using StatefulModel;
using System.Collections.Generic;
using System.Linq;

namespace AeroBeatTools.ViewModels
{
    public class ConfigViewModel : BindableViewModel<Config>
    {
        public class DeletableEntryViewModel<T>
        {
            private ObservableSynchronizedCollection<T> _collection;
            public T Entry { get; private set; }

            public DeletableEntryViewModel(T entry, ObservableSynchronizedCollection<T> collection)
            {
                Entry = entry;
                _collection = collection;
            }

            public void Delete()
            {
                _collection.Remove(Entry);
            }
        }

        public ConfigViewModel() : base(Config.Current)
        {
            BMSDirectories = Model.BMSDirectories
                .ToSyncedObservableSynchronizedCollection(s => new DeletableEntryViewModel<string>(s, Model.BMSDirectories))
                .ToSyncedReadOnlyNotifyChangedCollection();
            CompositeDisposable.Add(BMSDirectories);
        }

        public int ResolutionWidth
        {
            get { return Model.ResolutionWidth; }
            set { Model.ResolutionWidth = value; }
        }

        public int ResolutionHeight
        {
            get { return Model.ResolutionHeight; }
            set { Model.ResolutionHeight = value; }
        }

        public bool Fullscreen
        {
            get { return Model.Fullscreen; }
            set { Model.Fullscreen = value; }
        }

        public bool VSync
        {
            get { return Model.VSync; }
            set { Model.VSync = value; }
        }

        public bool SceneFilter
        {
            get { return Model.SceneFilter; }
            set { Model.SceneFilter = value; }
        }

        public bool ScanBMSAtLaunch
        {
            get { return Model.ScanBMSAtLaunch; }
            set { Model.ScanBMSAtLaunch = value; }
        }

        public bool DisablePortAudio
        {
            get { return Model.DisablePortAudio; }
            set { Model.DisablePortAudio = value; }
        }

        public bool UseDefaultAudioDevice
        {
            get { return Model.UseDefaultAudioDevice; }
            set { Model.UseDefaultAudioDevice = value; }
        }

        public int AudioDeviceIndex
        {
            get { return Model.AudioDeviceIndex; }
            set { Model.AudioDeviceIndex = value; }
        }

        public bool MinimizeMute
        {
            get { return Model.MinimizeMute; }
            set { Model.MinimizeMute = value; }
        }

        public ReadOnlyNotifyChangedCollection<DeletableEntryViewModel<string>> BMSDirectories { get; private set; }

        public IEnumerable<PortAudioDeviceViewModel> AudioDevices { get; } = Config.AudioDevices.Select(d => new PortAudioDeviceViewModel(d));

        public int DefaultAudioDeviceIndex => Config.DefaultAudioDeviceIndex;

        public void Save() => Model.Save();

        public void AddBMSDirectory(string path) => Model.AddBMSDirectory(path);
    }
}
