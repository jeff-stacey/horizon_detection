# horizon_detection
Software to determine spacecraft attitude using a single IR camera and a magnetometer. 
Implemented for a Xilinx Zynq MPSoC and tested using simulated camera and magnetometer data. 
Developed for ECE 499 at the University of Victoria by Ryan Blais, Hugo Burd, Byron Kontou, and Jeff Stacey. 
Thanks are due to Kepler Communications for lending us hardware and their general support with the project.

# Repository Layout
`prototypes` contains Python models used to develop and test the algorithm.

`test_data_generator` contains a C++ application that uses SDL to generate images and magnetometer readings for testing. 
It can introduce various types of parametrized distortion and noise. All image parameters can be randomized to produce datasets for batched testing.

`zynq_sw` contains the baremetal C implementation of the algorithm as well as testing scripts which allow the processing of generated data. 
Tests can be run on either the ZCU104 development kit or the version of the QEMU emulator that comes with Xilinx Vitis.
