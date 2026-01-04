/**
 * @file    g10asm/preprocessor_macros.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-03
 * 
 * @brief   Contains implementations for the macro structures and table used by
 *          the G10 assembler's preprocessor component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/preprocessor_macros.hpp>

/* Public Methods *************************************************************/

namespace g10asm
{
    auto pp_macro_table::define_text_sub_macro (
        const std::string& name,
        const std::vector<token>& replacement,
        const std::string& source_file,
        std::size_t source_line
    ) -> g10::result<void>
    {
        // Validate the macro name
        auto validate_result = validate_macro_name(name);
        if (validate_result.has_value() == false)
        {
            return g10::error(
                " - Failed to define macro '{}'.\n{}",
                name,
                validate_result.error()
            );
        }

        // Check if macro is already defined (redefinition is allowed but
        // we replace the old definition)
        m_text_sub_macros[name] = text_sub_macro {
            .name           = name,
            .replacement    = replacement,
            .source_file    = source_file,
            .source_line    = source_line
        };

        return {};
    }

    auto pp_macro_table::lookup_text_sub_macro (const std::string& name) const
        -> g10::result_cref<text_sub_macro>
    {
        auto it = m_text_sub_macros.find(name);
        if (it == m_text_sub_macros.end())
        {
            return g10::error(
                " - Macro '{}' is not defined.",
                name
            );
        }

        return std::cref(it->second);
    }

    auto pp_macro_table::undefine_macro (const std::string& name)
        -> g10::result<void>
    {
        // Try to remove from text-substitution macros
        auto it = m_text_sub_macros.find(name);
        if (it != m_text_sub_macros.end())
        {
            m_text_sub_macros.erase(it);
            return {};
        }

        // Macro not found
        return g10::error(
            " - Cannot undefine macro '{}': not defined.",
            name
        );
    }

    auto pp_macro_table::is_macro_defined (const std::string& name) const -> bool
    {
        return m_text_sub_macros.find(name) != m_text_sub_macros.end();
    }

    auto pp_macro_table::validate_macro_name (const std::string& name)
        -> g10::result<void>
    {
        // Name cannot be empty
        if (name.empty() == true)
        {
            return g10::error(" - Macro name cannot be empty.");
        }

        // First character must be a letter or underscore
        const char first = name[0];
        if (std::isalpha(first) == 0 && first != '_')
        {
            return g10::error(
                " - Macro name '{}' must begin with a letter or underscore.",
                name
            );
        }

        // Check for reserved double-underscore prefix
        if (name.size() >= 2 && name[0] == '_' && name[1] == '_')
        {
            return g10::error(
                " - Macro name '{}' cannot begin with '__' "
                "(reserved for built-in macros).",
                name
            );
        }

        // All characters must be alphanumeric or underscore
        for (std::size_t i = 1; i < name.size(); ++i)
        {
            const char ch = name[i];
            if (std::isalnum(ch) == 0 && ch != '_')
            {
                return g10::error(
                    " - Macro name '{}' contains invalid character '{}' "
                    "at position {}.",
                    name,
                    ch,
                    i
                );
            }
        }

        // Check if the name is a reserved keyword
        auto keyword_result = keyword_table::lookup_keyword(name);
        if (keyword_result.has_value() == true)
        {
            return g10::error(
                " - Macro name '{}' is a reserved keyword.",
                name
            );
        }

        return {};
    }
}
