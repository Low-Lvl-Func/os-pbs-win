#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define SIGNAL_EVENT_NAME _T("Local\\MySigUsr1Event")

// This thread simulates an external signal after 5 seconds
DWORD WINAPI SignalTriggerThread(LPVOID lpParam) {
    HANDLE hEvent = (HANDLE)lpParam;
    printf("[System] Signal will fire in 5 seconds...\n");
    Sleep(5000);
    printf("[System] Firing SIGUSR1 (Event)!\n");
    SetEvent(hEvent);
    return 0;
}

void ChildProcess() {
    printf("    [Child] Started. Waiting for SIGUSR1...\n");
    HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SIGNAL_EVENT_NAME);
    if (hEvent != NULL) {
        WaitForSingleObject(hEvent, INFINITE);
        printf("    [Child] Received SIGUSR1. Terminating.\n");
        CloseHandle(hEvent);
    }
}

int main(int argc, TCHAR* argv[]) {
    if (argc > 1 && _tcscmp(argv[1], _T("child")) == 0) {
        ChildProcess();
        return 0;
    }

    // Create the event first
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, SIGNAL_EVENT_NAME);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    // Use GetModuleFileName to get the full path to THIS exe
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    
    TCHAR szCmdline[MAX_PATH * 2];
    _stprintf_s(szCmdline, MAX_PATH * 2, _T("\"%s\" child"), szPath);

    printf("[Parent] Spawning child process...\n");
    if (!CreateProcess(NULL, szCmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed (%d). Ensure the exe exists!\n", GetLastError());
        return 1;
    }

    // Start a thread to trigger the "signal" automatically for testing
    CreateThread(NULL, 0, SignalTriggerThread, hEvent, 0, NULL);

    // Wait for Signal or Child Exit
    HANDLE waitHandles[2] = { hEvent, pi.hProcess };
    DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

    if (waitResult == WAIT_OBJECT_0) {
        printf("[Parent] Received SIGUSR1 first. Notifying child (via shared event)...\n");
    } else {
        printf("[Parent] Child terminated first (SIGUSR1 not received by parent).\n");
    }

    // wait() for child
    WaitForSingleObject(pi.hProcess, INFINITE);
    printf("[Parent] Child process synced. Exiting.\n");

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hEvent);
    return 0;
}


/*
$evt = [System.Threading.EventWaitHandle]::OpenExisting("Local\MySigUsr1Event")
$evt.Set()

Stop-Process -Name "winproj" -Force
*/
