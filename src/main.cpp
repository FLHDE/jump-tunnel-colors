#include "utils.h"
#include "Common.h"
#include "Freelancer.h"
#include <map>

UINT gateTunnelBretoniaId;

FlColor defaultTunnel = DEFAULT_FL_COLOR;
std::map<UINT, FlColor> tunnelMap;

// This hook gets called each time FL changes the current system.
Universe::ISystem const * get_system_Hook(UINT systemId)
{
    static UINT lastSystemId = NULL;

    // If the system hasn't changed, then there's no point in changing the color.
    if (systemId != lastSystemId)
    {
        lastSystemId = systemId;
        FlColor* systemTunnel = &defaultTunnel;

        std::map<UINT, FlColor>::iterator it = tunnelMap.find(systemId);

        // Try to find the tunnel color associated with this system.
        // If it can't be found, use the default color.
        if (it != tunnelMap.end())
            systemTunnel = &it->second;

        GateTunnel* gateTunnel = GetGateTunnel(&gateTunnelBretoniaId);

        // Set the color.
        if (gateTunnel)
            gateTunnel->jumptube5Color = *systemTunnel;
    }

    // Call the original function
    return Universe::get_system(systemId);
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
    #define UPDATE_SYS_GETSYS_CALL_ADDR_1 0x4C484F
    #define UPDATE_SYS_GETSYS_CALL_ADDR_2 0x4C4943

    gateTunnelBretoniaId = CreateID("gate_tunnel_bretonia");

    Hook(UPDATE_SYS_GETSYS_CALL_ADDR_1, get_system_Hook, 6);
    Hook(UPDATE_SYS_GETSYS_CALL_ADDR_2, get_system_Hook, 6);

    ParseTunnelColors();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpReserved);

    if (fdwReason == DLL_PROCESS_ATTACH)
        Init();

    return TRUE;
}
