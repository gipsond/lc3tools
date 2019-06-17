#include <cassert>
#include <random>

#include "device_regs.h"
#include "interface.h"

lc3::sim::sim(utils::IPrinter & printer, utils::IInputter & inputter, uint32_t print_level, bool propagate_exceptions) :
    printer(printer), simulator(*this, printer, inputter, print_level), propagate_exceptions(propagate_exceptions)
{
    simulator.registerPreInstructionCallback(lc3::sim::preInstructionCallback);
    simulator.registerPostInstructionCallback(lc3::sim::postInstructionCallback);
    simulator.registerInterruptEnterCallback(lc3::sim::interruptEnterCallback);
    simulator.registerInterruptExitCallback(lc3::sim::interruptExitCallback);
    simulator.registerSubEnterCallback(lc3::sim::subEnterCallback);
    simulator.registerSubExitCallback(lc3::sim::subExitCallback);
    if(propagate_exceptions) {
        simulator.loadOS();
    } else {
        try {
            simulator.loadOS();
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
        }
    }
    restart();
}

bool lc3::sim::loadObjectFile(std::string const & obj_filename)
{
    if(propagate_exceptions) {
        simulator.loadObjectFile(obj_filename);
    } else {
        try {
            simulator.loadObjectFile(obj_filename);
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return false;
        }
    }
    return true;
}

void lc3::sim::reinitialize(void)
{
    simulator.reinitialize();
    simulator.loadOS();
}

void lc3::sim::randomize(void)
{
    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> dis(0x0000, 0xffff);

    for(uint32_t i = 0x3000; i < 0xfe00; i += 1) {
        setMem(i, dis(gen));
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        setReg(i, dis(gen));
    }

    restart();
}

void lc3::sim::restart(void)
{
    uint32_t mcr = getMem(MCR);
    setMem(MCR, mcr | 0x8000);
}

void lc3::sim::setRunInstLimit(uint32_t inst_limit)
{
    target_inst_count = inst_exec_count + inst_limit;
    counted_run = true;
    step_out_run = false;
}

bool lc3::sim::run(void)
{
    restart();
    if(propagate_exceptions) {
        simulator.simulate();
    } else {
        try {
            simulator.simulate();
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return false;
        }
    }
    return true;
}

bool lc3::sim::runUntilHalt(void)
{
    until_halt_run = true;
    return run();
}

void lc3::sim::pause(void)
{
    simulator.pause();
}

bool lc3::sim::stepIn(void)
{
    setRunInstLimit(1);
    return run();
}

bool lc3::sim::stepOver(void)
{
    counted_run = false;
    step_out_run = true;
    until_halt_run = false;
    // this will immediately be incremented by the sub enter callback if it is about to enter a subroutine
    sub_depth = 0;
    return run();
}

bool lc3::sim::stepOut(void)
{
    counted_run = false;
    step_out_run = true;
    until_halt_run = false;
    // act like we are already in a subroutine
    sub_depth = 1;
    return run();
}

lc3::core::MachineState & lc3::sim::getMachineState(void) { return simulator.getMachineState(); }

lc3::core::MachineState const & lc3::sim::getMachineState(void) const { return simulator.getMachineState(); }

uint32_t lc3::sim::getInstExecCount(void) const { return inst_exec_count; }

std::vector<lc3::Breakpoint> const & lc3::sim::getBreakpoints(void) const { return breakpoints; }

uint32_t lc3::sim::getReg(uint32_t id) const
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
#else
    id &= 0x7;
#endif
    return getMachineState().regs[id];
}

uint32_t lc3::sim::getMem(uint32_t addr) const
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return getMachineState().readMemRaw(addr);
}

std::string lc3::sim::getMemLine(uint32_t addr) const
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return getMachineState().mem[addr].getLine();
}

uint32_t lc3::sim::getPC(void) const { return getMachineState().pc; }

uint32_t lc3::sim::getPSR(void) const { return getMem(PSR); }

uint32_t lc3::sim::getMCR(void) const { return getMem(MCR); }

char lc3::sim::getCC(void) const
{
    uint32_t value = getPSR() & 0x7;
#ifdef _ENABLE_DEBUG
    assert(value == 0x4 || value == 0x2 || value == 0x1);
#else
    if(value == 0x4 || value == 0x2 || value == 0x1) {
        value = 0x1;
    }
#endif
    if(value == 0x4) { return 'N'; }
    else if(value == 0x2) { return 'Z'; }
    else { return 'P'; }
}

void lc3::sim::setReg(uint32_t id, uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
    assert(value <= 0xffff);
#else
    id &= 0x7;
    value &= 0xffff;
#endif
    getMachineState().regs[id] = value;
}

void lc3::sim::setMem(uint32_t addr, uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
    assert(value <= 0xffff);
#else
    addr &= 0xffff;
    value &= 0xffff;
#endif
    getMachineState().writeMemSafe(addr, value);
    getMachineState().mem[addr].setLine("");
}

void lc3::sim::setMemString(uint32_t addr, std::string const & value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    for(uint32_t i = 0; i < value.size(); i += 1) {
        getMachineState().writeMemRaw(addr + i, static_cast<uint32_t>(value[i]));
        getMachineState().mem[addr + i].setLine(std::string(1, value[i]));
    }
    getMachineState().writeMemRaw(addr + value.size(), 0);
    getMachineState().mem[addr + value.size()].setLine(value);
}

void lc3::sim::setMemLine(uint32_t addr, std::string const & value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    getMachineState().mem[addr].setLine(value);
}

void lc3::sim::setPC(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(value <= 0xfe00u);
#else
    value = std::min(value & 0xffff, 0xfe00u);
#endif
    getMachineState().pc = value;
    while(! getMachineState().sys_call_types.empty()) {
        getMachineState().sys_call_types.pop();
    }
    sub_depth = 0;

    // re-enable system
    restart();
}

void lc3::sim::setPSR(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8707) & 0xffff)) == 0x0000);
#else
    value &= 0xffff;
#endif
    setMem(PSR, value);
}

void lc3::sim::setMCR(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8000) & 0xffff)) == 0x0000);
#else
    value &= 0xffff;
#endif
    setMem(MCR, value);
}

void lc3::sim::setCC(char value)
{
    value |= 0x20;
#ifdef _ENABLE_DEBUG
    assert(value == 'n' || value == 'z' || value == 'p');
#else
    if(! (value == 'n' || value == 'z' || value == 'p')) { return; }
#endif
    uint32_t new_value = 0;
    if(value == 'n') { new_value = 0x4; }
    else if(value == 'z') { new_value = 0x2; }
    else { new_value = 0x1; }
    uint32_t psr = getPSR();
    setPSR((psr & 0xfff8) | new_value);
}

lc3::Breakpoint lc3::sim::setBreakpoint(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    Breakpoint bp(breakpoint_id, addr, this);
    breakpoints.push_back(bp);
    breakpoint_id += 1;
    return bp;
}

bool lc3::sim::removeBreakpointByID(uint32_t id)
{
    auto it = breakpoints.begin();
    bool found = false;
    for(; it != breakpoints.end(); ++it) {
        if(it->id == id) {
            found = true;
            break;
        }
    }

    if(found) {
        breakpoints.erase(it);
    }

    return found;
}

bool lc3::sim::removeBreakpointByAddr(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    auto it = breakpoints.begin();
    bool found = false;
    for(; it != breakpoints.end(); ++it) {
        if(it->loc == addr) {
            found = true;
            break;
        }
    }

    if(found) {
        breakpoints.erase(it);
    }

    return found;
}

void lc3::sim::registerPreInstructionCallback(callback_func_t func)
{
    pre_instruction_callback_v = true;
    pre_instruction_callback = func;
}

void lc3::sim::registerPostInstructionCallback(callback_func_t func)
{
    post_instruction_callback_v = true;
    post_instruction_callback = func;
}

void lc3::sim::registerInterruptEnterCallback(callback_func_t func)
{
    interrupt_enter_callback_v = true;
    interrupt_enter_callback = func;
}

void lc3::sim::registerInterruptExitCallback(callback_func_t func)
{
    interrupt_exit_callback_v = true;
    interrupt_exit_callback = func;
}

void lc3::sim::registerSubEnterCallback(callback_func_t func)
{
    sub_enter_callback_v = true;
    sub_enter_callback = func;
}


void lc3::sim::registerSubExitCallback(callback_func_t func)
{
    sub_exit_callback_v = true;
    sub_exit_callback = func;
}

void lc3::sim::registerBreakpointCallback(breakpoint_callback_func_t func)
{
    breakpoint_callback_v = true;
    breakpoint_callback = func;
}

lc3::utils::IPrinter const & lc3::sim::getPrinter(void) const { return printer; }
void lc3::sim::setPrintLevel(uint32_t print_level) { simulator.setPrintLevel(print_level); }
void lc3::sim::setPropagateExceptions(void) { propagate_exceptions = true; }
void lc3::sim::clearPropagateExceptions(void) { propagate_exceptions = false; }

void lc3::sim::preInstructionCallback(lc3::sim & sim_inst, lc3::core::MachineState & state)
{
    for(auto const & x : sim_inst.breakpoints) {
        if(state.pc == x.loc) {
            if(sim_inst.breakpoint_callback_v) {
                sim_inst.breakpoint_callback(state, x);
            }
            state.hit_breakpoint = true;
            break;
        }
    }

    if(sim_inst.until_halt_run && state.readMemRaw(state.pc) == 0xf025) {
        sim_inst.counted_run = false;
        sim_inst.step_out_run = false;
        sim_inst.until_halt_run = false;
        state.hit_breakpoint = true;
    }

    if(sim_inst.pre_instruction_callback_v) {
        sim_inst.pre_instruction_callback(state);
    }
}

void lc3::sim::postInstructionCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.inst_exec_count += 1;
    if((sim_inst.counted_run && sim_inst.inst_exec_count == sim_inst.target_inst_count) ||
        (sim_inst.step_out_run && sim_inst.sub_depth == 0))
    {
        sim_inst.counted_run = false;
        sim_inst.step_out_run = false;
        state.running = false;
    }

    if(sim_inst.post_instruction_callback_v) {
        sim_inst.post_instruction_callback(state);
    }
}

void lc3::sim::interruptEnterCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth += 1;
    if(sim_inst.interrupt_enter_callback_v) {
        sim_inst.interrupt_enter_callback(state);
    }
}

void lc3::sim::interruptExitCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth -= 1;
    if(sim_inst.interrupt_exit_callback_v) {
        sim_inst.interrupt_exit_callback(state);
    }
}

void lc3::sim::subEnterCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth += 1;
    if(sim_inst.sub_enter_callback_v) {
        sim_inst.sub_enter_callback(state);
    }
}

void lc3::sim::subExitCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth -= 1;
    if(sim_inst.sub_exit_callback_v) {
        sim_inst.sub_exit_callback(state);
    }
}

std::pair<bool, std::string> lc3::as::assemble(std::string const & asm_filename)
{
    std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
    if(propagate_exceptions) {
        assembler.assemble(asm_filename, obj_filename);
    } else {
        try {
            assembler.assemble(asm_filename, obj_filename);
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return std::make_pair(false, obj_filename);
        }
    }
    return std::make_pair(true, obj_filename);
}

std::pair<bool, std::string> lc3::conv::convertBin(std::string const & bin_filename)
{
    std::string obj_filename(bin_filename.substr(0, bin_filename.find_last_of('.')) + ".obj");
    if(propagate_exceptions) {
        converter.convertBin(bin_filename, obj_filename);
    } else {
        try {
            converter.convertBin(bin_filename, obj_filename);
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return std::make_pair(false, obj_filename);
        }
    }
    return std::make_pair(true, obj_filename);
}

void lc3::as::setPropagateExceptions(void) { propagate_exceptions = true; }
void lc3::as::clearPropagateExceptions(void) { propagate_exceptions = false; }
