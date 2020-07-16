## This should be sourced in the 'xsct' TCL shell to set up the project

set app_name "horizon_detection"

# Create the workspace
if [file exist workspace] {
    puts "Workspace already exists, so I'll leave it alone and just re-import the source files."
    puts "If you just checked out a new branch, it's a good idea to regenerate the workspace."
    puts "To do that, delete the workspace directory and run this script again."
    
    setws workspace
    cd workspace
} else {
    puts "workspace not found - creating it"
    if {$tcl_platform(platform) == "unix"} {
        puts "Creating workspace directory (unix)"
        exec mkdir -p workspace
    } else {
        puts "Creating workspace directory"
        exec mkdir -p workspace
    }

    setws workspace
    cd workspace

    # Set up the platform
    puts "Creating the platform"
    platform create -name "zcu104" -hw ../vitis_config/zcu104.xsa -os standalone -proc psu_cortexr5_0

    # Create our app for the Cortex R5
    puts "Creating R5 app"
    app create -name $app_name -platform zcu104 -proc psu_cortexr5_0 -os standalone -template {Empty Application}

    puts "Adding linker flag for math library"
    if [ catch { app config -name $app_name -add libraries {m} } ] {
    }
}

# Import source files
puts "Importing source files"
importsources -name $app_name -path ../src/

# Link source files
puts "Linking source files"
foreach source_file [glob -tails -directory ../src *] {
    if {$tcl_platform(platform) == "unix"} { 
        exec ln -f ../src/$source_file $app_name/src/$source_file
    } else {
        exec ln -f ../src/$source_file $app_name/src/$source_file
    }
}
    
# Turn on debug prints
app config -name $app_name define-compiler-symbols HD_DEBUG

# Build the app
app build -name $app_name

# copy in the stuff QEMU needs
exec cp -r ../misc/_vimage $app_name/Debug/_vimage
