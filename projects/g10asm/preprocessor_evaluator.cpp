/**
 * @file    g10asm/preprocessor_evaluator.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-01
 * 
 * @brief   Contains definitions for the expression evaluator used by the G10
 *          assembler's preprocessor component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/preprocessor_evaluator.hpp>
#include <g10asm/preprocessor_macros.hpp>
#include <g10asm/keyword_table.hpp>

/* Private Macros *************************************************************/

#define G10ASM_ERR_TOK(tok) \
    tok.source_file, \
    tok.source_line, \
    tok.source_column

/* Public Methods *************************************************************/

namespace g10asm
{
    pp_evaluator::pp_evaluator (
        const std::vector<token>& tokens,
        const pp_macro_table& macro_table
    ) noexcept :
        m_tokens { tokens },
        m_macro_table { macro_table }
    {
    }

    auto pp_evaluator::evaluate () -> g10::result<pp_value>
    {
        m_current = 0;

        if (m_tokens.empty() == true)
        {
            return g10::error(" - Empty expression.");
        }

        auto result = parse_expression();
        if (result.has_value() == false)
        {
            return result;
        }

        // Check that all tokens were consumed
        if (is_at_end() == false)
        {
            auto tok_result = current();
            if (tok_result.has_value() == true)
            {
                const token& tok = tok_result.value();
                return g10::error(
                    " - Unexpected token '{}' after expression.",
                    tok.lexeme
                );
            }
        }

        m_good = true;
        return result;
    }

    auto pp_evaluator::value_to_string (
        const pp_value& value,
        bool quote_strings
    ) -> std::string
    {
        return std::visit(
            [quote_strings] (const auto& v) -> std::string
            {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, pp_void>)
                {
                    return "";
                }
                else if constexpr (std::is_same_v<T, pp_integer>)
                {
                    return std::to_string(v);
                }
                else if constexpr (std::is_same_v<T, pp_number>)
                {
                    // Output as fixed-point or integer if no fraction
                    if (v.get_fractional_part() == 0)
                    {
                        return std::to_string(v.get_signed_integer());
                    }
                    else
                    {
                        return std::format("{}", v.get_float());
                    }
                }
                else if constexpr (std::is_same_v<T, pp_boolean>)
                {
                    return v ? "1" : "0";
                }
                else if constexpr (std::is_same_v<T, pp_string>)
                {
                    // Wrap string values in double quotes only if requested
                    if (quote_strings == true)
                    {
                        return std::format("\"{}\"", v);
                    }
                    else
                    {
                        return v;
                    }
                }
                else
                {
                    return "";
                }
            },
            value
        );
    }
}

/* Private Methods - Token Navigation *****************************************/

namespace g10asm
{
    auto pp_evaluator::current () const -> g10::result_cref<token>
    {
        if (m_current >= m_tokens.size())
        {
            return g10::error(" - Unexpected end of expression.");
        }
        return std::cref(m_tokens[m_current]);
    }

    auto pp_evaluator::peek (std::int64_t offset) const 
        -> g10::result_cref<token>
    {
        std::int64_t target = static_cast<std::int64_t>(m_current) + offset;
        if (target < 0 || 
            static_cast<std::size_t>(target) >= m_tokens.size())
        {
            return g10::error(" - Token peek out of bounds.");
        }
        return std::cref(m_tokens[static_cast<std::size_t>(target)]);
    }

    auto pp_evaluator::advance () -> void
    {
        if (m_current < m_tokens.size())
        {
            ++m_current;
        }
    }

    auto pp_evaluator::is_at_end () const -> bool
    {
        return m_current >= m_tokens.size();
    }

    auto pp_evaluator::check (token_type type) const -> bool
    {
        if (is_at_end() == true)
        {
            return false;
        }
        auto tok_result = current();
        if (tok_result.has_value() == false)
        {
            return false;
        }
        return tok_result.value().get().type == type;
    }

    auto pp_evaluator::match (token_type type) -> bool
    {
        if (check(type) == true)
        {
            advance();
            return true;
        }
        return false;
    }
}

/* Private Methods - Expression Parsing ***************************************/

namespace g10asm
{
    auto pp_evaluator::parse_primary () -> g10::result<pp_value>
    {
        // Check for literals
        if (check(token_type::integer_literal) == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == false)
            {
                return g10::error(tok_result.error());
            }
            const token& tok = tok_result.value();
            advance();
            return pp_value { tok.int_value.value_or(0) };
        }

        if (check(token_type::number_literal) == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == false)
            {
                return g10::error(tok_result.error());
            }
            const token& tok = tok_result.value();
            advance();
            return pp_value { pp_number { tok.number_value.value_or(0.0) } };
        }

        if (check(token_type::character_literal) == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == false)
            {
                return g10::error(tok_result.error());
            }
            const token& tok = tok_result.value();
            advance();
            return pp_value { static_cast<pp_integer>(tok.int_value.value_or(0)) };
        }

        if (check(token_type::string_literal) == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == false)
            {
                return g10::error(tok_result.error());
            }
            const token& tok = tok_result.value();
            advance();
            return pp_value { pp_string { tok.lexeme } };
        }

        // Check for identifiers (could be macro names or function calls)
        // Also check for keywords that are preprocessor functions
        bool is_identifier = check(token_type::identifier);
        bool is_pp_function = false;
        if (check(token_type::keyword) == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == true)
            {
                const token& tok = tok_result.value();
                if (tok.keyword_value.has_value() == true &&
                    tok.keyword_value.value().get().type == keyword_type::preprocessor_function)
                {
                    is_pp_function = true;
                }
            }
        }
        
        if (is_identifier == true || is_pp_function == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == false)
            {
                return g10::error(tok_result.error());
            }
            const token& tok = tok_result.value();
            std::string name { tok.lexeme };
            m_last_identifier = name;  // Store for defined() function
            advance();

            // Check if this is a function call (identifier followed by '(')
            if (check(token_type::left_parenthesis) == true)
            {
                return parse_function_call(name);
            }

            // Try to look up as a macro
            auto macro_result = m_macro_table.lookup_text_sub_macro(name);
            if (macro_result.has_value() == true)
            {
                const text_sub_macro& macro = macro_result.value();
                
                // Evaluate the macro's replacement tokens
                if (macro.replacement.empty() == true)
                {
                    return pp_value { pp_integer { 0 } };
                }

                // Recursively evaluate the replacement
                pp_evaluator sub_eval { macro.replacement, m_macro_table };
                return sub_eval.evaluate();
            }

            // Unknown identifier - treat as 0 or error
            return g10::error(
                " - Unknown identifier '{}' in expression.",
                name
            );
        }

        // Check for parenthesized expression
        if (match(token_type::left_parenthesis) == true)
        {
            auto expr_result = parse_expression();
            if (expr_result.has_value() == false)
            {
                return expr_result;
            }

            if (match(token_type::right_parenthesis) == false)
            {
                return g10::error(" - Expected ')' after expression.");
            }

            return expr_result;
        }

        // Unknown token
        auto tok_result = current();
        if (tok_result.has_value() == true)
        {
            const token& tok = tok_result.value();
            return g10::error(
                " - Unexpected token '{}' in expression.",
                tok.lexeme
            );
        }

        return g10::error(" - Unexpected end of expression.");
    }

    auto pp_evaluator::parse_function_call (const std::string& func_name)
        -> g10::result<pp_value>
    {
        // Consume the opening parenthesis
        if (match(token_type::left_parenthesis) == false)
        {
            return g10::error(
                " - Expected '(' after function name '{}'.",
                func_name
            );
        }

        // Parse arguments
        std::vector<pp_value> args {};

        // Special handling for 'defined' - it takes an identifier, not expr
        if (func_name == "defined")
        {
            if (check(token_type::identifier) == true)
            {
                auto tok_result = current();
                if (tok_result.has_value() == true)
                {
                    const token& tok = tok_result.value();
                    // Store the identifier name as a string value
                    args.push_back(pp_value { pp_string { std::string(tok.lexeme) } });
                    advance();
                }
            }
            else
            {
                return g10::error(" - 'defined' expects an identifier.");
            }
        }
        else
        {
            // Parse regular arguments as expressions
            if (check(token_type::right_parenthesis) == false)
            {
                do
                {
                    auto arg_result = parse_expression();
                    if (arg_result.has_value() == false)
                    {
                        return g10::error(
                            " - Error parsing argument for '{}': {}",
                            func_name,
                            arg_result.error()
                        );
                    }
                    args.push_back(arg_result.value());
                } while (match(token_type::comma) == true);
            }
        }

        // Consume the closing parenthesis
        if (match(token_type::right_parenthesis) == false)
        {
            return g10::error(
                " - Expected ')' after arguments to '{}'.",
                func_name
            );
        }

        // Dispatch the function
        return dispatch_function(func_name, args);
    }

    auto pp_evaluator::dispatch_function (
        const std::string& func_name,
        const std::vector<pp_value>& args
    ) -> g10::result<pp_value>
    {
        // Integer functions
        if (func_name == "high") { return fn_high(args); }
        if (func_name == "low") { return fn_low(args); }
        if (func_name == "bitwidth") { return fn_bitwidth(args); }
        if (func_name == "abs") { return fn_abs(args); }
        if (func_name == "min") { return fn_min(args); }
        if (func_name == "max") { return fn_max(args); }
        if (func_name == "clamp") { return fn_clamp(args); }

        // Fixed-point arithmetic
        if (func_name == "fmul") { return fn_fmul(args); }
        if (func_name == "fdiv") { return fn_fdiv(args); }
        if (func_name == "fmod") { return fn_fmod(args); }

        // Fixed-point conversion
        if (func_name == "fint") { return fn_fint(args); }
        if (func_name == "ffrac") { return fn_ffrac(args); }
        if (func_name == "round") { return fn_round(args); }
        if (func_name == "ceil") { return fn_ceil(args); }
        if (func_name == "floor") { return fn_floor(args); }
        if (func_name == "trunc") { return fn_trunc(args); }

        // Math functions
        if (func_name == "pow") { return fn_pow(args); }
        if (func_name == "sqrt") { return fn_sqrt(args); }
        if (func_name == "exp") { return fn_exp(args); }
        if (func_name == "ln") { return fn_ln(args); }
        if (func_name == "log2") { return fn_log2(args); }
        if (func_name == "log10") { return fn_log10(args); }
        if (func_name == "log") { return fn_log(args); }

        // Trigonometric functions
        if (func_name == "sin") { return fn_sin(args); }
        if (func_name == "cos") { return fn_cos(args); }
        if (func_name == "tan") { return fn_tan(args); }
        if (func_name == "asin") { return fn_asin(args); }
        if (func_name == "acos") { return fn_acos(args); }
        if (func_name == "atan") { return fn_atan(args); }
        if (func_name == "atan2") { return fn_atan2(args); }

        // String functions
        if (func_name == "strlen") { return fn_strlen(args); }
        if (func_name == "strcmp") { return fn_strcmp(args); }
        if (func_name == "substr") { return fn_substr(args); }
        if (func_name == "indexof") { return fn_indexof(args); }
        if (func_name == "toupper") { return fn_toupper(args); }
        if (func_name == "tolower") { return fn_tolower(args); }
        if (func_name == "concat") { return fn_concat(args); }

        // Miscellaneous functions
        if (func_name == "defined") { return fn_defined(args); }
        if (func_name == "typeof") { return fn_typeof(args); }

        return g10::error(" - Unknown function '{}'.", func_name);
    }

    auto pp_evaluator::parse_unary () -> g10::result<pp_value>
    {
        // Check for unary operators
        if (match(token_type::logical_not) == true)
        {
            auto operand = parse_unary();
            if (operand.has_value() == false)
            {
                return operand;
            }
            return pp_value { pp_integer { to_boolean(operand.value()) ? 0 : 1 } };
        }

        if (match(token_type::bitwise_not) == true)
        {
            auto operand = parse_unary();
            if (operand.has_value() == false)
            {
                return operand;
            }
            auto int_result = to_integer(operand.value());
            if (int_result.has_value() == false)
            {
                return g10::error(int_result.error());
            }
            return pp_value { ~int_result.value() };
        }

        if (match(token_type::minus) == true)
        {
            auto operand = parse_unary();
            if (operand.has_value() == false)
            {
                return operand;
            }
            
            // Handle fixed-point negation separately to preserve fractional part
            if (pp_values::is_number(operand.value()))
            {
                const pp_number& num = std::get<pp_number>(operand.value());
                return pp_value { pp_number { -num.get_float() } };
            }
            
            auto int_result = to_integer(operand.value());
            if (int_result.has_value() == false)
            {
                return g10::error(int_result.error());
            }
            return pp_value { -int_result.value() };
        }

        if (match(token_type::plus) == true)
        {
            return parse_unary();
        }

        return parse_primary();
    }

    auto pp_evaluator::parse_multiplicative () -> g10::result<pp_value>
    {
        auto left = parse_unary();
        if (left.has_value() == false)
        {
            return left;
        }

        while (true)
        {
            if (match(token_type::times) == true)
            {
                auto right = parse_unary();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { l.value() * r.value() };
            }
            else if (match(token_type::divide) == true)
            {
                auto right = parse_unary();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                if (r.value() == 0)
                {
                    return g10::error(" - Division by zero.");
                }
                left = pp_value { l.value() / r.value() };
            }
            else if (match(token_type::modulo) == true)
            {
                auto right = parse_unary();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                if (r.value() == 0)
                {
                    return g10::error(" - Modulo by zero.");
                }
                left = pp_value { l.value() % r.value() };
            }
            else
            {
                break;
            }
        }

        return left;
    }

    auto pp_evaluator::parse_additive () -> g10::result<pp_value>
    {
        auto left = parse_multiplicative();
        if (left.has_value() == false)
        {
            return left;
        }

        while (true)
        {
            if (match(token_type::plus) == true)
            {
                auto right = parse_multiplicative();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { l.value() + r.value() };
            }
            else if (match(token_type::minus) == true)
            {
                auto right = parse_multiplicative();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { l.value() - r.value() };
            }
            else
            {
                break;
            }
        }

        return left;
    }

    auto pp_evaluator::parse_shift () -> g10::result<pp_value>
    {
        auto left = parse_additive();
        if (left.has_value() == false)
        {
            return left;
        }

        while (true)
        {
            if (match(token_type::bitwise_shift_left) == true)
            {
                auto right = parse_additive();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { l.value() << r.value() };
            }
            else if (match(token_type::bitwise_shift_right) == true)
            {
                auto right = parse_additive();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { l.value() >> r.value() };
            }
            else
            {
                break;
            }
        }

        return left;
    }

    auto pp_evaluator::parse_comparison () -> g10::result<pp_value>
    {
        auto left = parse_shift();
        if (left.has_value() == false)
        {
            return left;
        }

        while (true)
        {
            if (match(token_type::compare_less) == true)
            {
                auto right = parse_shift();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { pp_integer { l.value() < r.value() ? 1 : 0 } };
            }
            else if (match(token_type::compare_less_equal) == true)
            {
                auto right = parse_shift();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { pp_integer { l.value() <= r.value() ? 1 : 0 } };
            }
            else if (match(token_type::compare_greater) == true)
            {
                auto right = parse_shift();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { pp_integer { l.value() > r.value() ? 1 : 0 } };
            }
            else if (match(token_type::compare_greater_equal) == true)
            {
                auto right = parse_shift();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { pp_integer { l.value() >= r.value() ? 1 : 0 } };
            }
            else
            {
                break;
            }
        }

        return left;
    }

    auto pp_evaluator::parse_equality () -> g10::result<pp_value>
    {
        auto left = parse_comparison();
        if (left.has_value() == false)
        {
            return left;
        }

        while (true)
        {
            if (match(token_type::compare_equal) == true)
            {
                auto right = parse_comparison();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { pp_integer { l.value() == r.value() ? 1 : 0 } };
            }
            else if (match(token_type::compare_not_equal) == true)
            {
                auto right = parse_comparison();
                if (right.has_value() == false)
                {
                    return right;
                }
                auto l = to_integer(left.value());
                auto r = to_integer(right.value());
                if (l.has_value() == false) return g10::error(l.error());
                if (r.has_value() == false) return g10::error(r.error());
                left = pp_value { pp_integer { l.value() != r.value() ? 1 : 0 } };
            }
            else
            {
                break;
            }
        }

        return left;
    }

    auto pp_evaluator::parse_bitwise_and () -> g10::result<pp_value>
    {
        auto left = parse_equality();
        if (left.has_value() == false)
        {
            return left;
        }

        while (match(token_type::bitwise_and) == true)
        {
            auto right = parse_equality();
            if (right.has_value() == false)
            {
                return right;
            }
            auto l = to_integer(left.value());
            auto r = to_integer(right.value());
            if (l.has_value() == false) return g10::error(l.error());
            if (r.has_value() == false) return g10::error(r.error());
            left = pp_value { l.value() & r.value() };
        }

        return left;
    }

    auto pp_evaluator::parse_bitwise_xor () -> g10::result<pp_value>
    {
        auto left = parse_bitwise_and();
        if (left.has_value() == false)
        {
            return left;
        }

        while (match(token_type::bitwise_xor) == true)
        {
            auto right = parse_bitwise_and();
            if (right.has_value() == false)
            {
                return right;
            }
            auto l = to_integer(left.value());
            auto r = to_integer(right.value());
            if (l.has_value() == false) return g10::error(l.error());
            if (r.has_value() == false) return g10::error(r.error());
            left = pp_value { l.value() ^ r.value() };
        }

        return left;
    }

    auto pp_evaluator::parse_bitwise_or () -> g10::result<pp_value>
    {
        auto left = parse_bitwise_xor();
        if (left.has_value() == false)
        {
            return left;
        }

        while (match(token_type::bitwise_or) == true)
        {
            auto right = parse_bitwise_xor();
            if (right.has_value() == false)
            {
                return right;
            }
            auto l = to_integer(left.value());
            auto r = to_integer(right.value());
            if (l.has_value() == false) return g10::error(l.error());
            if (r.has_value() == false) return g10::error(r.error());
            left = pp_value { l.value() | r.value() };
        }

        return left;
    }

    auto pp_evaluator::parse_logical_and () -> g10::result<pp_value>
    {
        auto left = parse_bitwise_or();
        if (left.has_value() == false)
        {
            return left;
        }

        while (match(token_type::logical_and) == true)
        {
            auto right = parse_bitwise_or();
            if (right.has_value() == false)
            {
                return right;
            }
            bool l = to_boolean(left.value());
            bool r = to_boolean(right.value());
            left = pp_value { pp_integer { (l && r) ? 1 : 0 } };
        }

        return left;
    }

    auto pp_evaluator::parse_logical_or () -> g10::result<pp_value>
    {
        auto left = parse_logical_and();
        if (left.has_value() == false)
        {
            return left;
        }

        while (match(token_type::logical_or) == true)
        {
            auto right = parse_logical_and();
            if (right.has_value() == false)
            {
                return right;
            }
            bool l = to_boolean(left.value());
            bool r = to_boolean(right.value());
            left = pp_value { pp_integer { (l || r) ? 1 : 0 } };
        }

        return left;
    }

    auto pp_evaluator::parse_expression () -> g10::result<pp_value>
    {
        return parse_logical_or();
    }
}

/* Private Methods - Value Operations *****************************************/

namespace g10asm
{
    auto pp_evaluator::to_integer (const pp_value& value) 
        -> g10::result<pp_integer>
    {
        return std::visit(
            [] (const auto& v) -> g10::result<pp_integer>
            {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, pp_void>)
                {
                    return pp_integer { 0 };
                }
                else if constexpr (std::is_same_v<T, pp_integer>)
                {
                    return v;
                }
                else if constexpr (std::is_same_v<T, pp_number>)
                {
                    return v.get_signed_integer();
                }
                else if constexpr (std::is_same_v<T, pp_boolean>)
                {
                    return pp_integer { v ? 1 : 0 };
                }
                else if constexpr (std::is_same_v<T, pp_string>)
                {
                    return g10::error(
                        " - Cannot convert string to integer."
                    );
                }
                else
                {
                    return pp_integer { 0 };
                }
            },
            value
        );
    }

    auto pp_evaluator::to_boolean (const pp_value& value) -> bool
    {
        return std::visit(
            [] (const auto& v) -> bool
            {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, pp_void>)
                {
                    return false;
                }
                else if constexpr (std::is_same_v<T, pp_integer>)
                {
                    return v != 0;
                }
                else if constexpr (std::is_same_v<T, pp_number>)
                {
                    return v.get_raw() != 0;
                }
                else if constexpr (std::is_same_v<T, pp_boolean>)
                {
                    return v;
                }
                else if constexpr (std::is_same_v<T, pp_string>)
                {
                    return v.empty() == false;
                }
                else
                {
                    return false;
                }
            },
            value
        );
    }

    auto pp_evaluator::to_number (const pp_value& value) 
        -> g10::result<pp_number>
    {
        return std::visit(
            [] (const auto& v) -> g10::result<pp_number>
            {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, pp_void>)
                {
                    return pp_number { 0.0 };
                }
                else if constexpr (std::is_same_v<T, pp_integer>)
                {
                    return pp_number { static_cast<double>(v) };
                }
                else if constexpr (std::is_same_v<T, pp_number>)
                {
                    return v;
                }
                else if constexpr (std::is_same_v<T, pp_boolean>)
                {
                    return pp_number { v ? 1.0 : 0.0 };
                }
                else if constexpr (std::is_same_v<T, pp_string>)
                {
                    return g10::error(
                        " - Cannot convert string to number."
                    );
                }
                else
                {
                    return pp_number { 0.0 };
                }
            },
            value
        );
    }

    auto pp_evaluator::to_string_value (const pp_value& value) 
        -> g10::result<pp_string>
    {
        return std::visit(
            [] (const auto& v) -> g10::result<pp_string>
            {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, pp_string>)
                {
                    return v;
                }
                else
                {
                    return g10::error(
                        " - Expected string argument."
                    );
                }
            },
            value
        );
    }
}

/* Private Methods - Built-in Integer Functions *******************************/

namespace g10asm
{
    auto pp_evaluator::fn_high (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - high() requires 1 argument.");
        }
        auto int_result = to_integer(args[0]);
        if (int_result.has_value() == false)
        {
            return g10::error(int_result.error());
        }
        pp_integer n = int_result.value();
        return pp_value { pp_integer { (n >> 8) & 0xFF } };
    }

    auto pp_evaluator::fn_low (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - low() requires 1 argument.");
        }
        auto int_result = to_integer(args[0]);
        if (int_result.has_value() == false)
        {
            return g10::error(int_result.error());
        }
        pp_integer n = int_result.value();
        return pp_value { pp_integer { n & 0xFF } };
    }

    auto pp_evaluator::fn_bitwidth (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - bitwidth() requires 1 argument.");
        }
        auto int_result = to_integer(args[0]);
        if (int_result.has_value() == false)
        {
            return g10::error(int_result.error());
        }
        pp_integer n = int_result.value();
        if (n == 0)
        {
            return pp_value { pp_integer { 0 } };
        }
        // Count bits needed to represent absolute value
        std::uint64_t val = static_cast<std::uint64_t>(n < 0 ? -n : n);
        pp_integer bits = 0;
        while (val > 0)
        {
            ++bits;
            val >>= 1;
        }
        return pp_value { pp_integer { bits } };
    }

    auto pp_evaluator::fn_abs (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - abs() requires 1 argument.");
        }
        
        // Handle both integer and fixed-point
        if (pp_values::is_number(args[0]))
        {
            const pp_number& num = std::get<pp_number>(args[0]);
            double val = num.get_float();
            return pp_value { pp_number { std::abs(val) } };
        }
        
        auto int_result = to_integer(args[0]);
        if (int_result.has_value() == false)
        {
            return g10::error(int_result.error());
        }
        pp_integer n = int_result.value();
        return pp_value { pp_integer { n < 0 ? -n : n } };
    }

    auto pp_evaluator::fn_min (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - min() requires 2 arguments.");
        }
        
        // Handle fixed-point
        if (pp_values::is_number(args[0]) || pp_values::is_number(args[1]))
        {
            auto a_result = to_number(args[0]);
            auto b_result = to_number(args[1]);
            if (a_result.has_value() == false) return g10::error(a_result.error());
            if (b_result.has_value() == false) return g10::error(b_result.error());
            double a = a_result.value().get_float();
            double b = b_result.value().get_float();
            return pp_value { pp_number { std::min(a, b) } };
        }
        
        auto a_result = to_integer(args[0]);
        auto b_result = to_integer(args[1]);
        if (a_result.has_value() == false) return g10::error(a_result.error());
        if (b_result.has_value() == false) return g10::error(b_result.error());
        return pp_value { pp_integer { std::min(a_result.value(), b_result.value()) } };
    }

    auto pp_evaluator::fn_max (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - max() requires 2 arguments.");
        }
        
        // Handle fixed-point
        if (pp_values::is_number(args[0]) || pp_values::is_number(args[1]))
        {
            auto a_result = to_number(args[0]);
            auto b_result = to_number(args[1]);
            if (a_result.has_value() == false) return g10::error(a_result.error());
            if (b_result.has_value() == false) return g10::error(b_result.error());
            double a = a_result.value().get_float();
            double b = b_result.value().get_float();
            return pp_value { pp_number { std::max(a, b) } };
        }
        
        auto a_result = to_integer(args[0]);
        auto b_result = to_integer(args[1]);
        if (a_result.has_value() == false) return g10::error(a_result.error());
        if (b_result.has_value() == false) return g10::error(b_result.error());
        return pp_value { pp_integer { std::max(a_result.value(), b_result.value()) } };
    }

    auto pp_evaluator::fn_clamp (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 3)
        {
            return g10::error(" - clamp() requires 3 arguments.");
        }
        
        // Handle fixed-point
        if (pp_values::is_number(args[0]) || pp_values::is_number(args[1]) || 
            pp_values::is_number(args[2]))
        {
            auto v_result = to_number(args[0]);
            auto lo_result = to_number(args[1]);
            auto hi_result = to_number(args[2]);
            if (v_result.has_value() == false) return g10::error(v_result.error());
            if (lo_result.has_value() == false) return g10::error(lo_result.error());
            if (hi_result.has_value() == false) return g10::error(hi_result.error());
            double v = v_result.value().get_float();
            double lo = lo_result.value().get_float();
            double hi = hi_result.value().get_float();
            return pp_value { pp_number { std::clamp(v, lo, hi) } };
        }
        
        auto v_result = to_integer(args[0]);
        auto lo_result = to_integer(args[1]);
        auto hi_result = to_integer(args[2]);
        if (v_result.has_value() == false) return g10::error(v_result.error());
        if (lo_result.has_value() == false) return g10::error(lo_result.error());
        if (hi_result.has_value() == false) return g10::error(hi_result.error());
        return pp_value { 
            pp_integer { std::clamp(v_result.value(), lo_result.value(), hi_result.value()) } 
        };
    }
}

/* Private Methods - Built-in Fixed-Point Arithmetic Functions ****************/

namespace g10asm
{
    auto pp_evaluator::fn_fmul (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - fmul() requires 2 arguments.");
        }
        auto a_result = to_number(args[0]);
        auto b_result = to_number(args[1]);
        if (a_result.has_value() == false) return g10::error(a_result.error());
        if (b_result.has_value() == false) return g10::error(b_result.error());
        
        double result = a_result.value().get_float() * b_result.value().get_float();
        return pp_value { pp_number { result } };
    }

    auto pp_evaluator::fn_fdiv (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - fdiv() requires 2 arguments.");
        }
        auto a_result = to_number(args[0]);
        auto b_result = to_number(args[1]);
        if (a_result.has_value() == false) return g10::error(a_result.error());
        if (b_result.has_value() == false) return g10::error(b_result.error());
        
        double b = b_result.value().get_float();
        if (b == 0.0)
        {
            return g10::error(" - Division by zero in fdiv().");
        }
        
        double result = a_result.value().get_float() / b;
        return pp_value { pp_number { result } };
    }

    auto pp_evaluator::fn_fmod (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - fmod() requires 2 arguments.");
        }
        auto a_result = to_number(args[0]);
        auto b_result = to_number(args[1]);
        if (a_result.has_value() == false) return g10::error(a_result.error());
        if (b_result.has_value() == false) return g10::error(b_result.error());
        
        double b = b_result.value().get_float();
        if (b == 0.0)
        {
            return g10::error(" - Division by zero in fmod().");
        }
        
        double result = std::fmod(a_result.value().get_float(), b);
        return pp_value { pp_number { result } };
    }
}

/* Private Methods - Built-in Fixed-Point Conversion Functions ****************/

namespace g10asm
{
    auto pp_evaluator::fn_fint (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - fint() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        // Use trunc on the float value for correct signed handling
        double val = num_result.value().get_float();
        pp_integer int_part = static_cast<pp_integer>(std::trunc(val));
        return pp_value { int_part };
    }

    auto pp_evaluator::fn_ffrac (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - ffrac() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        double int_part = 0.0;
        double frac_part = std::modf(val, &int_part);
        return pp_value { pp_number { frac_part } };
    }

    auto pp_evaluator::fn_round (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - round() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        pp_integer result = static_cast<pp_integer>(std::round(val));
        return pp_value { result };
    }

    auto pp_evaluator::fn_ceil (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - ceil() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        pp_integer result = static_cast<pp_integer>(std::ceil(val));
        return pp_value { result };
    }

    auto pp_evaluator::fn_floor (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - floor() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        pp_integer result = static_cast<pp_integer>(std::floor(val));
        return pp_value { result };
    }

    auto pp_evaluator::fn_trunc (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - trunc() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        pp_integer result = static_cast<pp_integer>(std::trunc(val));
        return pp_value { result };
    }
}

/* Private Methods - Built-in Math Functions **********************************/

namespace g10asm
{
    auto pp_evaluator::fn_pow (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - pow() requires 2 arguments.");
        }
        auto base_result = to_number(args[0]);
        auto exp_result = to_number(args[1]);
        if (base_result.has_value() == false) return g10::error(base_result.error());
        if (exp_result.has_value() == false) return g10::error(exp_result.error());
        
        double result = std::pow(
            base_result.value().get_float(),
            exp_result.value().get_float()
        );
        return pp_value { pp_number { result } };
    }

    auto pp_evaluator::fn_sqrt (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - sqrt() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        if (val < 0.0)
        {
            return g10::error(" - sqrt() of negative number.");
        }
        
        return pp_value { pp_number { std::sqrt(val) } };
    }

    auto pp_evaluator::fn_exp (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - exp() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        return pp_value { pp_number { std::exp(num_result.value().get_float()) } };
    }

    auto pp_evaluator::fn_ln (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - ln() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        if (val <= 0.0)
        {
            return g10::error(" - ln() of non-positive number.");
        }
        
        return pp_value { pp_number { std::log(val) } };
    }

    auto pp_evaluator::fn_log2 (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - log2() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        if (val <= 0.0)
        {
            return g10::error(" - log2() of non-positive number.");
        }
        
        return pp_value { pp_number { std::log2(val) } };
    }

    auto pp_evaluator::fn_log10 (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - log10() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        if (val <= 0.0)
        {
            return g10::error(" - log10() of non-positive number.");
        }
        
        return pp_value { pp_number { std::log10(val) } };
    }

    auto pp_evaluator::fn_log (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - log() requires 2 arguments.");
        }
        auto val_result = to_number(args[0]);
        auto base_result = to_number(args[1]);
        if (val_result.has_value() == false) return g10::error(val_result.error());
        if (base_result.has_value() == false) return g10::error(base_result.error());
        
        double val = val_result.value().get_float();
        double base = base_result.value().get_float();
        
        if (val <= 0.0)
        {
            return g10::error(" - log() of non-positive number.");
        }
        if (base <= 0.0 || base == 1.0)
        {
            return g10::error(" - log() base must be positive and not 1.");
        }
        
        return pp_value { pp_number { std::log(val) / std::log(base) } };
    }
}

/* Private Methods - Built-in Trigonometric Functions *************************/

namespace g10asm
{
    // Helper constant: 2*PI for converting turns to radians
    constexpr double TWO_PI = 6.28318530717958647693;

    auto pp_evaluator::fn_sin (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - sin() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        // Convert turns to radians
        double turns = num_result.value().get_float();
        double radians = turns * TWO_PI;
        return pp_value { pp_number { std::sin(radians) } };
    }

    auto pp_evaluator::fn_cos (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - cos() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double turns = num_result.value().get_float();
        double radians = turns * TWO_PI;
        return pp_value { pp_number { std::cos(radians) } };
    }

    auto pp_evaluator::fn_tan (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - tan() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double turns = num_result.value().get_float();
        double radians = turns * TWO_PI;
        return pp_value { pp_number { std::tan(radians) } };
    }

    auto pp_evaluator::fn_asin (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - asin() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        if (val < -1.0 || val > 1.0)
        {
            return g10::error(" - asin() argument must be in [-1, 1].");
        }
        
        // Result in radians, convert to turns
        double radians = std::asin(val);
        double turns = radians / TWO_PI;
        return pp_value { pp_number { turns } };
    }

    auto pp_evaluator::fn_acos (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - acos() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double val = num_result.value().get_float();
        if (val < -1.0 || val > 1.0)
        {
            return g10::error(" - acos() argument must be in [-1, 1].");
        }
        
        double radians = std::acos(val);
        double turns = radians / TWO_PI;
        return pp_value { pp_number { turns } };
    }

    auto pp_evaluator::fn_atan (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - atan() requires 1 argument.");
        }
        auto num_result = to_number(args[0]);
        if (num_result.has_value() == false) return g10::error(num_result.error());
        
        double radians = std::atan(num_result.value().get_float());
        double turns = radians / TWO_PI;
        return pp_value { pp_number { turns } };
    }

    auto pp_evaluator::fn_atan2 (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - atan2() requires 2 arguments.");
        }
        auto y_result = to_number(args[0]);
        auto x_result = to_number(args[1]);
        if (y_result.has_value() == false) return g10::error(y_result.error());
        if (x_result.has_value() == false) return g10::error(x_result.error());
        
        double radians = std::atan2(
            y_result.value().get_float(),
            x_result.value().get_float()
        );
        double turns = radians / TWO_PI;
        return pp_value { pp_number { turns } };
    }
}

/* Private Methods - Built-in String Functions ********************************/

namespace g10asm
{
    auto pp_evaluator::fn_strlen (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - strlen() requires 1 argument.");
        }
        auto str_result = to_string_value(args[0]);
        if (str_result.has_value() == false) return g10::error(str_result.error());
        
        return pp_value { pp_integer { 
            static_cast<pp_integer>(str_result.value().length()) 
        } };
    }

    auto pp_evaluator::fn_strcmp (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - strcmp() requires 2 arguments.");
        }
        auto s1_result = to_string_value(args[0]);
        auto s2_result = to_string_value(args[1]);
        if (s1_result.has_value() == false) return g10::error(s1_result.error());
        if (s2_result.has_value() == false) return g10::error(s2_result.error());
        
        int cmp = s1_result.value().compare(s2_result.value());
        return pp_value { pp_integer { cmp < 0 ? -1 : (cmp > 0 ? 1 : 0) } };
    }

    auto pp_evaluator::fn_substr (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() < 2 || args.size() > 3)
        {
            return g10::error(" - substr() requires 2 or 3 arguments.");
        }
        auto str_result = to_string_value(args[0]);
        auto start_result = to_integer(args[1]);
        if (str_result.has_value() == false) return g10::error(str_result.error());
        if (start_result.has_value() == false) return g10::error(start_result.error());
        
        const std::string& str = str_result.value();
        pp_integer start = start_result.value();
        
        if (start < 0 || static_cast<std::size_t>(start) >= str.length())
        {
            return pp_value { pp_string { "" } };
        }
        
        if (args.size() == 3)
        {
            auto len_result = to_integer(args[2]);
            if (len_result.has_value() == false) return g10::error(len_result.error());
            pp_integer len = len_result.value();
            if (len < 0) len = 0;
            return pp_value { pp_string { 
                str.substr(static_cast<std::size_t>(start), static_cast<std::size_t>(len))
            } };
        }
        
        return pp_value { pp_string { str.substr(static_cast<std::size_t>(start)) } };
    }

    auto pp_evaluator::fn_indexof (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 2)
        {
            return g10::error(" - indexof() requires 2 arguments.");
        }
        auto str_result = to_string_value(args[0]);
        auto search_result = to_string_value(args[1]);
        if (str_result.has_value() == false) return g10::error(str_result.error());
        if (search_result.has_value() == false) return g10::error(search_result.error());
        
        std::size_t pos = str_result.value().find(search_result.value());
        if (pos == std::string::npos)
        {
            return pp_value { pp_integer { -1 } };
        }
        return pp_value { pp_integer { static_cast<pp_integer>(pos) } };
    }

    auto pp_evaluator::fn_toupper (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - toupper() requires 1 argument.");
        }
        auto str_result = to_string_value(args[0]);
        if (str_result.has_value() == false) return g10::error(str_result.error());
        
        std::string result = str_result.value();
        for (char& c : result)
        {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        return pp_value { pp_string { result } };
    }

    auto pp_evaluator::fn_tolower (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - tolower() requires 1 argument.");
        }
        auto str_result = to_string_value(args[0]);
        if (str_result.has_value() == false) return g10::error(str_result.error());
        
        std::string result = str_result.value();
        for (char& c : result)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return pp_value { pp_string { result } };
    }

    auto pp_evaluator::fn_concat (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() < 2)
        {
            return g10::error(" - concat() requires at least 2 arguments.");
        }
        
        std::string result {};
        for (const auto& arg : args)
        {
            auto str_result = to_string_value(arg);
            if (str_result.has_value() == false) 
            {
                return g10::error(str_result.error());
            }
            result += str_result.value();
        }
        return pp_value { pp_string { result } };
    }
}

/* Private Methods - Built-in Miscellaneous Functions *************************/

namespace g10asm
{
    auto pp_evaluator::fn_defined (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - defined() requires 1 argument.");
        }
        
        // The argument should be a string containing the identifier name
        auto str_result = to_string_value(args[0]);
        if (str_result.has_value() == false)
        {
            return g10::error(str_result.error());
        }
        
        const std::string& name = str_result.value();
        auto macro_result = m_macro_table.lookup_text_sub_macro(name);
        
        return pp_value { pp_integer { macro_result.has_value() ? 1 : 0 } };
    }

    auto pp_evaluator::fn_typeof (const std::vector<pp_value>& args) 
        -> g10::result<pp_value>
    {
        if (args.size() != 1)
        {
            return g10::error(" - typeof() requires 1 argument.");
        }
        
        std::string_view type_name = pp_values::type_of(args[0]);
        return pp_value { pp_string { std::string(type_name) } };
    }
}
