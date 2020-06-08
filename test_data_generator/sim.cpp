#include "sim.h"
#include <fstream>
#include <iomanip>
#include <string>
#include <limits>
#include <iostream>

bool SimulationState::load_state(const char* filename)
{
    std::ifstream save_file(filename, std::ios::binary | std::ios::ate);
    if (save_file.tellg() != sizeof(*this))
    {
        std::cout << "Badly formatted save file" << std::endl;
        return false;
    }
    save_file.seekg(std::ios::beg);
    save_file.read((char*)this, sizeof(*this));
    return true;
}

void SimulationState::save_state(char* filename)
{
    std::ofstream save_file(filename, std::ios::trunc | std::ios::binary);
    save_file.write((const char*)this, sizeof(*this));
}
