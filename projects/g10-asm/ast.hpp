/**
 * @file    g10-asm/ast.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-19
 * 
 * @brief   Contains definitions for the G10 CPU assembler's program structure
 *          and abstract syntax tree (AST) nodes and types.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/cpu.hpp>
#include <g10-asm/token.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{
    /**
     * @brief   Strongly enumerates the types of operands which can appear in
     *          G10 assembly language instructions.
     */
    enum class operand_type : std::uint8_t
    {
        none,                       /** @brief No operand (instructions like `NOP` and `HALT`). */
        register_name,              /** @brief A CPU register (e.g., `D0`, `W1`, `H2`). */
        immediate_value,            /** @brief An immediate value (e.g., `42`, `0xFF`). */
        memory_address,             /** @brief A memory address (e.g., `[$8000]`, `[0xFFFF]`). */
        indirect_address,           /** @brief An indirect register address (e.g., `[D0]`, `[W1]`). */
        label_reference,            /** @brief A label reference (e.g., `loop_start`, `function_end`). */
        expression                  /** @brief An expression (e.g., `(5 * 2)`, `(SIZE + 4)`). */
    };

    /**
     * @brief   Strongly enumerates the types of statements which can appear
     *          in G10 assembly language source code.
     */
    enum class statement_type : std::uint8_t
    {
        label_definition,           /** @brief A label definition (e.g., `start:`). */
        instruction,                /** @brief An instruction statement (e.g., `LD D0, 42`). */
        directive_org,              /** @brief The `.ORG` directive (`.ORG 0x8000`). */
        directive_byte,             /** @brief The `.BYTE` directive (`.BYTE 0x12, 0x34`). */
        directive_word,             /** @brief The `.WORD` directive (`.WORD 0x1234, 0x5678`). */
        directive_dword,            /** @brief The `.DWORD` directive (`.DWORD 0x12345678`). */
        directive_global,           /** @brief The `.GLOBAL` directive (`.GLOBAL my_subroutine`). */
        directive_extern            /** @brief The `.EXTERN` directive (`.EXTERN my_subroutine`). */
    };
}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing an operand in a G10 assembly
     *          language instruction.
     */
    struct operand final
    {
        operand_type    type { operand_type::none };    /** @brief The type of operand. */
        token           source_token;                   /** @brief The source token which produced this operand. */
        
        // Value Storage - Register Operands
        std::uint8_t    register_index { 0 };           /** @brief For register operands, the index of the register (0 - 15). */
        std::uint8_t    register_size { 0 };            /** @brief For register operands, the size of the register, in bytes (1, 2, or 4). */

        // Value Storage - Immediate and Address Operands
        std::int64_t    immediate_value { 0 };          /** @brief For immediate/address operands, the immediate integer value. */
        bool            is_address { false };           /** @brief For immediate/address operands, indicates if the value is an address. */
        bool            is_indirect { false };          /** @brief For immediate/address operands, indicates if the address is indirect. */
    };

    /**
     * @brief   Defines a structure representing a statement in a G10
     *          assembly language program.
     */
    struct statement final
    {
        statement_type      type;                       /** @brief The type of statement. */
        token               source_token;               /** @brief The source token which produced this statement. */
        std::size_t         line_number;                /** @brief The line number in the source file where this statement appears. */
        std::string_view    source_file;                /** @brief The source file from which this statement was extracted. */

        // Statement Data - Instructions
        g10::instruction        inst;                   /** @brief For instruction statements, the specific instruction. */
        std::vector<operand>    operands;               /** @brief For instruction statements, the list of operands. There should not be more than two. */

        // Statement Data - Directives
        std::int64_t                org_address { 0 };      /** @brief For `.ORG` directives, the new origin address. */
        std::vector<std::int64_t>   data_values;            /** @brief For `.BYTE`, `.WORD`, and `.DWORD` directives, the data values. */
        std::vector<std::string>    symbol_names;           /** @brief For `.GLOBAL` and `.EXTERN` directives, the symbol names being exported/imported. */
        std::string                 label_name;             /** @brief For label definitions, the name of the label. */
    };

    /**
     * @brief   Defines a structure representing a complete G10 assembly
     *          language program.
     */
    struct program final
    {
        std::vector<statement>      statements;             /** @brief The list of statements in the program, in source order. */
        std::vector<std::string>    global_symbols;         /** @brief The list of global symbols defined by the program. */
        std::vector<std::string>    extern_symbols;         /** @brief The list of external symbols required by the program. */
        std::unordered_map<std::string, std::size_t>
                                    label_table;            /** @brief A mapping of label names to their corresponding statement indices in the `statements` vector. */
    };
}
