using System;
using System.Runtime.InteropServices;

namespace AeroBeatTools.Interop
{
    public static class PortAudio
    {
        private const string portaudio_dll = "portaudio_x86.dll";

        public const int paNoError = 0;
        public const int paNoDevice = -1;

        [StructLayout(LayoutKind.Sequential)]
        public struct PaHostApiInfo
        {
            public int structVersion;
            public uint type;
            [MarshalAs(UnmanagedType.LPStr)]
            public string name;
            public int deviceCount;
            public int defaultInputDevice;
            public int defaultOutputDevice;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct PaDeviceInfo
        {
            public int structVersion;
            [MarshalAs(UnmanagedType.LPStr)]
            public string name;
            public int hostApi;
            public int maxInputChannels;
            public int maxOutputChannels;
            public double defaultLowInputLatency;
            public double defaultLowOutputLatency;
            public double defaultHighInputLatency;
            public double defaultHighOutputLatency;
            public double defaultSampleRate;
        }

        [DllImport(portaudio_dll)]
        public extern static int Pa_Initialize();

        [DllImport(portaudio_dll)]
        public extern static int Pa_GetHostApiCount();

        [DllImport(portaudio_dll, CallingConvention = CallingConvention.Cdecl)]
        public extern static IntPtr Pa_GetHostApiInfo(int hostApi);

        [DllImport(portaudio_dll)]
        public extern static int Pa_GetDeviceCount();

        [DllImport(portaudio_dll, CallingConvention = CallingConvention.Cdecl)]
        public extern static IntPtr Pa_GetDeviceInfo(int device);

        [DllImport(portaudio_dll)]
        public extern static int Pa_GetDefaultOutputDevice();

        [DllImport(portaudio_dll)]
        public extern static int Pa_Terminate();
    }
}
