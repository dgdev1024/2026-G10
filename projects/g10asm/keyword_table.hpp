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
        preprocessor_function,          /** @brief A preprocessor function (e.g. `fint(FP)`, etc.) */
        preprocessor_directive,         /** @brief A preprocessor directive (e.g., `.include`, `.define`, etc.) */
        pragma,                         /** @brief A pragma directive (e.g., `once`, etc.) */
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
        // Preprocessor Directives
        pragma,     /** @brief The `.pragma` directive is used to specify special instructions to the assembler. */
        include,    /** @brief The `.include` directive is used to include the contents of another file. */
        define,     /** @brief The `.define` directive is used to create a text-substitution macro. */
        macro,      /** @brief The `.macro` directive is used to define a parameterized macro. */
        shift,      /** @brief The `.shift` directive is used to shift a parameterized macro call's argument list. */
        endmacro,   /** @brief The `.endm` directive marks the end of a macro definition. */
        undef,      /** @brief The `.undef` directive is used to undefine a previously defined macro. */
        ifdef_,     /** @brief The `.ifdef` directive begins a conditional assembly block if a macro is defined. */
        ifndef_,    /** @brief The `.ifndef` directive begins a conditional assembly block if a macro is not defined. */
        if_,        /** @brief The `.if` directive begins a conditional assembly block. */
        elseif,     /** @brief The `.elseif` directive continues a conditional assembly block. */
        else_,      /** @brief The `.else` directive provides an alternative block in a conditional assembly. */
        endif,      /** @brief The `.endif` directive ends a conditional assembly block. */
        repeat,     /** @brief The `.repeat` directive begins a repeat assembly block. */
        endrepeat,  /** @brief The `.endrep` directive ends a repeat assembly block. */
        for_,       /** @brief The `.for` directive begins a for-loop assembly block. */
        endfor,     /** @brief The `.endfor` directive ends a for-loop assembly block. */
        while_,     /** @brief The `.while` directive begins a while-loop assembly block. */
        endwhile,   /** @brief The `.endw` directive ends a while-loop assembly block. */
        continue_,  /** @brief The `.continue` directive skips to the next iteration of a loop. */
        break_,     /** @brief The `.break` directive exits the current loop. */
        info,       /** @brief The `.info` directive outputs an informational message during preprocessing. */
        warning,    /** @brief The `.warning` directive outputs a warning message during preprocessing. */
        error,      /** @brief The `.error` directive outputs an error message during preprocessing. */
        fatal,      /** @brief The `.fatal` directive outputs a fatal error message and halts preprocessing. */
        assert_,    /** @brief The `.assert` directive checks a condition and outputs an error if the condition is false. */

        // Assembler Directives
        metadata,   /** @brief The `.meta` directive begins a metadata section, for storing program information. */
        int_,       /** @brief The `.int XX` directive begins an interrupt handler subroutine for interrupt vector `XX`. */
        code,       /** @brief The `.code` directive begins a code section, for storing executable instructions in ROM. */
        data,       /** @brief The `.data` directive begins a data section, for storing read-only data in ROM. */
        bss,        /** @brief The `.bss` directive begins a BSS section, for reserving uninitialized data in RAM. */
        org,        /** @brief The `.org` directive sets the current location counter to a specified address. */
        byte,       /** @brief The `.byte` directive stores/reserves one or more bytes of data. */
        word,       /** @brief The `.word` directive stores/reserves one or more words (2 bytes) of data. */
        dword,      /** @brief The `.dword` directive stores/reserves one or more double-words (4 bytes) of data. */
        space,      /** @brief The `.space` directive stores/reserves a specified number of bytes of padding/uninitialized data. */
        global,     /** @brief The `.global` directive declares one or more address labels as global symbols. */
        extern_     /** @brief The `.extern` directive declares one or more external symbols. */
    };

    /**
     * @brief   Strongly enumerates the different types of pragmas recognized
     *          in the G10 assembler's preprocessor and parsing systems.
     */
    enum class pragma_type
    {
        // Preprocessor Pragmas
        //
        // These pragmas affect the behavior of the preprocessor, and should be
        // encountered and processed during the preprocessing phase.
        once,                       /** @brief `.pragma once`: Instructs the preprocessor to include the current file only once. */
        max_recursion_depth,        /** @brief `.pragma max_recursion_depth N`: Sets the maximum macro recursion depth to `N`. */
        max_include_depth,          /** @brief `.pragma max_include_depth N`: Sets the maximum include depth to `N`. */

        // Inserted Pragmas
        //
        // These pragmas are inserted automatically by the assembler's preprocessor,
        // and affect the behavior of the assembler's parser. These should not
        // be used explicitly in source code.
        push_file,          /** @brief `.pragma push_file "FILENAME"`: Informs the parser of a new source file context. */
        pop_file            /** @brief `.pragma pop_file`: Informs the parser to revert to the previous source file context. */
    };

    /**
     * @brief   Strongly enumerates the different types of functions recognized
     *          in the G10 assembly language's preprocessor expression system.
     */
    enum class function_type
    {
        // Numeric Functions
        fint,           /** @brief `fint(FP)`: Returns the integer part of the fixed-point number `FP`. */
        ffrac,          /** @brief `ffrac(FP)`: Returns the fractional part of the fixed-point number `FP`. */

        // String Functions
        strlen_,        /** @brief `strlen(STR)`: Returns the length of the string `STR` in characters. */
        strcmp_,        /** @brief `strcmp(STR1, STR2)`: Compares two strings lexicographically. */
        substr_,        /** @brief `substr(STR, START, LENGTH)`: Returns a substring of `STR` starting at index `START` with length `LENGTH`. */
        indexof,        /** @brief `indexof(STR, SUBSTR)`: Returns the index of the first occurrence of `SUBSTR` in `STR`, or `-1` if not found. */
        toupper_,       /** @brief `toupper(STR)`: Converts all characters in `STR` to uppercase. */
        tolower_,       /** @brief `tolower(STR)`: Converts all characters in `STR` to lowercase. */
        concat,         /** @brief `concat(STR1, STR2, ...)`: Concatenates all provided strings into a single string. */

        // Miscellaneous Functions
        defined,        /** @brief `defined(NAME)`: Returns `1` if a macro (either text-sub or parameterized) named `NAME` is defined, `0` otherwise. */
        typeof_         /** @brief `typeof(EXPR)`: Returns a string indicating the type of the expression: `"integer"`,  `"fixed-point"`, `"string"`, or `"undefined"`. */
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

        /**
         * @brief   Converts a keyword type to its string representation.
         * 
         * @param   type    The keyword type to convert.
         * 
         * @return  A string view representing the keyword type.
         */
        static auto stringify_keyword (keyword_type type) -> std::string_view;

    private:

        /**
         * @brief   The assembler's keyword lookup table.
         */
        static const std::vector<keyword> s_keywords;  

    };
}