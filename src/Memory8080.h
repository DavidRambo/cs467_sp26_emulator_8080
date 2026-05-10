#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace intel_8080 {
class Memory8080 {
 public:
  static constexpr int kSIZE = 0x10000;  // 0xFFFF + 1 for inclusive indices

  Memory8080();

  uint8_t read(uint16_t mem_location);

  void write(uint16_t mem_location, uint8_t data);

  void clear_mem();

  void clear_rom();

  void load_rom(std::string const& file_path);

  void load_data(std::vector<uint8_t> const& data, uint16_t start = 0);

 private:
  std::array<uint8_t, kSIZE> mem_buffer_;
  uint16_t end_of_ROM_;
};
}  // namespace intel_8080
