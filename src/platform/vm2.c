// Moemon Kai: the VM2 over USB (include/vm2.h; the protocol in docs/VM2_USB_PROTOCOL.md, as the VMU manager's
// vmu_pet/manager/vm2link.cpp does it).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm2.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#endif

#define TIMEOUT_MS 500   // the VM2 GUI's

static char sInfo[96];
static uint8_t sBuf[4096];
static size_t sHave, sAt;

// ---- the serial port

#ifdef _WIN32

static HANDLE sPort;

static int PortOpen(const char *name, int baud)
{
    char path[64];
    DCB dcb;
    HANDLE h;
    snprintf(path, sizeof path, "\\\\.\\%s", name);
    h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return 0;
    memset(&dcb, 0, sizeof dcb);
    dcb.DCBlength = sizeof dcb;
    GetCommState(h, &dcb);
    dcb.BaudRate = (DWORD)baud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    if (!SetCommState(h, &dcb))
    {
        CloseHandle(h);
        return 0;
    }
    SetupComm(h, 1 << 16, 1 << 16);
    PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
    sPort = h;
    sHave = sAt = 0;
    return 1;
}

static void PortClose(void)
{
    if (sPort)
        CloseHandle(sPort);
    sPort = NULL;
    sHave = sAt = 0;
}

static int PortIsOpen(void) { return sPort != NULL; }

static int PortWrite(const void *data, size_t n)
{
    const uint8_t *p = data;
    while (n)
    {
        DWORD done = 0;
        if (!WriteFile(sPort, p, (DWORD)n, &done, NULL))
            return 0;
        p += done;
        n -= done;
    }
    return 1;
}

static int PortReadByte(int timeoutMs)
{
    COMMTIMEOUTS t;
    DWORD got = 0;
    if (sAt < sHave)
        return sBuf[sAt++];
    if (!sPort)
        return -1;
    t.ReadIntervalTimeout = MAXDWORD;
    t.ReadTotalTimeoutMultiplier = MAXDWORD;
    t.ReadTotalTimeoutConstant = (DWORD)timeoutMs;
    t.WriteTotalTimeoutMultiplier = 0;
    t.WriteTotalTimeoutConstant = 2000;
    SetCommTimeouts(sPort, &t);
    if (!ReadFile(sPort, sBuf, sizeof sBuf, &got, NULL) || got == 0)
        return -1;
    sHave = got;
    sAt = 1;
    return sBuf[0];
}

static void PortFlushInput(void)
{
    sHave = sAt = 0;
    if (sPort)
        PurgeComm(sPort, PURGE_RXCLEAR);
}

int Vm2_FindPort(char *name, size_t size)
{
    HDEVINFO set = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    SP_DEVINFO_DATA info;
    DWORD i;
    int found = 0;
    if (set == INVALID_HANDLE_VALUE)
        return 0;
    info.cbSize = sizeof info;
    for (i = 0; !found && SetupDiEnumDeviceInfo(set, i, &info); i++)
    {
        char friendly[256] = "";
        char *a, *b;
        SetupDiGetDeviceRegistryPropertyA(set, &info, SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendly, sizeof friendly, NULL);
        if (!strstr(friendly, "CH340"))
            continue;
        a = strstr(friendly, "(COM");
        b = a ? strchr(a, ')') : NULL;
        if (!a || !b || (size_t)(b - a - 1) >= size)
            continue;
        memcpy(name, a + 1, b - a - 1);
        name[b - a - 1] = 0;
        found = 1;
    }
    SetupDiDestroyDeviceInfoList(set);
    return found;
}

#else  // POSIX

static int sFd = -1;

static speed_t SpeedFor(int baud)
{
    switch (baud)
    {
#ifdef B921600
    case 921600: return B921600;
#endif
#ifdef B460800
    case 460800: return B460800;
#endif
    default: return B115200;
    }
}

static int PortOpen(const char *name, int baud)
{
    struct termios t;
    int fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
        return 0;
    if (tcgetattr(fd, &t) == 0)
    {
        cfmakeraw(&t);
        t.c_cflag |= CLOCAL | CREAD;
        t.c_cflag &= ~(CRTSCTS | CSTOPB | PARENB);
        cfsetispeed(&t, SpeedFor(baud));
        cfsetospeed(&t, SpeedFor(baud));
        tcsetattr(fd, TCSANOW, &t);
    }
    tcflush(fd, TCIOFLUSH);
    sFd = fd;
    sHave = sAt = 0;
    return 1;
}

static void PortClose(void)
{
    if (sFd >= 0)
        close(sFd);
    sFd = -1;
    sHave = sAt = 0;
}

static int PortIsOpen(void) { return sFd >= 0; }

static int PortWrite(const void *data, size_t n)
{
    const uint8_t *p = data;
    while (n)
    {
        ssize_t w = write(sFd, p, n);
        if (w < 0)
        {
            struct pollfd pf = {sFd, POLLOUT, 0};
            if (poll(&pf, 1, 2000) <= 0)
                return 0;
            continue;
        }
        p += w;
        n -= (size_t)w;
    }
    return 1;
}

static int PortReadByte(int timeoutMs)
{
    struct pollfd pf;
    ssize_t got;
    if (sAt < sHave)
        return sBuf[sAt++];
    if (sFd < 0)
        return -1;
    pf.fd = sFd;
    pf.events = POLLIN;
    pf.revents = 0;
    if (poll(&pf, 1, timeoutMs) <= 0)
        return -1;
    got = read(sFd, sBuf, sizeof sBuf);
    if (got <= 0)
        return -1;
    sHave = (size_t)got;
    sAt = 1;
    return sBuf[0];
}

static void PortFlushInput(void)
{
    sHave = sAt = 0;
    if (sFd >= 0)
        tcflush(sFd, TCIFLUSH);
}

static int ReadSmall(const char *path, char *out, size_t size)
{
    FILE *f = fopen(path, "r");
    size_t n;
    if (!f)
        return 0;
    n = fread(out, 1, size - 1, f);
    fclose(f);
    out[n] = 0;
    while (n && (out[n - 1] == '\n' || out[n - 1] == '\r'))
        out[--n] = 0;
    return n > 0;
}

int Vm2_FindPort(char *name, size_t size)
{
    // MOEMON_VM2_PORT names one outright (a test's pseudo-terminal)
    const char *forced = getenv("MOEMON_VM2_PORT");
    DIR *d;
    struct dirent *e;
    int found = 0;
    if (forced && *forced)
    {
        snprintf(name, size, "%s", forced);
        return access(forced, F_OK) == 0;
    }
    d = opendir("/sys/class/tty");
    if (!d)
        return 0;
    while (!found && (e = readdir(d)) != NULL)
    {
        char path[300], vid[16];
        if (strncmp(e->d_name, "ttyUSB", 6) != 0 && strncmp(e->d_name, "ttyACM", 6) != 0)
            continue;
        snprintf(path, sizeof path, "/sys/class/tty/%s/device/../idVendor", e->d_name);
        if (!ReadSmall(path, vid, sizeof vid))
        {
            snprintf(path, sizeof path, "/sys/class/tty/%s/device/../../idVendor", e->d_name);
            ReadSmall(path, vid, sizeof vid);
        }
        if (strcmp(vid, "1a86") != 0)   // WCH, the CH340's maker
            continue;
        snprintf(name, size, "/dev/%s", e->d_name);
        found = 1;
    }
    closedir(d);
    return found;
}

#endif

// ---- the protocol

static int ReadLine(char *out, size_t size, int timeoutMs)
{
    size_t n = 0;
    for (;;)
    {
        int c = PortReadByte(timeoutMs);
        if (c < 0)
            return 0;
        if (c == '\n')
            break;
        if (c != '\r' && n + 1 < size)
            out[n++] = (char)c;
    }
    out[n] = 0;
    return 1;
}

static int IsAck(const char *line)
{
    return strstr(line, "ACK") && !strstr(line, "NACK");
}

static int WriteStr(const char *s)
{
    return PortWrite(s, strlen(s));
}

static int TryBaud(const char *port, int baud)
{
    char line[128];
    if (!PortOpen(port, baud))
        return 0;
    WriteStr("@HAND00");
    if (ReadLine(line, sizeof line, TIMEOUT_MS) && IsAck(line))
        return 1;
    PortClose();
    return 0;
}

int Vm2_Open(const char *port)
{
    static const int bauds[] = {460800, 921600};
    char line[128];
    int i;
    Vm2_Close();
    for (i = 0; i < 2; i++)
    {
        if (!TryBaud(port, bauds[i]))
            continue;
        sInfo[0] = 0;
        WriteStr("@INFO00");
        if (ReadLine(line, sizeof line, TIMEOUT_MS))
        {
            const char *sp = strchr(line, ' ');   // "<word> <HW> <FW> <ID>": the words after the first
            snprintf(sInfo, sizeof sInfo, "%s", sp ? sp + 1 : "");
        }
        return 1;
    }
    return 0;
}

void Vm2_Close(void)
{
    PortClose();
}

int Vm2_IsOpen(void)
{
    return PortIsOpen();
}

const char *Vm2_Info(void)
{
    return sInfo;
}

static int Handshake(void)
{
    char line[128];
    PortFlushInput();
    WriteStr("@HAND00");
    return ReadLine(line, sizeof line, TIMEOUT_MS) && IsAck(line);
}

int Vm2_ReadInternal(uint8_t *image)
{
    int blk, i;
    if (!PortIsOpen() || !Handshake())
        return 0;
    for (blk = 0; blk < 256; blk++)
    {
        uint8_t n = (uint8_t)blk;
        WriteStr("@RMEM1");
        PortWrite(&n, 1);
        for (i = 0; i < VM2_BLOCK_SIZE; i++)
        {
            int c = PortReadByte(TIMEOUT_MS * 4);
            if (c < 0)
                return 0;
            image[blk * VM2_BLOCK_SIZE + i] = (uint8_t)c;
        }
    }
    return 1;
}

int Vm2_WriteBlock(int block, const uint8_t *data)
{
    uint8_t x = 0, n = (uint8_t)block;
    char line[128];
    int i, tries;
    if (!PortIsOpen() || !Handshake())
        return 0;
    for (i = 0; i < VM2_BLOCK_SIZE; i++)
        x ^= data[i];
    for (tries = 0; tries < 5; tries++)
    {
        WriteStr("@WMEM1");
        PortWrite(&n, 1);
        PortWrite(data, VM2_BLOCK_SIZE);
        PortWrite(&x, 1);
        if (ReadLine(line, sizeof line, TIMEOUT_MS * 4) && IsAck(line))
            return 1;
    }
    return 0;
}

void Vm2_Restart(void)
{
    if (PortIsOpen() && Handshake())
        WriteStr("@REST00");
    PortClose();
}
