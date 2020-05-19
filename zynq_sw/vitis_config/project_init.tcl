## This should be sourced in the 'xsct' TCL shell to set up the project

set app_name "horizon_detection"

# Create the workspace
if {$tcl_platform(platform) == "unix"} {
    puts "Creating workspace directory (unix)"
    exec mkdir -p workspace
} else {
    puts "Creating workspace directory"
    exec if not exist \"workspace\" mkdir \"workspace\"
}
setws workspace
cd workspace

# Set up the platform
puts "Creating the platform"
platform create -name "zcu104" -hw ../vitis_config/zcu104.xsa -os standalone -proc psu_cortexr5_0

# Create our app for the Cortex R5
puts "Creating R5 app"
app create -name $app_name -platform zcu104 -proc psu_cortexr5_0 -os standalone -template {Empty Application}

# Import source files
puts "Importing source files"
importsources -name $app_name -path ../src/

# Link source files
puts "Linking source files"
foreach source_file [glob -tails -directory ../src *] {
    if {$tcl_platform(platform) == "unix"} { 
        exec ln -f ../src/$source_file $app_name/src/$source_file
    } else {
        exec mklink /H $app_name\src\$source_file ..\src\$source_file
    }
}
    

# Build the app
#app build -name $app_name
