using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using static AeroBeatTools.Interop.PortAudio;

namespace AeroBeatTools.Interop
{
    public class PortAudioDevice
    {
        public int Index { get; private set; }
        public string Name { get; private set; }
        public string Type { get; private set; }

        public static IEnumerable<PortAudioDevice> GetDevices(out int defaultDeviceIndex)
        {
            List<PortAudioDevice> list = new List<PortAudioDevice>();

            if (Pa_Initialize() != paNoError)
                throw new Exception("Failed to initialize PortAudio.");

            int apiCount = Pa_GetHostApiCount();
            if (apiCount < 0)
                throw new Exception("Error has occurred in Pa_GetHostApiCount.");

            List<string> apiNames = new List<string>();

            for (int i = 0; i < apiCount; i++)
                apiNames.Add(Marshal.PtrToStructure<PaHostApiInfo>(Pa_GetHostApiInfo(i)).name);

            int deviceCount = Pa_GetDeviceCount();
            if (deviceCount < 0)
                throw new Exception("Error has occurred in Pa_GetDeviceCount.");

            for (int i = 0; i < deviceCount; i++)
            {
                try
                {
                    var info = Marshal.PtrToStructure<PaDeviceInfo>(Pa_GetDeviceInfo(i));
                    if (info.maxOutputChannels == 0) continue;

                    var device = new PortAudioDevice();
                    device.Index = i;
                    device.Name = info.name;
                    device.Type = apiNames[info.hostApi];
                    list.Add(device);
                }
                catch { }
            }

            int defaultIndex = Pa_GetDefaultOutputDevice();
            if (defaultIndex == paNoDevice)
                defaultIndex = 0;
            defaultDeviceIndex = defaultIndex;

            Pa_Terminate();

            return list;
        } 
    }
}
