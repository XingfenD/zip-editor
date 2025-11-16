#include "utils.hpp"

template uint16_t readLittleEndian<uint16_t>(std::ifstream& file);
template uint32_t readLittleEndian<uint32_t>(std::ifstream& file);

template void writeLittleEndian(std::ofstream& file, uint32_t value);
template void writeLittleEndian(std::ofstream& file, uint16_t value);
