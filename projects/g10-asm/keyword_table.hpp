/**
 * @file    g10-asm/keyword_table.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains definitions for the G10 CPU assembler's keyword table.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{
    /**
     * @brief   Strongly enumerates the types of keywords which can be
     *          encountered in the G10 assembly language.
     */
    enum class keyword_type : std::uint8_t
    {
        none,                           /** @brief Not a keyword. */
        instruction,                    /** @brief An instruction mnemonic (`NOP`, `LD`, etc.). */
        register_name,                  /** @brief A general-purpose register name (`D0`, `W1`, `H2`, `L3`, etc.). */
        condition_code,                 /** @brief A branching condition name (`NC`, `ZS`, `CC`, `VS`, etc.). */
        directive,                      /** @brief An assembler directive (`.BYTE`, `.ORG`, `.GLOBAL`, `.EXTERN`, etc.). Note that all directives begin with a period `.`. */
    };

    /**
     * @brief   Strongly enumerates the types of assembler directives
     *          supported by the G10 assembly language.
     */
    enum class directive_type : std::uint8_t
    {
        byte,           /** @brief `.BYTE` - Defines one or more bytes of data. */
        word,           /** @brief `.WORD` - Defines one or more words of data. */
        dword,          /** @brief `.DWORD` - Defines one or more double-words of data. */
        org,            /** @brief `.ORG` - Sets the current assembly location counter. */
        global_,        /** @brief `.GLOBAL` - Declares one or more global symbols. */
        extern_,        /** @brief `.EXTERN` - Declares one or more external symbols. */
    };
}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing a keyword in the G10 assembly
     *          language.
     */
    struct keyword final
    {
        std::string_view    lexeme;         /** @brief The keyword's lexeme as it appears in source code. */
        keyword_type        type;           /** @brief The keyword's strongly-enumerated type. */
        std::uint8_t        param1 { 0 };   /** @brief An optional parameter further describing the keyword. */
        std::uint8_t        param2 { 0 };   /** @brief An optional parameter further describing the keyword. */
    };
}

/* Public Classes *************************************************************/

namespace g10asm
{

    /**
     * @brief   Defines a static class which provides functions for resolving
     *          tokens' lexemes to reserved keywords in the G10 assembly language.
     * 
     * These keywords include mnemonics for instructions, operands such as
     * register names and condition codes, assembler directives, and other
     * reserved language constructs.
     */
    class keyword_table final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Attempts to resolve the given lexeme to a keyword in the
         *          G10 assembly language's keyword table.
         * 
         * @param   lexeme      The lexeme to resolve.
         * 
         * @return  If resolved successfully, a const reference to the keyword
         *          structure representing the keyword; 
         *          Otherwise, an error string describing the failure.
         */
        static auto lookup (std::string_view lexeme) 
            -> g10::result_cref<keyword>;
    
    private: /* Private Members ***********************************************/

        /**
         * @brief   The lookup table mapping lexemes to their corresponding
         *          keyword types.
         */
        static const std::vector<keyword> s_lookup_table;

    };

}
