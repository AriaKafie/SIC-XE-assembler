/*
  Names:Aria Kafie, Htet Hnin Su Wai
  Class Accounts : cssc4055, cssc4043
  Class Information : CS 530, Spring 2024
  Assignment Information : Assignment #2, XE assembler program
  File name : parse.h
*/
#ifndef PARSE_H
#define PARSE_H

#include "types.h"

#include <string>

Flag evaluate_flag(const std::string& expr, const SymbolTable& stab); // helper for getting the flag type of a symbol

int evaluate_expr(const std::string& expr, const SymbolTable& stab, int locctr); // helper for parsing and evaluating an expression
int ascii_to_int(const std::string& ascii); // helper for converting an ascii string to its integer equivalent
int instruction_length_of(const std::string& line); // helper for getting the byte length of an instruction

char mnemonic_prefix_of(const std::string& line); // helper for getting the char preceding a mnemonic
char operand_prefix_of(const std::string& line); // gets the char preceding a line's operand field

bool is_a_comment(const std::string& line); // checks if a line is a comment or not

std::string generate_object_code(const std::string& line, // helper for getting a hex string of object code from a line
                                 const SymbolTable& stab,
                                 const LiteralTable& littab,
                                 int locctr,
                                 int program_counter,
                                 int base);

std::string symbol_of(const std::string& line); // returns the symbol of a line
std::string mnemonic_of(const std::string& line); // returns the mnemonic of a line
std::string operand_of(const std::string& line); // returns the operand string of a line
std::string literal_name_of(const std::string& line); // returns the literal name of a line

#endif
