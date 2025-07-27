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

    FlColor(float _r, float _g, float _b)
        : r(_r), g(_g), b(_b)
    {}

    FlColor Mix(const FlColor &other)
    {
        return FlColor(
            (r + other.r) / 2.0f,
            (g + other.g) / 2.0f,
            (b + other.b) / 2.0f
        );
    }

    float r, g, b;
};

struct GateTunnel
{
    BYTE x00[0x48];
    FlColor jumptube5Color;
};

#define DEFAULT_FL_COLOR FlColor((BYTE) 100, (BYTE) 200, (BYTE) 255)
#define CURRENT_SYSTEM_ID *((PUINT) 0x673354)
#define GET_GATE_TUNNEL_NOT_FOUND 0x04FEDF6

GateTunnel* GetGateTunnel_Original(PUINT jumpTunnelId)
{
    #define GET_GATE_TUNNEL_ADDR 0x4FEDD0
    typedef GateTunnel* GetGateTunnelFunc(PUINT);
    return ((GetGateTunnelFunc*) GET_GATE_TUNNEL_ADDR)(jumpTunnelId);
}

inline GateTunnel* GetGateTunnel(UINT jumpTunnelId)
{
    // FL generates a spew warning if the gate tunnel cannot be found, but in some cases this is inevitable for the current code setup.
    // Hence patch out the warning temporarily
    *((PWORD) GET_GATE_TUNNEL_NOT_FOUND) = 0x24EB;
    GateTunnel* result = GetGateTunnel_Original(&jumpTunnelId);
    *((PWORD) GET_GATE_TUNNEL_NOT_FOUND) = 0x02BE;

    return result;
}
