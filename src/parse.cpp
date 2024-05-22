/*
  Names:Aria Kafie, Htet Hnin Su Wai
  Class Accounts : cssc4055, cssc4043
  Class Information : CS 530, Spring 2024
  Assignment Information : Assignment #2, XE assembler program
  File name : parse.cpp
*/
#include "optab.h"
#include "parse.h"

#include <iomanip>
#include <sstream>

bool is_numeric(const std::string& s); // helper to check if a string is numeric

std::string generate_object_code(const std::string& line, // generates an object code string from a given line of code
  const SymbolTable& stab,
  const LiteralTable& littab,
  int locctr,
  int pc,
  int base)
{
  if (optab.find(mnemonic_of(line)) == optab.end()) // return an empty string if the mnemonic can't be found
    return "";

  std::string operand = operand_of(line); // extract various data from the line of code

  OpInfo info = optab[mnemonic_of(line)];

  char mnemonic_prefix = mnemonic_prefix_of(line);
  char operand_prefix = operand_prefix_of(line);

  int instruction_format = mnemonic_prefix == '+' ? FORMAT_4 : info.format;
  int obj = info.opcode << 8 * (instruction_format - 1); // initialize object code to the opcode shifted left by some varying amount, determined by the instruction format

  int disp = 0; // initialize the displacement bits to 0

  std::stringstream ss;
  ss << std::right << std::setw(instruction_format * 2) << std::setfill('0') << std::hex << std::uppercase; // initialize a stringstream object for formatting

  if (instruction_format == FORMAT_1)
  {
    ss << obj; // return just the opcode if format_1
    return ss.str();
  }

  if (instruction_format == FORMAT_2)
  { // else if format_2, handle register mnemonics/inline constants
    std::string operand_1 = operand.substr(0, operand.find(","));
    std::string operand_2 = operand.find(",") == std::string::npos ? "" : operand.substr(operand.find(",") + 1);

    obj |= register_table[operand_1] << 4; // use the lookup functionality of the register table

    if (!operand_2.empty())
      obj |= is_numeric(operand_2) ? std::stoi(operand_2) : register_table[operand_2]; // operand_2 could possibly be numeric

    ss << obj;
    return ss.str();
  }

  if (operand_prefix == '=')
    operand = literal_name_of(line); // check for a literal

  if (instruction_format == FORMAT_3)
  {
    if (mnemonic_of(line) == "RSUB") // RSUB can be hard coded
      return "4F0000";

    obj |= (base == NO_ADDR ? FLAG_P : FLAG_B) << 12; // set b or p flags accordingly

    if (operand_prefix == '@') // set n and i flags if necessary
      obj |= FLAG_N << 12;
    else if (operand_prefix == '#')
      obj |= FLAG_I << 12;
    else
      obj |= (FLAG_N | FLAG_I) << 12;

    if (operand.find(",X") != std::string::npos) {
      obj |= FLAG_X << 12;
      operand = operand.substr(0, operand.find(",")); // set the x flag if necessary
    }

    if (obj & (FLAG_I << 12) && !(obj & (FLAG_N << 12))) // disable pc relative mode if in immediate mode
      obj &= ~(FLAG_P << 12);

    if (!operand.empty())
      if ((obj & (FLAG_B << 12)) && (evaluate_expr(operand, stab, locctr) < base)) // swap the b and p flags if base indexing isn't possible
        obj ^= ((FLAG_B | FLAG_P) << 12);

    if (operand_prefix == '=') // adjust the displacement bits according to the flag values
      disp = littab[operand].address - (operand_prefix == '#' ? 0 : (obj & (FLAG_B << 12)) ? base : pc);
    else if (operand_prefix == '#')
      disp = evaluate_expr(operand, stab, locctr);
    else if (obj & (FLAG_B << 12))
      disp = evaluate_expr(operand, stab, locctr) - base;
    else if (obj & (FLAG_P << 12))
      disp = evaluate_expr(operand, stab, locctr) - pc;

    obj |= disp & 0xfff; // mask the bits in case of negative numbers (2's complement)
  }

  if (instruction_format == FORMAT_4)
  {
    obj |= FLAG_E << 20; // set the 'e' flag for format 4

    if (operand_prefix == '@') // set the n and i flags if necessary
      obj |= FLAG_N << 20;
    else if (operand_prefix == '#')
      obj |= FLAG_I << 20;
    else
      obj |= (FLAG_N | FLAG_I) << 20;

    if (operand_prefix == '=')
      obj |= littab[operand].address; // handle inline literals
    else
      obj |= evaluate_expr(operand, stab, locctr) & 0xfffff; // mask the bits in case of negative numbers
  }

  ss << obj; // convert into a properly formatted hex string
  return ss.str();
}

std::string literal_name_of(const std::string& line) {
  return line.substr(20, line.substr(20).find("\'")); // helper to extract the literal name from a line
}

void trim(std::string& s) { // helper to remove leading and trailing whitespace from a string
  size_t begin = s.find_first_not_of(" \t\r");
  size_t end = s.find_last_not_of(" \t\r");
  s = s.substr(begin, end - begin + 1);
}

bool is_numeric(const std::string& s) { // helper to check if a string is numeric
  for (char c : s)
    if (!std::isdigit(c))
      return false;
  return true;
}

int evaluate_term(std::string term, const SymbolTable& stab, int locctr) { // helper to statically evaluate a single term
  trim(term);
  if (term == "*")
    return locctr;
  if (is_numeric(term))
    return std::stoi(term);
  if (stab.contains(term))
    return stab[term].value;
}

size_t least_precedence_operator_pos(const std::string& str) { // helper to determine the character index of a string's least precedence operator
  if (str.find_first_of("+-") != std::string::npos) {
    for (int i = str.size() - 1; i >= 0; i--)
      if (str[i] == '+' || str[i] == '-')
        return i;
  }
  else {
    for (int i = str.size() - 1; i >= 0; i--)
      if (str[i] == '*' || str[i] == '/')
        return i;
  }
}

Flag evaluate_flag(const std::string& expr, const SymbolTable& stab) { // helper to deduce the flag of an expression

  int relative_additions = 0, relative_subtractions = 0;

  std::stringstream token;
  char c, last_operator;
  std::istringstream is(expr);

  is >> std::skipws;

  while (is >> c && std::string("*+-/").find(c) == std::string::npos) // consume the first token
    token << c;
  last_operator = c;

  if (stab.contains(token.str()) && stab[token.str()].flag == RELATIVE) // if it was a label, consider it an addition
    relative_additions++;
  token.str("");

  while (is >> c) { // continue consuming tokens until none are left
    if (std::string("*+-/").find(c) != std::string::npos) {
      if (stab.contains(token.str()) && stab[token.str()].flag == RELATIVE) {
        if (last_operator == '+') // check for additions
          relative_additions++;
        else if (last_operator == '-') // check for subtractions
          relative_subtractions++;
      }
      last_operator = c;
      token.str("");
    }
    else
      token << c;
  }

  if (stab.contains(token.str()) && stab[token.str()].flag == RELATIVE) { // check the flags of previously evaluated symbols
    if (last_operator == '+')
      relative_additions++;
    else if (last_operator == '-')
      relative_subtractions++;
  }

  return relative_additions == relative_subtractions ? ABSOLUTE : RELATIVE; // return absolute only if the number of relative additions and subtractions was equal
}

int evaluate_expr(const std::string& expr, const SymbolTable& stab, int locctr) { // recursively evaluate an expression using left to right associativity and operator precedence

  if (expr.find_first_of("+*-/") == std::string::npos || expr.find_first_not_of("* \t\r") == std::string::npos)
    return evaluate_term(expr, stab, locctr);

  size_t split = least_precedence_operator_pos(expr); // recursively split the expression at its least precedence operator position as to evaluate only those expressions with highest precedence first

  switch (expr[split])
  {
  case '+': return evaluate_expr(expr.substr(0, split), stab, locctr) + evaluate_expr(expr.substr(split + 1), stab, locctr);
  case '-': return evaluate_expr(expr.substr(0, split), stab, locctr) - evaluate_expr(expr.substr(split + 1), stab, locctr);
  case '*': return evaluate_expr(expr.substr(0, split), stab, locctr) * evaluate_expr(expr.substr(split + 1), stab, locctr);
  case '/': return evaluate_expr(expr.substr(0, split), stab, locctr) / evaluate_expr(expr.substr(split + 1), stab, locctr);
  }
}

int ascii_to_int(const std::string& ascii) { // helper to convert an ascii string into its integer equivalent
  int value = 0;
  for (char c : ascii) {
    value <<= 8;
    value |= static_cast<unsigned char>(c);
  }
  return value;
}

int instruction_length_of(const std::string& line) { // helper to determine the length in bytes of an instruction

  std::string mnemonic = mnemonic_of(line);

  if (optab.find(mnemonic) != optab.end()) // if in the optab, simply return the format
    return mnemonic_prefix_of(line) == '+' ? 4 : optab[mnemonic].format;
  else if (mnemonic == "WORD") // if WORD, we know it is just 3 bytes
    return 3;

  std::string operand = operand_of(line);

  if (mnemonic == "RESW") // if RESW, it must be 3 * the number of words allocated
    return 3 * std::stoi(operand);
  else if (mnemonic == "RESB") // if RESB, it is simply number of bytes allocated
    return std::stoi(operand);
  else if (mnemonic == "BYTE") // else it is the size of a BYTE constant
    return 3 * (operand.find_last_of("\'") - operand.find("\'") - 1);

  return 0;
}

bool is_a_comment(const std::string& line) { // helper that checks if a line is a comment
  return line[0] == '.';
}

std::string symbol_of(const std::string& line) { // helper to extract the symbol from a line
  return std::isspace(line[0]) ? "" : line.substr(0, line.find_first_of(" \t\r"));
}

std::string mnemonic_of(const std::string& line) { // helper to extract the mnemonic from a line
  return line.size() < 11 || is_a_comment(line) ? "" : line.substr(10, line.substr(10).find_first_of(" \t\r"));
}

bool is_whitespace(const std::string str) { // helper to check if a string is just whitespace
  for (char c : str)
    if (!std::isspace(c))
      return false;
  return true;
}

char mnemonic_prefix_of(const std::string& line) { // helper to extract the char preceding a line's mnemonic
  return line.size() < 10 ? ' ' : line[9];
}

char operand_prefix_of(const std::string& line) { // helper to extract the char preceding a line's operand
  return line.size() < 18 ? ' ' : line[17];
}

std::string operand_of(const std::string& line) { // helper to extract the operand of a line of code

  if (line.size() >= 42 && !is_whitespace(line.substr(41)))
    return line.substr(18, line.substr(18, 23).find_last_not_of(" \t\r") + 1);

  return line.size() < 19 || is_whitespace(line.substr(18)) ? "" : line.substr(18, line.substr(18).find_last_not_of(" \t\r") + 1);
}
