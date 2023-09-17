#include <iostream>
#include <vector>
#include <format>
#include <string>
#include <cstdint>
#include <array>
#include <atomic>
#include <thread>
#include <fstream>
#include <chrono>
#include <ncurses/ncurses.h>

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
    SUB_BAK
};

class Machine{
protected:
    std::atomic_bool debug = false;
    bool zero = false;
    uint16_t origin = 0x0;
    uint16_t address = 0x0, reg_hl = 0x0, reg_hl_bak = 0x0;
    uint8_t reg_acc = 0x0, reg_bak = 0x0;
    std::array<unsigned char, 0x10000> memory;
    std::atomic_bool run_exec = false;
    std::atomic_bool stop_execution = false;
    std::thread *exec_thread = nullptr;
    std::vector<uint16_t> stack;
    void exec();
    void step();
public:
    Machine();
    virtual ~Machine();
    std::atomic_bool is_finished = false;
    template<size_t ArraySize>
    void setMemory(std::array<unsigned char, ArraySize> data);
    void setMemory(std::string data);
    void setOrigin(uint16_t new_origin){ this->origin = new_origin; }
    void start(){ this->run_exec = true; this->is_finished = false; }
    void stop(){ this->run_exec = false; this->is_finished = true; }
    void reset();
    void setDebug(bool new_debug){ this->debug = new_debug; }
};

Machine::Machine(){
    this->reset();
    this->exec_thread = new std::thread(Machine::exec, this);
}

Machine::~Machine(){
    this->run_exec = false;
    this->stop_execution = true;
    this->exec_thread->join();
    delete this->exec_thread;
    this->exec_thread = nullptr;
}

template<size_t ArraySize>
void Machine::setMemory(std::array<unsigned char, ArraySize> data){
    this->run_exec = false;
    for(auto& i : data){
        this->memory.at(this->address++) = i;
    }
}
void Machine::setMemory(std::string data){
    this->run_exec = false;
    for(size_t i = 0; i < data.size() - 1; i += 2){
        unsigned char command = data.at(i);
        unsigned char arg = data.at(i+1);
        if(!(command == 0xFF && arg < 0xFF)){
            this->memory[this->address++] = command;
            this->memory[this->address++] = arg;
        }
    }
}

void Machine::reset(){
    this->run_exec = false;
    this->is_finished = false;
    this->zero = false;
    this->address = this->origin;
    this->reg_hl = 0x0;
    this->reg_hl_bak = 0x0;
    this->reg_acc = 0x0;
    this->reg_bak = 0x0;
    this->stack.clear();
}

void Machine::step(){
    unsigned char int_cmd = this->memory.at(this->address++);
    unsigned char arg = this->memory.at(this->address++);
    OPERATORS command = (int_cmd == 0xFF && arg == 0xFF) ? OPERATORS::RET : static_cast<OPERATORS>(int_cmd);
    switch(command){
    case OPERATORS::INC:
        this->reg_hl++;
        break;
    case OPERATORS::DEC:
        this->reg_hl--;
        break;
    case OPERATORS::SET_L:
        this->reg_hl = (this->reg_hl & 0xFF00) + arg;
        break;
    case OPERATORS::SET_H:
        this->reg_hl = (this->reg_hl & 0x00FF) + (static_cast<uint16_t>(arg) << 8);
        break;
    case OPERATORS::SET_ACC:
        this->reg_acc = arg;
        break;
    case OPERATORS::ADD:
        this->reg_acc += arg;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::SUB:
        this->reg_acc -= arg;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::XOR:
        this->reg_acc ^= arg;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::OR:
        this->reg_acc |= arg;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::STO:
        this->memory.at(this->reg_hl) = this->reg_acc;
        break;
    case OPERATORS::RCL:
        this->reg_acc = this->memory.at(this->reg_hl);
        break;
    case OPERATORS::SET_DREF:
        this->memory.at(this->reg_hl) = arg;
        break;
    case OPERATORS::PUSH_A:
        this->stack.push_back(static_cast<uint16_t>(this->reg_acc));
        break;
    case OPERATORS::POP_A:
        this->reg_acc = static_cast<unsigned char>(this->stack.back() & 0x00FF);
        this->stack.pop_back();
        break;
    case OPERATORS::AND:
        this->reg_acc &= arg;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::JMP:
        this->address = this->reg_hl;
        break;
    case OPERATORS::JNZ:
        if(!this->zero) this->address += static_cast<signed char>(arg);
        break;
    case OPERATORS::JEZ:
        if(this->zero) this->address += static_cast<signed char>(arg);
        break;
    case OPERATORS::CMP:
        if(this->reg_acc - arg == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::OUT:
        if(this->reg_acc != 0x0D) addch(this->reg_acc);
        refresh();
        //std::this_thread::sleep_for(1ms);
        break;
    case OPERATORS::IN:{
        int input = getch();
        //for(;input == std::cin.eof(); input = std::cin.get());
        this->reg_acc = static_cast<unsigned char>(input);
        break;
    }
    case OPERATORS::SET_L_A:
        this->reg_hl = (this->reg_hl & 0xFF00) + this->reg_acc;
        break;
    case OPERATORS::SET_H_A:
        this->reg_hl = (this->reg_hl & 0x00FF) + (static_cast<uint16_t>(this->reg_acc) << 8);
        break;
    case OPERATORS::SET_A_L:
        this->reg_acc = static_cast<unsigned char>(this->reg_hl & 0x00FF);
        break;
    case OPERATORS::SET_A_H:
        this->reg_acc = static_cast<unsigned char>((this->reg_hl & 0xFF00) >> 8);
        break;
    case OPERATORS::BAK:
        this->reg_bak = this->reg_acc;
        break;
    case OPERATORS::RES:
        this->reg_acc = this->reg_bak;
        break;
    case OPERATORS::JREL:
        this->address += static_cast<signed char>(arg);
        break;
    case OPERATORS::PUSH_HL:
        this->stack.push_back(this->reg_hl);
        break;
    case OPERATORS::POP_HL:
        this->reg_hl = this->stack.back();
        this->stack.pop_back();
        break;
    case OPERATORS::EX_HL:{
        uint16_t temp = this->reg_hl_bak;
        this->reg_hl_bak = this->reg_hl;
        this->reg_hl = temp;
        break;
    }
    case OPERATORS::RET:
        this->stop();
        break;
    case OPERATORS::ADD_BAK:
        this->reg_acc += this->reg_bak;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    case OPERATORS::SUB_BAK:
        this->reg_acc -= this->reg_bak;
        if(this->reg_acc == 0) this->zero = true;
        else this->zero = false;
        break;
    }
    if(this->debug){
        std::cerr << std::format("cmd={:02X},arg={:02X},acc={:02X},bak={:02X},hl={:04X},hl_bak={:04X},addr={:04X}\n", int_cmd, arg, this->reg_acc, this->reg_bak, this->reg_hl, this->reg_hl_bak, this->address);
    }
}

void Machine::exec(){
    while(!this->stop_execution){
        while(this->run_exec){
            auto t_time = std::chrono::steady_clock::now() + 1us;
            this->step();
            while(std::chrono::steady_clock::now() < t_time);
        }
    }
}

int main(int argc, char **argv){
    if(argc < 2) return 1;
    std::ifstream file(argv[1], std::ios::binary);
    if(!file.is_open()){
        file.close();
        std::cerr << "Failed to open exec\n";
        return 2;
    }
    std::string exec_data;
    char tmp;
    while(!file.get(tmp).eof()) exec_data += tmp;
    file.close();
    
    initscr(); //start ncurses stuff
    clear();
    scrollok(stdscr, true);
    noecho();
    cbreak();
    refresh();

    Machine machine;
    machine.setOrigin(0x0100);
    machine.reset();
    machine.setMemory(exec_data);
    machine.reset();
    machine.setDebug(argc > 2);
    machine.start();
    while(!machine.is_finished);
    
    getnstr(nullptr, INT32_MAX);
    clear();
    endwin();

    return 0;
}