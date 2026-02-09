#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// Use 'snprintf' for standard compliance, or '_snprintf' in older MSVC
#define BUFFER_SIZE 32768

int main(int argc, char* argv[]) {
    int rem;
    // Simple logic to determine remaining count
    if (argc >= 2) {
        rem = atoi(argv[1]);
    } else {
        // Only the very first process uses the default N
        // (You might want to change this to default to 3 if no args)
        rem = 3; 
    }

    printf("PID: %lu | Remaining: %d\n", GetCurrentProcessId(), rem);

    if (rem > 1) {
        // 1. USE SPECIFIC 'A' STRUCTURE FOR ANSI
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        
        char cmdLine[BUFFER_SIZE] = { 0 };
        char exePath[BUFFER_SIZE] = { 0 };

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // 2. USE 'A' FUNCTION TO READ INTO CHAR ARRAY
        if (!GetModuleFileNameA(NULL, exePath, BUFFER_SIZE)) {
            fprintf(stderr, "Failed to get path. Error: %lu\n", GetLastError());
            return EXIT_FAILURE;
        }

        // Format: "C:\Path\To\Exe" 2
        // using snprintf is generally safer/standard than sprintf_s outside MSVC
        snprintf(cmdLine, BUFFER_SIZE, "\"%s\" %d", exePath, rem - 1);
        
        // Debug print to see exactly what we are trying to run
        printf("Spawning child with command: %s\n", cmdLine);

        // 3. USE 'A' FUNCTION TO CREATE PROCESS
        if (!CreateProcessA(
            NULL,           
            cmdLine,        // Pass the ANSI char array here
            NULL,           
            NULL,           
            FALSE,          
            0,              
            NULL,           
            NULL,           
            &si,            // Pass address of STARTUPINFOA
            &pi)            
            ) {
            fprintf(stderr, "CreateProcess failed. Error Code: %lu\n", GetLastError());
            return EXIT_FAILURE;
        }

        // Wait for child
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        // Clean up handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        printf("End of chain at PID %lu.\n", GetCurrentProcessId());
    }

    return EXIT_SUCCESS;
}
