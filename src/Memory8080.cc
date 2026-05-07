#include "Memory8080.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

intel_8080::Memory8080::Memory8080() : mem_buffer_{} {
  mem_buffer_.fill(0x00);
  end_of_ROM_ = 0x0000;
  std::cout << mem_buffer_.size() << std::endl;
}
std::uint8_t intel_8080::Memory8080::read(std::uint16_t mem_location) {
  return mem_buffer_[mem_location];
}

void intel_8080::Memory8080::write(std::uint16_t mem_location,
                                   std::uint8_t data) {
  if (mem_location <= end_of_ROM_) {
    return;
  }
  mem_buffer_[mem_location] = data;
}

void intel_8080::Memory8080::clear_mem() { mem_buffer_.fill(0xFF); }

void intel_8080::Memory8080::clear_rom() {
  std::fill(mem_buffer_.begin(), mem_buffer_.begin() + end_of_ROM_ + 1, 0xFF);
  end_of_ROM_ = 0x0000;
}

void intel_8080::Memory8080::load_rom(std::string file_path) {
  if (end_of_ROM_ != 0x0000) {
    clear_rom();
  }

  std::ifstream file(file_path, std::ios::in | std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Error opening file." << std::endl;
    std::exit(1);
  }
  std::uintmax_t file_size = std::filesystem::file_size(file_path);

  if (file_size > kSIZE) {
    std::cerr << "Error: File is bigger than 64KB" << std::endl;
    std::exit(1);
  }

  file.read(reinterpret_cast<char*>(mem_buffer_.data()), file_size);

  if (file.gcount() != (int)file_size) {
    std::cerr << "Warning: File size mismatch" << std::endl;
    std::exit(1);
  }
  end_of_ROM_ = file_size - 1;

  file.close();
};

void intel_8080::Memory8080::load_data(std::vector<uint8_t> data,
                                       uint16_t start) {
  if (data.size() > kSIZE + start) {
    std::cerr << "Data is too large for memory" << std::endl;
    std::exit(1);
  }

  for (uint8_t value : data) {
    mem_buffer_[start] = value;
    start++;
  }
}
