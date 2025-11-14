#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <memory>
#include <fstream>

/* read specified size of data and convert it to little endian */
template<typename T>
inline T readLittleEndian(std::ifstream& file) {
    T value;
    file.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

#endif /* UTILS_HPP */
