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
}

std::span<unsigned char, 0x1C00> intel_8080::Memory8080::get_vram_span() {
  // First create a span to view the entire buffer.
  std::span<unsigned char, 0x10000> full_span{mem_buffer_};

  // Then create a subspan of the video ram section, which starts at 0x2400 and
  // is 0x1C00 bytes.
  std::span<unsigned char, 0x1C00> vram_span{full_span.subspan(0x2400, 0x1C00)};

  return vram_span;
}

std::uint8_t intel_8080::Memory8080::read(std::uint16_t mem_location) {
  return mem_buffer_[mem_location];
}

void intel_8080::Memory8080::write(uint16_t mem_location, uint8_t data) {
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

void intel_8080::Memory8080::load_rom(std::string const& file_path) {
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

  if (file.gcount() != static_cast<int>(file_size)) {
    std::cerr << "Warning: File size mismatch" << std::endl;
    std::exit(1);
  }
  end_of_ROM_ = file_size - 1;

  file.close();
};

void intel_8080::Memory8080::load_rom_at_addr(std::string const& file_path,
                                              uint16_t addr) {
  if (addr == 0x0000) {
    clear_rom();
  }

  std::ifstream file(file_path, std::ios::in | std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Error opening file." << std::endl;
    std::exit(1);
  }
  std::uintmax_t file_size = std::filesystem::file_size(file_path);

  if (file_size + addr > kSIZE) {
    std::cerr << "Error: loading the file at that address would overflow memory"
              << std::endl;
    std::exit(1);
  }

  for (uintmax_t data_count{0}; data_count < file_size; data_count++) {
    mem_buffer_[addr++] = file.get();
  }

  end_of_ROM_ = addr - 1;

  file.close();
};

void intel_8080::Memory8080::load_data(std::vector<uint8_t> const& data,
                                       uint16_t start) {
  if (data.size() + start > kSIZE) {
    std::cerr << "Data is too large for memory" << std::endl;
    std::exit(1);
  }

  for (uint8_t value : data) {
    mem_buffer_[start] = value;
    start++;
  }
}
