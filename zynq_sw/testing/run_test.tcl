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
    set base_addr [lindex [print &$var_name] 2]
    mrd -bin -size h -file $file_name $base_addr [expr 160*120]
}

proc isnan { x } {
    if { ![string is double $x] || $x != $x} {
        return 1
    } else {
        return 0
    }
}

# creates a quaternion dict out of components
proc build_quat { w x y z } {
    set q [dict create w $w x $x y $y z $z]
    return $q
}

# multiplies two quaternion dicts 
proc quat_mult { l r } {
    set q [dict create w [expr [dict get $l w]*[dict get $r w] - [dict get $l x]*[dict get $r x] - [dict get $l y]*[dict get $r y] - [dict get $l z]*[dict get $r z]]]
    dict set q x [expr [dict get $l w]*[dict get $r x] + [dict get $l x]*[dict get $r w] + [dict get $l y]*[dict get $r z] - [dict get $l z]*[dict get $r y]]
    dict set q y [expr [dict get $l w]*[dict get $r y] - [dict get $l x]*[dict get $r z] + [dict get $l y]*[dict get $r w] + [dict get $l z]*[dict get $r x]]
    dict set q z [expr [dict get $l w]*[dict get $r z] + [dict get $l x]*[dict get $r y] - [dict get $l y]*[dict get $r x] + [dict get $l z]*[dict get $r w]]
    return $q
}

# inverts a quaternion dict
proc quat_inv { a } {
    set q [dict create w [dict get $a w]]
    dict set q x [expr -1*[dict get $a x]] 
    dict set q y [expr -1*[dict get $a y]] 
    dict set q z [expr -1*[dict get $a z]]
    return $q
}

# rotates v by r (computes rvr^{-1})
proc quat_rotate { r v } {
    set r_inv [quat_inv $r]

    set t [quat_mult $r $v]
    set q [quat_mult $t $r_inv]

    return $q
}

# rotates the vector (vx, vy, vz) by the quaternion (rw, rx, ry, rz)
proc quat_rotate_vector { rw rx ry rz vx vy vz } {
    set r [build_quat $rw $rx $ry $rz]
    set v [build_quat 0 $vx $vy $vz]
    
    set q [quat_rotate $r $v]

    return $q
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
    set testfiles $argv
} else {
    set testfiles [lsort -dictionary [glob $args(tdir)/*.bin]]
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
    puts $csvf "testfile,alg_choice,dist_corr,err_angle,reject,num_points,mean_sq_error,mean_abs_error,circ_cx,circ_cy,circ_r,noise_stdev,visible_atmosphere_height,runtime,qwmes,qxmes,qymes,qzmes,nxmes,nymes,nzmes,qwref,qxref,qyref,qzref,mquatw,mquatx,mquaty,mquatz,altitude,latitude,longitude,noise_seed,nxref,nyref,nzref,magx,magy,magz,magreadingx,magreadingy,magreadingz,north_err_angle"
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

# disable the breakpoint at main so we don't hit it when looping back to it
bpdisable 0

foreach testfile $testfiles {
    puts "Starting test for $testfile"

    # open the image parameter file and load the data
    set hrz_filename [concat [lindex [split $testfile .] 0].hrz]
    set hrz_file [open $hrz_filename "rb"]
    set hrz [read $hrz_file]
    close $hrz_file

    # extract values from the file
    binary scan $hrz ffffffffffffffffffffsssf16 qwref qxref qyref qzref mquatw mquatx mquaty mquatz altitude latitude longitude noise_seed noise_stdev visible_atmosphere_height nxref nyref nzref magx magy magz magreadingx magreadingy magreadingz mag_trans

    # insert the image into memory
    set image_base_addr [lindex [print &$image_variable_name] 2]
    puts "\tCopying image data from $testfile into memory"
    mwr -bin -size h -file $testing_dir/$testfile $image_base_addr [expr 160*120]

    #imread TestImg out.bin
    
    # write the magnetometer transformation into memory
    for {set i 0} {$i < 16} {incr i} {
        print -s magnetometer_transformation[$i] [lindex $mag_trans $i]
    }

    # set the altitude in memory
    print -s altitude $altitude

    # write the magnetometer reading into memory
    print -s magnetometer_reading[0] $magreadingx
    print -s magnetometer_reading[1] $magreadingy
    print -s magnetometer_reading[2] $magreadingz

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

    # grab the result status
    set reject [vread reject]

    # grab the runtime
    set cycles [vread cycles]
    set runtime [expr $cycles * 64 * 1/500e6]

    puts "\tTook $runtime seconds ($cycles cycles)"

    # grab the nadir vector
    set nxmes [vread nadir[0]]
    set nymes [vread nadir[1]]
    set nzmes [vread nadir[2]]

    # grab the quaternion (NOT IMPLEMENTED)
    set qwmes [vread orientation.w]
    set qxmes [vread orientation.x]
    set qymes [vread orientation.y]
    set qzmes [vread orientation.z]

    # grab the number of points the edge detection found
    set num_points [vread num_points]

    # compare values and print results
    if {$alg_choice == 0} {
        puts "\tUsed edge detection and least-squares fit"
    } elseif {$alg_choice == 1} {
        puts "\tUsed edge detection and chord fit"
    }

    # grab the goodness of fit measures
    set mean_sq_error [vread mean_sq_error]
    set mean_abs_error [vread mean_abs_error]
    puts "\tMean squared error: $mean_sq_error"
    puts "\tMean absolute error: $mean_abs_error"

    # grab the circle fit results
    set circ_cx [vread circ_params[0]]
    set circ_cy [vread circ_params[1]]
    set circ_r [vread circ_params[2]]

    # grab whether barrel distortion was corrected
    set dist_corr [vread correct_barrel_dist]

    puts "\tCircle paramters: x = $circ_cx, y = $circ_cy, r = $circ_r"

    puts "\tEdge Detection found $num_points points"

    puts "\tNadir Vector:"

    set nancount 0

    if { [isnan $nxmes] } {
        puts [format "\tx: %+.10f  nan" $nxref]
        incr nancount
    } else {
        puts [format "\tx: %+.10f  %+.10f" $nxref $nxmes]
    }
    if { [isnan $nymes] } {
        # nans don't equal each other
        puts [format "\ty: %+.10f  nan" $nyref]
        incr nancount
    } else {
        puts [format "\ty: %+.10f  %+.10f" $nyref $nymes]
    }
    if { [isnan $nzmes] } {
        # nans don't equal each other
        puts [format "\tz: %+.10f  nan" $nzref]
        incr nancount
    } else {
        puts [format "\tz: %+.10f  %+.10f" $nzref $nzmes]
    }


    if { $reject } {
        if { $reject == 1 } {
            puts "\t***Horizon determination failed - not enough points***"
        } else {
            puts "\t***Horizon determination failed - radius too small***"
        }
        set err_angle NaN
        set north_err_angle NaN
    } else {
        if { $nancount == 0 } {
                set pi [expr acos(-1.0)]
                set dot_prod_r [expr $nxref * $nxmes + $nyref * $nymes + $nzref * $nzmes]
                if {$dot_prod_r > 1} {
                    # clamp to 1
                    set dot_prod_r 1
                }
                set err_angle [expr 180 / $pi * acos($dot_prod_r)]
                puts [format "\tDetected nadir vector is off by %.4f degrees" $err_angle]

                puts "\tOutput Quaternion"
                puts "\t   Baseline       Detected"
                puts [format "\tw: %+.10f  %+.10f" $qwref $qwmes]
                puts [format "\tx: %+.10f  %+.10f" $qxref $qxmes]
                puts [format "\ty: %+.10f  %+.10f" $qyref $qymes]
                puts [format "\tz: %+.10f  %+.10f" $qzref $qzmes]

                # rotate the vector pointing north from the base reference frame to the
                # camera reference frame two ways:

                # first by the reference orientation quaternion
                set ref_north_q [quat_rotate_vector $qwref [expr -$qxref] [expr -$qyref] [expr -$qzref] 0 1 0]
                set ref_north_x [dict get $ref_north_q x]
                set ref_north_y [dict get $ref_north_q y]
                set ref_north_z [dict get $ref_north_q z]

                # then by the measured one
                set mes_north_q [quat_rotate_vector $qwmes $qxmes $qymes $qzmes 0 1 0]
                set mes_north_x [dict get $mes_north_q x]
                set mes_north_y [dict get $mes_north_q y]
                set mes_north_z [dict get $mes_north_q z]

                set dot_prod [expr $ref_north_x*$mes_north_x + $ref_north_z*$mes_north_z + $ref_north_y*$mes_north_y]

                if { $dot_prod > 1 } {
                    puts "Dot product of north vectors is $dot_prod"
                    set dot_prod 1
                }

                # compute the angle between the two
                set north_err_angle [expr 180 / $pi * acos($dot_prod)]
                puts [format "\tDetected north is %.4f degrees off" $north_err_angle]
        } else {
            error "One or more components of the nadir vector is NaN - something has gone wrong"
        }
    }

    if { $use_csv } {
        # write results to CSV file
        puts $csvf "$testfile,$alg_choice,$dist_corr,$err_angle,$reject,$num_points,$mean_sq_error,$mean_abs_error,$circ_cx,$circ_cy,$circ_r,$noise_stdev,$visible_atmosphere_height,$runtime,$qwmes,$qxmes,$qymes,$qzmes,$nxmes,$nymes,$nzmes,$qwref,$qxref,$qyref,$qzref,$mquatw,$mquatx,$mquaty,$mquatz,$altitude,$latitude,$longitude,$noise_seed,$nxref,$nyref,$nzref,$magx,$magy,$magz,$magreadingx,$magreadingy,$magreadingz,$north_err_angle"
    }

}

if {$use_csv} {
    close $csvf
}
