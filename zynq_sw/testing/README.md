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
opening up a shell and running the `start_qemu.sh` script.

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

## Recurring Problems

There are a couple things that seem to go wrong every once in a while, but
thankfully they're easy to fix. 

If you get a Java runtime error while building, just try again. It seems to go
away the second time you build.

If starting QEMU fails and it tells you that it couldn't allocate memory, you
need to close some other programs to free up some RAM. Browser tabs are usually
the easiest way for me to do that.

If your build failed because the linker can't find the library `-lxil`, just
delete the `workspace` directory and run `setup.sh` again.

If removing the `workspace` directory fails, try using `ls -a` to find hidden
files. They'll usually look like `.fuse_hidden...`. They're files kept open by
Vitis processes that didn't get killed properly. You can run `lsof` on these
files to find which process owns them, and then kill them using their process
ID.
