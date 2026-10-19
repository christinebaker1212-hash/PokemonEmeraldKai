// Moemon Co-op: opening the host's router to the internet automatically.
//
// When a player hosts on their own PC, this asks the router to forward the
// game's TCP port to this PC, using UPnP (most home routers) or NAT-PMP
// (Apple and some others). The router reports the home's public address,
// which becomes a short invite code friends anywhere can type. The mapping
// is removed when the server stops. It's also requested with a lease, and
// recorded in moemon_portmap.txt, so a crash can't leave the port open for
// long: the lease runs out, and the next launch removes a leftover mapping.
//
// Everything runs on a background thread so the game never stalls.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mp_sock.h"
#ifdef _WIN32
#include <windows.h>
#include <iphlpapi.h>
#else
#include <pthread.h>
#include <time.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif

#define PM_LEASE_SECONDS   7200
#define PM_RENEW_SECONDS   1800
#define PM_PORT_TRIES      8       // the invite code can carry 8 different ports
#define PM_RECORD_FILE     "moemon_portmap.txt"
#define PM_DESCRIPTION     "Moemon Co-op"
#define HTTP_BUF           16384

// ---------------------------------------------------------------------------
// Small portability helpers

static void SleepMs(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts = {ms / 1000, (ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
#endif
}

static void NetStartup(void)
{
#ifdef _WIN32
    static int done;
    if (!done)
    {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        done = 1;
    }
#endif
}

static void SetTimeouts(mp_sock s, int ms)
{
#ifdef _WIN32
    DWORD t = ms;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&t, sizeof(t));
#else
    struct timeval tv = {ms / 1000, (ms % 1000) * 1000};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

static int ParseIpv4(const char *s, uint32_t *out)
{
    unsigned a, b, c, d;
    char tail;
    if (sscanf(s, "%u.%u.%u.%u%c", &a, &b, &c, &d, &tail) != 4 || a > 255 || b > 255 || c > 255 || d > 255)
        return 0;
    *out = (a << 24) | (b << 16) | (c << 8) | d;
    return 1;
}

static void Ipv4ToString(uint32_t ip, char *out, int len)
{
    snprintf(out, len, "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
}

// Private, shared (carrier-grade NAT) or otherwise not reachable from the internet
static int IsNonPublic(uint32_t ip)
{
    return (ip >> 24) == 10 || (ip >> 24) == 127 || (ip >> 24) == 0
        || (ip >> 20) == (172u << 4 | 1)          // 172.16.0.0/12
        || (ip >> 16) == (192u << 8 | 168)        // 192.168.0.0/16
        || (ip >> 22) == (100u << 2 | 1)          // 100.64.0.0/10 (CGNAT)
        || (ip >> 16) == (169u << 8 | 254);       // link local
}

// ---------------------------------------------------------------------------
// Invite codes: the public IPv4 address plus which of the 8 game ports,
// 7 Crockford base-32 characters and a check character: "K7QM-3XPA"

static const char kAlphabet[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

void MPNet_MakeInviteCode(uint32_t ip, int port, char *out, int len)
{
    uint64_t v = ((uint64_t)((port - MP_DEFAULT_PORT) & 7) << 32) | ip;
    char c[8];
    int i, sum = 0;
    for (i = 6; i >= 0; i--)
    {
        c[i] = kAlphabet[v & 31];
        v >>= 5;
    }
    for (i = 0; i < 7; i++)
        sum += (int)(strchr(kAlphabet, c[i]) - kAlphabet) * (i + 3);
    c[7] = kAlphabet[sum % 31];
    snprintf(out, len, "%.4s-%.4s", c, c + 4);
}

static int CodeValue(char ch)
{
    const char *p;
    ch = (char)toupper((unsigned char)ch);
    if (ch == 'O') ch = '0';
    if (ch == 'I' || ch == 'L') ch = '1';
    if (ch == 'U') ch = 'V';
    p = strchr(kAlphabet, ch);
    return (p && ch) ? (int)(p - kAlphabet) : -1;
}

// TRUE when the code is valid; fills the address and port
int MPNet_ParseInviteCode(const char *code, uint32_t *ip, int *port)
{
    int vals[8], n = 0, i, sum = 0;
    uint64_t v = 0;
    for (; *code; code++)
    {
        int x;
        if (*code == '-' || *code == ' ')
            continue;
        x = CodeValue(*code);
        if (x < 0 || n >= 8)
            return 0;
        vals[n++] = x;
    }
    if (n != 8)
        return 0;
    for (i = 0; i < 7; i++)
    {
        v = (v << 5) | (uint64_t)vals[i];
        sum += vals[i] * (i + 3);
    }
    if (vals[7] != sum % 31)
        return 0;
    *ip = (uint32_t)v;
    *port = MP_DEFAULT_PORT + (int)((v >> 32) & 7);
    return *ip != 0;
}

// ---------------------------------------------------------------------------
// VPN adapters (last resort): Radmin VPN uses 26.x.x.x, Hamachi 25.x.x.x

int MPNet_FindVpnAddress(char *out, int len, char *nameOut, int nameLen)
{
#ifdef _WIN32
    ULONG size = 32 * 1024;
    IP_ADAPTER_ADDRESSES *list;
    int found = 0;
    NetStartup();
    list = (IP_ADAPTER_ADDRESSES *)malloc(size);
    if (list && GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, list, &size) == NO_ERROR)
    {
        IP_ADAPTER_ADDRESSES *a;
        for (a = list; a && !found; a = a->Next)
        {
            IP_ADAPTER_UNICAST_ADDRESS *u;
            if (a->OperStatus != IfOperStatusUp)
                continue;
            for (u = a->FirstUnicastAddress; u && !found; u = u->Next)
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)u->Address.lpSockaddr;
                uint32_t ip;
                if (sin->sin_family != AF_INET)
                    continue;
                ip = ntohl(sin->sin_addr.s_addr);
                if ((ip >> 24) == 26 || (ip >> 24) == 25)
                {
                    Ipv4ToString(ip, out, len);
                    snprintf(nameOut, nameLen, "%s", (ip >> 24) == 26 ? "Radmin VPN" : "Hamachi");
                    found = 1;
                }
            }
        }
    }
    free(list);
    return found;
#else
    struct ifaddrs *ifs = NULL, *it;
    int found = 0;
    if (getifaddrs(&ifs) != 0)
        return 0;
    for (it = ifs; it && !found; it = it->ifa_next)
    {
        uint32_t ip;
        if (!it->ifa_addr || it->ifa_addr->sa_family != AF_INET || !(it->ifa_flags & IFF_UP))
            continue;
        ip = ntohl(((struct sockaddr_in *)it->ifa_addr)->sin_addr.s_addr);
        if ((ip >> 24) == 26 || (ip >> 24) == 25)
        {
            Ipv4ToString(ip, out, len);
            snprintf(nameOut, nameLen, "%s", (ip >> 24) == 26 ? "Radmin VPN" : "Hamachi");
            found = 1;
        }
    }
    freeifaddrs(ifs);
    return found;
#endif
}

// ---------------------------------------------------------------------------
// HTTP (tiny, blocking; runs on the worker thread)

struct Url
{
    char host[64];
    int port;
    char path[256];
};

static int ParseUrl(const char *s, struct Url *u)
{
    const char *p, *slash, *colon;
    if (strncmp(s, "http://", 7) != 0)
        return 0;
    p = s + 7;
    slash = strchr(p, '/');
    if (!slash)
        slash = p + strlen(p);
    colon = memchr(p, ':', slash - p);
    if (colon)
    {
        snprintf(u->host, sizeof(u->host), "%.*s", (int)(colon - p), p);
        u->port = atoi(colon + 1);
    }
    else
    {
        snprintf(u->host, sizeof(u->host), "%.*s", (int)(slash - p), p);
        u->port = 80;
    }
    snprintf(u->path, sizeof(u->path), "%s", *slash ? slash : "/");
    return u->host[0] != 0;
}

// Sends a request and reads the whole reply. Returns the HTTP status (0 = failed).
// localIp (optional) receives this PC's address on the route to the host.
static int HttpExchange(const char *host, int port, const char *request, int reqLen,
                        char *reply, int replyCap, uint32_t *localIp)
{
    struct sockaddr_in sa;
    uint32_t ip;
    mp_sock s;
    int got = 0, n, status = 0;

    if (!ParseIpv4(host, &ip))
    {
        struct hostent *he = gethostbyname(host);
        if (!he || he->h_addrtype != AF_INET)
            return 0;
        memcpy(&sa.sin_addr, he->h_addr_list[0], 4);
        ip = ntohl(sa.sin_addr.s_addr);
    }
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == MP_BADSOCK)
        return 0;
    SetTimeouts(s, 3000);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)port);
    sa.sin_addr.s_addr = htonl(ip);
    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) != 0)
    {
        mp_closesock(s);
        return 0;
    }
    if (localIp)
    {
        struct sockaddr_in me;
        mp_socklen ml = sizeof(me);
        if (getsockname(s, (struct sockaddr *)&me, &ml) == 0)
            *localIp = ntohl(me.sin_addr.s_addr);
    }
    if (send(s, request, reqLen, 0) != reqLen)
    {
        mp_closesock(s);
        return 0;
    }
    while (got < replyCap - 1 && (n = recv(s, reply + got, replyCap - 1 - got, 0)) > 0)
        got += n;
    reply[got] = 0;
    mp_closesock(s);
    if (sscanf(reply, "HTTP/%*d.%*d %d", &status) != 1)
        status = 0;
    return status;
}

// Case-insensitive search
static const char *FindNoCase(const char *hay, const char *needle)
{
    size_t n = strlen(needle);
    for (; *hay; hay++)
    {
        size_t i;
        for (i = 0; i < n && hay[i] && tolower((unsigned char)hay[i]) == tolower((unsigned char)needle[i]); i++)
            ;
        if (i == n)
            return hay;
    }
    return NULL;
}

// Text between <tag> and </tag> (first match at or after start)
static int XmlValue(const char *start, const char *tag, char *out, int len)
{
    char open[64], close[64];
    const char *a, *b;
    snprintf(open, sizeof(open), "<%s>", tag);
    snprintf(close, sizeof(close), "</%s>", tag);
    a = FindNoCase(start, open);
    if (!a)
        return 0;
    a += strlen(open);
    b = FindNoCase(a, close);
    if (!b)
        return 0;
    while (a < b && isspace((unsigned char)*a))
        a++;
    while (b > a && isspace((unsigned char)b[-1]))
        b--;
    snprintf(out, len, "%.*s", (int)(b - a), a);
    return 1;
}

// ---------------------------------------------------------------------------
// UPnP Internet Gateway Device

struct Igd
{
    struct Url control;
    char service[128];
    uint32_t localIp;
};

static int SsdpDiscover(char locations[][256], int max)
{
    static const char *const targets[] = {
        "urn:schemas-upnp-org:device:InternetGatewayDevice:1",
        "urn:schemas-upnp-org:device:InternetGatewayDevice:2",
        "urn:schemas-upnp-org:service:WANIPConnection:1",
        "urn:schemas-upnp-org:service:WANPPPConnection:1",
    };
    struct sockaddr_in sa;
    mp_sock s;
    int count = 0, round, t;
    char buf[2048];

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == MP_BADSOCK)
        return 0;
    SetTimeouts(s, 400);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1900);
    sa.sin_addr.s_addr = inet_addr("239.255.255.250");
    for (round = 0; round < 2 && count == 0; round++)
    {
        int waited;
        for (t = 0; t < (int)(sizeof(targets) / sizeof(targets[0])); t++)
        {
            int len = snprintf(buf, sizeof(buf),
                "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\nST: %s\r\n\r\n", targets[t]);
            sendto(s, buf, len, 0, (struct sockaddr *)&sa, sizeof(sa));
        }
        // Collect answers for about 2.4 seconds
        for (waited = 0; waited < 6; waited++)
        {
            int n;
            while ((n = recv(s, buf, sizeof(buf) - 1, 0)) > 0)
            {
                const char *loc;
                buf[n] = 0;
                loc = FindNoCase(buf, "\nLOCATION:");
                if (loc)
                {
                    char url[256];
                    int i, dup = 0;
                    loc += 10;
                    while (*loc == ' ')
                        loc++;
                    for (i = 0; loc[i] && loc[i] != '\r' && loc[i] != '\n' && i < 255; i++)
                        url[i] = loc[i];
                    url[i] = 0;
                    for (i = 0; i < count; i++)
                        dup |= strcmp(locations[i], url) == 0;
                    if (!dup && count < max)
                        snprintf(locations[count++], 256, "%s", url);
                }
            }
            if (count > 0 && waited >= 2)
                break;
        }
    }
    mp_closesock(s);
    return count;
}

static int LoadIgd(const char *location, struct Igd *igd)
{
    struct Url loc, base;
    char *reply = malloc(HTTP_BUF), req[512], urlBase[256], ctrl[200];
    const char *p, *body;
    int status, found = 0;

    if (!reply || !ParseUrl(location, &loc))
    {
        free(reply);
        return 0;
    }
    snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", loc.path, loc.host, loc.port);
    status = HttpExchange(loc.host, loc.port, req, (int)strlen(req), reply, HTTP_BUF, &igd->localIp);
    if (status != 200 || !(body = strstr(reply, "\r\n\r\n")))
    {
        free(reply);
        return 0;
    }
    base = loc;
    if (XmlValue(body, "URLBase", urlBase, sizeof(urlBase)) && ParseUrl(urlBase, &base))
        ;
    // Find the WAN connection service and its control URL
    for (p = body; (p = FindNoCase(p, "<service>")) != NULL; p++)
    {
        char type[128];
        const char *end = FindNoCase(p, "</service>");
        if (!end)
            break;
        if (!XmlValue(p, "serviceType", type, sizeof(type)))
            continue;
        if (!(strstr(type, "WANIPConnection:") || strstr(type, "WANPPPConnection:")))
            continue;
        if (!XmlValue(p, "controlURL", ctrl, sizeof(ctrl)) || FindNoCase(p, "<controlURL>") > end)
            continue;
        snprintf(igd->service, sizeof(igd->service), "%s", type);
        if (strncmp(ctrl, "http://", 7) == 0)
            ParseUrl(ctrl, &igd->control);
        else
        {
            igd->control = base;
            snprintf(igd->control.path, sizeof(igd->control.path), "%s%s", ctrl[0] == '/' ? "" : "/", ctrl);
        }
        found = 1;
        break;
    }
    free(reply);
    return found;
}

// Calls a UPnP action. Returns the HTTP status; *errorCode gets the UPnP error (or 0).
static int SoapCall(const struct Igd *igd, const char *action, const char *args, char *reply, int replyCap, int *errorCode)
{
    char body[2048], *req;
    int bodyLen, reqLen, status;
    char code[16];

    bodyLen = snprintf(body, sizeof(body),
        "<?xml version=\"1.0\"?>\r\n"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<s:Body><u:%s xmlns:u=\"%s\">%s</u:%s></s:Body></s:Envelope>\r\n",
        action, igd->service, args, action);
    req = malloc(bodyLen + 1024);
    if (!req)
        return 0;
    reqLen = snprintf(req, bodyLen + 1024,
        "POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: text/xml; charset=\"utf-8\"\r\n"
        "SOAPAction: \"%s#%s\"\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
        igd->control.path, igd->control.host, igd->control.port, igd->service, action, bodyLen, body);
    status = HttpExchange(igd->control.host, igd->control.port, req, reqLen, reply, replyCap, NULL);
    free(req);
    *errorCode = 0;
    if (status != 200 && XmlValue(reply, "errorCode", code, sizeof(code)))
        *errorCode = atoi(code);
    return status;
}

static int UpnpExternalIp(const struct Igd *igd, uint32_t *ip)
{
    char *reply = malloc(HTTP_BUF), addr[64];
    int err, ok = 0;
    if (!reply)
        return 0;
    if (SoapCall(igd, "GetExternalIPAddress", "", reply, HTTP_BUF, &err) == 200
     && XmlValue(reply, "NewExternalIPAddress", addr, sizeof(addr)))
        ok = ParseIpv4(addr, ip);
    free(reply);
    return ok;
}

// Returns 1 on success, 0 = refused, -1 = this port is taken by another PC
static int UpnpAdd(const struct Igd *igd, int extPort, int intPort, int *lease)
{
    char *reply = malloc(HTTP_BUF), args[1024], lan[32];
    int err, status, attempt;
    if (!reply)
        return 0;
    Ipv4ToString(igd->localIp, lan, sizeof(lan));
    for (attempt = 0; attempt < 2; attempt++)
    {
        snprintf(args, sizeof(args),
            "<NewRemoteHost></NewRemoteHost><NewExternalPort>%d</NewExternalPort><NewProtocol>TCP</NewProtocol>"
            "<NewInternalPort>%d</NewInternalPort><NewInternalClient>%s</NewInternalClient><NewEnabled>1</NewEnabled>"
            "<NewPortMappingDescription>" PM_DESCRIPTION "</NewPortMappingDescription><NewLeaseDuration>%d</NewLeaseDuration>",
            extPort, intPort, lan, *lease);
        status = SoapCall(igd, "AddPortMapping", args, reply, HTTP_BUF, &err);
        if (status == 200)
        {
            free(reply);
            return 1;
        }
        if (err == 725 && *lease != 0) // OnlyPermanentLeasesSupported
        {
            *lease = 0;
            continue;
        }
        break;
    }
    free(reply);
    MPNet_Log("UPnP refused port %d (error %d)", extPort, err);
    return (err == 718 || err == 724) ? -1 : 0; // ConflictInMappingEntry / SamePortValuesRequired
}

static void UpnpDelete(const struct Igd *igd, int extPort)
{
    char *reply = malloc(HTTP_BUF), args[256];
    int err;
    if (!reply)
        return;
    snprintf(args, sizeof(args), "<NewRemoteHost></NewRemoteHost><NewExternalPort>%d</NewExternalPort><NewProtocol>TCP</NewProtocol>", extPort);
    SoapCall(igd, "DeletePortMapping", args, reply, HTTP_BUF, &err);
    free(reply);
}

// ---------------------------------------------------------------------------
// NAT-PMP (RFC 6886)

static int DefaultGateway(uint32_t *gw)
{
#ifdef _WIN32
    MIB_IPFORWARDROW row;
    if (GetBestRoute(inet_addr("8.8.8.8"), 0, &row) != NO_ERROR)
        return 0;
    *gw = ntohl(row.dwForwardNextHop);
    return *gw != 0;
#else
    FILE *f = fopen("/proc/net/route", "r");
    char line[256];
    int ok = 0;
    if (!f)
        return 0;
    while (fgets(line, sizeof(line), f))
    {
        char iface[32];
        unsigned dest, gateway, flags;
        if (sscanf(line, "%31s %x %x %x", iface, &dest, &gateway, &flags) == 4 && dest == 0 && gateway != 0)
        {
            *gw = ntohl(gateway);
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
#endif
}

static int PmpRequest(uint32_t gw, const uint8_t *req, int reqLen, uint8_t *resp, int respCap)
{
    struct sockaddr_in sa;
    mp_sock s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int timeout = 250, tries, n = -1;
    if (s == MP_BADSOCK)
        return -1;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(5351);
    sa.sin_addr.s_addr = htonl(gw);
    for (tries = 0; tries < 4; tries++, timeout *= 2)
    {
        SetTimeouts(s, timeout);
        sendto(s, (const char *)req, reqLen, 0, (struct sockaddr *)&sa, sizeof(sa));
        n = recv(s, (char *)resp, respCap, 0);
        if (n > 0)
            break;
    }
    mp_closesock(s);
    return n;
}

static int PmpExternalIp(uint32_t gw, uint32_t *ip)
{
    uint8_t req[2] = {0, 0}, resp[16];
    int n = PmpRequest(gw, req, 2, resp, sizeof(resp));
    if (n < 12 || resp[1] != 128 || resp[2] || resp[3])
        return 0;
    *ip = ((uint32_t)resp[8] << 24) | (resp[9] << 16) | (resp[10] << 8) | resp[11];
    return 1;
}

// Returns the external port given, or 0
static int PmpMap(uint32_t gw, int intPort, int extPort, int lifetime)
{
    uint8_t req[12] = {0, 2, 0, 0}, resp[16];
    int n;
    req[4] = intPort >> 8; req[5] = intPort & 255;
    req[6] = extPort >> 8; req[7] = extPort & 255;
    req[8] = lifetime >> 24; req[9] = (lifetime >> 16) & 255; req[10] = (lifetime >> 8) & 255; req[11] = lifetime & 255;
    n = PmpRequest(gw, req, 12, resp, sizeof(resp));
    if (n < 16 || resp[1] != 130 || resp[2] || resp[3])
        return 0;
    return lifetime == 0 ? 1 : (resp[10] << 8) | resp[11];
}

// ---------------------------------------------------------------------------
// Public address check (catches double NAT / carrier-grade NAT the router
// can't see): ask a plain-HTTP "what is my IP" service. Optional.

static int AskPublicIp(uint32_t *ip)
{
    char *reply = malloc(4096), *body;
    const char *req = "GET / HTTP/1.1\r\nHost: api.ipify.org\r\nConnection: close\r\n\r\n";
    int ok = 0;
    if (!reply)
        return 0;
    if (HttpExchange("api.ipify.org", 80, req, (int)strlen(req), reply, 4096, NULL) == 200
     && (body = strstr(reply, "\r\n\r\n")) != NULL)
    {
        char addr[32];
        sscanf(body + 4, "%31s", addr);
        ok = ParseIpv4(addr, ip);
    }
    free(reply);
    return ok;
}

// ---------------------------------------------------------------------------
// Worker

static volatile int sState = MP_PM_IDLE;
static volatile int sError;
static volatile int sStop;
static int sInternalPort, sExternalPort, sLease, sMethod;
static uint32_t sPublicIp;
static struct Igd sIgd;
static uint32_t sGateway;
static char sCode[16];
static char sPublicText[32];
static int sAtExit;
#ifdef _WIN32
static HANDLE sThread;
#else
static pthread_t sThread;
static int sThreadValid;
#endif

static void WriteRecord(void)
{
    FILE *f = fopen(PM_RECORD_FILE, "w");
    if (f)
    {
        fprintf(f, "%d %d\n", sMethod, sExternalPort);
        fclose(f);
    }
}

static void RemoveMapping(void)
{
    if (sMethod == MP_PM_UPNP)
        UpnpDelete(&sIgd, sExternalPort);
    else if (sMethod == MP_PM_NATPMP)
        PmpMap(sGateway, sInternalPort, sExternalPort, 0);
    remove(PM_RECORD_FILE);
    MPNet_Log("closed port %d on the router", sExternalPort);
}

static int TryUpnp(void)
{
    char locations[6][256];
    int n = SsdpDiscover(locations, 6), i, p;
    for (i = 0; i < n; i++)
    {
        int lease = PM_LEASE_SECONDS, refusedAll = 1;
        memset(&sIgd, 0, sizeof(sIgd));
        if (!LoadIgd(locations[i], &sIgd))
            continue;
        MPNet_Log("UPnP router at %s:%d (%s)", sIgd.control.host, sIgd.control.port, sIgd.service);
        // A mapping left over from a crash
        {
            FILE *f = fopen(PM_RECORD_FILE, "r");
            int m, old;
            if (f)
            {
                if (fscanf(f, "%d %d", &m, &old) == 2 && m == MP_PM_UPNP)
                    UpnpDelete(&sIgd, old);
                fclose(f);
            }
        }
        for (p = 0; p < PM_PORT_TRIES; p++)
        {
            int r = UpnpAdd(&sIgd, MP_DEFAULT_PORT + p, sInternalPort, &lease);
            if (r == 1)
            {
                sExternalPort = MP_DEFAULT_PORT + p;
                sLease = lease;
                sMethod = MP_PM_UPNP;
                if (!UpnpExternalIp(&sIgd, &sPublicIp))
                    sPublicIp = 0;
                return 1;
            }
            if (r == 0)
                break;
            refusedAll = 0;
        }
        if (refusedAll)
            sError = MP_PM_ERR_REFUSED;
    }
    return 0;
}

static int TryNatPmp(void)
{
    int p;
    if (!DefaultGateway(&sGateway))
        return 0;
    if (!PmpExternalIp(sGateway, &sPublicIp))
        return 0;
    MPNet_Log("NAT-PMP router at %u.%u.%u.%u", sGateway >> 24, (sGateway >> 16) & 255, (sGateway >> 8) & 255, sGateway & 255);
    for (p = 0; p < PM_PORT_TRIES; p++)
    {
        int ext = PmpMap(sGateway, sInternalPort, MP_DEFAULT_PORT + p, PM_LEASE_SECONDS);
        if (ext >= MP_DEFAULT_PORT && ext < MP_DEFAULT_PORT + PM_PORT_TRIES)
        {
            sExternalPort = ext;
            sLease = PM_LEASE_SECONDS;
            sMethod = MP_PM_NATPMP;
            return 1;
        }
        if (ext)
            PmpMap(sGateway, sInternalPort, ext, 0); // a port the invite code can't carry
    }
    sError = MP_PM_ERR_REFUSED;
    return 0;
}

static void Renew(void)
{
    if (sMethod == MP_PM_UPNP && sLease > 0)
    {
        int lease = sLease;
        UpnpAdd(&sIgd, sExternalPort, sInternalPort, &lease);
    }
    else if (sMethod == MP_PM_NATPMP)
        PmpMap(sGateway, sInternalPort, sExternalPort, PM_LEASE_SECONDS);
}

static void Work(void)
{
    uint32_t seen;
    int waited;

    NetStartup();
    sError = MP_PM_ERR_NO_ROUTER;
    sMethod = MP_PM_NONE;
    if (!TryUpnp() && (sStop || !TryNatPmp()))
    {
        sState = sStop ? MP_PM_IDLE : MP_PM_FAILED;
        return;
    }
    WriteRecord();
    MPNet_Log("opened port %d -> %d on the router (%s, lease %ds)", sExternalPort, sInternalPort,
              sMethod == MP_PM_UPNP ? "UPnP" : "NAT-PMP", sLease);

    // The router's own public address must really be public
    if (sPublicIp == 0 || IsNonPublic(sPublicIp))
    {
        if (AskPublicIp(&seen) && !IsNonPublic(seen) && sPublicIp == 0)
            sPublicIp = seen;
        else
        {
            MPNet_Log("the router's internet address isn't public: carrier-grade or double NAT");
            RemoveMapping();
            sMethod = MP_PM_NONE;
            sError = MP_PM_ERR_CGNAT;
            sState = MP_PM_FAILED;
            return;
        }
    }
    else if (!getenv("MOEMON_NO_IPCHECK") && AskPublicIp(&seen) && seen != sPublicIp)
    {
        // Web traffic leaves through another address (a VPN, or a provider
        // with several exits). The router's address is still the one friends need.
        MPNet_Log("note: web traffic leaves through %u.%u.%u.%u, the router says %u.%u.%u.%u",
                  seen >> 24, (seen >> 16) & 255, (seen >> 8) & 255, seen & 255,
                  sPublicIp >> 24, (sPublicIp >> 16) & 255, (sPublicIp >> 8) & 255, sPublicIp & 255);
    }
    Ipv4ToString(sPublicIp, sPublicText, sizeof(sPublicText));
    MPNet_MakeInviteCode(sPublicIp, sExternalPort, sCode, sizeof(sCode));
    sState = MP_PM_OPEN;

    // Keep the mapping alive until the server stops
    for (waited = 0; !sStop; waited++)
    {
        SleepMs(1000);
        if (waited >= PM_RENEW_SECONDS)
        {
            waited = 0;
            Renew();
        }
    }
    RemoveMapping();
    sState = MP_PM_IDLE;
}

#ifdef _WIN32
static DWORD WINAPI ThreadMain(LPVOID arg)
{
    (void)arg;
    Work();
    return 0;
}
#else
static void *ThreadMain(void *arg)
{
    (void)arg;
    Work();
    return NULL;
}
#endif

static void JoinThread(void)
{
#ifdef _WIN32
    if (sThread)
    {
        WaitForSingleObject(sThread, 8000);
        CloseHandle(sThread);
        sThread = NULL;
    }
#else
    if (sThreadValid)
    {
        pthread_join(sThread, NULL);
        sThreadValid = 0;
    }
#endif
}

static void AtExitClose(void)
{
    MPPortMap_Close();
}

void MPPortMap_Open(int internalPort)
{
    if (sState == MP_PM_WORKING || sState == MP_PM_OPEN)
        return;
    JoinThread();
    sInternalPort = internalPort;
    sStop = 0;
    sCode[0] = 0;
    sPublicText[0] = 0;
    sState = MP_PM_WORKING;
    if (!sAtExit)
    {
        atexit(AtExitClose);
        sAtExit = 1;
    }
#ifdef _WIN32
    sThread = CreateThread(NULL, 0, ThreadMain, NULL, 0, NULL);
    if (!sThread)
        sState = MP_PM_FAILED;
#else
    if (pthread_create(&sThread, NULL, ThreadMain, NULL) == 0)
        sThreadValid = 1;
    else
        sState = MP_PM_FAILED;
#endif
}

// Removes the mapping (waits a moment for the router to answer)
void MPPortMap_Close(void)
{
    if (sState == MP_PM_IDLE)
        return;
    sStop = 1;
    JoinThread();
    sState = MP_PM_IDLE;
}

int MPPortMap_State(void) { return sState; }
int MPPortMap_Error(void) { return sError; }
int MPPortMap_Method(void) { return sMethod; }
const char *MPPortMap_InviteCode(void) { return sState == MP_PM_OPEN ? sCode : ""; }
const char *MPPortMap_PublicAddress(void) { return sState == MP_PM_OPEN ? sPublicText : ""; }
int MPPortMap_ExternalPort(void) { return sExternalPort; }
