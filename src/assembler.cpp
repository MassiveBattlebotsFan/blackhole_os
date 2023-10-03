#include <iostream>
#include <array>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <format>
#include <cstdint>
#include <cstring>

using namespace std::literals;

enum class OPERATORS{
    NUL = 0,
    INC,
    DEC,
    SET_L,
    SET_H,
    SET_ACC,
    ADD,
    SUB,
    XOR,
    OR,
    STO,
    RCL,
    SET_DREF,
    PUSH_A,
    POP_A,
    AND,
    JMP,
    JNZ,
    JEZ,
    CMP,
    OUT,
    IN,
    SET_L_A,
    SET_H_A,
    SET_A_L,
    SET_A_H,
    BAK,
    RES,
    JREL,
    PUSH_HL,
    POP_HL,
    EX_HL,
    RET,
    ADD_BAK,
    SUB_BAK,
    JGZ,
    JLZ
};

const std::unordered_map<std::string, OPERATORS> OP_CONV{
    {"NUL"s,OPERATORS::NUL},
    {"INC"s,OPERATORS::INC},
    {"DEC"s,OPERATORS::DEC},
    {"SET_L"s,OPERATORS::SET_L},
    {"SET_H"s,OPERATORS::SET_H},
    {"SET_ACC"s,OPERATORS::SET_ACC},
    {"ADD"s,OPERATORS::ADD},
    {"SUB"s,OPERATORS::SUB},
    {"XOR"s,OPERATORS::XOR},
    {"OR"s,OPERATORS::OR},
    {"STO"s,OPERATORS::STO},
    {"RCL"s,OPERATORS::RCL},
    {"SET_DREF"s,OPERATORS::SET_DREF},
    {"PUSH_A"s,OPERATORS::PUSH_A},
    {"POP_A"s,OPERATORS::POP_A},
    {"AND"s,OPERATORS::AND},
    {"JMP"s,OPERATORS::JMP},
    {"JNZ"s,OPERATORS::JNZ},
    {"JEZ"s,OPERATORS::JEZ},
    {"CMP"s,OPERATORS::CMP},
    {"NOP"s,OPERATORS::NUL},
    {"OUT"s,OPERATORS::OUT},
    {"IN"s,OPERATORS::IN},
    {"SET_L_A"s,OPERATORS::SET_L_A},
    {"SET_H_A"s,OPERATORS::SET_H_A},
    {"SET_A_L"s,OPERATORS::SET_A_L},
    {"SET_A_H"s,OPERATORS::SET_A_H},
    {"BAK"s,OPERATORS::BAK},
    {"RES"s,OPERATORS::RES},
    {"JREL"s,OPERATORS::JREL},
    {"PUSH_HL"s,OPERATORS::PUSH_HL},
    {"POP_HL"s,OPERATORS::POP_HL},
    {"EX_HL"s,OPERATORS::EX_HL},
    {"RET"s,OPERATORS::RET},
    {"ADD_BAK"s,OPERATORS::ADD_BAK},
    {"SUB_BAK"s,OPERATORS::SUB_BAK},
    {"JGZ"s,OPERATORS::JGZ},
    {"JLZ"s,OPERATORS::JLZ}
};

int main(int argc, char **argv){
    bool DEBUG_MESSAGES = false;
    uint16_t HEX_TARGET = 0x8000;
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " [flags] <source>\n";
        return 1;
    }
    for(size_t i = 1; i < argc; ++i){
        if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) DEBUG_MESSAGES = true;
        if(strcmp(argv[i], "--address") == 0 || strcmp(argv[i], "-a") == 0){
            if(i < argc - 1){
                HEX_TARGET = static_cast<uint16_t>(std::strtol(argv[++i], nullptr, 0));
                continue;
            }else{
                std::cerr << "Address required\n";
                return 4;
            }
        }
        if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
            std::cerr << "Usage: " << argv[0] << " [flags] <source>\nFlags:\n-h, --help\t\tPrints this message\n-v, --verbose\t\tEnables extra debug information\n-a, --address <addr>\tSets hex origin address\n";
            return 0;
        }
    }
    std::string input_filename = argv[argc - 1];
    std::ifstream source_file(input_filename /* source file arg */);
    if(!source_file.is_open()){
        source_file.close();
        std::cerr << "Error: Source file failed to open\n";
        return 2;
    }
    std::vector<std::string> source_file_lines;
    std::string tmp;
    while(!std::getline(source_file, tmp).eof()) source_file_lines.push_back(tmp);
    source_file_lines.push_back(tmp);
    source_file.close();

    std::string compiled_output;
    size_t current_addr = 0x80;
    std::unordered_map<std::string, size_t> labels;
    for(auto& line : source_file_lines){
        if(line.size() == 0) continue;
        if(line.at(0) == ';') continue;
        size_t pos = line.find(" ");
        std::string cmd = line.substr(0, pos);
        std::string arg;
        std::for_each(cmd.begin(), cmd.end(), [](char &c){ c = std::toupper(c); });
        //std::cout << "command: " << cmd << std::endl;
        if(pos != std::string::npos){
            arg = line.substr(pos+1);
            arg = arg.substr(0, arg.find(';'));
        }else{
            arg = "0";
        }
        if(cmd == "ORG"){
            uint16_t new_addr = static_cast<uint16_t>(std::strtol(arg.c_str(), nullptr, 0));
            if(new_addr > current_addr) current_addr = new_addr;
        }
        else if(cmd == "LBL"){
            arg.erase(std::remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
            labels[arg] = current_addr;
            std::cout << "Label " << arg << std::format(" = {:04X}", current_addr) << std::endl;
            line.at(0) = ';';
        }else if(cmd == "ADDR"){
            current_addr++;
        }else if(cmd == "WORD"){
            size_t temp_sep = arg.find(',');
            unsigned int num_seps = 1;
            while(temp_sep != std::string::npos){
                temp_sep = arg.find(',', temp_sep+1);
                num_seps++;
            }
            current_addr += num_seps;
        }else if(cmd == "STR"){
            size_t len = arg.size();
            if(len % 2) len++;
            else len += 2;
            len /= 2;
            current_addr += len;
        }else{
            current_addr++;
        }
    }
    current_addr = -1;
    for(auto& line : source_file_lines){
        if(line.size() == 0) continue;
        if(line.at(0) == ';') continue;
        size_t pos = line.find(" ");
        std::string cmd = line.substr(0, pos);
        std::string arg;
        std::for_each(cmd.begin(), cmd.end(), [](char &c){ c = std::toupper(c); });
        if(DEBUG_MESSAGES){
            std::cout << "command: " << cmd << std::endl;
        }
        if(pos != std::string::npos){
            arg = line.substr(pos+1);
            arg = arg.substr(0, arg.find(';'));
        }else{
            arg = "0";
        }
        if(cmd == "ORG"){
            uint16_t new_addr = static_cast<uint16_t>(std::strtol(arg.c_str(), nullptr, 0));
            if(DEBUG_MESSAGES){
                std::cout << std::format("Setting address to 0x{:04X} (real addr: 0x{:04X})\n", new_addr, new_addr*2);
            }
            if(current_addr == -1){
                current_addr = new_addr;
            } else {
                while(new_addr > current_addr){
                    compiled_output.push_back(0x0);
                    compiled_output.push_back(0x0);
                    current_addr++;
                }
            }
            continue;
        }
        if(current_addr == -1) current_addr = 0x80;
        if(auto cmd_exists = OP_CONV.find(cmd); cmd_exists != OP_CONV.end()){
            unsigned char uint8_arg;
            arg.erase(std::remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
            if(auto arg_exists = labels.find(arg); arg_exists != labels.end()){
                if(cmd_exists->second == OPERATORS::JNZ || cmd_exists->second == OPERATORS::JEZ || cmd_exists->second == OPERATORS::JREL || cmd_exists->second == OPERATORS::JGZ || cmd_exists->second == OPERATORS::JLZ){
                    if(DEBUG_MESSAGES){
                        std::cout << "Replacing " << arg_exists->first << " with " << static_cast<int16_t>(static_cast<char>(arg_exists->second - current_addr)) << std::endl;
                    }
                    uint8_arg = arg_exists->second - current_addr;
                    if(abs(static_cast<int>(static_cast<char>(uint8_arg))) > 60) std::cerr << "Warn addr " << current_addr << ": Jump to label \'" << arg_exists->first << "\' distance " << abs(static_cast<int>(static_cast<char>(uint8_arg))) << " > 60\n";
                }else if(cmd_exists->second == OPERATORS::SET_H){
                    uint8_arg = static_cast<unsigned char>(((arg_exists->second*2) & 0xFF00) >> 8);
                    if(DEBUG_MESSAGES){
                        std::cout << "Replacing " << arg_exists->first << std::format(" with {:02X}", static_cast<int16_t>(uint8_arg)) << std::endl;
                    }
                }else{
                    uint8_arg = static_cast<unsigned char>((arg_exists->second*2) & 0x00FF);
                    if(DEBUG_MESSAGES){
                        std::cout << "Replacing " << arg_exists->first << std::format(" with {:02X}", static_cast<int16_t>(uint8_arg)) << std::endl;
                    }
                }
            }else{
                uint8_arg = static_cast<unsigned char>(std::strtol(arg.c_str(), nullptr, 0));
            }
            if(cmd_exists->second == OPERATORS::JNZ || cmd_exists->second == OPERATORS::JEZ || cmd_exists->second == OPERATORS::JREL || cmd_exists->second == OPERATORS::JGZ || cmd_exists->second == OPERATORS::JLZ){
                uint8_arg -= 1;
                uint8_arg *= 2;
            }            
            compiled_output.push_back(static_cast<unsigned char>(cmd_exists->second));
            compiled_output.push_back(uint8_arg);
            current_addr++;
        }else if(cmd == "WORD"){
            if(DEBUG_MESSAGES){
                std::cout << "Inserting words\n";
            }
            size_t arg_last_sep_loc = 0;
            size_t arg_next_sep_loc = arg.find(',');
            std::string temp_arg;
            std::vector<uint16_t> word_list;
            do{
                temp_arg = arg.substr((arg_last_sep_loc == 0) ? 0 : arg_last_sep_loc+1, arg_next_sep_loc);
                arg_last_sep_loc = arg_next_sep_loc;
                arg_next_sep_loc = arg.find(',', arg_last_sep_loc+1);
                uint16_t temp_arg_int = static_cast<uint16_t>(std::strtol(temp_arg.c_str(), nullptr, 0));
                if(DEBUG_MESSAGES){
                    std::cout << std::format("{:04X}", temp_arg_int) << std::endl;
                }
                word_list.push_back(temp_arg_int);
            }while(arg_last_sep_loc != std::string::npos);
            if(DEBUG_MESSAGES){
                std::cout << std::format("Delimiter = 0xFF{:02X}\nEnd insert words\n", static_cast<uint16_t>(word_list.size())*2);
            }
            compiled_output.push_back(0xFF);
            compiled_output.push_back(static_cast<uint16_t>(word_list.size())*2);
            for(auto& i : word_list){
                compiled_output.push_back(static_cast<unsigned char>((i & 0xFF00) >> 8));
                compiled_output.push_back(static_cast<unsigned char>(i & 0x00FF));
                current_addr++;
            }
        }else if(cmd == "ADDR"){
            unsigned char byte_1 = 0;
            unsigned char byte_2 = 0;
            arg.erase(std::remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
            if(DEBUG_MESSAGES) std::cout << "Adding ADDR word\n";
            if(auto arg_exists = labels.find(arg); arg_exists != labels.end()){
                byte_1 = static_cast<unsigned char>((arg_exists->second*2) & 0x00FF);
                byte_2 = static_cast<unsigned char>(((arg_exists->second*2) & 0xFF00) >> 8);
                if(DEBUG_MESSAGES) std::cout << "Replacing " << arg_exists->first << std::format(" with bytes {:02X}, {:02X}\n", byte_1, byte_2);
            }else{
                std::cerr << "Error: Unrecognized label \'" << arg << "\' at address " << current_addr << " in ADDR command\n";
            }
            compiled_output.push_back(0xFF);
            compiled_output.push_back(0x02);
            compiled_output.push_back(byte_1);
            compiled_output.push_back(byte_2);
            current_addr++;
        }else if(cmd == "STR"){
            if(DEBUG_MESSAGES){
                std::cout << "Inserting string bytes\n";
            }
            uint16_t temp = 0;
            std::vector<uint16_t> word_list;
            for(size_t i = 0; i < arg.size(); ++i){
                if(i % 2){
                    temp |= static_cast<uint16_t>(arg.at(i))&0x00FF;
                    word_list.push_back(temp);
                    temp = 0;
                }else{
                    temp = (static_cast<uint16_t>(arg.at(i)) << 8) & 0xFF00;
                }
            }
            if(temp) word_list.push_back(temp); // just in case
            else word_list.push_back(0x0000);
            if(DEBUG_MESSAGES){
                std::cout << std::format("Delimiter = 0xFF{:02X}\nEnd insert words\n", static_cast<uint16_t>(word_list.size())*2);
            }
            compiled_output.push_back(0xFF);
            compiled_output.push_back(static_cast<uint16_t>(word_list.size())*2);
            for(auto& i : word_list){
                compiled_output.push_back(static_cast<unsigned char>((i & 0xFF00) >> 8));
                compiled_output.push_back(static_cast<unsigned char>(i & 0x00FF));
                current_addr++;
            }
        }else{
            std::cerr << "Error: Unrecognized opcode \'" << cmd << "\' at address " << current_addr << std::endl;
        }
    }
    compiled_output.push_back(0xFF);
    compiled_output.push_back(0xFF);
    if(DEBUG_MESSAGES){
    //std::cout << "Compiled bytestring:\n" << compiled_output << std::endl;
    }
    
    std::ofstream compiled_file(input_filename.substr(0,input_filename.find('.'))+".o", std::ios::trunc | std::ios::binary);
    if(!compiled_file.is_open()){
        compiled_file.close();
        std::cerr << "Error: Could not open bin output file\n";
        return 3;
    }
    for(auto& comp_byte : compiled_output){
        compiled_file.put(comp_byte);
    }
    compiled_file.close();

    std::string hex_string;
    std::string compiled_substring;
    uint16_t address = HEX_TARGET;
    uint16_t checksum = 0x00;
    while(compiled_output.size() > 0x20){
        compiled_substring = compiled_output.substr(0, 0x20);
        compiled_output.erase(0, 0x20);
        if(compiled_substring == std::string(0x20, 0x0)){
            address += 0x20;
            continue;
        }
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
    //std::cout << hex_string << std::endl;
    compiled_file.open(input_filename.substr(0,input_filename.find('.'))+".hex", std::ios::trunc | std::ios::binary);
    if(!compiled_file.is_open()){
        compiled_file.close();
        std::cerr << "Error: Could not open hex output file\n";
        return 4;
    }
    for(auto& hex_byte : hex_string){
        compiled_file.put(hex_byte);
    }
    compiled_file.close();
    return 0;
}