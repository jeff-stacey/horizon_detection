#!xsct

set image_variable_name "TestImg"

set app_name "horizon_detection"

# Change this to use non-debug build - not tested
set build_dir "../workspace/$app_name/Debug"
set hw_dir "../workspace/zcu104/hw"

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

set using_hw [lsearch -exact $argv "-b"]

if {$using_hw >= 0} {
    puts "Connecting to hardware"
    # remove this argument from argv
    set argv_new {}
    foreach item $argv {
        if {$item ni "-b"} {
            lappend argv_new $item
        }
        set argv $argv_new
    }

    # get vitis install directory
    set vitis_path [join [lrange [split [exec which xsct] /] 0 end-2] /]

    #load the Zynq tools
    source $vitis_path/scripts/vitis/util/zynqmp_utils.tcl

    #load the script for dealing with the zynq PSU
    source $hw_dir/psu_init.tcl

    connect

    targets -set -filter {name =~"PSU"}

    psu_init
    
    puts "Selecting Cortex-R5 #0"
    targets -set -filter {name =~"*R5*0"}

    rst -processor

} else {
    puts "Connecting to emulator"
    # annoyingly this doesn't have a return status so I can't check if it failed
    gdbremote connect localhost:1137

    puts "Selecting Cortex-R5 #0"
    targets -set -filter {name =~"*R5*0"}

}


puts "Copying executable into memory"
dow $build_dir/$app_name.elf

puts "Resetting processor"
mask_write 0xff5e023c [expr (1 << 0) | 0x14] 0
mwr 0xff9a0000 0x80000218

bpadd -addr &main

bpadd -addr &end_of_main

puts "Running until start of main"
con -block -timeout 5

set image_base_addr [lindex [print &$image_variable_name] 2]

set testfile [lindex $argv 0]

puts "Copying image data from $testfile into memory"
mwr -bin -file $testing_dir/$testfile $image_base_addr [expr 160*120]

puts "Running program"
con -block

puts "Main finished, reading results"
# should stop at end_of_main label
set result_addr [lindex [print &highRatio] 2]
set result_value [lindex [mrd $result_addr] 1]

# parse result as float
binary scan [binary format i 0x$result_value] f y
puts $y

con

# TODO: find a way for the program to indicate it's done so we can run multiple
# tests in a loop.
