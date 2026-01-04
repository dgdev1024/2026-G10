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

    auto pp_evaluator::value_to_string (const pp_value& value) -> std::string
    {
        return std::visit(
            [] (const auto& v) -> std::string
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
                    return v;
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

        // Check for identifiers (could be macro names)
        if (check(token_type::identifier) == true)
        {
            auto tok_result = current();
            if (tok_result.has_value() == false)
            {
                return g10::error(tok_result.error());
            }
            const token& tok = tok_result.value();
            std::string name { tok.lexeme };
            advance();

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
}
