#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "vftable.h"
#include <list>

inline float ByteColorToFloat(BYTE byte)
{
    return static_cast<float>(byte) / 255.0f;
}

struct FlColor
{
    FlColor(BYTE rByte, BYTE gByte, BYTE bByte)
        : r(ByteColorToFloat(rByte)),
        g(ByteColorToFloat(gByte)),
        b(ByteColorToFloat(bByte))
    {}

    float r, g, b;
};

struct GateTunnel
{
    BYTE x00[0x48];
    FlColor jumptube5Color;
};

inline GateTunnel* GetGateTunnel(PUINT jumpTunnelId)
{
    #define GET_GATE_TUNNEL_ADDR 0x4FEDD0

    typedef GateTunnel* GetGateTunnelFunc(PUINT);
    return ((GetGateTunnelFunc*) GET_GATE_TUNNEL_ADDR)(jumpTunnelId);
}

struct Client
{
    void SystemSwitchOut_Hook(DWORD unk1, DWORD unk2);

private:
    typedef void (Client::*SystemSwitchOut)(DWORD unk1, DWORD unk2);
};

#define DEFAULT_FL_COLOR FlColor(100, 200, 255)
#define CURRENT_SYSTEM_ID *((PUINT) 0x673354)
#define GATE_TUNNELS_VECTOR_PTR ((std::list<GateTunnel>*) 0x674F9C)
