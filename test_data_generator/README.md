# Test Image Generator

## Building

Run `make` to build.

I have packaged most dependencies, but it needs to link with OpenGL in order to build.
If compilation fails with `/usr/bin/ld: cannot find -lGL`, then try `sudo apt install libgl1-mesa-dev` on Ubuntu.
If you don't want to install anything, this stackoverflow answer should help: https://stackoverflow.com/a/32184137

## Command Line Interface

There are several command line options. They can be applied in any combination, but not every combination is useful.
 - `--load <filename>` loads a `.hrz` file. `.hrz` files are output by the test data generator and store the combination of parameters and outputs associated with an image.
 - `--export <filename>` exports an image without starting GUI. It can be combined with `--load`, and the image is generated from the loaded parameters. Creates the files `<filename>.png`, `<filename>.bin` and `<filename>.hrz`. The `<filename>.hrz` is a different file from the `--load` input, and it contains outputs generated from the inputs (e.g. nadir vector, magnetometer values). 
 - `--fuzz_options <fuzz options> end` selects which parameters to randomize. The list of fuzz options needs to terminate with `end`. Fuzz options are
     - `orientation`
     - `magnetometer_orientation`
     - `atmosphere_height`
     - `altitude`
     - `latitude`
     - `longitude`
     - `noise_seed`
     - `noise_stdev`
 - `--fuzz_count <number of fuzz runs>`
 - `--fuzz_seed <fuzz seed>`
 - `--mag_stdev <stdev>` supplies the (floating point) standard deviation of magnetometer readings between randomizations.
 
 Fuzz parameters are randomized within a range hard-coded into the application.
 
 Examples:
 ```
 # start with GUI
 ./test_image_generator
 
 # load .hrz in GUI
 ./test_image_generator --load filename.hrz
 
 # generate 100 images, with altitude and orientaiton randomized
 # (note: fuzz_seed is optional, it defaults to 1)
 ./test_image_generator --fuzz_options altitude orientation end --fuzz_count 100 --export images/test_image
 ```
