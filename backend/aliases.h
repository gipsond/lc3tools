#ifndef ALIASES_H
#define ALIASES_H

#include <map>
#include <memory>
#include <string>

namespace lc3
{
namespace core
{
    class IOperand;
    class IInstruction;
    class IEvent;

    using PIOperand = std::shared_ptr<IOperand>;
    using PIInstruction = std::shared_ptr<IInstruction>;
    using PIEvent = std::shared_ptr<IEvent>;

    using SymbolTable = std::map<std::string, uint32_t>;
};
};

#endif
