#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "vftable.h"

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

    FlColor()
        : r(0.0f),
        g(0.0f),
        b(0.0f)
    {}

    FlColor Mix(const FlColor &other)
    {
        FlColor result;
        result.r = (r + other.r) / 2.0f;
        result.g = (g + other.g) / 2.0f;
        result.b = (b + other.b) / 2.0f;

        return result;
    }

    float r, g, b;
};

struct GateTunnel
{
    BYTE x00[0x48];
    FlColor jumptube5Color;
};

#define DEFAULT_FL_COLOR FlColor(100, 200, 255)
#define CURRENT_SYSTEM_ID *((PUINT) 0x673354)
#define GET_GATE_TUNNEL_NOT_FOUND 0x04FEDF6

inline GateTunnel* GetGateTunnel(PUINT jumpTunnelId)
{
    // FL generates a spew warning if the gate tunnel cannot be found, but in some cases this is inevitable for the current code setup.
    // Hence patch out the warning temporarily
    *((PWORD) GET_GATE_TUNNEL_NOT_FOUND) = 0x24EB;
    GateTunnel* result = GetGateTunnel_Original(jumpTunnelId);

    *((PWORD) GET_GATE_TUNNEL_NOT_FOUND) = 0x02BE;

    return result;
}

GateTunnel* GetGateTunnel_Original(PUINT jumpTunnelId)
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
