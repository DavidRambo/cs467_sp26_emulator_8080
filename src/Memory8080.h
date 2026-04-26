#pragma once 
#include <cstdint>
#include <array>
#include <string>

using namespace std;
namespace MemoryIntel8080 {
  class Memory8080 {
  public:
    static constexpr int SIZE = 0x10000; // 0xFFFF + 1 for inclusive indices
    Memory8080();
    
    uint8_t Read(uint16_t mem_location);

    void Write(uint16_t mem_loation, uint8_t data);

    void ClearMem();
 
    void ClearROM();
    
    void LoadROM(string file_path);

  private:
    array<uint8_t, SIZE> mem_buffer_;
    uint16_t end_of_ROM_;
  };
}


