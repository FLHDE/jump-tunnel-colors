# jump-tunnel-colors
Allows the jump tunnel colors to be modified per-system for the game Freelancer.

## Installation instructions
Download the latest `JumpTunnelColors.dll` from [Releases](https://github.com/FLHDE/jump-tunnel-colors/releases). Extract the `dll` to the `EXE` folder of your Freelancer installation. Next, open the `dacom.ini` file and append `JumpTunnelColors.dll` to the `[Libraries]` section.

## Usage instructions
Create a file named `jump_tunnel_colors.ini` in `DATA/FX`. To this new file, add content with the following format:
```ini
[DefaultTunnel]
color = 100, 200, 255

[Tunnel]
color = 50, 255, 95
system = Li01
system = Li03

[Tunnel]
color = 255, 30, 80
system = Li02
system = Iw03
```

The `color` keys represent the red, green, and blue values ranging from 0 to 255 (integers). For every `[Tunnel]` entry, you can specify one or more systems by nickname. If you are in a jump tunnel within a system which does not have a color listed, then the `[DefaultTunnel]`'s color will be used. Note that for every `[Tunnel]`, the `color` key must always come first, and only below that can the `system` keys be specified.

## Build instructions
Build the project using the `NMAKE` tool from Microsoft Visual C++ 7.0 (VC7) + the VC6 runtime libraries (you may also need the D3D8 header files).
This will output the `JumpTunnelColors.dll` file in the `bin` folder.
Building the project using other compilers may require the code to be modified.
