#!xsct

package require cmdline

##############
# Procedures #
##############

# reads a variable from memory by name
proc vread { name } {
    return [lindex [print $name] 2]
}

# reads 160x120 image to binary file called file_name
# i'm not using this right now, but it's indended for edge detection debugging
proc imread { var_name file_name } {
    set base_addr [lindex [print &$name] 2]
    mrd -bin -file $file_name [expr 160*120]
}

######################
# Parse Command-line #
######################

set parameters {
    { build     "Build the BSP and application before testing"}
    { hw        "Run the test on a connected Zynq MPSoC" }
    { tdir      "Test on every available image in the specified directory" }
    { alg.arg 0 "Select which algorithm to use: 0 - edge detection and least-squares, 1 - edge detection and chord fit, 2 - vsearch" }
}

# this parses the specified parameters into an array and leaves any other arguments
array set args [cmdline::getoptions argv $parameters]

# detemine what test data we want to use 
# because of the way i'm parsing arguments, this has to happen after parsing all other flags
if { $args(tdir) } {
    set testfiles [glob [lindex $argv 0]/*.bin]
} else {
    set testfiles [lindex $argv 0]
}

##########################
# General Test Run Setup #
##########################

set image_variable_name "TestImg"
set app_name "horizon_detection"
set build_dir "../workspace/$app_name/Debug"
set hw_dir "../workspace/zcu104/hw"

set testing_dir [pwd]

# set the workspace
if [file exist ../workspace] {
    setws ../workspace
} else {
    puts "Error, couldn't find the workspace directory." 
    puts "You should be running this from the testing directory and"
    puts "you should have run the project setup script to generate the app."
    exit
}


# maybe build the application
if { $args(build) } {
    puts "Building application - pay attention! I can't tell if the build fails."
    app build -name $app_name
}

######################################
# Connect to either hardware or QEMU #
######################################

if { $args(hw) } {
    puts "Connecting to hardware"
    # get vitis install directory
    set vitis_path [join [lrange [split [exec which xsct] /] 0 end-2] /]

    #load the Zynq tools
    source $vitis_path/scripts/vitis/util/zynqmp_utils.tcl

    #load the script for dealing with the zynq PSU
    source $hw_dir/psu_init.tcl

    # connect to the device
    connect

    # select the PSU target
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

#############
# Run Tests #
#############

puts "Copying executable into memory"
dow $build_dir/$app_name.elf

puts "\tResetting processor"
mask_write 0xff5e023c [expr (1 << 0) | 0x14] 0
mwr 0xff9a0000 0x80000218

# add starting and ending breakpoints
bpadd -addr &main
bpadd -addr &end_of_main

puts "\tRunning until start of main"
con -block -timeout 10

foreach testfile $testfiles {
    puts "Starting test for $testfile"

    set image_base_addr [lindex [print &$image_variable_name] 2]
    puts "\tCopying image data from $testfile into memory"
    mwr -bin -file $testing_dir/$testfile $image_base_addr [expr 160*120]

    # Set this to 
    # 0 for edge detection and least-squares curve fit
    # 1 for edge detection and chord curve fit
    # 2 for vsearch (not implemented in C yet)
    print -set alg_choice $args(alg)

    puts "\tRunning program"
    #start running at the start of main
    con -block -addr [lindex [print main] 2]

    puts "\tMain finished, reading results"
    # should stop at end_of_main label

    set alg_choice [vread alg_choice]

    # grab the nadir vector
    set nadir_x [vread nadir[0]]
    set nadir_y [vread nadir[1]]
    set nadir_z [vread nadir[2]]

    # open the image parameter file and load the data
    set hrz_filename [concat [lindex [split $testfile .] 0].hrz]
    set hrz_file [open $hrz_filename "rb"]
    set hrz [read $hrz_file]
    close $hrz_file

    # extract values from the file
    binary scan $hrz fffffffffffffffffffffff qw qx qy qz mquatw mquatx mquaty mquatz altitude latitude longitude noise_seed noise_stdev visible_atmosphere_height nx ny nz magx magy magz magreadingx magreadingy magreadingz

    # compare values and print results
    if {$alg_choice == 0} {
        puts "\tUsed edge detection and least-squares fit"
    } elseif {$alg_choice == 1} {
        puts "\tUsed edge detection and chord fit"
    }

    puts "\tNadir Vector:"
    puts "\t   Baseline    Detected    "
    puts [format "\tx: %.10f  %.10f" $nx $nadir_x]
    puts [format "\ty: %.10f  %.10f" $ny $nadir_y]
    puts [format "\tz: %.10f  %.10f" $nz $nadir_z]

    set pi [expr acos(-1.0)]
    set err_angle [expr 180 / $pi * acos($nx * $nadir_x + $ny * $nadir_y + $nz * $nadir_z)]
    puts [format "\tDetected nadir vector is off by %.4f degrees" $err_angle]
}
