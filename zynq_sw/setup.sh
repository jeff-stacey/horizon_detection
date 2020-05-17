
if [ -x "$(command -v xsct)" ]; then
    xsct vitis_config/project_init.tcl
else
    echo "xsct not found! Try sourcing 'settings64.sh' from the Vitis install directory"
fi

