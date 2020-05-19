## Install Vitis

Get the installer from [here](https://www.xilinx.com/member/forms/download/xef.html?filename=Xilinx_Unified_2019.2_1106_2127_Lin64.bin). If you don't have a Xilinx account you'll need to make one but it's not hard.

Make the downloaded file executable and run it.

It might warn you about your OS not being supported but it should be fine.

Log in with your Xilinx credentials and choose download and install now.

Agree to the license stuff.

Select Vitis.

Installation options:

- Design tools:
    - [ ] Vitis Unified
        - [x] Vitis
        - [x] Vivado
        - [ ] System Generator for DSP
    - [x] DocNav
- Devices:
    - [ ] Install devices for Alveo and Xilinx edge acceleration platforms
    - [ ] Devices for custom platforms
        - [ ] SoCs
            - [ ] Zynq-7000
            - [x] Zynq Ultrascale+ MPSoC
            - [ ] Zynq Ultrascale+ RFSoC
        - [x] 7 Series
            - [x] Artix-7
            - [x] Kintex-7
            - [x] Spartan-7
            - [x] Virtex-7
        - [ ] Ultrascale
        - [ ] Ultrascale+
    - [ ] Engineering Sample Devices for Custom Platforms
- Installation Options
    - [x] Acquire or manage a License Key
    - [x] Enable WebTalk for Vivado to send usage statistic to Xilinx (Always enabled for WebPACK licens)

Choose a place to install and run the installer. It'll take a bit to install.

## Set up the project

Open up a shell and go to the directory these instructions are in.

Get the Xilinx tools into scope by running 

`source <Xilinx install directory>/Vitis/2019.2/settings64.sh`

Set up the workspace using:

`xsct vitis_config/project_init.tcl`

## Build and emulate

Once that's done, open Vitis (it should show up as a regular desktop app) and choose the `workspace` directory we just created as the workspace.

Close the welcome screen by clicking the little x on the tab.

In the Explorer window on the left, open Horizon Detection System > Horizon Detection > src > hello.c. 

Click the hammer symbol in the top bar to build the project and all of the libraries.

Click the dropdown arrow next to the bug symbol and hit Debug Configurations.

Double-click Single Application Debug, check the Emulation box, and click Debug at the bottom left. It'll take a couple seconds, but this should open a debug view and start up QEMU. If it takes a long time to start and you get a bunch of errors in the Emulation Console on the bottom left, you might need to install `netstat`.

If you click the continue button (the green triangle), it should print "Hello World" in the emulation console.
