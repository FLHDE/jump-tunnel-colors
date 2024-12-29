#include "utils.h"
#include "Common.h"
#include "Freelancer.h"
#include <map>

UINT gateTunnelBretoniaId;
DWORD systemSwitchOutOg;
bool switchColorsWhileJumping = true;

FlColor defaultTunnel = DEFAULT_FL_COLOR;
std::map<UINT, FlColor> tunnelMap;

void UpdateGateTunnel(UINT systemId)
{
    static UINT lastSystemId = NULL;

    // If the system hasn't changed, then there's no point in changing the color.
    if (systemId == lastSystemId)
        return;

    GateTunnel* gateTunnel = GetGateTunnel(&gateTunnelBretoniaId);

    // Also if the gate tunnel cannot be found, its color can't be changed either.
    if (!gateTunnel)
        return;

    lastSystemId = systemId;
    FlColor* systemTunnel = &defaultTunnel;

    std::map<UINT, FlColor>::iterator it = tunnelMap.find(systemId);

    // Try to find the tunnel color associated with this system.
    // If it can't be found, use the default color.
    if (it != tunnelMap.end())
        systemTunnel = &it->second;

    // Set the color.
    gateTunnel->jumptube5Color = *systemTunnel;
}

// This hook gets called each time FL changes the current system.
Universe::ISystem const * get_system_Hook(UINT systemId)
{
    UpdateGateTunnel(systemId);

    // Call the original function.
    return Universe::get_system(systemId);
}

// This hook gets called each time a player jumps to a different system using a jumpgate or jumphole.
void Client::SystemSwitchOut_Hook(DWORD unk1, DWORD unk2)
{
    UpdateGateTunnel(CURRENT_SYSTEM_ID);

    // Call the original function.
    SystemSwitchOut initElementsFunc = GetFuncDef<SystemSwitchOut>(systemSwitchOutOg);
    (this->*initElementsFunc)(unk1, unk2);
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
        if (reader.is_header("DefaultTunnel"))
        {
            while (reader.read_value())
            {
                if (reader.is_value("switch_colors_while_jumping"))
                {
                    switchColorsWhileJumping = reader.get_value_bool(0);
                }

                if (reader.is_value("color"))
                {
                    defaultTunnel.r = ByteColorToFloat(reader.get_value_int(0));
                    defaultTunnel.g = ByteColorToFloat(reader.get_value_int(1));
                    defaultTunnel.b = ByteColorToFloat(reader.get_value_int(2));
                }
            }
        }

        if (reader.is_header("Tunnel"))
        {
            while (reader.read_value())
            {
                if (reader.is_value("color"))
                {
                    tunnel.r = ByteColorToFloat(reader.get_value_int(0));
                    tunnel.g = ByteColorToFloat(reader.get_value_int(1));
                    tunnel.b = ByteColorToFloat(reader.get_value_int(2));
                }

                if (reader.is_value("system"))
                {
                    // Insert the color for the system.
                    // Hopefully the color has been set correctly.
                    UINT systemId = CreateID(reader.get_value_string(0));
                    tunnelMap.insert(std::pair<UINT, FlColor>(systemId, tunnel));
                }
            }
        }
    }
}

void Init()
{
    #define UPDATE_SYS_GETSYS_CALL_ADDR 0x4C484F
    #define SYSTEM_SWITCH_OUT_ADDR 0x5E6758

    gateTunnelBretoniaId = CreateID("gate_tunnel_bretonia");

    // Allows us to prevent the spew warning from occurring when the gate tunnel cannot be found.
    DWORD _;
    VirtualProtect((PVOID) GET_GATE_TUNNEL_NOT_FOUND, sizeof(WORD), PAGE_EXECUTE_READWRITE, &_);

    ParseTunnelColors();

    if (switchColorsWhileJumping)
        Hook(UPDATE_SYS_GETSYS_CALL_ADDR, get_system_Hook, 6);
    systemSwitchOutOg = SetPointer(SYSTEM_SWITCH_OUT_ADDR, &Client::SystemSwitchOut_Hook);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpReserved);

    if (fdwReason == DLL_PROCESS_ATTACH)
        Init();

    return TRUE;
}
