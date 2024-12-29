#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

#define DEFAULT_FL_COLOR FlColor(100, 200, 255)
