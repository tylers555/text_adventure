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
    Win32PerfCounterFrequency = PerformanceCounterFrequencyResult.QuadPart;
    
    if(!AttachConsole(ATTACH_PARENT_PROCESS)){
        Assert(GetLastError() == ERROR_INVALID_HANDLE);
        Assert(AllocConsole());
    }
    
    if(!AssetProcessorMain()){
        return -1;
    }
    
    return 0;
}
