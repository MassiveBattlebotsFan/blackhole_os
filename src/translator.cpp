#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <format>
#include <unordered_map>

/*
mapping for z80:

00 00  -> 00 00
01 00  -> 23 00
02 00  -> 2B 00
03 arg -> 2E arg
04 arg -> 26 arg
05 arg -> 3E arg
06 arg -> C6 arg
07 arg -> D6 arg
08 arg -> EE arg
09 arg -> F6 arg
0A 00  -> 77 00
0B 00  -> 7E 00
0C arg -> 36 arg
0D 00  -> F5 00
0E 00  -> F1 00
0F arg -> E6 arg
10 00  -> E9 00
11 arg -> 20 arg
12 arg -> 28 arg
13 arg -> FE arg
14 00  -> E7 00
15 00  -> DF 00
16 00  -> 6F 00
17 00  -> 67 00
18 00  -> 7D 00
19 00  -> 7C 00
1A 00  -> 47 00
1B 00  -> 78 00
1C arg -> 18 arg
1D 00  -> E5 00
1E 00  -> E1 00
1F 00  -> EB 00
20 00  -> C9 00
*/

const std::unordered_map<int, int> Z80_OPCODES = {
    {0x00, 0x00},
    {0x01, 0x23},
    {0x02, 0x2B},
    {0x03, 0x2E},
    {0x04, 0x26},
    {0x05, 0x3E},
    {0x06, 0xC6},
    {0x07, 0xD6},
    {0x08, 0xEE},
    {0x09, 0xF6},
    {0x0A, 0x77},
    {0x0B, 0x7E},
    {0x0C, 0x36},
    {0x0D, 0xF5},
    {0x0E, 0xF1},
    {0x0F, 0xE6},
    {0x10, 0xE9},
    {0x11, 0x20},
    {0x12, 0x28},
    {0x13, 0xFE},
    {0x14, 0xE7},
    {0x15, 0xDF},
    {0x16, 0x6F},
    {0x17, 0x67},
    {0x18, 0x7D},
    {0x19, 0x7C},
    {0x1A, 0x47},
    {0x1B, 0x78},
    {0x1C, 0x18},
    {0x1D, 0xE5},
    {0x1E, 0xE1},
    {0x1F, 0xEB},
    {0x20, 0xC9}
};

int main(int argc, char **argv){
    if(argc < 3){
        std::cerr << "Usage: " << argv[0] << " <source> <dest>\n";
        return 1;
    }
    std::ifstream source_file(argv[1] /* source file arg */);
    if(!source_file.is_open()){
        source_file.close();
        std::cerr << "Error: Source file failed to open\n";
        return 2;
    }
    std::string source_data;
    char tmp;
    while(!source_file.get(tmp).eof()) source_data.push_back(tmp);
    //source_data.push_back(tmp);
    source_file.close();

    std::string compiled_output;
    for(size_t i = 0; i < source_data.size(); i += 2){
        compiled_output.push_back(static_cast<char>(
            Z80_OPCODES.at(static_cast<int>(source_data.at(i)))
            ));
        compiled_output.push_back(source_data.at(i+1));
    }

    std::string hex_string;
    std::string compiled_substring;
    uint16_t address = 0x8000;
    uint16_t checksum = 0x00;
    while(compiled_output.size() > 0x20){
        compiled_substring = compiled_output.substr(0, 0x20);
        compiled_output.erase(0, 0x20);
        checksum = (0x20 + ((address & 0xFF00)>>8) + (address & 0x00FF)) & 0x00FF;
        hex_string += std::format(":20{:04X}00", address);
        for(char& i : compiled_substring){
            hex_string += std::format("{:02X}", static_cast<unsigned char>(i));
            checksum += static_cast<unsigned char>(i);
            checksum &= 0x00FF;
        }
        checksum = ((checksum ^ 0x00FF) + 1) & 0x00FF;
        hex_string += std::format("{:02X}\n", static_cast<unsigned char>(checksum));
        address += 0x20;
    }
    checksum = (static_cast<unsigned char>(compiled_output.size()) + ((address & 0xFF00)>>8) + (address & 0x00FF)) & 0x00FF;
    hex_string += std::format(":{:02X}{:04X}00", static_cast<unsigned char>(compiled_output.size()), address);
    for(char& i : compiled_output){
        hex_string += std::format("{:02X}", static_cast<unsigned char>(i));
        checksum += static_cast<unsigned char>(i);
        checksum &= 0x00FF;
    }
    checksum = ((checksum ^ 0x00FF) + 1) & 0x00FF;
    hex_string += std::format("{:02X}\n", static_cast<unsigned char>(checksum));
    hex_string += ":00000001FF";
    std::cout << hex_string << std::endl;
    std::ofstream compiled_file(argv[2], std::ios::trunc | std::ios::binary);
    if(!compiled_file.is_open()){
        compiled_file.close();
        std::cerr << "Error: Could not open output file\n";
        return 3;
    }
    for(auto& hex_byte : hex_string){
        compiled_file.put(hex_byte);
    }
    compiled_file.close();
    return 0;
}
