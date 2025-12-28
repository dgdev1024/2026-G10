/**
 * @file    g10asm/keyword_table.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains the definition of the assembler's keyword lookup table.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{
    /**
     * @brief   Strongly enumerates the different types of keywords recognized
     *          in the G10 assembly language.
     */
    enum class keyword_type
    {
        none,                           /** @brief No keyword. */
        instruction_mnemonic,           /** @brief An instruction mnemonic (e.g., `NOP`, `LD`, `ADD`, etc.) */
        assembler_directive,            /** @brief An assembler directive (e.g., `.org`, `.byte`, etc.) */
        register_name,                  /** @brief A general-purpose CPU register name (e.g. `D0`, `W1`, etc.) */
        branching_condition             /** @brief A branching condition (e.g., `NC`, `ZS`, `CC`, etc.) */
    };

    /**
     * @brief   Strongly enumerates the different types of assembler directives
     *          recognized in the G10 assembly language.
     */
    enum class directive_type
    {
        org,        /** @brief The `.org` directive sets the location counter to a specified address. */
        rom,        /** @brief The `.rom` directive switches the location counter to the ROM region. */
        ram,        /** @brief The `.ram` directive switches the location counter to the RAM region. */
        int_,       /** @brief The `.int` directive sets the location counter to an interrupt vector subroutine. */
        byte,       /** @brief The `.byte` directive defines or reserves one or more bytes of data. */
        word,       /** @brief The `.word` directive defines or reserves one or more words (2 bytes) of data. */
        dword,      /** @brief The `.dword` directive defines or reserves one or more double words (4 bytes) of data. */
        global,     /** @brief The `.global` directive exports symbols for linking with other modules. */
        extern_,    /** @brief The `.extern` directive declares symbols defined in other modules. */
        let,        /** @brief The `.let` directive declares a mutable assembler variable. */
        const_,     /** @brief The `.const` directive declares an immutable assembler constant. */
    };
}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a struct which represents a keyword entry in the
     *          assembler's keyword lookup table.
     */
    struct keyword final
    {
        std::string_view    name;           /** @brief The keyword's string contents. All keywords are stored in lowercase. */
        keyword_type        type;           /** @brief The type of keyword (mnemonic, directive, register, etc.). */
        std::uint8_t        param1 { 0 };   /** @brief An additional parameter further describing the keyword. */
        std::uint8_t        param2 { 0 };   /** @brief An additional parameter further describing the keyword. */
        std::uint8_t        param3 { 0 };   /** @brief An additional parameter further describing the keyword. */
    };
}

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a static class representing the G10 assembler tool's
     *          keyword lookup table.
     * 
     * The lexer uses this table to identify language keywords (such as instruction
     * mnemonics, directives, and registers) during the tokenization process.
     */
    class keyword_table final
    {
    public:

        /**
         * @brief   Checks to see if the given string exists as a keyword in
         *          the assembler's keyword table.
         * 
         * The name given is case-insensitive, and will be transformed to
         * lowercase prior to lookup.
         * 
         * @param   name    The name of the keyword to look up.
         * 
         * @return  If the keyword is found, a const reference to the
         *          keyword entry structure;
         *          Otherwise, an error string.
         */
        static auto lookup_keyword (std::string_view name) 
            -> g10::result_cref<keyword>;

    private:

        /**
         * @brief   The assembler's keyword lookup table.
         */
        static const std::vector<keyword> s_keywords;  

    };
}