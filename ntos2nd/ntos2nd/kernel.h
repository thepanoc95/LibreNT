#ifndef NTOS2ND_KERNEL_H
#define NTOS2ND_KERNEL_H

/*
    ntos2nd - Usermode Windows NT-like Kernel
    
    A sandboxed kernel that bridges Windows APIs to an open-source environment.
    Programs run as child processes inside the kernel's sandbox.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* NTSTATUS type and common codes for mingw-w64 compatibility */
#ifndef NTSTATUS_TYPEDEF
#define NTSTATUS_TYPEDEF
typedef LONG NTSTATUS;
#endif

/* Common NTSTATUS codes */
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000DL)
#endif
#ifndef STATUS_INSUFFICIENT_RESOURCES
#define STATUS_INSUFFICIENT_RESOURCES ((NTSTATUS)0xC000009AL)
#endif
#ifndef STATUS_NOT_FOUND
#define STATUS_NOT_FOUND ((NTSTATUS)0xC0000225L)
#endif
#ifndef STATUS_NOT_IMPLEMENTED
#define STATUS_NOT_IMPLEMENTED ((NTSTATUS)0xC00000BBL)
#endif
#ifndef STATUS_ACCESS_DENIED
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xC0000022L)
#endif
#ifndef STATUS_TIMEOUT
#define STATUS_TIMEOUT ((NTSTATUS)0x00000102L)
#endif
#ifndef STATUS_INVALID_HANDLE
#define STATUS_INVALID_HANDLE ((NTSTATUS)0xC0000008L)
#endif

/* NT_SUCCESS macro */
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

/* Version */
#define NTOS2ND_VERSION_MAJOR 0
#define NTOS2ND_VERSION_MINOR 1
#define NTOS2ND_VERSION_PATCH 0

/* Maximum limits */
#define NTOS2ND_MAX_PROCESSES 256
#define NTOS2ND_MAX_THREADS_PER_PROCESS 64
#define NTOS2ND_MAX_API_BRIDGES 1024
#define NTOS2ND_MAX_PATH 260
#define NTOS2ND_PIPE_BUFFER_SIZE 4096

/* Process states */
typedef enum _NTOS2ND_PROCESS_STATE {
    ProcessStateCreated = 0,
    ProcessStateRunning,
    ProcessStateSuspended,
    ProcessStateWaiting,
    ProcessStateTerminated
} NTOS2ND_PROCESS_STATE;

/* Thread states */
typedef enum _NTOS2ND_THREAD_STATE {
    ThreadStateReady = 0,
    ThreadStateRunning,
    ThreadStateWaiting,
    ThreadStateTerminated
} NTOS2ND_THREAD_STATE;

/* Object header for all kernel objects */
typedef struct _NTOS2ND_OBJECT_HEADER {
    uint32_t Signature;
    uint32_t Type;
    LONG RefCount;
} NTOS2ND_OBJECT_HEADER;

/* Process Control Block */
typedef struct _NTOS2ND_PROCESS {
    NTOS2ND_OBJECT_HEADER Header;
    uint32_t ProcessId;
    uint32_t ParentProcessId;
    NTOS2ND_PROCESS_STATE State;
    HANDLE ProcessHandle;
    HANDLE PipeRead;
    HANDLE PipeWrite;
    char ProcessName[NTOS2ND_MAX_PATH];
    void* ProcessMemory;
    size_t ProcessMemorySize;
    CRITICAL_SECTION Lock;
    BOOL bIsSandboxed;
} NTOS2ND_PROCESS;

/* Thread Control Block */
typedef struct _NTOS2ND_THREAD {
    NTOS2ND_OBJECT_HEADER Header;
    uint32_t ThreadId;
    uint32_t ProcessId;
    NTOS2ND_THREAD_STATE State;
    HANDLE ThreadHandle;
    HANDLE PipeRead;
    HANDLE PipeWrite;
    DWORD ThreadIdWindows;
} NTOS2ND_THREAD;

/* Syscall numbers (mirrors Windows NT syscall interface) */
#define NTOS2ND_SYS_OPEN_PROCESS          1
#define NTOS2ND_SYS_CLOSE_HANDLE          2
#define NTOS2ND_SYS_READ_FILE             3
#define NTOS2ND_SYS_WRITE_FILE            4
#define NTOS2ND_SYS_CREATE_FILE           5
#define NTOS2ND_SYS_DELETE_FILE           6
#define NTOS2ND_SYS_ALLOC_MEMORY          7
#define NTOS2ND_SYS_FREE_MEMORY           8
#define NTOS2ND_SYS_CREATE_THREAD         9
#define NTOS2ND_SYS_TERMINATE_PROCESS     10
#define NTOS2ND_SYS_QUERY_INFORMATION     11
#define NTOS2ND_SYS_SET_INFORMATION       12
#define NTOS2ND_SYS_WAIT_FOR_SINGLE_OBJECT 13

/* API Bridge types */
#define NTOS2ND_API_BRIDGE_KNOWN        1
#define NTOS2ND_API_BRIDGE_UNKNOWN      2
#define NTOS2ND_API_BRIDGE_CUSTOM       3

/* API Bridge entry */
typedef struct _NTOS2ND_API_BRIDGE {
    uint32_t ApiNumber;
    const char* ApiName;
    uint32_t BridgeType;
    void* Implementation;
    void* ForwardAddress;
} NTOS2ND_API_BRIDGE;

/* IPC message structure */
typedef struct _NTOS2ND_IPC_MESSAGE {
    uint32_t MessageType;
    uint32_t ProcessId;
    uint32_t ThreadId;
    uint32_t SyscallNumber;
    uint64_t Parameter1;
    uint64_t Parameter2;
    uint64_t Parameter3;
    uint64_t Parameter4;
    uint64_t Parameter5;
    uint64_t Result;
    uint32_t Status;
} NTOS2ND_IPC_MESSAGE;

/* Message types */
#define NTOS2ND_MSG_SYSCALL_REQUEST     1
#define NTOS2ND_MSG_SYSCALL_RESPONSE    2
#define NTOS2ND_MSG_PROCESS_CREATE      3
#define NTOS2ND_MSG_PROCESS_EXIT        4
#define NTOS2ND_MSG_THREAD_CREATE       5
#define NTOS2ND_MSG_THREAD_EXIT         6
#define NTOS2ND_MSG_EXCEPTION           7

/* Kernel initialization */
NTSTATUS NTOS2NDInitializeKernel(void);
NTSTATUS NTOS2NDShutdownKernel(void);

/* Process management */
NTOS2ND_PROCESS* NTOS2NDCreateProcess(const char* ProcessPath, BOOL bSandboxed);
NTSTATUS NTOS2NDTerminateProcess(NTOS2ND_PROCESS* Process);
NTSTATUS NTOS2NDWaitForProcess(NTOS2ND_PROCESS* Process, DWORD dwMilliseconds);

/* Thread management */
NTOS2ND_THREAD* NTOS2NDCreateThread(NTOS2ND_PROCESS* Process, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);
NTSTATUS NTOS2NDTerminateThread(NTOS2ND_THREAD* Thread);

/* Syscall handler */
NTSTATUS NTOS2NDHandleSyscall(NTOS2ND_PROCESS* Process, uint32_t SyscallNumber, 
                               uint64_t* Parameters, uint32_t ParameterCount, uint64_t* Result);

/* IPC functions */
NTSTATUS NTOS2NDSendIPCMessage(NTOS2ND_PROCESS* Process, NTOS2ND_IPC_MESSAGE* Message);
NTSTATUS NTOS2NDReceiveIPCMessage(NTOS2ND_PROCESS* Process, NTOS2ND_IPC_MESSAGE* Message);

/* API bridge */
NTSTATUS NTOS2NDRegisterAPI(const char* ApiName, void* Implementation, uint32_t BridgeType);
void* NTOS2NDGetAPIImplementation(const char* ApiName);

/* Forward declaration */
struct _NTOS2ND_SANDBOX_CONFIG;

/* Main kernel control functions */
NTSTATUS NTOS2NDInitialize(void);
NTSTATUS NTOS2NDShutdown(void);
NTOS2ND_PROCESS* NTOS2NDRunProgram(const char* ProgramPath, struct _NTOS2ND_SANDBOX_CONFIG* Config);
NTSTATUS NTOS2NDWaitForProcessTermination(NTOS2ND_PROCESS* Process, DWORD dwMilliseconds);

#endif /* NTOS2ND_KERNEL_H */