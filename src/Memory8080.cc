#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#include "Memory8080.h"


intel_8080::Memory8080::Memory8080() : mem_buffer_{}{
  mem_buffer_.fill(0x00);
  end_of_ROM_ = 0x0000;
    std::cout << mem_buffer_.size() << std::endl;
}
std::uint8_t intel_8080::Memory8080::Read(std::uint16_t mem_location) {
  return mem_buffer_[mem_location];
 }

void intel_8080::Memory8080::Write(std::uint16_t mem_location, std::uint8_t data) {
    if (mem_location <= end_of_ROM_) {
      return;
    }
    mem_buffer_[mem_location] = data;
 }

void intel_8080::Memory8080::ClearMem() {
  mem_buffer_.fill(0xFF);
}

void intel_8080::Memory8080::ClearROM() {
  std::fill(mem_buffer_.begin(), mem_buffer_.begin() + end_of_ROM_ + 1, 0xFF);
  end_of_ROM_ = 0x0000;
}

void intel_8080::Memory8080::LoadROM(std::string file_path) {
  if(end_of_ROM_ != 0x0000){
    ClearROM();
  }

    std::ifstream file(file_path, std::ios::in | std::ios::binary);

    if(!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        std::exit(1);
    }
    std::uintmax_t file_size = std::filesystem::file_size(file_path);

    if (file_size > SIZE) {
      std::cerr << "Error: File is bigger than 64KB" << std::endl;
        std::exit(1);
    }

    file.read(reinterpret_cast<char*>(mem_buffer_.data()), file_size);

    if (file.gcount() != file_size) {
        std::cerr << "Warning: File size mismatch" << std::endl; 
        std::exit(1);
    }
    end_of_ROM_ = file_size - 1;
  
    file.close();
};
