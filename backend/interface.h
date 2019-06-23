#ifndef CORE_H
#define CORE_H

#ifndef DEFAULT_PRINT_LEVEL
    #ifdef _ENABLE_DEBUG
        #define DEFAULT_PRINT_LEVEL 9
    #else
        #define DEFAULT_PRINT_LEVEL 6
    #endif
#endif

#include <functional>
#include <utility>

#include "assembler.h"
#include "converter.h"
#include "simulator.h"

namespace lc3
{
    class sim;

    struct Breakpoint
    {
        // TODO: make this moveable and replace sim_inst with a reference
        Breakpoint(uint32_t id, uint32_t loc, sim * sim_int) : id(id), loc(loc), sim_int(sim_int) {}

        uint32_t id, loc;
        sim const * sim_int;
    };

    using callback_func_t = std::function<void(core::MachineState &)>;
    using breakpoint_callback_func_t = std::function<void(core::MachineState & state, Breakpoint const & bp)>;

    class sim
    {
    public:
        sim(utils::IPrinter & printer, utils::IInputter & inputter,
            uint32_t print_level = DEFAULT_PRINT_LEVEL, bool propagate_exceptions = false);
        ~sim(void) = default;

        bool loadObjectFile(std::string const & obj_filename);
        void reinitialize(void);
        void randomize(void);
        void restart(void);

        void setRunInstLimit(uint32_t inst_limit);
        bool run(void);
        bool runUntilHalt(void);
        bool runUntilInputPoll(void);
        void pause(void);
        bool stepIn(void);
        bool stepOver(void);
        bool stepOut(void);

        core::MachineState & getMachineState(void);
        core::MachineState const & getMachineState(void) const;
        uint32_t getInstExecCount(void) const;
        std::vector<Breakpoint> const & getBreakpoints() const;

        uint32_t getReg(uint32_t id) const;
        uint32_t getMem(uint32_t addr) const;
        std::string getMemLine(uint32_t addr) const;
        uint32_t getPC(void) const;
        uint32_t getPSR(void) const;
        uint32_t getMCR(void) const;
        char getCC(void) const;
        void setReg(uint32_t id, uint32_t value);
        void setMem(uint32_t addr, uint32_t value);
        void setMemString(uint32_t addr, std::string const & value);
        void setMemLine(uint32_t addr, std::string const & value);
        void setPC(uint32_t value);
        void setPSR(uint32_t value);
        void setMCR(uint32_t value);
        void setCC(char value);

        Breakpoint setBreakpoint(uint32_t addr);
        bool removeBreakpointByID(uint32_t id);
        bool removeBreakpointByAddr(uint32_t addr);

        void registerPreInstructionCallback(callback_func_t func);
        void registerPostInstructionCallback(callback_func_t func);
        void registerInterruptEnterCallback(callback_func_t func);
        void registerInterruptExitCallback(callback_func_t func);
        void registerSubEnterCallback(callback_func_t func);
        void registerSubExitCallback(callback_func_t func);
        void registerBreakpointCallback(breakpoint_callback_func_t func);

        utils::IPrinter const & getPrinter(void) const;
        void setPrintLevel(uint32_t print_level);
        void setPropagateExceptions(void);
        void clearPropagateExceptions(void);

    private:
        utils::IPrinter & printer;
        core::Simulator simulator;

        friend class core::Simulator;
        static void preInstructionCallback(sim & sim_int, core::MachineState & state);
        static void postInstructionCallback(sim & sim_int, core::MachineState & state);
        static void interruptEnterCallback(sim & sim_int, core::MachineState & state);
        static void interruptExitCallback(sim & sim_int, core::MachineState & state);
        static void subEnterCallback(sim & sim_int, core::MachineState & state);
        static void subExitCallback(sim & sim_int, core::MachineState & state);
        static void inputPollCallback(sim & sim_int, core::MachineState & state);

        uint32_t inst_exec_count = 0;
        uint32_t target_inst_count = 0;
        bool counted_run = false;
        bool step_out_run = false;
        bool until_halt_run = false;
        bool until_input_run = false;
        int32_t sub_depth = 0;

        bool pre_instruction_callback_v = false;
        bool post_instruction_callback_v = false;
        bool interrupt_enter_callback_v = false;
        bool interrupt_exit_callback_v = false;
        bool sub_enter_callback_v = false;
        bool sub_exit_callback_v = false;
        bool input_poll_callback_v = false;
        bool breakpoint_callback_v = false;
        callback_func_t pre_instruction_callback;
        callback_func_t post_instruction_callback;
        callback_func_t interrupt_enter_callback;
        callback_func_t interrupt_exit_callback;
        callback_func_t sub_enter_callback;
        callback_func_t sub_exit_callback;
        callback_func_t input_poll_callback;
        breakpoint_callback_func_t breakpoint_callback;

        uint32_t breakpoint_id = 0;
        std::vector<Breakpoint> breakpoints;

        bool propagate_exceptions;
    };

    class as
    {
    public:
        as(utils::IPrinter & printer, uint32_t print_level = DEFAULT_PRINT_LEVEL, bool propagate_exceptions = false) :
            printer(printer), assembler(printer, print_level), propagate_exceptions(propagate_exceptions) {}
        ~as(void) = default;

        std::pair<bool, std::string> assemble(std::string const & asm_filename);

        void setPropagateExceptions(void);
        void clearPropagateExceptions(void);

    private:
        utils::IPrinter & printer;
        core::Assembler assembler;

        bool propagate_exceptions;
    };

    class conv
    {
    public:
        conv(utils::IPrinter & printer, uint32_t print_level = DEFAULT_PRINT_LEVEL, bool propagate_exceptions = false) :
            printer(printer), converter(printer, print_level), propagate_exceptions(propagate_exceptions) {}
        std::pair<bool, std::string> convertBin(std::string const & asm_filename);

        void setPropagateExceptions(void);
        void clearPropagateExceptions(void);

    private:
        utils::IPrinter & printer;
        core::Converter converter;

        bool propagate_exceptions;
    };
};

#endif
