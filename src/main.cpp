/*
  Names:Aria Kafie, Htet Hnin Su Wai
  Class Accounts : cssc4055, cssc4043
  Class Information : CS 530, Spring 2024
  Assignment Information : Assignment #2, XE assembler program
  File name : main.cpp
*/
#include "optab.h"
#include "parse.h"
#include "types.h"

#include <iomanip>
#include <iostream>
#include <fstream>

void process_source(const std::string& path);

int main(int argc, char* argv[]) {

  if (argc == 1) { // throw an error if no file paths are provided
    std::cout << "error: please provide one or more file paths\n";
    return 1;
  }

  OpTab::init(); // initialize the opcode table

  for (int i = 1; i < argc; i++) // process every file passed with argv
    process_source(argv[i]);

  return 0;
}

void process_source(const std::string& path) {

  std::string filename = path.substr(0, path.find(".sic"));

  SymbolTable symtab; // table declarations
  LiteralTable littab;

  std::ofstream out(filename + ".l"); // file objects for reading/writing
  std::ofstream st_file(filename + ".st");
  std::ifstream source(path);

  if (!source.is_open()) { // throw an error in case of invalid filepath
    std::cout << "sorry, file open failed\n";
    return;
  }

  int location_counter = 0; // initialize locctr for pass 1
  std::string line; // holds the current line

  for (std::getline(source, line); is_a_comment(line); std::getline(source, line)); // get the first non-comment line

  location_counter = mnemonic_of(line) == "START" ? std::stoi(operand_of(line)) : 0; // extract START information from that line
  symtab.csect = { symbol_of(line), location_counter, 0 };

  do // first pass to fill up the symtab & littab
  {
    std::getline(source, line);

    if (is_a_comment(line)) // comments can be ignored on pass 1
      continue;

    if (line[0] == '*') {
      std::string literal_name = line.substr(12, line.substr(12).find("\'"));
      littab[literal_name].address = location_counter;
      location_counter += littab[literal_name].length;
      continue;
    }

    if (operand_prefix_of(line) == '=' && !littab.contains(line.substr(20, line.substr(20).find("\'")))) // check for a new literal
    { // found a new literal on this line
      char literal_prefix = line[18]; // save the prefix ('C' or 'X')

      std::string literal_name; // declare some variables for the littab
      int literal_value;
      int literal_length; // can't deduce the address until LTORG or end of source

      literal_name = literal_name_of(line);

      if (literal_prefix == 'X') {
        literal_value = std::stoi(literal_name, nullptr, 16); // parse the literal in hex mode
        literal_length = literal_name.size() / 2;
        littab.literals.push_back
        ({ literal_name, literal_value, NO_ADDR, literal_length, false, false, 'C' });
      }
      else if (literal_prefix == 'C') {
        literal_value = ascii_to_int(literal_name); // parse the literal in ascii mode
        literal_length = literal_name.size();
        littab.literals.push_back
        ({ literal_name, literal_value, NO_ADDR, literal_length, false, false, 'C' });
      }
    }

    if (!symbol_of(line).empty() && line[0] != '*') // let the symboltable handle a new symbol if this line has one
      symtab.handle(line, location_counter);

    if (mnemonic_of(line) == "END" || mnemonic_of(line) == "LTORG")
      littab.pool(location_counter); // assign addresses to all literals encountered thus far

    location_counter += instruction_length_of(line); // increment the location counter
  } while (mnemonic_of(line) != "END"); // stop reading once the END directive is reached

  symtab.csect.length = location_counter - symtab.csect.address; // calculate the length of the control section

  source.clear();
  source.seekg(0, std::ios::beg); // reset the file object to the first line for pass 2

  location_counter = symtab.csect.address; // reset the location counter
  int program_counter = location_counter; // inititalize the program counter
  int base = NO_ADDR; // set base register to NO_ADDR (-1) by default

  while (std::getline(source, line) && is_a_comment(line)); // ignore comments that come before the START directive

  out << std::left << std::setw(8) << to_hex_string(location_counter, 4) << line.substr(0, 35) << "\n"; // output the start line

  do // second pass to create the listing file
  {
    std::getline(source, line);

    if (operand_prefix_of(line) == '=')
      littab.set_visited(literal_name_of(line)); // mark any new literals as visited

    if (is_a_comment(line)) {
      out << "        " << line << "\n"; // print comments with proper spacing
      continue;
    }
    if (line[0] == '*') { // define the literal specified on this line
      std::string literal_name = line.substr(12, line.substr(12).find("\'"));
      littab[literal_name].defined = true;
      out << std::left << std::setw(8) << to_hex_string(location_counter, 4) << std::setw(43) << line << to_hex_string(littab[literal_name].value) << "\n";
      location_counter += littab[literal_name].length;
      program_counter += littab[literal_name].length;
      continue;
    }

    if (line.size() >= 36)
      line = line.substr(0, 35); // truncate the current line for safer processing

    if (mnemonic_of(line) == "BASE")
      base = symtab[operand_of(line)].value; // switch to base mode when necessary

    if (mnemonic_of(line) == "NOBASE") // switch back to pc mode when necessary
      base = NO_ADDR;

    program_counter += instruction_length_of(line); // increment the program_counter before any more processing

    if (mnemonic_of(line) == "END" || mnemonic_of(line) == "LTORG") // define literals
    { // define all literals encountered thus far
      out << "        " << line << "\n";
      for (Literal& l : littab.literals) {
        if (l.visited && !l.defined) { // only print inline definitions for literals we've already passed over (on pass 2), and that haven't already been defined
          l.defined = true;
          out
            << std::left << std::setw(8) << to_hex_string(location_counter, 4)   // handle formatting
            << "*        =" << l.prefix << "\'" << std::setw(31) << (l.name + "\'")
            << std::hex << std::uppercase << l.value << "\n";
          location_counter += l.length; // increment location counter and pc register
          program_counter += l.length;
        }
      }
      continue;
    }

    std::string object_code =
      generate_object_code(line, symtab, littab, location_counter, program_counter, base); // get object code to print

    if (mnemonic_of(line) != "LTORG") // handle object code formatting
      out
      << std::left << std::setw(8) << to_hex_string(location_counter, 4)
      << std::setw(43) << line << object_code << "\n";

    location_counter += instruction_length_of(line); // increment location counter

  } while (mnemonic_of(line) != "END"); // keep reading until END mnemonic

  st_file << symtab.to_string() << "\nLiteral Table\n" << littab.to_string(); // print symtab and littab to their corresponding file
}
