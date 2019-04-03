using AeroBeatTools.Mvvm;
using Livet.Messaging;
using Livet.Messaging.IO;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AeroBeatTools.ViewModels
{
    public class MainWindowViewModel : BindableViewModel
    {
        public ConfigViewModel ConfigViewModel { get; } = new ConfigViewModel();

        public MainWindowViewModel()
        {
            if (ConfigViewModel.ResolutionWidth == 640 && ConfigViewModel.ResolutionHeight == 480)
                IsCheckedResolutionSD = true;
            else if (ConfigViewModel.ResolutionWidth == 1280 && ConfigViewModel.ResolutionHeight == 720)
                IsCheckedResolutionHD = true;
            else
                IsCheckedResolutionCustom = true;

            CustomResolutionWidthInput = ConfigViewModel.ResolutionWidth.ToString();
            CustomResolutionHeightInput = ConfigViewModel.ResolutionHeight.ToString();

            var dev = ConfigViewModel.AudioDevices.Where(d => d.Index == ConfigViewModel.AudioDeviceIndex).FirstOrDefault();
            if (dev != null)
                _selectedDeviceIndex = ConfigViewModel.AudioDevices.Select((d, i) => new { i = i, d = d }).Where(d => d.d.Index == dev.Index).First().i;
            else
            {
                var def = ConfigViewModel.AudioDevices.Where(d => d.Index == ConfigViewModel.DefaultAudioDeviceIndex).FirstOrDefault();
                if (def != null)
                    _selectedDeviceIndex = ConfigViewModel.AudioDevices.Select((d, i) => new { i = i, d = d }).Where(d => d.d.Index == def.Index).First().i;
            }
        }

        protected override void Dispose(bool disposing)
        {
            ConfigViewModel.Save();
            base.Dispose(disposing);
        }

        private bool _isCheckedResolutionSD;
        public bool IsCheckedResolutionSD
        {
            get { return _isCheckedResolutionSD; }
            set
            {
                SetProperty(ref _isCheckedResolutionSD, value);
                if (value)
                {
                    ConfigViewModel.ResolutionWidth = 640;
                    ConfigViewModel.ResolutionHeight = 480;
                }
            }
        }

        private bool _isCheckedResolutionHD;
        public bool IsCheckedResolutionHD
        {
            get { return _isCheckedResolutionHD; }
            set
            {
                SetProperty(ref _isCheckedResolutionHD, value);
                if (value)
                {
                    ConfigViewModel.ResolutionWidth = 1280;
                    ConfigViewModel.ResolutionHeight = 720;
                }
            }
        }

        private bool _isCheckedResolutionCustom;
        public bool IsCheckedResolutionCustom
        {
            get { return _isCheckedResolutionCustom; }
            set
            {
                SetProperty(ref _isCheckedResolutionCustom, value);
                if (value)
                {
                    int w, h;
                    if (int.TryParse(CustomResolutionWidthInput, out w))
                        ConfigViewModel.ResolutionWidth = w;
                    if (int.TryParse(CustomResolutionHeightInput, out h))
                        ConfigViewModel.ResolutionHeight = h;
                }
            }
        }

        private string _customResolutionWidthInput;
        public string CustomResolutionWidthInput
        {
            get { return _customResolutionWidthInput; }
            set
            {
                if (SetProperty(ref _customResolutionWidthInput, value) && IsCheckedResolutionCustom)
                {
                    int w;
                    if (int.TryParse(value, out w))
                        ConfigViewModel.ResolutionWidth = w;
                }
            }
        }

        private string _customResolutionHeightInput;
        public string CustomResolutionHeightInput
        {
            get { return _customResolutionHeightInput; }
            set
            {
                if (SetProperty(ref _customResolutionHeightInput, value) && IsCheckedResolutionCustom)
                {
                    int h;
                    if (int.TryParse(value, out h))
                        ConfigViewModel.ResolutionHeight = h;
                }
            }
        }

        private int _selectedDeviceIndex;
        public int SelectedDeviceIndex
        {
            get { return _selectedDeviceIndex; }
            set
            {
                if (SetProperty(ref _selectedDeviceIndex, value))
                    ConfigViewModel.AudioDeviceIndex = ConfigViewModel.AudioDevices.ElementAt(value).Index;
            }
        }

        public void StartAeroBeat()
        {
            Messenger.Raise(new InteractionMessage("Close"));
            Process.Start("AeroBeat.exe");
        }

        public void AddBMSFolder(FolderSelectionMessage msg)
        {
            if (msg.Response != null)
            {
                var path = msg.Response.Replace('\\', '/');
                if (!ConfigViewModel.BMSDirectories.Any(vm => vm.Entry == path))
                {
                    ConfigViewModel.AddBMSDirectory(path);
                    ConfigViewModel.ScanBMSAtLaunch = true;
                }
            }
        }
    }
}
