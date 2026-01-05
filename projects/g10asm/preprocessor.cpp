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
        m_conditional_stack.clear();

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

            // If we're in an inactive conditional branch, skip tokens
            // Note: Directive handling must still occur to track nested conditionals
            if (is_conditionally_active() == false)
            {
                // Skip inactive conditional block
                auto skip_result = skip_conditional_block();
                if (skip_result.has_value() == false)
                {
                    return g10::error(
                        "{}\n - In file '{}:{}:{}'",
                        skip_result.error(),
                        tok.source_file,
                        tok.source_line,
                        tok.source_column
                    );
                }
                continue;
            }

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

        // Check for unclosed conditional blocks
        if (m_conditional_stack.empty() == false)
        {
            const auto& unclosed = m_conditional_stack.back();
            return g10::error(
                " - Unclosed conditional block.\n"
                " - Started at '{}:{}'",
                unclosed.source_file,
                unclosed.source_line
            );
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

            case directive_type::if_:
                return handle_if_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::ifdef_:
                return handle_ifdef_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::ifndef_:
                return handle_ifndef_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::elseif:
                return handle_elif_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::else_:
                return handle_else_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::endif:
                return handle_endif_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::repeat:
                return handle_repeat_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::endrepeat:
                return handle_endrepeat_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::for_:
                return handle_for_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::endfor:
                return handle_endfor_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::while_:
                return handle_while_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::endwhile:
                return handle_endwhile_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::continue_:
                return handle_continue_directive()
                    .and_then([] () -> g10::result<bool> { return true; });

            case directive_type::break_:
                return handle_break_directive()
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

/* Private Methods - Conditional Assembly *************************************/

namespace g10asm
{
    auto preprocessor::is_conditionally_active () const -> bool
    {
        // If no conditionals on the stack, we're active
        if (m_conditional_stack.empty() == true)
        {
            return true;
        }

        // Otherwise, check if the top of the stack is active
        return m_conditional_stack.back().currently_active;
    }

    auto preprocessor::skip_conditional_block () -> g10::result<void>
    {
        // Skip tokens until we find a matching .elif, .else, or .endif
        // Must track nested conditionals to properly match
        std::size_t nesting_depth = 1;  // We're inside one conditional

        while (is_at_end() == false && nesting_depth > 0)
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false)
            {
                break;
            }

            const token& tok = tok_result.value();

            // Check if this is a preprocessor directive
            if (tok.type == token_type::keyword &&
                tok.keyword_value.has_value() == true)
            {
                const keyword& kw = tok.keyword_value.value();
                if (kw.type == keyword_type::preprocessor_directive)
                {
                    const auto dir_type = static_cast<directive_type>(kw.param1);

                    switch (dir_type)
                    {
                        case directive_type::if_:
                        case directive_type::ifdef_:
                        case directive_type::ifndef_:
                            // Nested conditional - increase depth
                            ++nesting_depth;
                            advance();
                            break;

                        case directive_type::endif:
                            --nesting_depth;
                            if (nesting_depth == 0)
                            {
                                // This is our matching .endif
                                return handle_endif_directive();
                            }
                            advance();
                            break;

                        case directive_type::elseif:
                            if (nesting_depth == 1)
                            {
                                // This is at our level - handle it
                                return handle_elif_directive();
                            }
                            advance();
                            break;

                        case directive_type::else_:
                            if (nesting_depth == 1)
                            {
                                // This is at our level - handle it
                                return handle_else_directive();
                            }
                            advance();
                            break;

                        default:
                            advance();
                            break;
                    }
                }
                else
                {
                    advance();
                }
            }
            else
            {
                advance();
            }
        }

        // If we get here with nesting_depth > 0, we have an unclosed conditional
        if (nesting_depth > 0)
        {
            return g10::error(" - Unexpected end of file in conditional block.");
        }

        return {};
    }

    auto preprocessor::validate_no_braces (
        const std::vector<token>& tokens,
        std::string_view directive_name
    ) -> g10::result<void>
    {
        for (const auto& tok : tokens)
        {
            if (tok.type == token_type::left_brace ||
                tok.type == token_type::right_brace)
            {
                return g10::error(
                    " - Curly braces are not allowed in '{}' expressions.\n"
                    " - The entire argument is already an expression; "
                    "braces are not required.",
                    directive_name
                );
            }
        }
        return {};
    }

    auto preprocessor::evaluate_condition () -> g10::result<bool>
    {
        // Collect tokens until newline
        std::vector<token> condition_tokens {};

        while (is_at_end() == false)
        {
            // Handle line continuation
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

            if (tok.type == token_type::new_line)
            {
                advance();
                break;
            }

            condition_tokens.push_back(tok);
            advance();
        }

        if (condition_tokens.empty() == true)
        {
            return g10::error(" - Expected condition after directive.");
        }

        // Validate no braces in condition expression
        auto validate_result = validate_no_braces(condition_tokens, ".if/.elif");
        if (validate_result.has_value() == false)
        {
            return g10::error(validate_result.error());
        }

        // Evaluate the condition expression
        pp_evaluator evaluator { condition_tokens, m_macro_table };
        auto eval_result = evaluator.evaluate();
        if (eval_result.has_value() == false)
        {
            return g10::error(
                " - Failed to evaluate condition:\n{}",
                eval_result.error()
            );
        }

        // Convert result to boolean
        return pp_evaluator::to_boolean(eval_result.value());
    }

    auto preprocessor::handle_if_directive () -> g10::result<void>
    {
        // Store source info for error reporting
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .if token.");
        }
        const token& tok = tok_result.value();
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Advance past .if
        advance();

        // Evaluate the condition
        auto condition_result = evaluate_condition();
        if (condition_result.has_value() == false)
        {
            return g10::error(condition_result.error());
        }

        bool condition = condition_result.value();

        // Determine if this branch should be active
        // It's only active if parent is active AND condition is true
        bool parent_active = is_conditionally_active();
        bool should_be_active = parent_active && condition;

        // Push new conditional state
        m_conditional_stack.push_back({
            .condition_met = should_be_active,
            .currently_active = should_be_active,
            .else_seen = false,
            .source_file = source_file,
            .source_line = source_line
        });

        return {};
    }

    auto preprocessor::handle_ifdef_directive () -> g10::result<void>
    {
        // Store source info for error reporting
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .ifdef token.");
        }
        const token& tok = tok_result.value();
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Advance past .ifdef
        advance();

        // Get the macro name
        auto name_result = current_token();
        if (name_result.has_value() == false)
        {
            return g10::error(" - Expected macro name after '.ifdef'.");
        }

        const token& name_tok = name_result.value();
        if (name_tok.type != token_type::identifier)
        {
            return g10::error(
                " - Expected identifier after '.ifdef', got '{}'.",
                token::stringify_type(name_tok.type)
            );
        }

        std::string macro_name { name_tok.lexeme };
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Check if macro is defined
        bool is_defined = m_macro_table.is_macro_defined(macro_name);

        // Determine if this branch should be active
        bool parent_active = is_conditionally_active();
        bool should_be_active = parent_active && is_defined;

        // Push new conditional state
        m_conditional_stack.push_back({
            .condition_met = should_be_active,
            .currently_active = should_be_active,
            .else_seen = false,
            .source_file = source_file,
            .source_line = source_line
        });

        return {};
    }

    auto preprocessor::handle_ifndef_directive () -> g10::result<void>
    {
        // Store source info for error reporting
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .ifndef token.");
        }
        const token& tok = tok_result.value();
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Advance past .ifndef
        advance();

        // Get the macro name
        auto name_result = current_token();
        if (name_result.has_value() == false)
        {
            return g10::error(" - Expected macro name after '.ifndef'.");
        }

        const token& name_tok = name_result.value();
        if (name_tok.type != token_type::identifier)
        {
            return g10::error(
                " - Expected identifier after '.ifndef', got '{}'.",
                token::stringify_type(name_tok.type)
            );
        }

        std::string macro_name { name_tok.lexeme };
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Check if macro is NOT defined
        bool is_not_defined = (m_macro_table.is_macro_defined(macro_name) == false);

        // Determine if this branch should be active
        bool parent_active = is_conditionally_active();
        bool should_be_active = parent_active && is_not_defined;

        // Push new conditional state
        m_conditional_stack.push_back({
            .condition_met = should_be_active,
            .currently_active = should_be_active,
            .else_seen = false,
            .source_file = source_file,
            .source_line = source_line
        });

        return {};
    }

    auto preprocessor::handle_elif_directive () -> g10::result<void>
    {
        // Check if there's a matching .if
        if (m_conditional_stack.empty() == true)
        {
            return g10::error(" - '.elif'/'.elseif' without matching '.if'.");
        }

        conditional_state& state = m_conditional_stack.back();

        // Check if .else was already seen
        if (state.else_seen == true)
        {
            return g10::error(" - '.elif'/'.elseif' after '.else'.");
        }

        // Advance past .elif
        advance();

        // Evaluate the condition
        auto condition_result = evaluate_condition();
        if (condition_result.has_value() == false)
        {
            return g10::error(condition_result.error());
        }

        bool condition = condition_result.value();

        // Determine if this branch should be active
        // Only active if no previous branch was taken AND condition is true
        // AND parent (if any) is active
        bool parent_active = true;
        if (m_conditional_stack.size() > 1)
        {
            // Check parent's state (second from top)
            parent_active = m_conditional_stack[m_conditional_stack.size() - 2].currently_active;
        }

        if (state.condition_met == true)
        {
            // A previous branch was already taken - stay inactive
            state.currently_active = false;
        }
        else if (parent_active && condition)
        {
            // This branch should be active
            state.condition_met = true;
            state.currently_active = true;
        }
        else
        {
            state.currently_active = false;
        }

        return {};
    }

    auto preprocessor::handle_else_directive () -> g10::result<void>
    {
        // Check if there's a matching .if
        if (m_conditional_stack.empty() == true)
        {
            return g10::error(" - '.else' without matching '.if'.");
        }

        conditional_state& state = m_conditional_stack.back();

        // Check if .else was already seen
        if (state.else_seen == true)
        {
            return g10::error(" - Duplicate '.else' in conditional block.");
        }

        // Mark that we've seen .else
        state.else_seen = true;

        // Advance past .else
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false) break;
            if (tok_result.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Determine if this branch should be active
        // Only active if no previous branch was taken AND parent is active
        bool parent_active = true;
        if (m_conditional_stack.size() > 1)
        {
            parent_active = m_conditional_stack[m_conditional_stack.size() - 2].currently_active;
        }

        if (state.condition_met == true)
        {
            // A previous branch was already taken - stay inactive
            state.currently_active = false;
        }
        else if (parent_active)
        {
            // This is the fallback branch - activate it
            state.condition_met = true;
            state.currently_active = true;
        }
        else
        {
            state.currently_active = false;
        }

        return {};
    }

    auto preprocessor::handle_endif_directive () -> g10::result<void>
    {
        // Check if there's a matching .if
        if (m_conditional_stack.empty() == true)
        {
            return g10::error(" - '.endif'/'.endc' without matching '.if'.");
        }

        // Pop the conditional state
        m_conditional_stack.pop_back();

        // Advance past .endif
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false) break;
            if (tok_result.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        return {};
    }
}

/* Private Methods - Loop Assembly ********************************************/

namespace g10asm
{
    auto preprocessor::is_in_loop () const -> bool
    {
        return (m_loop_stack.empty() == false);
    }

    auto preprocessor::collect_loop_body (directive_type end_directive)
        -> g10::result<std::vector<token>>
    {
        std::vector<token> body_tokens {};
        std::size_t nesting_depth = 1;

        // Determine which start directives to track for nesting
        auto is_loop_start = [] (directive_type dt) -> bool
        {
            return dt == directive_type::repeat ||
                   dt == directive_type::for_ ||
                   dt == directive_type::while_;
        };

        auto is_loop_end = [] (directive_type dt) -> bool
        {
            return dt == directive_type::endrepeat ||
                   dt == directive_type::endfor ||
                   dt == directive_type::endwhile;
        };

        while (is_at_end() == false && nesting_depth > 0)
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false)
            {
                break;
            }

            const token& tok = tok_result.value();

            // Check if this is a loop directive
            if (tok.type == token_type::keyword &&
                tok.keyword_value.has_value() == true)
            {
                const keyword& kw = tok.keyword_value.value();
                if (kw.type == keyword_type::preprocessor_directive)
                {
                    const auto dir_type = static_cast<directive_type>(kw.param1);

                    if (is_loop_start(dir_type))
                    {
                        ++nesting_depth;
                    }
                    else if (is_loop_end(dir_type))
                    {
                        --nesting_depth;
                        if (nesting_depth == 0)
                        {
                            // Don't include the end directive in body
                            break;
                        }
                    }
                }
            }

            body_tokens.push_back(tok);
            advance();
        }

        if (nesting_depth > 0)
        {
            return g10::error(" - Unclosed loop block.");
        }

        return body_tokens;
    }

    auto preprocessor::process_loop_iteration (
        const std::vector<token>& body_tokens,
        loop_state& loop
    ) -> g10::result<void>
    {
        // Save current input tokens and index
        std::vector<token> saved_tokens = std::move(m_input_tokens);
        std::size_t saved_index = m_current_index;
        
        // Save conditional stack depth - conditionals opened in loop should
        // be closed within the loop, but if break/continue exits early, we
        // need to restore to this depth
        std::size_t saved_cond_depth = m_conditional_stack.size();

        // Set body tokens as input
        m_input_tokens = body_tokens;
        m_current_index = 0;

        // Define/update loop variable if present
        if (loop.variable_name.empty() == false)
        {
            // Create a synthetic token for the variable value
            token value_token {};
            value_token.type = token_type::integer_literal;
            value_token.lexeme = std::to_string(loop.current_value);
            value_token.int_value = loop.current_value;
            value_token.source_file = loop.source_file;
            value_token.source_line = loop.source_line;

            std::vector<token> replacement { value_token };
            m_macro_table.define_text_sub_macro(
                loop.variable_name,
                replacement,
                loop.source_file,
                loop.source_line
            );
        }

        // Process the loop body
        while (is_at_end() == false)
        {
            // Check if loop should break or continue
            if (loop.should_break || loop.should_continue)
            {
                break;
            }

            // Handle line continuation
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

            // Handle newlines
            if (tok.type == token_type::new_line)
            {
                append_newline();
                advance();
                continue;
            }

            // If in inactive conditional, skip
            if (is_conditionally_active() == false)
            {
                auto skip_result = skip_conditional_block();
                if (skip_result.has_value() == false)
                {
                    // Restore state before returning error
                    m_input_tokens = std::move(saved_tokens);
                    m_current_index = saved_index;
                    return g10::error(skip_result.error());
                }
                continue;
            }

            // Handle directives
            auto directive_result = handle_directive();
            if (directive_result.has_value() == false)
            {
                m_input_tokens = std::move(saved_tokens);
                m_current_index = saved_index;
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

            // Handle identifier interpolation
            auto ident_result = handle_identifier_interpolation();
            if (ident_result.has_value() == false)
            {
                m_input_tokens = std::move(saved_tokens);
                m_current_index = saved_index;
                return g10::error(ident_result.error());
            }
            if (ident_result.value() == true)
            {
                continue;
            }

            // Handle string interpolation
            auto string_result = handle_string_interpolation();
            if (string_result.has_value() == false)
            {
                m_input_tokens = std::move(saved_tokens);
                m_current_index = saved_index;
                return g10::error(string_result.error());
            }
            if (string_result.value() == true)
            {
                continue;
            }

            // Handle braced expressions
            auto braced_result = handle_braced_expression();
            if (braced_result.has_value() == false)
            {
                m_input_tokens = std::move(saved_tokens);
                m_current_index = saved_index;
                return g10::error(braced_result.error());
            }
            if (braced_result.value() == true)
            {
                continue;
            }

            // Try macro expansion
            if (try_expand_macro() == true)
            {
                continue;
            }

            // Append token
            append_token(tok);
            advance();
        }

        // Restore conditional stack to saved depth (in case break/continue
        // exited early from within a conditional block)
        while (m_conditional_stack.size() > saved_cond_depth)
        {
            m_conditional_stack.pop_back();
        }

        // Restore state
        m_input_tokens = std::move(saved_tokens);
        m_current_index = saved_index;

        return {};
    }

    auto preprocessor::handle_repeat_directive () -> g10::result<void>
    {
        // Store source info
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .repeat token.");
        }
        const token& tok = tok_result.value();
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Advance past .repeat
        advance();

        // Collect count expression tokens until comma or newline
        std::vector<token> count_tokens {};
        std::string var_name {};

        while (is_at_end() == false)
        {
            if (handle_line_continuation()) continue;

            auto t = current_token();
            if (t.has_value() == false) break;
            const token& curr = t.value();

            if (curr.type == token_type::new_line)
            {
                advance();
                break;
            }

            if (curr.type == token_type::comma)
            {
                advance();
                // Next token should be variable name
                auto var_result = current_token();
                if (var_result.has_value() == true)
                {
                    const token& var_tok = var_result.value();
                    if (var_tok.type == token_type::identifier)
                    {
                        var_name = var_tok.lexeme;
                        advance();
                    }
                }
                // Skip to end of line
                while (is_at_end() == false)
                {
                    auto nl = current_token();
                    if (nl.has_value() == false) break;
                    if (nl.value().get().type == token_type::new_line)
                    {
                        advance();
                        break;
                    }
                    advance();
                }
                break;
            }

            count_tokens.push_back(curr);
            advance();
        }

        if (count_tokens.empty())
        {
            return g10::error(" - Expected count expression after '.repeat'.");
        }

        // Validate no braces in count expression
        auto validate_result = validate_no_braces(count_tokens, ".repeat/.rept");
        if (validate_result.has_value() == false)
        {
            return g10::error(validate_result.error());
        }

        // Evaluate count
        pp_evaluator evaluator { count_tokens, m_macro_table };
        auto eval_result = evaluator.evaluate();
        if (eval_result.has_value() == false)
        {
            return g10::error(
                " - Failed to evaluate repeat count:\n{}",
                eval_result.error()
            );
        }

        auto count_result = pp_evaluator::to_integer(eval_result.value());
        if (count_result.has_value() == false)
        {
            return g10::error(" - Repeat count must be an integer.");
        }

        std::int64_t count = count_result.value();

        // Collect loop body
        auto body_result = collect_loop_body(directive_type::endrepeat);
        if (body_result.has_value() == false)
        {
            return g10::error(
                "{}\n - In '.repeat' starting at '{}:{}'",
                body_result.error(),
                source_file,
                source_line
            );
        }

        std::vector<token> body_tokens = std::move(body_result.value());

        // Skip the .endrepeat directive
        advance();
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Execute loop
        if (count <= 0)
        {
            return {};  // Skip body entirely
        }

        loop_state loop {};
        loop.type = loop_type::repeat;
        loop.variable_name = var_name;
        loop.max_iterations = static_cast<std::size_t>(count);
        loop.source_file = source_file;
        loop.source_line = source_line;

        m_loop_stack.push_back(loop);

        for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i)
        {
            m_loop_stack.back().iteration_count = i;
            m_loop_stack.back().current_value = static_cast<std::int64_t>(i);
            m_loop_stack.back().should_continue = false;

            auto iter_result = process_loop_iteration(
                body_tokens, 
                m_loop_stack.back()
            );

            if (iter_result.has_value() == false)
            {
                m_loop_stack.pop_back();
                if (var_name.empty() == false)
                {
                    m_macro_table.undefine_macro(var_name);
                }
                return g10::error(iter_result.error());
            }

            if (m_loop_stack.back().should_break)
            {
                break;
            }
        }

        m_loop_stack.pop_back();

        // Remove loop variable
        if (var_name.empty() == false)
        {
            m_macro_table.undefine_macro(var_name);
        }

        return {};
    }

    auto preprocessor::handle_endrepeat_directive () -> g10::result<void>
    {
        // This should only be reached if there's an unmatched .endrepeat
        return g10::error(" - '.endrepeat'/'.endr' without matching '.repeat'/'.rept'.");
    }

    auto preprocessor::handle_for_directive () -> g10::result<void>
    {
        // Store source info
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .for token.");
        }
        const token& tok = tok_result.value();
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Advance past .for
        advance();

        // Parse: <var>, <start>, <end> [, <step>]
        std::string var_name {};
        std::vector<token> start_tokens {};
        std::vector<token> end_tokens {};
        std::vector<token> step_tokens {};

        // Get variable name
        auto var_result = current_token();
        if (var_result.has_value() == false)
        {
            return g10::error(" - Expected variable name after '.for'.");
        }
        
        const token& var_tok = var_result.value();
        if (var_tok.type != token_type::identifier)
        {
            return g10::error(
                " - Expected identifier for loop variable, got '{}'.",
                token::stringify_type(var_tok.type)
            );
        }
        var_name = var_tok.lexeme;
        advance();

        // Expect comma
        auto comma1 = current_token();
        if (comma1.has_value() == false || 
            comma1.value().get().type != token_type::comma)
        {
            return g10::error(" - Expected ',' after loop variable in '.for'.");
        }
        advance();

        // Collect start expression
        while (is_at_end() == false)
        {
            if (handle_line_continuation()) continue;
            auto t = current_token();
            if (t.has_value() == false) break;
            const token& curr = t.value();
            if (curr.type == token_type::comma || 
                curr.type == token_type::new_line)
            {
                break;
            }
            start_tokens.push_back(curr);
            advance();
        }

        if (start_tokens.empty())
        {
            return g10::error(" - Expected start value in '.for'.");
        }

        // Expect comma
        auto comma2 = current_token();
        if (comma2.has_value() == false || 
            comma2.value().get().type != token_type::comma)
        {
            return g10::error(" - Expected ',' after start value in '.for'.");
        }
        advance();

        // Collect end expression
        while (is_at_end() == false)
        {
            if (handle_line_continuation()) continue;
            auto t = current_token();
            if (t.has_value() == false) break;
            const token& curr = t.value();
            if (curr.type == token_type::comma || 
                curr.type == token_type::new_line)
            {
                break;
            }
            end_tokens.push_back(curr);
            advance();
        }

        if (end_tokens.empty())
        {
            return g10::error(" - Expected end value in '.for'.");
        }

        // Check for optional step
        auto maybe_comma = current_token();
        if (maybe_comma.has_value() == true && 
            maybe_comma.value().get().type == token_type::comma)
        {
            advance();
            // Collect step expression
            while (is_at_end() == false)
            {
                if (handle_line_continuation()) continue;
                auto t = current_token();
                if (t.has_value() == false) break;
                const token& curr = t.value();
                if (curr.type == token_type::new_line)
                {
                    break;
                }
                step_tokens.push_back(curr);
                advance();
            }
        }

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Validate no braces in expressions
        auto validate_start = validate_no_braces(start_tokens, ".for");
        if (validate_start.has_value() == false)
        {
            return g10::error(validate_start.error());
        }

        auto validate_end = validate_no_braces(end_tokens, ".for");
        if (validate_end.has_value() == false)
        {
            return g10::error(validate_end.error());
        }

        if (step_tokens.empty() == false)
        {
            auto validate_step = validate_no_braces(step_tokens, ".for");
            if (validate_step.has_value() == false)
            {
                return g10::error(validate_step.error());
            }
        }

        // Evaluate start, end, step
        pp_evaluator start_eval { start_tokens, m_macro_table };
        auto start_result = start_eval.evaluate();
        if (start_result.has_value() == false)
        {
            return g10::error(" - Failed to evaluate start value: {}", 
                start_result.error());
        }
        auto start_int = pp_evaluator::to_integer(start_result.value());
        if (start_int.has_value() == false)
        {
            return g10::error(" - Start value must be an integer.");
        }
        std::int64_t start_value = start_int.value();

        pp_evaluator end_eval { end_tokens, m_macro_table };
        auto end_result = end_eval.evaluate();
        if (end_result.has_value() == false)
        {
            return g10::error(" - Failed to evaluate end value: {}", 
                end_result.error());
        }
        auto end_int = pp_evaluator::to_integer(end_result.value());
        if (end_int.has_value() == false)
        {
            return g10::error(" - End value must be an integer.");
        }
        std::int64_t end_value = end_int.value();

        std::int64_t step_value = 1;
        if (step_tokens.empty() == false)
        {
            pp_evaluator step_eval { step_tokens, m_macro_table };
            auto step_result = step_eval.evaluate();
            if (step_result.has_value() == false)
            {
                return g10::error(" - Failed to evaluate step value: {}", 
                    step_result.error());
            }
            auto step_int = pp_evaluator::to_integer(step_result.value());
            if (step_int.has_value() == false)
            {
                return g10::error(" - Step value must be an integer.");
            }
            step_value = step_int.value();
        }

        if (step_value == 0)
        {
            return g10::error(" - Step value cannot be zero in '.for' loop.");
        }

        // Collect loop body
        auto body_result = collect_loop_body(directive_type::endfor);
        if (body_result.has_value() == false)
        {
            return g10::error(
                "{}\n - In '.for' starting at '{}:{}'",
                body_result.error(),
                source_file,
                source_line
            );
        }

        std::vector<token> body_tokens = std::move(body_result.value());

        // Skip the .endfor directive
        advance();
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Execute loop
        loop_state loop {};
        loop.type = loop_type::for_loop;
        loop.variable_name = var_name;
        loop.current_value = start_value;
        loop.end_value = end_value;
        loop.step_value = step_value;
        loop.source_file = source_file;
        loop.source_line = source_line;

        m_loop_stack.push_back(loop);

        std::size_t iteration = 0;
        std::int64_t i = start_value;

        auto should_continue_loop = [&] () -> bool
        {
            if (step_value > 0)
            {
                return i < end_value;
            }
            else
            {
                return i > end_value;
            }
        };

        while (should_continue_loop())
        {
            m_loop_stack.back().iteration_count = iteration;
            m_loop_stack.back().current_value = i;
            m_loop_stack.back().should_continue = false;

            auto iter_result = process_loop_iteration(
                body_tokens, 
                m_loop_stack.back()
            );

            if (iter_result.has_value() == false)
            {
                m_loop_stack.pop_back();
                m_macro_table.undefine_macro(var_name);
                return g10::error(iter_result.error());
            }

            if (m_loop_stack.back().should_break)
            {
                break;
            }

            i += step_value;
            ++iteration;
        }

        m_loop_stack.pop_back();
        m_macro_table.undefine_macro(var_name);

        return {};
    }

    auto preprocessor::handle_endfor_directive () -> g10::result<void>
    {
        return g10::error(" - '.endfor'/'.endf' without matching '.for'.");
    }

    auto preprocessor::handle_while_directive () -> g10::result<void>
    {
        // Store source info
        auto tok_result = current_token();
        if (tok_result.has_value() == false)
        {
            return g10::error(" - Internal error: expected .while token.");
        }
        const token& tok = tok_result.value();
        std::string source_file { tok.source_file };
        std::size_t source_line = tok.source_line;

        // Advance past .while
        advance();

        // Collect condition tokens until comma or newline
        std::vector<token> condition_tokens {};
        std::string var_name {};

        while (is_at_end() == false)
        {
            if (handle_line_continuation()) continue;

            auto t = current_token();
            if (t.has_value() == false) break;
            const token& curr = t.value();

            if (curr.type == token_type::new_line)
            {
                advance();
                break;
            }

            if (curr.type == token_type::comma)
            {
                advance();
                // Next token should be variable name
                auto var_result = current_token();
                if (var_result.has_value() == true)
                {
                    const token& var_tok = var_result.value();
                    if (var_tok.type == token_type::identifier)
                    {
                        var_name = var_tok.lexeme;
                        advance();
                    }
                }
                // Skip to end of line
                while (is_at_end() == false)
                {
                    auto nl = current_token();
                    if (nl.has_value() == false) break;
                    if (nl.value().get().type == token_type::new_line)
                    {
                        advance();
                        break;
                    }
                    advance();
                }
                break;
            }

            condition_tokens.push_back(curr);
            advance();
        }

        if (condition_tokens.empty())
        {
            return g10::error(" - Expected condition after '.while'.");
        }

        // Validate no braces in condition expression
        auto validate_result = validate_no_braces(condition_tokens, ".while");
        if (validate_result.has_value() == false)
        {
            return g10::error(validate_result.error());
        }

        // Collect loop body
        auto body_result = collect_loop_body(directive_type::endwhile);
        if (body_result.has_value() == false)
        {
            return g10::error(
                "{}\n - In '.while' starting at '{}:{}'",
                body_result.error(),
                source_file,
                source_line
            );
        }

        std::vector<token> body_tokens = std::move(body_result.value());

        // Skip the .endwhile directive
        advance();
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Execute loop
        loop_state loop {};
        loop.type = loop_type::while_loop;
        loop.variable_name = var_name;
        loop.condition_tokens = condition_tokens;
        loop.source_file = source_file;
        loop.source_line = source_line;

        m_loop_stack.push_back(loop);

        std::size_t iteration = 0;
        constexpr std::size_t MAX_ITERATIONS = 1000000;  // Safety limit

        while (iteration < MAX_ITERATIONS)
        {
            // Evaluate condition
            pp_evaluator cond_eval { condition_tokens, m_macro_table };
            auto cond_result = cond_eval.evaluate();
            if (cond_result.has_value() == false)
            {
                m_loop_stack.pop_back();
                if (var_name.empty() == false)
                {
                    m_macro_table.undefine_macro(var_name);
                }
                return g10::error(
                    " - Failed to evaluate while condition:\n{}",
                    cond_result.error()
                );
            }

            bool condition = pp_evaluator::to_boolean(cond_result.value());
            if (condition == false)
            {
                break;
            }

            m_loop_stack.back().iteration_count = iteration;
            m_loop_stack.back().current_value = 
                static_cast<std::int64_t>(iteration);
            m_loop_stack.back().should_continue = false;

            auto iter_result = process_loop_iteration(
                body_tokens, 
                m_loop_stack.back()
            );

            if (iter_result.has_value() == false)
            {
                m_loop_stack.pop_back();
                if (var_name.empty() == false)
                {
                    m_macro_table.undefine_macro(var_name);
                }
                return g10::error(iter_result.error());
            }

            if (m_loop_stack.back().should_break)
            {
                break;
            }

            ++iteration;
        }

        if (iteration >= MAX_ITERATIONS)
        {
            m_loop_stack.pop_back();
            if (var_name.empty() == false)
            {
                m_macro_table.undefine_macro(var_name);
            }
            return g10::error(
                " - While loop exceeded maximum iterations ({}).\n"
                " - Possible infinite loop at '{}:{}'",
                MAX_ITERATIONS,
                source_file,
                source_line
            );
        }

        m_loop_stack.pop_back();
        if (var_name.empty() == false)
        {
            m_macro_table.undefine_macro(var_name);
        }

        return {};
    }

    auto preprocessor::handle_endwhile_directive () -> g10::result<void>
    {
        return g10::error(" - '.endwhile'/'.endw' without matching '.while'.");
    }

    auto preprocessor::handle_continue_directive () -> g10::result<void>
    {
        if (m_loop_stack.empty())
        {
            return g10::error(" - '.continue' outside of loop.");
        }

        // Advance past .continue
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Set flag on innermost loop
        m_loop_stack.back().should_continue = true;

        return {};
    }

    auto preprocessor::handle_break_directive () -> g10::result<void>
    {
        if (m_loop_stack.empty())
        {
            return g10::error(" - '.break' outside of loop.");
        }

        // Advance past .break
        advance();

        // Skip to end of line
        while (is_at_end() == false)
        {
            auto t = current_token();
            if (t.has_value() == false) break;
            if (t.value().get().type == token_type::new_line)
            {
                advance();
                break;
            }
            advance();
        }

        // Set flag on innermost loop
        m_loop_stack.back().should_break = true;

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

        bool quote_string = true;
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

                // If the token after this is a colon, do not quote the string
                if (brace_depth == 0 && quote_string == true)
                {
                    auto next_result = peek_token(1);
                    if (next_result.has_value() == true)
                    {
                        const token& next_tok = next_result.value();
                        if (next_tok.type == token_type::colon)
                        {
                            quote_string = false;
                        }
                    }
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
        if (
            std::holds_alternative<pp_string>(eval_result.value()) == true &&
            quote_string == false
        )
        {
            // `result_str` has quotes around it - remove them.
            if (result_str.size() >= 2 &&
                result_str.front() == '"' &&
                result_str.back() == '"')
            {
                result_str = result_str.substr(1, result_str.size() - 2);
            }
        }

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

                result_ident += pp_evaluator::value_to_string(eval_result.value(), false);

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

        return pp_evaluator::value_to_string(eval_result.value(), false);
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
