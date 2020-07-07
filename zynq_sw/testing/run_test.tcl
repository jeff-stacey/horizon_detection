set image_variable_name "TestImg"

set app_name "horizon_detection"

# Change this to use non-debug build - not tested
set build_dir "../workspace/$app_name/Debug"

set testing_dir [pwd]

if [file exist ../workspace] {
    setws ../workspace
} else {
    puts "Error, couldn't find the workspace directory." 
    puts "You should be running this from the testing directory and"
    puts "you should have run the project setup script to generate the app."
    exit
}

set skip_build_idx [lsearch -exact $argv "-s"]

if {$skip_build_idx >= 0} {
    puts "Skipping build"
    set argv_new {}
    foreach item $argv {
        if {$item ni "-s"} {
            lappend argv_new $item
        }
    }
    set argv $argv_new
} else {
    puts "Building application - pay attention! I can't tell if the build fails."
    app build -name $app_name
}

puts "Connecting to emulator"
# annoyingly this doesn't have a return status so I can't check if it failed
gdbremote connect localhost:1137

puts "Selecting Cortex-R5 #0"
targets 8

puts "Copying executable into memory"
dow $build_dir/$app_name.elf

puts "Resetting processor"
mask_write 0xff5e023c [expr (1 << 0) | 0x14] 0
mwr 0xff9a0000 0x80000218

bpadd -addr &main

puts "Running until start of main"
con -block -timeout 5

set image_base_addr [lindex [print &$image_variable_name] 2]

set testfile [lindex $argv 0]

puts "Copying image data from $testfile into memory"
mwr -bin -file $testing_dir/$testfile $image_base_addr [expr 160*120]

puts "Running program"
con

# TODO: find a way for the program to indicate it's done so we can run multiple
# tests in a loop.