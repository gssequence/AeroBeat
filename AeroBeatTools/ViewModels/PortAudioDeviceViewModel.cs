using AeroBeatTools.Interop;
using AeroBeatTools.Mvvm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AeroBeatTools.ViewModels
{
    public class PortAudioDeviceViewModel : BindableViewModel
    {
        private PortAudioDevice _model;

        public int Index => _model.Index;
        public string Name => _model.Name;
        public string Type => _model.Type;

        public PortAudioDeviceViewModel(PortAudioDevice model)
        {
            _model = model;
        }
    }
}
