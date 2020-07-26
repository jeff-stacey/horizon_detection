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

proc isnan { x } {
    if { ![string is double $x] || $x != $x} {
        return 1
    } else {
        return 0
    }
}

######################
# Parse Command-line #
######################

set parameters {
    { build         "Build the BSP and application before testing"}
    { hw            "Run the test on a connected Zynq MPSoC" }
    { tdir.arg ""   "Test on every available image in the specified directory" }
    { alg.arg 0     "Select which algorithm to use: 0 - edge detection and least-squares, 1 - edge detection and chord fit, 2 - vsearch" }
    { csv.arg ""    "Write results to CSV file of specified name"}
}

set usage "usage: xsct run_test.tcl \[-build\] \[-hw\] \[-alg \{0 | 1 | 2\}\] \{-tdir test_dir | bin_file\}"

# this parses the specified parameters into an array and leaves any other arguments
array set args [cmdline::getoptions argv $parameters $usage]

if { $args(csv) eq ""} {
    set use_csv 0
} else {
    set use_csv 1
}

# detemine what test data we want to use 
if { $args(tdir) eq "" } {
    set testfiles [lindex $argv 0]
} else {
    set testfiles [glob $args(tdir)/*.bin]
}

##########################
# General Test Run Setup #
##########################

set image_variable_name "TestImg"
set app_name "horizon_detection"
set build_dir "../workspace/$app_name/Debug"
set hw_dir "../workspace/zcu104/hw"

set testing_dir [pwd]

if { $use_csv } {
    # open a .csv file for results
    set csvf [open $args(csv) w]
    # write heading line to the csv
    puts $csvf "testfile,alg_choice,err_angle,num_points,noise_stdev,visible_atmosphere_height,runtime,qwmes,qxmes,qymes,qzmes,nxmes,nymes,nzmes,qwref,qxref,qyref,qzref,mquatw,mquatx,mquaty,mquatz,altitude,latitude,longitude,noise_seed,nxref,nyref,nzref,magx,magy,magz,magreadingx,magreadingy,magreadingz"
}


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
    set t0 [ clock microseconds ]
    con -block -addr [lindex [print main] 2]

    set t1 [clock microseconds]
    set dt [expr $t1 - $t0]
    puts "\tMain finished after $dt us, reading results"
    # should stop at end_of_main label

    # grab which algorithm was used
    set alg_choice [vread alg_choice]

    # grab the runtime
    set cycles [vread cycles]
    set runtime [expr $cycles * 64 * 1/500e6]

    puts "\tTook $runtime seconds ($cycles cycles)"

    # grab the nadir vector
    set nxmes [vread nadir[0]]
    set nymes [vread nadir[1]]
    set nzmes [vread nadir[2]]

    # grab the quaternion (NOT IMPLEMENTED)
    set qwmes 0
    set qxmes 0
    set qymes 0
    set qzmes 0

    # grab the number of points the edge detection found
    set num_points [vread num_points]

    # open the image parameter file and load the data
    set hrz_filename [concat [lindex [split $testfile .] 0].hrz]
    set hrz_file [open $hrz_filename "rb"]
    set hrz [read $hrz_file]
    close $hrz_file

    # extract values from the file
    binary scan $hrz fffffffffffffffffffffff qwref qxref qyref qzref mquatw mquatx mquaty mquatz altitude latitude longitude noise_seed noise_stdev visible_atmosphere_height nxref nyref nzref magx magy magz magreadingx magreadingy magreadingz

    # compare values and print results
    if {$alg_choice == 0} {
        puts "\tUsed edge detection and least-squares fit"
    } elseif {$alg_choice == 1} {
        puts "\tUsed edge detection and chord fit"
    }

    puts "\tEdge Detection found $num_points points"

    puts "\tNadir Vector:"
    puts "\t   Baseline    Detected    "

    set nancount 0

    if { [isnan $nxmes] } {
        # nans don't equal each other
        puts [format "\tx: %.10f  nan" $nxref]
        incr nancount
    } else {
        puts [format "\ty: %.10f  %.10f" $nxref $nxmes]
    }
    if { [isnan $nymes] } {
        # nans don't equal each other
        puts [format "\tx: %.10f  nan" $nyref]
        incr nancount
    } else {
        puts [format "\ty: %.10f  %.10f" $nyref $nymes]
    }
    if { [isnan $nzmes] } {
        # nans don't equal each other
        puts [format "\tx: %.10f  nan" $nzref]
        incr nancount
    } else {
        puts [format "\ty: %.10f  %.10f" $nzref $nzmes]
    }

    if { $nancount == 0 } {
        set pi [expr acos(-1.0)]
        set err_angle [expr 180 / $pi * acos($nxref * $nxmes + $nyref * $nymes + $nzref * $nzmes)]
        puts [format "\tDetected nadir vector is off by %.4f degrees" $err_angle]
    } else {
        set err_angle NaN
    }

    if { $use_csv } {
        # write results to CSV file
        puts $csvf "$testfile,$alg_choice,$err_angle,$num_points,$noise_stdev,$visible_atmosphere_height,$runtime,$qwmes,$qxmes,$qymes,$qzmes,$nxmes,$nymes,$nzmes,$qwref,$qxref,$qyref,$qzref,$mquatw,$mquatx,$mquaty,$mquatz,$altitude,$latitude,$longitude,$noise_seed,$nxref,$nyref,$nzref,$magx,$magy,$magz,$magreadingx,$magreadingy,$magreadingz"
    }
}

if {$use_csv} {
    close $csvf
}
