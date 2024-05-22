/*
  Names:Aria Kafie, Htet Hnin Su Wai
  Class Accounts : cssc4055, cssc4043
  Class Information : CS 530, Spring 2024
  Assignment Information : Assignment #2, XE assembler program
  File name : types.h
*/
#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <unordered_map>
#include <vector> // data structure libraries

#define NO_ADDR -1

std::string to_hex_string(int num, int width = 0);

enum Flag { RELATIVE, ABSOLUTE }; // symbol flags

struct CsectInfo { // all relevant info for a control section label
  std::string name;
  int address;
  int length;
};

struct SymbolInfo { // all relevant symbol info
  int value;
  Flag flag;
};

struct Literal { // all relevant literal info
  std::string name;
  int value;
  int address;
  int length;
  bool defined;
  bool visited;
  char prefix;
};

class LiteralTable { // literal table data structure
public:
  void set_visited(const std::string& name);
  void pool(int& locctr);
  std::vector<Literal> literals; // implemented as a dynamic array of literal objects
  bool contains(const std::string& name);
  std::string to_string();
  const Literal& operator[](const std::string& key) const;
  Literal& operator[](const std::string& key);
};

class SymbolTable { // symbol table data structure

public:
  void handle(const std::string& line, int locctr); // helper functions
  bool contains(const std::string& key) const;
  std::string to_string();

  CsectInfo csect; // control section label is a distinct data member
  std::unordered_map<std::string, SymbolInfo> table;

  const SymbolInfo& operator[](const std::string& key) const { // subscript operator overloads for reading/writing
    return table.find(key)->second;
  }

  SymbolInfo& operator[](const std::string& key) {
    return table[key];
  }
};

#endif
