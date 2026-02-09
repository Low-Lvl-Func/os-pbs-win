#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BUFFER_SIZE 32768

// Helper function to handle the "Recursive" logic
int SpawnRecursiveChild(int n, const char* exePath) {
    // BASE CASE: If n <= 1, we are the leaf node
    if (n <= 1) {
        printf("  [Leaf] PID %lu reached the bottom. Returning 1.\n", GetCurrentProcessId());
        return 1;
    }

    // RECURSIVE STEP: Spawn child with n - 1
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdLine[BUFFER_SIZE] = { 0 };
    DWORD childExitCode = 0;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Prepare command line: "ExePath" n-1
    snprintf(cmdLine, BUFFER_SIZE, "\"%s\" %d", exePath, n - 1);

    printf("[Parent] PID %lu spawning child (n=%d)...\n", GetCurrentProcessId(), n - 1);

    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Recursion broken! CreateProcess failed (%lu)\n", GetLastError());
        return 0; 
    }

    // WAIT: This mimics the "stack frame" staying open while waiting for the recursive call
    WaitForSingleObject(pi.hProcess, INFINITE);

    // RETURN: Get the result (exit code) from the child
    if (GetExitCodeProcess(pi.hProcess, &childExitCode)) {
        printf("[Parent] PID %lu received %lu from child. Returning %lu.\n", 
               GetCurrentProcessId(), childExitCode, childExitCode + 1);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // The result of this "frame" is 1 (myself) + child's result
    return (int)childExitCode + 1;
}

int main(int argc, char* argv[]) {
    int n = 0;
    char exePath[BUFFER_SIZE] = { 0 };

    // 1. Determine Input (Default to 3 if no args provided)
    if (argc >= 2) {
        n = atoi(argv[1]);
    } else {
        n = 3; 
    }

    // 2. Get Path (Required for CreateProcess recursion)
    if (!GetModuleFileNameA(NULL, exePath, BUFFER_SIZE)) {
        return EXIT_FAILURE;
    }

    // 3. Execute Recursive Logic
    int result = SpawnRecursiveChild(n, exePath);

    // 4. If we are the specific Root process (conceptually), print final result
    // (We check argc < 2 to distinguish the manual user launch from the automated child launches, 
    // though in a perfect chain, every parent prints their 'return' log above)
    if (argc < 2) {
        printf("\n--- Final Result ---\n");
        printf("Total chain depth calculated recursively: %d\n", result);
    }

    // 5. Pass result up to the parent (Operating System or Previous Process)
    return result;
}
