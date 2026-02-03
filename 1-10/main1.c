#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h>
#include <tlhelp32.h>

static DWORD GetParentPID() {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0, currPid = GetCurrentProcessId();

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    pe32.dwSize = sizeof pe32;
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == currPid) {
                ppid = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return ppid;
}

static void CreateChild(int idx) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof si);
    si.cb = sizeof si;
    ZeroMemory(&pi, sizeof pi);

    TCHAR szExePath[MAX_PATH];
    GetModuleFileName(NULL, szExePath, MAX_PATH);

    // Pass 'child' and the current 'idx' as command line arguments
    TCHAR szCmdLine[MAX_PATH];
    _stprintf_s(szCmdLine, MAX_PATH, _T("\"%s\" child %d"), szExePath, idx);

    if (!CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Create proc failed (%d)!\n", (int)GetLastError());
        return;
    }

    _tprintf(_T("Parent (PID: %d): Created Child %d with PID: %d\n"),
        GetCurrentProcessId(), idx, pi.dwProcessId);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main(int argc, char *argv[]) {
    // Check if we are in the child process
    if (argc > 2 && strcmp(argv[1], "child") == 0) {
        int childIdx = atoi(argv[2]); // Convert the index string back to an int
        
        const char* fmt = "Child (Index %d, PID %d): My parent's PID is %d\n";
        printf(fmt, childIdx, GetCurrentProcessId(), GetParentPID());
        return EXIT_SUCCESS;
    }

    const int n = 3;
    for (int i = 0; i < n; i++) {
        CreateChild(i);
    }

    // Give children time to print before parent exits
    Sleep(1000);
    return EXIT_SUCCESS;
}
