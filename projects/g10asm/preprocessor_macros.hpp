/**
 * @file    g10asm/preprocessor_macros.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-03
 * 
 * @brief   Contains definitions for the macro structures and table used by
 *          the G10 assembler's preprocessor component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/token.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{

}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing a text-substitution macro.
     * 
     * Text-substitution macros are defined using the `.define` directive and
     * perform simple token replacement. When the macro name is encountered in
     * the source code, it is replaced with the stored replacement tokens.
     */
    struct text_sub_macro final
    {
        /**
         * @brief   The name of the macro.
         */
        std::string name {};

        /**
         * @brief   The replacement tokens for this macro.
         * 
         * When the macro is expanded, these tokens replace the macro name
         * identifier in the output.
         */
        std::vector<token> replacement {};

        /**
         * @brief   The source file where this macro was defined.
         */
        std::string source_file {};

        /**
         * @brief   The line number where this macro was defined.
         */
        std::size_t source_line { 0 };
    };
}

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing the macro table used by the G10
     *          assembler's preprocessor component.
     * 
     * This subcomponent is responsible for storing and managing all defined
     * macros during the preprocessing stage.
     */
    class pp_macro_table final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Defines a new text-substitution macro.
         * 
         * @param   name            The name of the macro.
         * @param   replacement     The replacement tokens for the macro.
         * @param   source_file     The source file where the macro is defined.
         * @param   source_line     The line number where the macro is defined.
         * 
         * @return  If the macro is successfully defined, returns `void`;
         *          Otherwise, returns an error indicating why the definition
         *          failed.
         */
        auto define_text_sub_macro (
            const std::string& name,
            const std::vector<token>& replacement,
            const std::string& source_file,
            std::size_t source_line
        ) -> g10::result<void>;

        /**
         * @brief   Looks up a text-substitution macro by name.
         * 
         * @param   name    The name of the macro to look up.
         * 
         * @return  If the macro is found, returns a constant reference to the
         *          macro structure;
         *          Otherwise, returns an error.
         */
        auto lookup_text_sub_macro (const std::string& name) const
            -> g10::result_cref<text_sub_macro>;

        /**
         * @brief   Undefines (removes) a macro by name.
         * 
         * This removes both text-substitution and parameterized macros.
         * 
         * @param   name    The name of the macro to undefine.
         * 
         * @return  If the macro was found and removed, returns `void`;
         *          Otherwise, returns an error.
         */
        auto undefine_macro (const std::string& name) -> g10::result<void>;

        /**
         * @brief   Checks if a macro with the given name is defined.
         * 
         * @param   name    The name of the macro to check.
         * 
         * @return  If the macro is defined, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto is_macro_defined (const std::string& name) const -> bool;

        /**
         * @brief   Validates a macro name according to the naming rules.
         * 
         * Macro names must:
         * - Begin with a letter (A-Z, a-z) or underscore (_)
         * - Contain only letters, digits (0-9), and underscores
         * - Not begin with double underscore (__) - reserved for built-ins
         * - Not be a reserved keyword
         * 
         * @param   name    The name to validate.
         * 
         * @return  If the name is valid, returns `void`;
         *          Otherwise, returns an error describing why.
         */
        static auto validate_macro_name (const std::string& name)
            -> g10::result<void>;

    private: /* Private Members ***********************************************/

        /**
         * @brief   Map of text-substitution macro names to their definitions.
         */
        std::unordered_map<std::string, text_sub_macro> m_text_sub_macros {};

    };
}
