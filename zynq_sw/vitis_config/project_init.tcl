## This should be sourced in the 'xsct' TCL shell to set up the project

set app_name "horizon_detection"

# Create the workspace
exec mkdir -p workspace
setws workspace
cd workspace

# Set up the platform
platform create -name "zcu104" -hw ../vitis_config/zcu104.xsa -os standalone -proc psu_cortexr5_0

# Create our app for the Cortex R5
app create -name $app_name -platform zcu104 -proc psu_cortexr5_0 -os standalone -template {Empty Application}

# Import source files
importsources -name $app_name -path ../src/

# Link source files
foreach source_file [glob -tails -directory ../src *] {
    exec ln -f ../src/$source_file $app_name/src/$source_file
}
    

# Build the app
#app build -name $app_name
