#include <iostream>

#include "CPU8080.h"

// For printing hex bytes to stdout
#include <iomanip>

namespace intel_8080 {
// For printing hex bytes to stdout
static void print_hex_byte(uint8_t value) {
  std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
            << static_cast<int>(value) << std::dec;
}

static void print_hex_word(uint16_t value) {
  std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
            << static_cast<int>(value) << std::dec;
}

void CPU8080::print_debug() {
  std::cout << "==== CPU8080 ====\n";
  State state = get_state();
  std::cout << "PC: ";
  print_hex_word(state.program_counter);
  std::cout << "\n";
  std::cout << "SP: ";
  print_hex_word(state.stack_pointer);
  std::cout << "\n";
  std::cout << "B: ";
  print_hex_byte(state.registers.reg_b);
  std::cout << ", ";
  std::cout << "C: ";
  print_hex_byte(state.registers.reg_c);
  std::cout << ", ";
  std::cout << "D: ";
  print_hex_byte(state.registers.reg_d);
  std::cout << ", ";
  std::cout << "E: ";
  print_hex_byte(state.registers.reg_e);
  std::cout << ", ";
  std::cout << "H: ";
  print_hex_byte(state.registers.reg_h);
  std::cout << ", ";
  std::cout << "L: ";
  print_hex_byte(state.registers.reg_l);
  std::cout << "\n";
  std::cout << "A: ";
  print_hex_byte(state.registers.reg_a);
  std::cout << "\n";
  std::cout << "Sign: " << +state.flags.sign << "\n";
  std::cout << "Zero: " << +state.flags.zero << "\n";
  std::cout << "Parity: " << +state.flags.parity << "\n";
  std::cout << "Carry: " << +state.flags.carry << "\n" << std::endl;
}

void CPU8080::print_instruction(uint8_t opcode) {
  // Get the next two bytes in case they're needed as immediate data.
  uint8_t b1 = mem_access_->read(program_counter_);
  uint8_t b2 = mem_access_->read(program_counter_ + 1);

  switch (opcode) {
    case 0x00:
      std::cout << "NOP" << std::endl;
      break;
    case 0x01:
      std::cout << "LXI B, d16\n";
      break;
    case 0x02:
      std::cout << "STAX B\n";
      break;
    case 0x03:
      std::cout << "INX B\n";
      break;
    case 0x04:
      std::cout << "INR B\n";
      break;
    case 0x05:
      std::cout << "DCR B\n";
      break;
    case 0x06:
      std::cout << "MOV B, D8 (";
      print_hex_byte(b1);
      std::cout << ")\n";
      break;
    case 0x07:
      std::cout << "RLC\n";
      break;
    case 0x08:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x09:
      std::cout << "DAD B\n";
      break;
    case 0x0A:
      std::cout << "LDAX B\n";
      break;
    case 0x0B:
      std::cout << "DCX B\n";
      break;
    case 0x0C:
      std::cout << "INR C\n";
      break;
    case 0x0D:
      std::cout << "DCR C\n";
      break;
    case 0x0E:
      std::cout << "MOV C, D8\n";
      break;
    case 0x0F:
      std::cout << "RRC" << std::endl;
      break;
    case 0x10:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x11:
      std::cout << "LXI D, D16\n";
      break;
    case 0x12:
      std::cout << "STAX D\n";
      break;
    case 0x13:
      std::cout << "INX D\n";
      break;
    case 0x14:
      std::cout << "INR D\n";
      break;
    case 0x15:
      std::cout << "DCR D\n";
      break;
    case 0x16:
      std::cout << "MOV D, D8\n";
      break;
    case 0x17:
      std::cout << "RAL \n";
      break;
    case 0x18:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x19:
      std::cout << "DAD D\n";
      break;
    case 0x1A:
      std::cout << "LDAX D\n";
      break;
    case 0x1B:
      std::cout << "DCX D\n";
      break;
    case 0x1C:
      std::cout << "INR E\n";
      break;
    case 0x1D:
      std::cout << "DCR E\n";
      break;
    case 0x1E:
      std::cout << "MOV E, d8\n";
      break;
    case 0x1F:
      std::cout << "RAR\n";
      break;
    case 0x20:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x21:
      std::cout << "LXI H, d16\n";
      break;
    case 0x22:
      std::cout << "SHLD D8\n";
      break;
    case 0x23:
      std::cout << "INX H" << std::endl;
      break;
    case 0x24:
      std::cout << "INR H" << std::endl;
      break;
    case 0x25:
      std::cout << "DCR H" << std::endl;
      break;
    case 0x26:
      std::cout << "MVI H, d8\n";
      break;
    case 0x27:
      std::cout << "DAA" << std::endl;
      break;
    case 0x28:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x29:
      std::cout << "DAD H" << std::endl;
      break;
    case 0x2A:
      std::cout << "LHLD d8\n";
      break;
    case 0x2B:
      std::cout << "DCX H" << std::endl;
      break;
    case 0x2C:
      std::cout << "INR L" << std::endl;
      break;
    case 0x2D:
      std::cout << "DCR L" << std::endl;
      break;
    case 0x2E:
      std::cout << "MVI L, d8\n";
      break;
    case 0x2F:
      std::cout << "CMA" << std::endl;
      break;
    case 0x30:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x31:
      std::cout << "LXI SP, d16 \n";
      break;
    case 0x32:
      std::cout << "STA, d16\n";
      break;
    case 0x33:
      std::cout << "INX SP" << std::endl;
      break;
    case 0x34:
      std::cout << "INR M" << std::endl;
      break;
    case 0x35:
      std::cout << "DCR M" << std::endl;
      break;
    case 0x36:
      std::cout << "MVI M, d8\n";
      break;
    case 0x37:
      std::cout << "STC" << std::endl;
      break;
    case 0x38:
      std::cout << "NOP*\n";
      break;
    case 0x39:
      std::cout << "DAD SP" << std::endl;
      break;
    case 0x3A:
      std::cout << "LDA ";
      print_hex_byte(b2);
      print_hex_byte(b1);
      std::cout << "\n";
      break;
    case 0x3B:
      std::cout << "DCX SP" << std::endl;
      break;
    case 0x3C:
      std::cout << "INR A" << std::endl;
      break;
    case 0x3D:
      std::cout << "DCR A" << std::endl;
      break;
    case 0x3E:  // MVI A, D8
      std::cout << "MVI A, D8" << std::endl;
      break;
    case 0x3F:
      std::cout << "CMC" << std::endl;
      break;
    case 0x40:
      std::cout << "MOV B, B\n";
      break;
    case 0x41:
      std::cout << "MOV B, C\n";
      break;
    case 0x42:
      std::cout << "MOV B, D\n";
      break;
    case 0x43:
      std::cout << "MOV B, E\n";
      break;
    case 0x44:
      std::cout << "MOV B, H\n";
      break;
    case 0x45:
      std::cout << "MOV B, L\n";
      break;
    case 0x46:
      std::cout << "MOV B, M\n";
      break;
    case 0x47:
      std::cout << "MOV B, A\n";
      break;
    case 0x48:
      std::cout << "MOV C, B\n";
      break;
    case 0x49:
      std::cout << "MOV c, c\n";
      break;
    case 0x4A:
      std::cout << "MOV c, d\n";
      break;
    case 0x4B:
      std::cout << "MOV c, e\n";
      break;
    case 0x4C:
      std::cout << "MOV c, h\n";
      break;
    case 0x4D:
      std::cout << "MOV c, l\n";
      break;
    case 0x4E:
      std::cout << "MOV c, m\n";
      break;
    case 0x4F:
      std::cout << "MOV c, a\n";
      break;
    case 0x50:
      std::cout << "MOV d, C\n";
      break;
    case 0x51:
      std::cout << "MOV D, C\n";
      break;
    case 0x52:
      std::cout << "MOV D, d\n";
      break;
    case 0x53:
      std::cout << "MOV D, e\n";
      break;
    case 0x54:
      std::cout << "MOV D, h\n";
      break;
    case 0x55:
      std::cout << "MOV D, L\n";
      break;
    case 0x56:
      std::cout << "MOV D, M\n";
      break;
    case 0x57:
      std::cout << "MOV D, C\n";
      break;
    case 0x58:
      std::cout << "MOV E, b\n";
      break;
    case 0x59:
      std::cout << "MOV E, C\n";
      break;
    case 0x5A:
      std::cout << "MOV E, d\n";
      break;
    case 0x5B:
      std::cout << "MOV E, e\n";
      break;
    case 0x5C:
      std::cout << "MOV E, h\n";
      break;
    case 0x5D:
      std::cout << "MOV E, l\n";
      break;
    case 0x5E:
      std::cout << "MOV E, M\n";
      break;
    case 0x5F:
      std::cout << "MOV E, a\n";
      break;
    case 0x60:
      std::cout << "MOV H, b\n";
      break;
    case 0x61:
      std::cout << "MOV H, C\n";
      break;
    case 0x62:
      std::cout << "MOV H, d\n";
      break;
    case 0x63:
      std::cout << "MOV H, e\n";
      break;
    case 0x64:
      std::cout << "MOV H, h\n";
      break;
    case 0x65:
      std::cout << "MOV H, l\n";
      break;
    case 0x66:
      std::cout << "MOV H, M\n";
      break;
    case 0x67:
      std::cout << "MOV H, A\n";
      break;
    case 0x68:
      std::cout << "MOV L, B\n";
      break;
    case 0x69:
      std::cout << "MOV L, C\n";
      break;
    case 0x6A:
      std::cout << "MOV L, D\n";
      break;
    case 0x6B:
      std::cout << "MOV L, E\n";
      break;
    case 0x6C:
      std::cout << "MOV L, H\n";
      break;
    case 0x6D:
      std::cout << "MOV L, L\n";
      break;
    case 0x6E:
      std::cout << "MOV L, M\n";
      break;
    case 0x6F:
      std::cout << "MOV L, a\n";
      break;
    case 0x70:
      std::cout << "MOV M, B\n";
      break;
    case 0x71:
      std::cout << "MOV M, C\n";
      break;
    case 0x72:
      std::cout << "MOV M, d\n";
      break;
    case 0x73:
      std::cout << "MOV M, e\n";
      break;
    case 0x74:
      std::cout << "MOV M, h\n";
      break;
    case 0x75:
      std::cout << "MOV M, l\n";
      break;
    case 0x76:
      std::cout << "HLT\n";
      break;
    case 0x77:
      std::cout << "MOV M, a\n";
      break;
    case 0x78:
      std::cout << "MOV A, b\n";
      break;
    case 0x79:
      std::cout << "MOV A, c\n";
      break;
    case 0x7A:
      std::cout << "MOV A, d\n";
      break;
    case 0x7B:
      std::cout << "MOV A, e\n";
      break;
    case 0x7C:
      std::cout << "MOV A, h\n";
      break;
    case 0x7D:
      std::cout << "MOV A, l\n";
      break;
    case 0x7E:
      std::cout << "MOV A, M\n";
      break;
    case 0x7F:
      std::cout << "MOV A, A\n";
      break;
    case 0x80:
      std::cout << "ADD B\n";
      break;
    case 0x81:
      std::cout << "ADD C\n";
      break;
    case 0x82:
      std::cout << "ADD d\n";
      break;
    case 0x83:
      std::cout << "ADD e\n";
      break;
    case 0x84:
      std::cout << "ADD h\n";
      break;
    case 0x85:
      std::cout << "ADD l\n";
      break;
    case 0x86:
      std::cout << "ADD M\n";
      break;
    case 0x87:
      std::cout << "ADD A\n";
      break;
    case 0x88:
      std::cout << "ADC b\n";
      adc(registers_.reg_b);
      break;
    case 0x89:
      std::cout << "ADC c\n";
      adc(registers_.reg_c);
      break;
    case 0x8A:
      std::cout << "ADC d\n";
      adc(registers_.reg_d);
      break;
    case 0x8B:
      std::cout << "ADC e\n";
      adc(registers_.reg_e);
      break;
    case 0x8C:
      std::cout << "ADC h\n";
      adc(registers_.reg_h);
      break;
    case 0x8D:
      std::cout << "ADC l\n";
      adc(registers_.reg_l);
      break;
    case 0x8E:
      std::cout << "ADC m\n";
      break;
    case 0x8F:
      std::cout << "ADC a\n";
      adc(registers_.reg_a);
      break;
    case 0x90:
      std::cout << "SUB B\n";
      sub(registers_.reg_b);
      break;
    case 0x91:
      std::cout << "SUB C\n";
      sub(registers_.reg_c);
      break;
    case 0x92:
      std::cout << "SUB d\n";
      sub(registers_.reg_d);
      break;
    case 0x93:
      std::cout << "SUB e\n";
      sub(registers_.reg_e);
      break;
    case 0x94:
      std::cout << "SUB h\n";
      sub(registers_.reg_h);
      break;
    case 0x95:
      std::cout << "SUB l\n";
      sub(registers_.reg_l);
      break;
    case 0x96:
      std::cout << "SUB M\n";
      break;
    case 0x97:
      std::cout << "SUB A\n";
      sub(registers_.reg_a);
      break;
    case 0x98:
      std::cout << "SUB B\n";
      sbb(registers_.reg_b);
      break;
    case 0x99:
      std::cout << "SUB C\n";
      sbb(registers_.reg_c);
      break;
    case 0x9A:
      std::cout << "SUB D\n";
      sbb(registers_.reg_d);
      break;
    case 0x9B:
      std::cout << "SUB E\n";
      sbb(registers_.reg_e);
      break;
    case 0x9C:
      std::cout << "SUB H\n";
      sbb(registers_.reg_h);
      break;
    case 0x9D:
      std::cout << "SUB L\n";
      sbb(registers_.reg_l);
      break;
    case 0x9E:
      std::cout << "SUB M\n";
      break;
    case 0x9F:
      std::cout << "SUB A\n";
      sbb(registers_.reg_a);
      break;
    case 0xA0:
      std::cout << "ANA B\n";
      break;
    case 0xA1:
      std::cout << "ANA c\n";
      break;
    case 0xA2:
      std::cout << "ANA d\n";
      break;
    case 0xA3:
      std::cout << "ANA e\n";
      break;
    case 0xA4:
      std::cout << "ANA h\n";
      break;
    case 0xA5:
      std::cout << "ANA l\n";
      break;
    case 0xA6:
      std::cout << "ANA M\n";
      break;
    case 0xA7:
      std::cout << "ANA a\n";
      break;
    case 0xA8:
      std::cout << "XRA B\n";
      break;
    case 0xA9:
      std::cout << "XRA C\n";
      break;
    case 0xAA:
      std::cout << "XRA D\n";
      break;
    case 0xAB:
      std::cout << "XRA E\n";
      break;
    case 0xAC:
      std::cout << "XRA H\n";
      break;
    case 0xAD:
      std::cout << "XRA L\n";
      break;
    case 0xAE:
      std::cout << "XRA M\n";
      break;
    case 0xAF:
      std::cout << "XRA A\n";
      break;
    case 0xB0:
      std::cout << "ORA B\n";
      break;
    case 0xB1:
      std::cout << "ORA C\n";
      break;
    case 0xB2:
      std::cout << "ORA D\n";
      break;
    case 0xB3:
      std::cout << "ORA E\n";
      break;
    case 0xB4:
      std::cout << "ORA H\n";
      break;
    case 0xB5:
      std::cout << "ORA L\n";
      break;
    case 0xB6:
      std::cout << "ORA M\n";
      break;
    case 0xB7:
      std::cout << "ORA A\n";
      break;
    case 0xB8:
      std::cout << "CMP B\n";
      break;
    case 0xB9:
      std::cout << "CMP c\n";
      break;
    case 0xBA:
      std::cout << "CMP d\n";
      break;
    case 0xBB:
      std::cout << "CMP e\n";
      break;
    case 0xBC:
      std::cout << "CMP h\n";
      break;
    case 0xBD:
      std::cout << "CMP l\n";
      break;
    case 0xBE: {
      std::cout << "CMP M\n";
      break;
    }
    case 0xBF:
      std::cout << "CMP A\n";
      break;
    case 0xC0:
      std::cout << "RNZ\n";
      break;
    case 0xC1:
      std::cout << "POP B\n";
      break;
    case 0xC2: {
      std::cout << "JNZ\n";
    } break;
    case 0xC3: {
      std::cout << "JMP ";
      print_hex_byte(b2);
      print_hex_byte(b1);
      std::cout << "\n";
    } break;
    case 0xC4:
      std::cout << "CNZ\n";
      break;
    case 0xC5:
      std::cout << "PUSH B\n";
      break;
    case 0xC6:
      std::cout << "ADI D8\n";
      break;
    case 0xC7:
      std::cout << "RST 0\n";
      break;
    case 0xC8:
      std::cout << "RZ\n";
      break;
    case 0xC9:
      std::cout << "RET\n";
      break;
    case 0xCA: {
      std::cout << "JZ";
      print_hex_byte(b2);
      print_hex_byte(b1);
      std::cout << "\n";
      break;
    }
    case 0xCB:
      std::cout << "JMP";
      print_hex_byte(b2);
      print_hex_byte(b1);
      std::cout << "\n";
      break;
    case 0xCC:
      std::cout << "CZ\n";
      break;
    case 0xCD:
      std::cout << "CALL \n";
      break;
    case 0xCE:
      std::cout << "ACI, D8\n";
      break;
    case 0xCF:
      break;
    case 0xD0:
      std::cout << "RST 1\n";
      break;
    case 0xD1:
      std::cout << "POP D\n";
      break;
    case 0xD2:
      std::cout << "JNC\n";
      break;
    case 0xD3:
      std::cout << "OUT d8\n";
      break;
    case 0xD4:
      std::cout << "CNC\n";
      break;
    case 0xD5:
      std::cout << "PUSH D\n";
      break;
    case 0xD6:
      std::cout << "SUI d8\n";
      break;
    case 0xD7:
      std::cout << "RST 2\n";
      break;
    case 0xD8:
      std::cout << "RC\n";
      break;
    case 0xD9:
      std::cout << "RET\n";
      break;
    case 0xDA:
      std::cout << "JC\n";
      break;
    case 0xDB:  // IN instruction + D8 (input port number)
      std::cout << "IN d8\n";
      break;
    case 0xDC:
      std::cout << "CC\n";
      break;
    case 0xDD:
      std::cout << "*CALL A16\n";
      break;
    case 0xDE:
      std::cout << "SBI d8\n";
      break;
    case 0xDF:
      std::cout << "RST 3\n";
      break;
    case 0xE0:
      std::cout << "RPO\n";
      break;
    case 0xE1:
      std::cout << "POP H\n";
      break;
    case 0xE2:
      std::cout << "JPO\n";
      break;
    case 0xE3:
      std::cout << "XTHL\n";
      break;
    case 0xE4:
      std::cout << "CPO\n";
      break;
    case 0xE5:
      std::cout << "PUSH H\n";
      break;
    case 0xE6:
      std::cout << "ANI D8\n";
      break;
    case 0xE7:
      std::cout << "RST 4\n";
      break;
    case 0xE8:
      std::cout << "RPE\n";
      break;
    case 0xE9:
      std::cout << "PCHL\n";
      break;
    case 0xEA:
      std::cout << "JPE\n";
      break;
    case 0xEB:
      std::cout << "XCHG\n";
      break;
    case 0xEC:
      std::cout << "CPE\n";
      break;
    case 0xED:
      std::cout << "*CALL\n";
      break;
    case 0xEE:
      std::cout << "XRA\n";
      break;
    case 0xEF:
      std::cout << "RST 5\n";
      break;
    case 0xF0:
      std::cout << "RP\n";
      break;
    case 0xF1: {
      std::cout << "POP PSW\n";
      break;
    }
    case 0xF2:
      std::cout << "JP\n";
      break;
    case 0xF3:
      std::cout << "DI\n";
      break;
    case 0xF4:
      std::cout << "CP\n";
      break;
    case 0xF5:
      std::cout << "PUSH PSW\n";
      break;
    case 0xF6:
      std::cout << "ORA\n";
      break;
    case 0xF7:
      std::cout << "RST 6\n";
      break;
    case 0xF8:
      std::cout << "RM\n";
      break;
    case 0xF9:
      std::cout << "SPHL\n";
      break;
    case 0xFA:
      std::cout << "JM\n";
      break;
    case 0xFB:
      std::cout << "EI\n";
      break;
    case 0xFC:
      std::cout << "CM\n";
      break;
    case 0xFD:
      std::cout << "*CALL\n";
      break;
    case 0xFE:
      std::cout << "CPI\n";
      break;
    case 0xFF:
      std::cout << "RST 7\n";
      break;
    default:
      std::cerr << "CPU8080:execute() : default case for opcode switch\n";
      break;
  }
}
}  // namespace intel_8080
