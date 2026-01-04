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
#include <g10asm/lexer.hpp>

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

                    std::println(stderr,
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
                case token_type::end_of_file:
                    // Skip end-of-file tokens
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
        m_current_index = 0;
        m_output_string.clear();
        m_needs_space = false;

        while (is_at_end() == false)
        {
            // Handle line continuation first (backslash + newline)
            if (handle_line_continuation() == true)
            {
                continue;
            }

            auto tok_result = current_token();
            if (tok_result.has_value() == false)
            {
                break;
            }

            const token& tok = tok_result.value();

            // Handle newlines specially - output newline and reset spacing
            if (tok.type == token_type::new_line)
            {
                append_newline();
                advance();
                continue;
            }

            // Check for preprocessor directives
            auto directive_result = handle_directive();
            if (directive_result.has_value() == false)
            {
                return g10::error(
                    "{}\n - In file '{}:{}:{}'",
                    directive_result.error(),
                    tok.source_file,
                    tok.source_line,
                    tok.source_column
                );
            }
            if (directive_result.value() == true)
            {
                continue;
            }

            // Check for identifier interpolation (identifier adjacent to braced expr)
            auto ident_interp_result = handle_identifier_interpolation();
            if (ident_interp_result.has_value() == false)
            {
                return g10::error(
                    "{}\n - In file '{}:{}:{}'",
                    ident_interp_result.error(),
                    tok.source_file,
                    tok.source_line,
                    tok.source_column
                );
            }
            if (ident_interp_result.value() == true)
            {
                continue;
            }

            // Check for string interpolation
            auto string_interp_result = handle_string_interpolation();
            if (string_interp_result.has_value() == false)
            {
                return g10::error(
                    "{}\n - In file '{}:{}:{}'",
                    string_interp_result.error(),
                    tok.source_file,
                    tok.source_line,
                    tok.source_column
                );
            }
            if (string_interp_result.value() == true)
            {
                continue;
            }

            // Check for standalone braced expressions
            auto braced_result = handle_braced_expression();
            if (braced_result.has_value() == false)
            {
                return g10::error(
                    "{}\n - In file '{}:{}:{}'",
                    braced_result.error(),
                    tok.source_file,
                    tok.source_line,
                    tok.source_column
                );
            }
            if (braced_result.value() == true)
            {
                continue;
            }

            // Try to expand macros for identifiers
            if (try_expand_macro() == true)
            {
                continue;
            }

            // For all other tokens, append to output with appropriate spacing
            append_token(tok);
            advance();
        }

        m_good = true;
        return {};
    }
}

/* Private Methods - Token Navigation *****************************************/

namespace g10asm
{
    auto preprocessor::current_token () const -> g10::result_cref<token>
    {
        if (m_current_index >= m_input_tokens.size())
        {
            return g10::error(
                " - Current token index ({}) out of bounds.",
                m_current_index
            );
        }

        return std::cref(m_input_tokens[m_current_index]);
    }

    auto preprocessor::peek_token (std::int64_t offset) const 
        -> g10::result_cref<token>
    {
        std::int64_t target = 
            static_cast<std::int64_t>(m_current_index) + offset;
        
        if (target < 0 || 
            static_cast<std::size_t>(target) >= m_input_tokens.size())
        {
            return g10::error(
                " - Token peek offset out of bounds.\n"
                " - Current index: {}, Offset: {}, Target: {}.",
                m_current_index,
                offset,
                target
            );
        }
        
        return std::cref(m_input_tokens[static_cast<std::size_t>(target)]);
    }

    auto preprocessor::advance (std::size_t count) -> void
    {
        m_current_index += count;
        if (m_current_index > m_input_tokens.size())
        {
            m_current_index = m_input_tokens.size();
        }
    }

    auto preprocessor::is_at_end () const -> bool
    {
        return m_current_index >= m_input_tokens.size();
    }
}

/* Private Methods - Output Building ******************************************/

namespace g10asm
{
    auto preprocessor::is_no_space_before (token_type type) const -> bool
    {
        switch (type)
        {
            case token_type::comma:
            case token_type::colon:
            case token_type::right_parenthesis:
            case token_type::right_bracket:
            case token_type::right_brace:
            case token_type::new_line:
                return true;
            default:
                return false;
        }
    }

    auto preprocessor::is_no_space_after (token_type type) const -> bool
    {
        switch (type)
        {
            case token_type::left_parenthesis:
            case token_type::left_bracket:
            case token_type::left_brace:
            case token_type::new_line:
                return true;
            default:
                return false;
        }
    }

    auto preprocessor::append_token (const token& tok) -> void
    {
        // Add space if needed and token type allows
        if (m_needs_space == true && is_no_space_before(tok.type) == false)
        {
            m_output_string += ' ';
        }

        // Handle string and character literals specially - add quotes back
        if (tok.type == token_type::string_literal)
        {
            m_output_string += '"';
            m_output_string += tok.lexeme;
            m_output_string += '"';
        }
        else if (tok.type == token_type::character_literal)
        {
            m_output_string += '\'';
            m_output_string += tok.lexeme;
            m_output_string += '\'';
        }
        else if (tok.type == token_type::integer_literal && 
                 tok.lexeme.empty() == true && 
                 tok.int_value.has_value() == true)
        {
            // Handle tokens with int_value but no lexeme (from evaluated expressions)
            m_output_string += std::to_string(tok.int_value.value());
        }
        else
        {
            // Append the token's lexeme directly
            m_output_string += tok.lexeme;
        }

        // Update spacing state
        m_needs_space = (is_no_space_after(tok.type) == false);
    }

    auto preprocessor::append_string_view (std::string_view str) -> void
    {
        m_output_string += str;
    }

    auto preprocessor::append_string (const std::string& str) -> void
    {
        m_output_string += str;
    }

    auto preprocessor::append_newline () -> void
    {
        // If a newline was just appended, do not append another, in order to
        // avoid multiple consecutive newlines.
        if (m_output_string.empty() == false &&
            m_output_string.back() == '\n')
        {
            return;
        }

        m_output_string += '\n';
        m_needs_space = false;
    }
}

/* Private Methods - Line Continuation ****************************************/

namespace g10asm
{
    auto preprocessor::handle_line_continuation () -> bool
    {
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return false;
        }

        const token& tok = tok_result.value();

        // Check for backslash token
        if (tok.type != token_type::backslash)
        {
            return false;
        }

        // Check if next token is a newline
        auto next_result = peek_token(1);
        if (next_result.has_value() == false)
        {
            return false;
        }

        const token& next_tok = next_result.value();
        if (next_tok.type != token_type::new_line)
        {
            return false;
        }

        // Line continuation found - skip both backslash and newline
        advance(2);
        return true;
    }
}

/* Private Methods - Directive Handling ***************************************/

namespace g10asm
{
    auto preprocessor::handle_directive () -> g10::result<bool>
    {
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return false;
        }

        const token& tok = tok_result.value();

        // Check if this is a keyword token
        if (tok.type != token_type::keyword)
        {
            return false;
        }

        // Check if it's a preprocessor directive
        if (tok.keyword_value.has_value() == false)
        {
            return false;
        }

        const keyword& kw = tok.keyword_value.value();
        if (kw.type != keyword_type::preprocessor_directive)
        {
            return false;
        }

        // Get the directive type from param1
        const auto dir_type = static_cast<directive_type>(kw.param1);

        switch (dir_type)
        {
            case directive_type::define:
                return handle_define_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::undef:
                return handle_undef_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            default:
                // Not a directive we handle yet - pass through
                return false;
        }
    }

    auto preprocessor::handle_define_directive () -> g10::result<void>
    {
        // Current token is `.define` - store source info before advancing
        auto define_tok_result = current_token();
        if (define_tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .define token.");
        }
        
        const std::string define_source_file { 
            define_tok_result.value().get().source_file 
        };
        const std::size_t define_source_line { 
            define_tok_result.value().get().source_line 
        };
        advance();

        // Next token should be an identifier (the macro name)
        auto name_result = current_token();
        if (name_result.has_value() == false)
        {
            return g10::error(
                " - Expected macro name after '.define'."
            );
        }

        const token& name_tok = name_result.value();
        if (name_tok.type != token_type::identifier)
        {
            return g10::error(
                " - Expected identifier for macro name, got '{}'.",
                token::stringify_type(name_tok.type)
            );
        }

        std::string macro_name { name_tok.lexeme };
        advance();

        // Collect replacement tokens until newline or end of input
        // Evaluate braced expressions inline during collection
        std::vector<token> replacement_tokens {};

        while (is_at_end() == false)
        {
            // Handle line continuation within the replacement text
            if (handle_line_continuation() == true)
            {
                continue;
            }

            auto tok_result = current_token();
            if (tok_result.has_value() == false)
            {
                break;
            }

            const token& tok = tok_result.value();

            // Stop at newline
            if (tok.type == token_type::new_line)
            {
                advance();
                break;
            }

            // Check for braced expression - evaluate and create result token
            if (tok.type == token_type::left_brace)
            {
                // Store source location
                const std::string brace_file { tok.source_file };
                const std::size_t brace_line { tok.source_line };
                const std::size_t brace_col { tok.source_column };
                
                advance();

                // Collect tokens until matching right brace
                std::vector<token> expr_tokens {};
                std::size_t brace_depth = 1;

                while (is_at_end() == false && brace_depth > 0)
                {
                    // Handle line continuation within braced expr
                    if (handle_line_continuation() == true)
                    {
                        continue;
                    }

                    auto inner_tok_result = current_token();
                    if (inner_tok_result.has_value() == false)
                    {
                        break;
                    }

                    const token& inner_tok = inner_tok_result.value();

                    if (inner_tok.type == token_type::left_brace)
                    {
                        ++brace_depth;
                        expr_tokens.push_back(inner_tok);
                        advance();
                    }
                    else if (inner_tok.type == token_type::right_brace)
                    {
                        --brace_depth;
                        if (brace_depth > 0)
                        {
                            expr_tokens.push_back(inner_tok);
                        }
                        advance();
                    }
                    else if (inner_tok.type == token_type::new_line)
                    {
                        // Skip newlines within braced expressions
                        advance();
                    }
                    else
                    {
                        expr_tokens.push_back(inner_tok);
                        advance();
                    }
                }

                if (brace_depth > 0)
                {
                    return g10::error(" - Unmatched '{{' in macro definition.");
                }

                if (expr_tokens.empty() == true)
                {
                    return g10::error(" - Empty braced expression in macro definition.");
                }

                // Evaluate the expression
                pp_evaluator evaluator { expr_tokens, m_macro_table };
                auto eval_result = evaluator.evaluate();
                if (eval_result.has_value() == false)
                {
                    return g10::error(eval_result.error());
                }

                // Create a token for the evaluated result
                std::string result_str = pp_evaluator::value_to_string(eval_result.value());
                
                // Create an integer literal token for the result
                token result_token {};
                result_token.type = token_type::integer_literal;
                result_token.source_file = brace_file;
                result_token.source_line = brace_line;
                result_token.source_column = brace_col;
                
                // Store the result string in the token's lexeme
                // We need to store the lexeme in a way that persists
                // For now, we'll use int_value and construct the lexeme later
                auto int_result = pp_evaluator::to_integer(eval_result.value());
                if (int_result.has_value() == true)
                {
                    result_token.int_value = int_result.value();
                }
                
                replacement_tokens.push_back(result_token);
                continue;
            }

            // Add token to replacement
            replacement_tokens.push_back(tok);
            advance();
        }

        // Define the macro
        auto define_result = m_macro_table.define_text_sub_macro(
            macro_name,
            replacement_tokens,
            define_source_file,
            define_source_line
        );

        if (define_result.has_value() == false)
        {
            return g10::error(define_result.error());
        }

        return {};
    }

    auto preprocessor::handle_undef_directive () -> g10::result<void>
    {
        // Current token is `.undef` or `.purge` - advance past it
        advance();

        // Next token should be an identifier (the macro name)
        auto name_result = current_token();
        if (name_result.has_value() == false)
        {
            return g10::error(
                " - Expected macro name after '.undef'/'.purge'."
            );
        }

        const token& name_tok = name_result.value();
        if (name_tok.type != token_type::identifier)
        {
            return g10::error(
                " - Expected identifier for macro name, got '{}'.",
                token::stringify_type(name_tok.type)
            );
        }

        std::string macro_name { name_tok.lexeme };
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false)
            {
                break;
            }

            const token& tok = tok_result.value();
            if (tok.type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Undefine the macro (ignore errors for undefined macros - just a warning)
        auto undef_result = m_macro_table.undefine_macro(macro_name);
        if (undef_result.has_value() == false)
        {
            // Emit a warning but continue
            std::println(stderr,
                "Preprocessor Warning:\n{}\n - In file '{}:{}:{}'",
                undef_result.error(),
                name_tok.source_file,
                name_tok.source_line,
                name_tok.source_column
            );
        }

        return {};
    }
}

/* Private Methods - Macro Expansion ******************************************/

namespace g10asm
{
    auto preprocessor::try_expand_macro () -> bool
    {
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return false;
        }

        const token& tok = tok_result.value();

        // Only expand identifiers
        if (tok.type != token_type::identifier)
        {
            return false;
        }

        // Look up the macro
        std::string name { tok.lexeme };
        auto macro_result = m_macro_table.lookup_text_sub_macro(name);
        if (macro_result.has_value() == false)
        {
            return false;
        }

        const text_sub_macro& macro = macro_result.value();

        // Skip the macro name token
        advance();

        // Output the replacement tokens
        for (const auto& repl_tok : macro.replacement)
        {
            append_token(repl_tok);
        }

        return true;
    }
}

/* Private Methods - Expression Evaluation ************************************/

namespace g10asm
{
    auto preprocessor::handle_braced_expression () -> g10::result<bool>
    {
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return false;
        }

        const token& tok = tok_result.value();

        // Check for left brace
        if (tok.type != token_type::left_brace)
        {
            return false;
        }

        // Skip the opening brace
        advance();

        // Collect tokens until matching right brace
        std::vector<token> expr_tokens {};
        std::size_t brace_depth = 1;

        while (is_at_end() == false && brace_depth > 0)
        {
            auto inner_tok_result = current_token();
            if (inner_tok_result.has_value() == false)
            {
                break;
            }

            const token& inner_tok = inner_tok_result.value();

            if (inner_tok.type == token_type::left_brace)
            {
                ++brace_depth;
                expr_tokens.push_back(inner_tok);
                advance();
            }
            else if (inner_tok.type == token_type::right_brace)
            {
                --brace_depth;
                if (brace_depth > 0)
                {
                    expr_tokens.push_back(inner_tok);
                }
                advance();
            }
            else if (inner_tok.type == token_type::new_line)
            {
                // Skip newlines within braced expressions
                advance();
            }
            else
            {
                expr_tokens.push_back(inner_tok);
                advance();
            }
        }

        // Check for unmatched brace
        if (brace_depth > 0)
        {
            return g10::error(" - Unmatched '{' in expression.");
        }

        // Check for empty expression
        if (expr_tokens.empty() == true)
        {
            return g10::error(" - Empty braced expression.");
        }

        // Evaluate the expression
        pp_evaluator evaluator { expr_tokens, m_macro_table };
        auto eval_result = evaluator.evaluate();
        if (eval_result.has_value() == false)
        {
            return g10::error(eval_result.error());
        }

        // Convert result to string and append to output
        std::string result_str = pp_evaluator::value_to_string(eval_result.value());
        
        // Add space before if needed
        if (m_needs_space == true && result_str.empty() == false)
        {
            m_output_string += ' ';
        }
        
        m_output_string += result_str;
        m_needs_space = true;

        return true;
    }

    auto preprocessor::handle_identifier_interpolation () -> g10::result<bool>
    {
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return false;
        }

        const token& tok = tok_result.value();

        // Check if this is an identifier followed by a left brace (adjacent)
        // or a left brace that might be followed by an identifier
        bool starts_with_brace = (tok.type == token_type::left_brace);
        bool starts_with_ident = (tok.type == token_type::identifier);

        if (starts_with_brace == false && starts_with_ident == false)
        {
            return false;
        }

        // For identifier, check if next token is an adjacent left brace
        if (starts_with_ident == true)
        {
            auto next_result = peek_token(1);
            if (next_result.has_value() == false)
            {
                return false;
            }

            const token& next_tok = next_result.value();
            if (next_tok.type != token_type::left_brace)
            {
                return false;
            }

            // Check adjacency
            if (are_tokens_adjacent(tok, next_tok) == false)
            {
                return false;
            }
        }

        // For left brace, check if after the closing brace there's an adjacent
        // identifier (or another brace)
        if (starts_with_brace == true)
        {
            // We need to scan ahead to find the closing brace and see what
            // follows
            std::size_t scan_index = m_current_index + 1;
            std::size_t brace_depth = 1;

            while (scan_index < m_input_tokens.size() && brace_depth > 0)
            {
                const token& scan_tok = m_input_tokens[scan_index];
                if (scan_tok.type == token_type::left_brace)
                {
                    ++brace_depth;
                }
                else if (scan_tok.type == token_type::right_brace)
                {
                    --brace_depth;
                }
                ++scan_index;
            }

            if (brace_depth > 0 || scan_index >= m_input_tokens.size())
            {
                return false;  // Not a valid identifier interpolation pattern
            }

            // Check what follows the closing brace
            const token& after_brace = m_input_tokens[scan_index];
            const token& closing_brace = m_input_tokens[scan_index - 1];

            if (after_brace.type != token_type::identifier)
            {
                return false;  // No identifier after brace
            }

            // Check if they're adjacent
            if (are_tokens_adjacent(closing_brace, after_brace) == false)
            {
                return false;
            }
        }

        // Now we know we have an identifier interpolation pattern
        // Build the concatenated identifier
        std::string result_ident {};
        const std::string source_file { tok.source_file };
        const std::size_t source_line { tok.source_line };
        const std::size_t source_col { tok.source_column };

        // Add space before if needed
        if (m_needs_space == true)
        {
            m_output_string += ' ';
        }

        while (is_at_end() == false)
        {
            auto curr_result = current_token();
            if (curr_result.has_value() == false)
            {
                break;
            }

            const token& curr_tok = curr_result.value();

            if (curr_tok.type == token_type::identifier)
            {
                result_ident += curr_tok.lexeme;
                advance();

                // Check if next is adjacent left brace
                auto next_res = current_token();
                if (next_res.has_value() == false)
                {
                    break;
                }

                const token& next_tok = next_res.value();
                if (next_tok.type == token_type::left_brace &&
                    are_tokens_adjacent(curr_tok, next_tok) == true)
                {
                    continue;  // Process the brace
                }
                else
                {
                    break;  // End of interpolation
                }
            }
            else if (curr_tok.type == token_type::left_brace)
            {
                // Evaluate the braced expression
                advance();  // Skip the opening brace

                std::vector<token> expr_tokens {};
                std::size_t brace_depth = 1;

                while (is_at_end() == false && brace_depth > 0)
                {
                    auto inner_result = current_token();
                    if (inner_result.has_value() == false)
                    {
                        break;
                    }

                    const token& inner_tok = inner_result.value();

                    if (inner_tok.type == token_type::left_brace)
                    {
                        ++brace_depth;
                        expr_tokens.push_back(inner_tok);
                        advance();
                    }
                    else if (inner_tok.type == token_type::right_brace)
                    {
                        --brace_depth;
                        if (brace_depth > 0)
                        {
                            expr_tokens.push_back(inner_tok);
                        }
                        advance();
                    }
                    else if (inner_tok.type == token_type::new_line)
                    {
                        advance();
                    }
                    else
                    {
                        expr_tokens.push_back(inner_tok);
                        advance();
                    }
                }

                if (brace_depth > 0)
                {
                    return g10::error(
                        " - Unmatched '{{' in identifier interpolation."
                    );
                }

                if (expr_tokens.empty() == true)
                {
                    return g10::error(
                        " - Empty braced expression in identifier."
                    );
                }

                // Evaluate and append result
                pp_evaluator evaluator { expr_tokens, m_macro_table };
                auto eval_result = evaluator.evaluate();
                if (eval_result.has_value() == false)
                {
                    return g10::error(eval_result.error());
                }

                result_ident += pp_evaluator::value_to_string(eval_result.value());

                // Check if next token is adjacent identifier or brace
                auto after_result = current_token();
                if (after_result.has_value() == false)
                {
                    break;
                }

                const token& after_tok = after_result.value();
                // Get the closing brace we just consumed (use peek back)
                auto closing_result = peek_token(-1);
                if (closing_result.has_value() == false)
                {
                    break;
                }

                const token& closing_tok = closing_result.value();

                if ((after_tok.type == token_type::identifier ||
                     after_tok.type == token_type::left_brace) &&
                    are_tokens_adjacent(closing_tok, after_tok) == true)
                {
                    continue;  // Continue building the identifier
                }
                else
                {
                    break;  // End of interpolation
                }
            }
            else
            {
                break;  // Not part of the identifier pattern
            }
        }

        m_output_string += result_ident;
        m_needs_space = true;

        return true;
    }

    auto preprocessor::handle_string_interpolation () -> g10::result<bool>
    {
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return false;
        }

        const token& tok = tok_result.value();

        // Only handle string literals
        if (tok.type != token_type::string_literal)
        {
            return false;
        }

        std::string_view content = tok.lexeme;
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Check if the string contains any braced expressions
        if (content.find('{') == std::string_view::npos)
        {
            return false;  // No interpolation needed
        }

        // Process the string content for interpolations
        std::string result {};
        std::size_t pos = 0;

        while (pos < content.size())
        {
            // Find the next '{'
            std::size_t brace_pos = content.find('{', pos);

            if (brace_pos == std::string_view::npos)
            {
                // No more braces, append rest of string
                result += content.substr(pos);
                break;
            }

            // Append text before the brace
            result += content.substr(pos, brace_pos - pos);

            // Find the matching '}'
            std::size_t expr_start = brace_pos + 1;
            std::size_t depth = 1;
            std::size_t expr_end = expr_start;

            while (expr_end < content.size() && depth > 0)
            {
                if (content[expr_end] == '{')
                {
                    ++depth;
                }
                else if (content[expr_end] == '}')
                {
                    --depth;
                }
                ++expr_end;
            }

            if (depth > 0)
            {
                return g10::error(
                    " - Unmatched '{{' in string literal."
                );
            }

            // Extract expression content (without braces)
            std::string expr_content { content.substr(expr_start, expr_end - expr_start - 1) };

            // Evaluate the expression
            auto eval_result = evaluate_inline_expression(
                expr_content,
                source_file,
                source_line
            );

            if (eval_result.has_value() == false)
            {
                return g10::error(eval_result.error());
            }

            result += eval_result.value();
            pos = expr_end;
        }

        // Add space before if needed
        if (m_needs_space == true)
        {
            m_output_string += ' ';
        }

        // Output the interpolated string with quotes
        m_output_string += '"';
        m_output_string += result;
        m_output_string += '"';
        m_needs_space = true;

        advance();
        return true;
    }

    auto preprocessor::evaluate_inline_expression (
        const std::string& expr_content,
        const std::string& source_file,
        std::size_t source_line
    ) -> g10::result<std::string>
    {
        // Tokenize the expression content
        lexer expr_lexer {};
        auto lex_result = expr_lexer.load_from_string(expr_content);
        if (lex_result.has_value() == false)
        {
            return g10::error(
                " - Failed to tokenize inline expression: {}",
                lex_result.error()
            );
        }

        // Collect tokens (excluding EOF)
        std::vector<token> expr_tokens {};
        while (true)
        {
            auto tok_result = expr_lexer.peek_token(0);
            if (tok_result.has_value() == false)
            {
                break;
            }

            const token& tok = tok_result.value();
            if (tok.type == token_type::end_of_file ||
                tok.type == token_type::new_line)
            {
                break;
            }

            expr_tokens.push_back(tok);
            expr_lexer.skip_tokens(1);
        }

        if (expr_tokens.empty() == true)
        {
            return g10::error(" - Empty expression in string interpolation.");
        }

        // Evaluate the expression
        pp_evaluator evaluator { expr_tokens, m_macro_table };
        auto eval_result = evaluator.evaluate();
        if (eval_result.has_value() == false)
        {
            return g10::error(eval_result.error());
        }

        return pp_evaluator::value_to_string(eval_result.value());
    }

    auto preprocessor::are_tokens_adjacent (
        const token& first,
        const token& second
    ) const -> bool
    {
        // Tokens are adjacent if they're on the same line and the second
        // starts immediately after the first ends
        if (first.source_line != second.source_line)
        {
            return false;
        }

        std::size_t first_end = first.source_column + first.lexeme.size();
        return (first_end == second.source_column);
    }
}
