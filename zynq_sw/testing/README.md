# Testing

The scripts in this directory will let you test the horizon detection stuff on
test images (which should be should be stored in `test_data`). 

First of all, to get all of the Xilinx tools on your path, run
```
source <Xilinx install directory>/Vitis/2019.2/settings64.sh
```

If you haven't already, follow the instructions in the README in `zynq_sw` to
set up the project workspace.

In order to run your tests, you'll have to start up QEMU. You can do this by
opening up a shell and going to the
`horizon_detection/zynq_sw/workspace/horizon_detection/Debug/` directory and running 
```
launch_emulator -device-family ultrascale -t sw_emu -gdb-port 1137
```
It will print a bunch of messages. You'll know it's ready to go when it prints
```
PMU_ROM Version: ....
```

To quit QEMU when you're done testing, go into the terminal where it's running and type
`<Ctrl-a x>`.

To run your code on the image stored in `TestImg` as declared in `main.c`, use
```
xsct run_test.tcl test_data/<image_file>.bin
```

The output from any print statements will appear in the QEMU console. To skip
building, add the `-s` flag in the test run command. 
