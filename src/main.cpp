#include "utils.h"
#include "Common.h"
#include "Freelancer.h"
#include <map>

typedef enum ColorPrecedence
{
    COLOR_PRECEDENCE_ARCH,
    COLOR_PRECEDENCE_SYS,
    COLOR_PRECEDENCE_MIX_ARCH,
    COLOR_PRECEDENCE_MIX_SYS,
} ColorPrecedence;

UINT gateTunnelBretoniaId;
UINT lastGateEnterArchId = NULL;
bool switchColorsWhileJumping = true;

FlColor defaultTunnel = DEFAULT_FL_COLOR;
std::map<UINT, FlColor> sysTunnelMap;
std::map<UINT, FlColor> archTunnelMap;

ColorPrecedence colorPrecedence = COLOR_PRECEDENCE_ARCH;

FlColor* FindFlColor(std::map<UINT, FlColor> &tunnelMap, UINT key)
{
    if (key == NULL)
        return NULL;

    std::map<UINT, FlColor>::iterator it = tunnelMap.find(key);

    if (it == tunnelMap.end())
        return NULL;

    return &it->second;
}

void UpdateGateTunnel(GateTunnel* gateTunnel)
{
    if (!gateTunnel)
        return;

    FlColor* sysColor = FindFlColor(sysTunnelMap, CURRENT_SYSTEM_ID);
    FlColor* archColor = FindFlColor(archTunnelMap, lastGateEnterArchId);

    if (colorPrecedence == COLOR_PRECEDENCE_SYS)
    {
        if (sysColor)
            gateTunnel->jumptube5Color = *sysColor;
        else if (archColor)
            gateTunnel->jumptube5Color = *archColor;
        else
            gateTunnel->jumptube5Color = defaultTunnel;
    }
    else if (colorPrecedence == COLOR_PRECEDENCE_ARCH)
    {
        if (archColor)
            gateTunnel->jumptube5Color = *archColor;
        else if (sysColor)
            gateTunnel->jumptube5Color = *sysColor;
        else
            gateTunnel->jumptube5Color = defaultTunnel;
    }
    else if (colorPrecedence == COLOR_PRECEDENCE_MIX_ARCH || colorPrecedence == COLOR_PRECEDENCE_MIX_SYS)
    {
        if (sysColor && archColor)
            gateTunnel->jumptube5Color = sysColor->Mix(*archColor);
        else if (archColor && colorPrecedence == COLOR_PRECEDENCE_MIX_ARCH)
            gateTunnel->jumptube5Color = *archColor;
        else if (sysColor && colorPrecedence == COLOR_PRECEDENCE_MIX_SYS)
            gateTunnel->jumptube5Color = *sysColor;
        else if (archColor)
            gateTunnel->jumptube5Color = *archColor;
        else if (sysColor)
            gateTunnel->jumptube5Color = *sysColor;
        else
            gateTunnel->jumptube5Color = defaultTunnel;
    }
}

GateTunnel* __stdcall GetGateTunnel_Custom(const CSolar& enteredGate, PUINT systemId)
{
    GateTunnel* gateTunnel = GetGateTunnel_Original(systemId);

    // Save the archId of the last gate that the player went through.
    lastGateEnterArchId = enteredGate.solarArch->solarArchId;

    UpdateGateTunnel(gateTunnel);

    return gateTunnel;

}

// This hook gets called each time a player jumps to a different system using a jumpgate or jumphole.
__declspec(naked) void GetGateTunnel_Hook()
{
    #define GET_GATE_TUNNEL_RET 0x503787

    __asm {
        push [esp+0x14] // CSolar (jumpgate/hole that has been docked with)
        call GetGateTunnel_Custom
        mov ecx, GET_GATE_TUNNEL_RET
        jmp ecx
    }
}

// This hook gets called each time FL changes the current system.
Universe::ISystem const * get_system_Hook(UINT systemId)
{
    UpdateGateTunnel(GetGateTunnel(gateTunnelBretoniaId));

    // Call the original function.
    return Universe::get_system(systemId);
}

void SetTunnelColor(FlColor& tunnel, INI_Reader& reader)
{
    tunnel.r = ByteColorToFloat(reader.get_value_int(0));
    tunnel.g = ByteColorToFloat(reader.get_value_int(1));
    tunnel.b = ByteColorToFloat(reader.get_value_int(2));
}

void ParseTunnelColors()
{
    INI_Reader reader;
    FlColor tunnel = defaultTunnel;

    if (!reader.open("..\\DATA\\FX\\jump_tunnel_colors.ini"))
        return;

    while (reader.read_header())
    {
        // Colors in the ini files are passed as ints from 0 to 255, but in-memory the colors are float values from 0 to 1.
        if (reader.is_header("Options"))
        {
            while (reader.read_value())
            {
                if (reader.is_value("switch_sys_colors_while_jumping"))
                {
                    switchColorsWhileJumping = reader.get_value_bool(0);
                }
                else if (reader.is_value("color_precedence"))
                {
                    colorPrecedence = (ColorPrecedence) reader.get_value_int(0);
                }
                else if (reader.is_value("default_color"))
                {
                    SetTunnelColor(defaultTunnel, reader);
                }
            }
        }

        if (reader.is_header("SysTunnel"))
        {
            while (reader.read_value())
            {
                if (reader.is_value("color"))
                {
                    SetTunnelColor(tunnel, reader);
                }
                else if (reader.is_value("system"))
                {
                    UINT numParameters = reader.get_num_parameters();
                    for (UINT i = 0; i < numParameters; ++i)
                    {
                        // Insert the color for the system.
                        // Hopefully the color has been set correctly.
                        UINT systemId = CreateID(reader.get_value_string(i));
                        sysTunnelMap.insert(std::pair<UINT, FlColor>(systemId, tunnel));
                    }
                }
            }
        }
        else if (reader.is_header("ArchTunnel"))
        {
            while (reader.read_value())
            {
                if (reader.is_value("color"))
                {
                    SetTunnelColor(tunnel, reader);
                }
                else if (reader.is_value("archetype"))
                {
                    UINT numParameters = reader.get_num_parameters();
                    for (UINT i = 0; i < numParameters; ++i)
                    {
                        // Insert the color for the archetype.
                        // Hopefully the color has been set correctly.
                        UINT archId = CreateID(reader.get_value_string(i));
                        archTunnelMap.insert(std::pair<UINT, FlColor>(archId, tunnel));
                    }
                }
            }
        }
    }
}

void Init()
{
    #define UPDATE_SYS_GETSYS_CALL_ADDR 0x4C484F
    #define JUMP_EFFECT_GET_GATE_TUNNEL_ADDR 0x50377F

    gateTunnelBretoniaId = CreateID("gate_tunnel_bretonia");

    // Allows us to prevent the spew warning from occurring when the gate tunnel cannot be found.
    DWORD _;
    VirtualProtect((PVOID) GET_GATE_TUNNEL_NOT_FOUND, sizeof(WORD), PAGE_EXECUTE_READWRITE, &_);

    ParseTunnelColors();

    if (switchColorsWhileJumping)
        Hook(UPDATE_SYS_GETSYS_CALL_ADDR, get_system_Hook, 6);
    Hook(JUMP_EFFECT_GET_GATE_TUNNEL_ADDR, GetGateTunnel_Hook, 8, true);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpReserved);

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);
        Init();
    }

    return TRUE;
}
