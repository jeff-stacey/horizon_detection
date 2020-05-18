# ece499-test-image-generator

Run `make` to build.

I have packaged most dependencies, but it needs to link with OpenGL in order to build.
If compilation fails with `/usr/bin/ld: cannot find -lGL`, then try `sudo apt install libgl1-mesa-dev` on Ubuntu.
If you don't want to install anything, this stackoverflow answer should help: https://stackoverflow.com/a/32184137
