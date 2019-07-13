#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <sstream>
#include <vector>

#include "instruction_encoder.h"
#include "logger.h"
#include "optional.h"
#include "printer.h"
#include "tokenizer.h"

namespace lc3
{
namespace core
{
    class Assembler
    {
    public:
        Assembler(lc3::utils::IPrinter & printer, uint32_t print_level) : logger(printer, print_level) {}
        Assembler(Assembler const &) = default;
        Assembler & operator=(Assembler const &) = delete;
        ~Assembler(void) = default;

        std::stringstream assemble(std::istream & buffer);
        void setFilename(std::string const & filename) { logger.setFilename(filename); }

    private:
        std::vector<std::string> file_buffer;
        lc3::utils::AssemblerLogger logger;

        asmbl::InstructionEncoder encoder;

        optional<SymbolTable> firstPass(std::vector<asmbl::Statement> const & statements);
        optional<std::vector<MemEntry>> secondPass(std::vector<asmbl::Statement> const & statements,
            SymbolTable const & symbol_table);

        asmbl::Statement makeStatement(std::vector<asmbl::Token> const & tokens);
        void markRegAndPseudoTokens(std::vector<asmbl::StatementToken> & tokens);
        void markInstTokens(std::vector<asmbl::StatementToken> & tokens);
        void markLabelTokens(std::vector<asmbl::StatementToken> & tokens);
        asmbl::Statement makeStatementFromTokens(std::vector<asmbl::StatementToken> & tokens);
        void markPC(std::vector<asmbl::Statement> & statements);
        optional<uint32_t> encodeInstruction(asmbl::Statement const & statement, SymbolTable const & symbol_table);
        optional<uint32_t> encodePseudo(asmbl::Statement const & statement, SymbolTable const & symbol_table);

        bool checkIfValidPseudoToken(asmbl::StatementToken const & tok, std::string const & check);
        bool checkIfValidPseudoStatement(asmbl::Statement const & state, std::string const & check,
            bool log_enable);
    };
};
};

#endif
