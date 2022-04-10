#include "asset_processor.cpp"

#include <Windows.h>
#include "win32/win32_os.cpp"

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode){
    //~ Initialize Win32 stuff
    LARGE_INTEGER PerformanceCounterFrequencyResult;
    QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
    GlobalPerfCounterFrequency = PerformanceCounterFrequencyResult.QuadPart;
    
    AssetProcessorMain();
    
    return 0;
}
