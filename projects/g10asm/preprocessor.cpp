/**
 * @file    g10asm/preprocessor.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-01
 * 
 * @brief   Contains definitions for the G10 assembler's preprocessor
 *          component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/preprocessor.hpp>

/* Private Macros *************************************************************/

#define G10ASM_ERR_TOK(tok) \
    tok.source_file, \
    tok.source_line, \
    tok.source_column

/* Private Constants and Enumerations *****************************************/

namespace g10asm
{

}

/* Private Unions and Structures **********************************************/

namespace g10asm
{

}

/* Public Methods *************************************************************/

namespace g10asm
{
    preprocessor::preprocessor (
        const preprocessor_config&  config,
        const lexer&                lexer
    ) :
        m_max_recursion_depth   { config.max_recursion_depth },
        m_max_include_depth     { config.max_include_depth }
    {
        resolve_include_paths(config.include_dirs)
            .and_then(
                [this, &lexer] () -> g10::result<void>
                    { return filter_input_tokens(lexer.get_tokens()); }
            )
            .and_then(
                [this] () -> g10::result<void>
                    { return preprocess(); }
            )
            .or_else(
                [this] (const std::string& err) -> g10::result<void>
                {
                    m_good = false;

                    // Error message format:
                    //
                    // "Preprocessing Error:"
                    // " - <error one>"
                    // " - <error two>"
                    // " - <...>"
                    // " - In file 'FILENAME:LINE:COLUMN'" (if applicable)

                    std::println(
                        "Preprocessing Error:\n{}",
                        err
                    );

                    return {};
                }
            );
    }
}

/* Private Methods - Initialization *******************************************/

namespace g10asm
{
    auto preprocessor::filter_input_tokens (const std::vector<token>& tokens) 
        -> g10::result<void>
    {
        m_input_tokens.clear();

        for (const auto& tok : tokens)
        {
            switch (tok.type)
            {
                case token_type::new_line:
                case token_type::end_of_file:
                    // Skip these tokens
                    break;

                default:
                    m_input_tokens.push_back(tok);
                    break;
            }
        }

        return {};
    }

    auto preprocessor::resolve_include_paths (
        const std::vector<std::string>& include_dirs)
            -> g10::result<void>
    {
        return {};
    }

    auto preprocessor::preprocess () -> g10::result<void>
    {
        // - Preprocessing good.
        m_good = true;
        return {};
    }
}
