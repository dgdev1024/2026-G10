/**
 * @file    g10asm/environment.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-28
 * 
 * @brief   Contains definitions for the G10ASM assembler's environment
 *          management component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/environment.hpp>

/* Private Constants and Enumerations *****************************************/

namespace g10asm
{

}

/* Private Unions and Structures **********************************************/

namespace g10asm
{

}

/* Private Static Members *****************************************************/

namespace g10asm
{
    std::unordered_map<std::string, environment_entry> environment::s_entries;
}

/* Public Methods *************************************************************/

namespace g10asm
{
    auto environment::clear () -> void
    {
        s_entries.clear();
    }

    auto environment::define_variable (
        const std::string& name,
        const value& init_value,
        std::string_view source_file,
        std::size_t source_line
    ) -> g10::result<void>
    {
        // Check if name already exists.
        if (auto it = s_entries.find(name); it != s_entries.end())
        {
            const auto& existing = it->second;
            return g10::error(
                "'${}' is already defined as a {} at '{}:{}'.",
                name,
                existing.is_constant ? "constant" : "variable",
                existing.source_file,
                existing.source_line
            );
        }

        // Create the entry.
        s_entries.emplace(name, environment_entry {
            .name = name,
            .current_value = init_value,
            .is_constant = false,
            .source_file = source_file,
            .source_line = source_line
        });

        return {};
    }

    auto environment::define_constant (
        const std::string& name,
        const value& init_value,
        std::string_view source_file,
        std::size_t source_line
    ) -> g10::result<void>
    {
        // Check if name already exists.
        if (auto it = s_entries.find(name); it != s_entries.end())
        {
            const auto& existing = it->second;
            return g10::error(
                "'${}' is already defined as a {} at '{}:{}'.",
                name,
                existing.is_constant ? "constant" : "variable",
                existing.source_file,
                existing.source_line
            );
        }

        // Create the entry.
        s_entries.emplace(name, environment_entry {
            .name = name,
            .current_value = init_value,
            .is_constant = true,
            .source_file = source_file,
            .source_line = source_line
        });

        return {};
    }

    auto environment::get_value (const std::string& name) -> g10::result<value>
    {
        auto it = s_entries.find(name);
        if (it == s_entries.end())
        {
            return g10::error("Undefined variable or constant '${}'. ", name);
        }

        return it->second.current_value;
    }

    auto environment::set_value (
        const std::string& name,
        const value& new_value
    ) -> g10::result<void>
    {
        auto it = s_entries.find(name);
        if (it == s_entries.end())
        {
            return g10::error("Undefined variable '${}'. ", name);
        }

        if (it->second.is_constant)
        {
            return g10::error(
                "Cannot modify constant '${}' (defined at '{}:{}').",
                name,
                it->second.source_file,
                it->second.source_line
            );
        }

        it->second.current_value = new_value;
        return {};
    }

    auto environment::exists (const std::string& name) -> bool
    {
        return s_entries.find(name) != s_entries.end();
    }

    auto environment::is_constant (const std::string& name) -> bool
    {
        auto it = s_entries.find(name);
        return it != s_entries.end() && it->second.is_constant;
    }
}

/* Private Methods ************************************************************/

namespace g10asm
{

}
