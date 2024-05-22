/*
  Names:Aria Kafie, Htet Hnin Su Wai
  Class Accounts : cssc4055, cssc4043
  Class Information : CS 530, Spring 2024
  Assignment Information : Assignment #2, XE assembler program
  File name : optab.h
*/
#ifndef OPTAB_H
#define OPTAB_H

#include <string>
#include <unordered_map>

enum NIXBPE { // enumeration for addressing mode flags
  FLAG_N = 1 << 5,
  FLAG_I = 1 << 4,
  FLAG_X = 1 << 3,
  FLAG_B = 1 << 2,
  FLAG_P = 1 << 1,
  FLAG_E = 1
};

enum InstructionFormat {
  FORMAT_1 = 1, FORMAT_2, FORMAT_3, FORMAT_4 // enumeration for all possible instruction formats
};

struct OpInfo { // all relevant info pertaining to a certain opcode
  InstructionFormat format;
  int opcode;
};

extern std::unordered_map<std::string, OpInfo> optab; // global optab object
extern std::unordered_map<std::string, int> register_table; // global register table object

namespace OpTab {
  void init(); // global initialization function
}

#endif
