/*
  Names:Aria Kafie, Htet Hnin Su Wai
  Class Accounts : cssc4055, cssc4043
  Class Information : CS 530, Spring 2024
  Assignment Information : Assignment #2, XE assembler program
  File name : types.cpp
*/
#include "types.h"

#include "parse.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

Literal& LiteralTable::operator[](const std::string& key) {
  for (Literal& l : literals)
    if (l.name == key) // return a read only literal object whose name corresponds to 'key'
      return l;
}

const Literal& LiteralTable::operator[](const std::string& key) const {
  for (const Literal& l : literals)
    if (l.name == key) // return a read only literal object whose name corresponds to 'key'
      return l;
}

void LiteralTable::set_visited(const std::string& name) {
  for (Literal& l : literals)
    if (l.name == name) // mark the literal with name 'name' as visited (useful for LTORG)
      l.visited = true;
}

void SymbolTable::handle(const std::string& line, int locctr) { // handles a line containing a symbol definition
  if (line.find("EQU") == std::string::npos)
    table[symbol_of(line)] = { locctr, RELATIVE }; // symbols defined without EQU can be handled differently
  else
    table[symbol_of(line)] = { evaluate_expr(operand_of(line), *this, locctr), evaluate_flag(operand_of(line), *this) };
}

bool SymbolTable::contains(const std::string& key) const {
  return table.find(key) != table.end(); // checks if a symbol name is in the table
}

std::string LiteralTable::to_string() {

  std::stringstream ss; // converts the literal table into a string
  ss << std::left << std::uppercase << std::hex << "Name  Operand   Address  Length:\n--------------------------------\n";

  for (Literal l : literals)
    ss << std::hex << std::setw(6) << l.name << std::setw(10) << l.value << std::setw(9) << l.address << l.length << "\n";

  return ss.str();
}

bool LiteralTable::contains(const std::string& name) {
  for (Literal l : literals) // checks if a literal is in the table
    if (l.name == name)
      return true;
  return false;
}

std::string SymbolTable::to_string() {

  std::stringstream ss; // converts the symbol table into a string

  ss << std::left << "CSect   Symbol  Value   LENGTH  Flags:\n--------------------------------------\n"
    << std::setw(16) << csect.name << to_hex_string(csect.address, 6) << "  " << to_hex_string(csect.length, 6) << "\n";

  std::vector<std::pair<std::string, SymbolInfo>> stab_sorted(table.begin(), table.end());

  std::sort(stab_sorted.begin(), stab_sorted.end(),
    [](const std::pair<std::string, SymbolInfo>& a, const std::pair<std::string, SymbolInfo>& b) { return a.second.value < b.second.value; });

  for (auto& pair : stab_sorted)
    ss << "        " << std::setw(8) << pair.first << std::setw(16) << to_hex_string(pair.second.value, 6) << "RA"[pair.second.flag] << "\n";

  return ss.str();
}

std::string to_hex_string(int num, int width) {
  std::stringstream ss; // converts an integer to its corresponding hex string, with optional fixed width and zero-fill
  if (width != 0)
    ss << std::right << std::setw(width) << std::hex << std::uppercase << std::setfill('0') << num;
  else
    ss << std::hex << std::uppercase << num;
  return ss.str();
}

void LiteralTable::pool(int& locctr) {
  for (Literal& l : literals) { // computes all currently unresolved literal addresses
    if (l.address == NO_ADDR) {
      l.address = locctr;
      locctr += l.length;
    }
  }
}
