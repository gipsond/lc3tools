#include <sstream>

#include "instructions.h"

lc3::core::IOperand::IOperand(OperType type, std::string const & type_str, uint32_t width)
{
    this->type = type;
    this->type_str = type_str;
    this->width = width;
    this->value = 0;
}

lc3::core::IInstruction::IInstruction(std::string const & name, std::vector<IOperand *> const & operands)
{
    this->name = name;
    this->operands = operands;
}

lc3::core::IInstruction::IInstruction(IInstruction const & that)
{
    this->name = that.name;
    for(IOperand * op : that.operands) {
        this->operands.push_back(op->clone());
    }
}

lc3::core::IInstruction::~IInstruction(void)
{
    for(uint32_t i = 0; i < operands.size(); i += 1) {
        delete operands[i];
    }
}

uint32_t lc3::core::IInstruction::getNumOperands(void) const
{
    uint32_t ret = 0;
    for(IOperand * operand : operands) {
        if(operand->type != OperType::OPER_TYPE_FIXED) {
            ret += 1;
        }
    }
    return ret;
}

void lc3::core::IInstruction::assignOperands(uint32_t encoded_inst)
{
    uint32_t cur_pos = 15;
    for(IOperand * op : operands) {
        if(op->type != OperType::OPER_TYPE_FIXED) {
            op->value = utils::getBits(encoded_inst, cur_pos, cur_pos - op->width + 1);
        }
        cur_pos -= op->width;
    }
}

std::string lc3::core::IInstruction::toFormatString(void) const
{
    std::stringstream assembly;
    assembly << name << " ";
    std::string prefix = "";
    for(IOperand * operand : operands) {
        if(operand->type != OperType::OPER_TYPE_FIXED) {
            assembly << prefix << operand->type_str;
            prefix = ", ";
        }
    }
    return assembly.str();
}

std::string lc3::core::IInstruction::toValueString(void) const
{
    std::stringstream assembly;
    assembly << name << " ";
    std::string prefix = "";
    for(IOperand * operand : operands) {
        if(operand->type != OperType::OPER_TYPE_FIXED) {
            std::string oper_str;
            if(operand->type == OperType::OPER_TYPE_NUM || operand->type == OperType::OPER_TYPE_LABEL) {
                if((operand->type == OperType::OPER_TYPE_NUM && ((NumOperand *) operand)->sext) ||
                    operand->type == OperType::OPER_TYPE_LABEL)
                {
                    oper_str = "#" + std::to_string((int32_t) utils::sextTo32(operand->value, operand->width));
                } else {
                    oper_str = "#" + std::to_string(operand->value);
                }
            } else if(operand->type == OperType::OPER_TYPE_REG) {
                oper_str = "r" + std::to_string(operand->value);
            }
            assembly << prefix << oper_str;
            prefix = ", ";
        }
    }
    return assembly.str();
}

bool lc3::core::IOperand::isEqualType(OperType other) const
{
    return type == other;
}

lc3::core::InstructionHandler::InstructionHandler(void)
{
    regs["r0"] = 0;
    regs["r1"] = 1;
    regs["r2"] = 2;
    regs["r3"] = 3;
    regs["r4"] = 4;
    regs["r5"] = 5;
    regs["r6"] = 6;
    regs["r7"] = 7;

    instructions.push_back(new ADDRInstruction());
    instructions.push_back(new ADDIInstruction());
    instructions.push_back(new ANDRInstruction());
    instructions.push_back(new ANDIInstruction());
    instructions.push_back(new BRInstruction());
    instructions.push_back(new BRnInstruction());
    instructions.push_back(new BRzInstruction());
    instructions.push_back(new BRpInstruction());
    instructions.push_back(new BRnzInstruction());
    instructions.push_back(new BRzpInstruction());
    instructions.push_back(new BRnpInstruction());
    instructions.push_back(new BRnzpInstruction());
    instructions.push_back(new NOP0Instruction());
    instructions.push_back(new NOP1Instruction());
    instructions.push_back(new JMPInstruction());
    instructions.push_back(new JSRInstruction());
    instructions.push_back(new JSRRInstruction());
    instructions.push_back(new LDInstruction());
    instructions.push_back(new LDIInstruction());
    instructions.push_back(new LDRInstruction());
    instructions.push_back(new LEAInstruction());
    instructions.push_back(new NOTInstruction());
    instructions.push_back(new RETInstruction());
    instructions.push_back(new RTIInstruction());
    instructions.push_back(new STInstruction());
    instructions.push_back(new STIInstruction());
    instructions.push_back(new STRInstruction());
    instructions.push_back(new TRAPInstruction());
    instructions.push_back(new GETCInstruction());
    instructions.push_back(new OUTInstruction());
    instructions.push_back(new PUTSInstruction());
    instructions.push_back(new INInstruction());
    instructions.push_back(new PUTSPInstruction());
    instructions.push_back(new HALTInstruction());
}

lc3::core::InstructionHandler::~InstructionHandler(void)
{
    for(uint32_t i = 0; i < instructions.size(); i += 1) {
        delete instructions[i];
    }
}

uint32_t lc3::core::FixedOperand::encode(Token const * oper, uint32_t oper_count,
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) oper;
    (void) oper_count;
    (void) regs;
    (void) symbols;
    (void) logger;

    return value & ((1 << width) - 1);
}

uint32_t lc3::core::RegOperand::encode(Token const * oper, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) symbols;

    uint32_t token_val = regs.at(std::string(oper->str)) & ((1 << width) - 1);

    logger.printf(lc3::utils::PrintType::PRINT_TYPE_EXTRA, true, "%d.%d: reg %s => %s", oper->row_num, oper_count,
        oper->str.c_str(), utils::udecToBin(token_val, width).c_str());

    return token_val;
}

uint32_t lc3::core::NumOperand::encode(Token const * oper, uint32_t oper_count,
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) regs;
    (void) symbols;

    uint32_t token_val = oper->num & ((1 << width) - 1);

    if(sext) {
        if((int32_t) oper->num < -(1 << (width - 1)) || (int32_t) oper->num > ((1 << (width - 1)) - 1)) {
            logger.printfMessage(lc3::utils::PrintType::PRINT_TYPE_WARNING, oper, "immediate %d truncated to %d",
                oper->num, utils::sextTo32(token_val, width));
            logger.newline();
        }
    } else {
        if(oper->num > ((1 << width) - 1)) {
            logger.printfMessage(lc3::utils::PrintType::PRINT_TYPE_WARNING, oper, "immediate %d truncated to %u",
                oper->num, token_val);
            logger.newline();
        }
    }

    logger.printf(lc3::utils::PrintType::PRINT_TYPE_EXTRA, true, "%d.%d: imm %d => %s", oper->row_num, oper_count,
        oper->num, utils::udecToBin(token_val, width).c_str());

    return token_val;
}

uint32_t lc3::core::LabelOperand::encode(Token const * oper, uint32_t oper_count,
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) regs;

    auto search = symbols.find(oper->str);
    if(search == symbols.end()) {
        logger.printfMessage(lc3::utils::PrintType::PRINT_TYPE_ERROR, oper, "unknown label \'%s\'", oper->str.c_str());
        logger.newline();
        throw utils::exception("unknown label");
    }

    uint32_t token_val = (((int32_t) search->second) - (oper->pc + 1)) & ((1 << width) - 1);

    logger.printf(lc3::utils::PrintType::PRINT_TYPE_EXTRA, true, "%d.%d: label %s (0x%0.4x) => %s", oper->row_num,
        oper_count, oper->str.c_str(), search->second, utils::udecToBin((uint32_t) token_val, width).c_str());

    return token_val;
}

std::vector<lc3::core::IEvent const *> lc3::core::ADDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val + sr2_val) & 0xffff;

    return std::vector<IEvent const *> {
        new PSREvent(utils::computePSRCC(result, state.psr)),
        new RegEvent(dr, result)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::ADDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val + imm_val) & 0xffff;

    return std::vector<IEvent const *> {
        new PSREvent(utils::computePSRCC(result, state.psr)),
        new RegEvent(dr, result)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::ANDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val & sr2_val) & 0xffff;

    return std::vector<IEvent const *> {
        new PSREvent(utils::computePSRCC(result, state.psr)),
        new RegEvent(dr, result)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::ANDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val & imm_val) & 0xffff;

    return std::vector<IEvent const *> {
        new PSREvent(utils::computePSRCC(result, state.psr)),
        new RegEvent(dr, result)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::BRInstruction::execute(MachineState const & state)
{
    std::vector<IEvent const *> ret;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    if((operands[1]->value & (state.psr & 0x0007)) != 0) {
        ret.push_back(new PCEvent(addr));
    }

    return ret;
}

std::vector<lc3::core::IEvent const *> lc3::core::JMPInstruction::execute(MachineState const & state)
{
    std::vector<IEvent const *> ret {
        new PCEvent(state.regs[operands[2]->value] & 0xffff)
    };
    if(operands[2]->value == 7) {
        ret.push_back(new CallbackEvent(state.sub_exit_callback_v, state.sub_exit_callback));
    }
    return ret;
}

std::vector<lc3::core::IEvent const *> lc3::core::JSRInstruction::execute(MachineState const & state)
{
    return std::vector<IEvent const *> {
        new RegEvent(7, state.pc & 0xffff),
        new PCEvent(utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width)),
        new CallbackEvent(state.sub_enter_callback_v, state.sub_enter_callback)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::JSRRInstruction::execute(MachineState const & state)
{
    return std::vector<IEvent const *> {
        new RegEvent(7, state.pc & 0xffff),
        new PCEvent(state.regs[operands[3]->value]),
        new CallbackEvent(state.sub_enter_callback_v, state.sub_enter_callback)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::LDInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem;
    IEvent * change;
    uint32_t value = state.readMem(addr, change_mem, change);

    std::vector<IEvent const *> ret {
        new PSREvent(utils::computePSRCC(value, state.psr)),
        new RegEvent(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    return ret;
}

std::vector<lc3::core::IEvent const *> lc3::core::LDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr1 = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem1;
    IEvent * change1;
    uint32_t addr2 = state.readMem(addr1, change_mem1, change1);

    bool change_mem2;
    IEvent * change2;
    uint32_t value = state.readMem(addr2, change_mem2, change2);

    std::vector<IEvent const *> ret {
        new PSREvent(utils::computePSRCC(value, state.psr)),
        new RegEvent(dr, value)
    };

    // TODO: what if the changes are the same?
    if(change_mem1) {
        ret.push_back(change1);
    }

    if(change_mem2) {
        ret.push_back(change2);
    }


    return ret;
}

std::vector<lc3::core::IEvent const *> lc3::core::LDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);

    bool change_mem;
    IEvent * change;
    uint32_t value = state.readMem(addr, change_mem, change);

    std::vector<IEvent const *> ret {
        new PSREvent(utils::computePSRCC(value, state.psr)),
        new RegEvent(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    return ret;
}

std::vector<lc3::core::IEvent const *> lc3::core::LEAInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    return std::vector<IEvent const *> {
        new PSREvent(utils::computePSRCC(addr, state.psr)),
        new RegEvent(dr, addr)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::NOTInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr_val = utils::sextTo32(state.regs[operands[2]->value], operands[2]->width);
    uint32_t result = (~sr_val) & 0xffff;

    return std::vector<IEvent const *> {
        new PSREvent(utils::computePSRCC(result, state.psr)),
        new RegEvent(dr, result)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::RTIInstruction::execute(MachineState const & state)
{
    if((state.pc & 0x8000) == 0x0000) {
        bool pc_change_mem;
        IEvent * pc_change;
        uint32_t pc_value = state.readMem(state.regs[6], pc_change_mem, pc_change);

        bool psr_change_mem;
        IEvent * psr_change;
        uint32_t psr_value = state.readMem(state.regs[6] + 1, psr_change_mem, psr_change);

        std::vector<IEvent const *> ret {
            new PCEvent(pc_value),
            new PSREvent(psr_value),
            new CallbackEvent(state.interrupt_exit_callback_v, state.interrupt_exit_callback)
        };

        if(pc_change_mem) {
            ret.push_back(pc_change);
        }

        if(psr_change_mem) {
            ret.push_back(psr_change);
        }

        return ret;
    }

    // TODO: trigger exception
    return std::vector<IEvent const *> {};
}

std::vector<lc3::core::IEvent const *> lc3::core::STInstruction::execute(MachineState const & state)
{
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IEvent const *> {
        new MemWriteEvent(addr, value)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::STIInstruction::execute(MachineState const & state)
{
    uint32_t addr1 = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1].getValue();
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IEvent const *> {
        new MemWriteEvent(addr2, value)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::STRInstruction::execute(MachineState const & state)
{
    uint32_t addr = utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
        operands[3]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IEvent const *> {
        new MemWriteEvent(addr, value)
    };
}

std::vector<lc3::core::IEvent const *> lc3::core::TRAPInstruction::execute(MachineState const & state)
{
    return std::vector<IEvent const *> {
        new PSREvent(state.psr & 0x7fff),
        new RegEvent(7, state.pc & 0xffff),
        new PCEvent(state.mem[operands[2]->value].getValue() & 0xffff),
        new CallbackEvent(state.sub_enter_callback_v, state.sub_enter_callback)
    };
}
