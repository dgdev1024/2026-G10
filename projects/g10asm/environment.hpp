/**
 * @file    g10asm/environment.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-28
 * 
 * @brief   Contains declarations for the G10ASM assembler's environment
 *          management component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/codegen.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{

}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Represents an entry in the assembler's environment table.
     * 
     * Each entry stores a variable or constant's name, current value, mutability
     * status, and source location for error reporting.
     */
    struct environment_entry final
    {
        std::string         name;           /** @brief The variable/constant name (without the `$` prefix). */
        value               current_value;  /** @brief The current value of this variable/constant. */
        bool                is_constant;    /** @brief If true, this entry is immutable (constant). */
        std::string_view    source_file;    /** @brief Source file where this entry was defined. */
        std::size_t         source_line;    /** @brief Source line where this entry was defined. */
    };
}

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a static class representing the G10 assembler's
     *          environment management system.
     * 
     * This component is responsible for facilitating the management of variables
     * and constants declared and used within G10 assembly source code.
     * 
     * Variables are mutable and can be modified via assignment operators.
     * Constants are immutable and cannot be modified after definition.
     * 
     * All variable and constant names are prefixed with `$` in source code,
     * but stored without the prefix in the environment table.
     */
    class environment final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Clears all entries from the environment table.
         * 
         * This should be called at the start of each new assembly run.
         */
        static auto clear () -> void;

        /**
         * @brief   Defines a new mutable variable in the environment.
         * 
         * @param   name        The variable name (without the `$` prefix).
         * @param   init_value  The initial value of the variable.
         * @param   source_file The source file where the variable was defined.
         * @param   source_line The source line where the variable was defined.
         * 
         * @return  If successful, returns void;
         *          Otherwise, returns an error if the name is already defined.
         */
        static auto define_variable (
            const std::string& name,
            const value& init_value,
            std::string_view source_file,
            std::size_t source_line
        ) -> g10::result<void>;

        /**
         * @brief   Defines a new immutable constant in the environment.
         * 
         * @param   name        The constant name (without the `$` prefix).
         * @param   init_value  The value of the constant.
         * @param   source_file The source file where the constant was defined.
         * @param   source_line The source line where the constant was defined.
         * 
         * @return  If successful, returns void;
         *          Otherwise, returns an error if the name is already defined.
         */
        static auto define_constant (
            const std::string& name,
            const value& init_value,
            std::string_view source_file,
            std::size_t source_line
        ) -> g10::result<void>;

        /**
         * @brief   Retrieves the current value of a variable or constant.
         * 
         * @param   name    The name to look up (without the `$` prefix).
         * 
         * @return  If found, returns the current value;
         *          Otherwise, returns an error indicating the name is undefined.
         */
        static auto get_value (const std::string& name) -> g10::result<value>;

        /**
         * @brief   Sets the value of a mutable variable.
         * 
         * @param   name        The variable name (without the `$` prefix).
         * @param   new_value   The new value to assign.
         * 
         * @return  If successful, returns void;
         *          Otherwise, returns an error if the name is undefined or
         *          if attempting to modify a constant.
         */
        static auto set_value (
            const std::string& name,
            const value& new_value
        ) -> g10::result<void>;

        /**
         * @brief   Checks whether a name exists in the environment.
         * 
         * @param   name    The name to check (without the `$` prefix).
         * 
         * @return  True if the name exists; false otherwise.
         */
        static auto exists (const std::string& name) -> bool;

        /**
         * @brief   Checks whether a name refers to a constant.
         * 
         * @param   name    The name to check (without the `$` prefix).
         * 
         * @return  True if the name exists and is a constant; false otherwise.
         */
        static auto is_constant (const std::string& name) -> bool;

    private: /* Private Methods ***********************************************/



    private: /* Private Members ***********************************************/

        /**
         * @brief   The environment table mapping names to their entries.
         */
        static std::unordered_map<std::string, environment_entry> s_entries;

    };
}
