#pragma once 
#include <cstdint>
#include <array>
#include <string>


namespace intel_8080 {
class Memory8080 {
public:
  static constexpr int SIZE = 0x10000; // 0xFFFF + 1 for inclusive indices
  Memory8080();
    
  uint8_t Read(std::uint16_t mem_location);

  void Write(uint16_t mem_loation, uint8_t data);

  void ClearMem();
 
  void ClearROM();
    
  void LoadROM(std::string file_path);

private:
  std::array<std::uint8_t, SIZE> mem_buffer_;
  std::uint16_t end_of_ROM_;
  };
}


