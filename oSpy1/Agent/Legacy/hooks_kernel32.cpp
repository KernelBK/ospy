#include "stdafx.h"
#include "hooking.h"
#include "logging.h"

#pragma managed(push, off)

#define DEVICEIOCONTROL_ARGS_SIZE (8 * 4)

static BOOL __cdecl
DeviceIoControl_called(BOOL carry_on,
                       DWORD ret_addr,
                       HANDLE hDevice,
                       DWORD dwIoControlCode,
                       LPVOID lpInBuffer,
                       DWORD nInBufferSize,
                       LPVOID lpOutBuffer,
                       DWORD nOutBufferSize,
                       LPDWORD lpBytesReturned,
                       LPOVERLAPPED lpOverlapped)
{
    void *bt_address = (char *) &carry_on + 8 + DEVICEIOCONTROL_ARGS_SIZE;

    message_logger_log(_T("DeviceIoControl"), bt_address, (DWORD) hDevice, MESSAGE_TYPE_PACKET, MESSAGE_CTX_INFO,
                       PACKET_DIRECTION_OUTGOING, NULL, NULL, (const char *) lpInBuffer, nInBufferSize,
                       _T("hDevice=0x%08x, dwIoControlCode=0x%08x, nInBufferSize=%u, nOutBufferSize=%u"),
                       hDevice, dwIoControlCode, nInBufferSize, nOutBufferSize);

    return TRUE;
}

static BOOL __stdcall
DeviceIoControl_done(BOOL retval,
                     HANDLE hDevice,
                     DWORD dwIoControlCode,
                     LPVOID lpInBuffer,
                     DWORD nInBufferSize,
                     LPVOID lpOutBuffer,
                     DWORD nOutBufferSize,
                     LPDWORD lpBytesReturned,
                     LPOVERLAPPED lpOverlapped)
{
    DWORD err = GetLastError();
    int ret_addr = *((DWORD *) ((DWORD) &retval - 4));

    SetLastError(err);
    return retval;
}

HOOK_GLUE_SPECIAL(DeviceIoControl, DEVICEIOCONTROL_ARGS_SIZE)

void
hook_kernel32()
{
    HMODULE h = HookManager::Obtain()->OpenLibrary(_T("kernel32.dll"));
    _ASSERT(h != NULL);
    
    HOOK_FUNCTION_SPECIAL(h, DeviceIoControl);
}

#pragma managed(pop)
