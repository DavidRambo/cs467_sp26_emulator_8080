// includes
#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>

#include "Memory8080.h"
using namespace std;
using namespace MemoryIntel8080;

Memory8080::Memory8080() : mem_buffer_{}{
  mem_buffer_.fill(0x00);
  end_of_ROM_ = 0x0000;
  cout << mem_buffer_.size() << endl;
  }
uint8_t Memory8080::Read(uint16_t mem_location) {
  return mem_buffer_[mem_location];
  }

void Memory8080::Write(uint16_t mem_location, uint8_t data) {
    if (mem_location <= end_of_ROM_) {
      return;
    }
    mem_buffer_[mem_location] = data;
 }

void Memory8080::ClearMem() {
  mem_buffer_.fill(0x00);
}

void Memory8080::ClearROM() {
  fill(mem_buffer_.begin(), mem_buffer_.begin() + end_of_ROM_ + 1, 0x00);
  end_of_ROM_ = 0x0000;
}

void Memory8080::LoadROM(string file_path) {
  if(end_of_ROM_ != 0x0000){
    ClearROM();
  }

    ifstream file(file_path, ios::in | ios::binary);

    if(file.is_open()) {
      uint16_t file_size = filesystem::file_size(file_path);

      if (file_size > this->SIZE) {
        cerr << "Error: File is bigger than 64KB" << endl;
      }

      file.read(reinterpret_cast<char*>(mem_buffer_.data()), file_size);

      if (file.gcount() != file_size) {
        cerr << "Warning: File size mismatch" << endl; 
      }
      end_of_ROM_ = file_size - 1;
  
      file.close();

    } else {
      cerr << "Error opening file." << endl;
      return;
    }

}
