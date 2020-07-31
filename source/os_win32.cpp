#include "main.cpp"

#include <string.h>

#include <Windows.h>

BOOL WINAPI default_handler_routine(DWORD control_signal) {
    switch(control_signal)
    {
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT: {
            is_running = false;
            save_game();
            ExitProcess(0);
        }break;
        default: {
            return false; // We didn't handle the control signal
        }break;
    }
}

internal void *alloc(umw bytes) {
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes);
}

internal void _free(void *memory) {
    HeapFree(GetProcessHeap(), 0, memory);
}

internal void print_to_console(char *format, ...) {
    va_list var_args;
    va_start(var_args, format);
    
    char buffer[512];
    stbsp_vsnprintf(buffer, sizeof(buffer), format, var_args);
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buffer, (DWORD)strlen(buffer), 0, 0);
    
    va_end(var_args);
}

internal char *read_console_input(uint *bytes_read){
    char *buffer = push_array(&temporary_arena, char, 512);
    ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buffer, 512, (LPDWORD)bytes_read, 0);
    *bytes_read -= 2;
    buffer[*bytes_read] = 0; // the -2 is because windows ends lines as "/r/n"
    return buffer;
}

//~ WinMain
int CALLBACK WinMain(HINSTANCE instance,
                     HINSTANCE previous_instance,
                     LPSTR     command_line,
                     int       show_command) {
    AllocConsole();
    SetConsoleCtrlHandler(default_handler_routine, true);
    
    return main_loop();
}
