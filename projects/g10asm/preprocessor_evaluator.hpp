/**
 * @file    g10asm/preprocessor_evaluator.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-03
 * 
 * @brief   Contains definitions for the expression evaluator used by the G10
 *          assembler's preprocessor component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/token.hpp>
#include <g10asm/preprocessor_values.hpp>

/* Public Types and Forward Declarations **************************************/

namespace g10asm
{
    /**
     * @brief   Forward declaration of the `preprocessor` class, which
     *          represents the G10 assembler's preprocessor component.
     */
    class preprocessor;

    /**
     * @brief   Forward declaration of the `pp_macro_table` class.
     */
    class pp_macro_table;
}

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{

}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing the expression evaluator used by
     *          the G10 assembler's preprocessor component.
     * 
     * This subcomponent is responsible for evaluating expressions found
     * within braced expressions `{}` in the preprocessing language.
     */
    class pp_evaluator final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a new preprocessor expression evaluator
         *          instance, providing it with the tokens that make up the
         *          expression to be evaluated.
         * 
         * @param   tokens          A vector of tokens representing the 
         *                          expression to be evaluated.
         * @param   macro_table     Reference to the macro table for looking
         *                          up macro values.
         */
        explicit pp_evaluator (
            const std::vector<token>& tokens,
            const pp_macro_table& macro_table
        ) noexcept;

        /**
         * @brief   Evaluates the expression and returns the result.
         * 
         * @return  If successful, returns the evaluated value;
         *          Otherwise, returns an error string.
         */
        auto evaluate () -> g10::result<pp_value>;

        /**
         * @brief   Indicates whether or not the expression was successfully
         *          evaluated.
         * 
         * @return  `true` if the expression was successfully evaluated;
         *          `false` otherwise.
         */
        inline auto is_good () const noexcept -> bool
            { return m_good; }

        /**
         * @brief   Converts a pp_value to its string representation for output.
         * 
         * @param   value   The value to convert.
         * 
         * @return  The string representation of the value.
         */
        static auto value_to_string (const pp_value& value) -> std::string;

        /**
         * @brief   Converts a value to an integer for arithmetic.
         * 
         * @param   value   The value to convert.
         * 
         * @return  If successful, returns the integer value;
         *          Otherwise, returns an error.
         */
        static auto to_integer (const pp_value& value) 
            -> g10::result<pp_integer>;

    private: /* Private Methods - Token Navigation ****************************/

        /**
         * @brief   Returns the current token.
         */
        auto current () const -> g10::result_cref<token>;

        /**
         * @brief   Peeks at a token at the specified offset.
         */
        auto peek (std::int64_t offset = 0) const -> g10::result_cref<token>;

        /**
         * @brief   Advances to the next token.
         */
        auto advance () -> void;

        /**
         * @brief   Checks if we're at the end of the token list.
         */
        auto is_at_end () const -> bool;

        /**
         * @brief   Checks if the current token matches the expected type.
         */
        auto check (token_type type) const -> bool;

        /**
         * @brief   Consumes the current token if it matches the expected type.
         */
        auto match (token_type type) -> bool;

    private: /* Private Methods - Expression Parsing **************************/

        /**
         * @brief   Parses a primary expression (literals, identifiers, parens).
         */
        auto parse_primary () -> g10::result<pp_value>;

        /**
         * @brief   Parses a unary expression (!, ~, +, -).
         */
        auto parse_unary () -> g10::result<pp_value>;

        /**
         * @brief   Parses multiplicative expressions (*, /, %).
         */
        auto parse_multiplicative () -> g10::result<pp_value>;

        /**
         * @brief   Parses additive expressions (+, -).
         */
        auto parse_additive () -> g10::result<pp_value>;

        /**
         * @brief   Parses shift expressions (<<, >>).
         */
        auto parse_shift () -> g10::result<pp_value>;

        /**
         * @brief   Parses comparison expressions (<, <=, >, >=).
         */
        auto parse_comparison () -> g10::result<pp_value>;

        /**
         * @brief   Parses equality expressions (==, !=).
         */
        auto parse_equality () -> g10::result<pp_value>;

        /**
         * @brief   Parses bitwise AND expressions (&).
         */
        auto parse_bitwise_and () -> g10::result<pp_value>;

        /**
         * @brief   Parses bitwise XOR expressions (^).
         */
        auto parse_bitwise_xor () -> g10::result<pp_value>;

        /**
         * @brief   Parses bitwise OR expressions (|).
         */
        auto parse_bitwise_or () -> g10::result<pp_value>;

        /**
         * @brief   Parses logical AND expressions (&&).
         */
        auto parse_logical_and () -> g10::result<pp_value>;

        /**
         * @brief   Parses logical OR expressions (||).
         */
        auto parse_logical_or () -> g10::result<pp_value>;

        /**
         * @brief   Parses the top-level expression.
         */
        auto parse_expression () -> g10::result<pp_value>;

    private: /* Private Methods - Value Operations ****************************/

        /**
         * @brief   Converts a value to a boolean.
         */
        static auto to_boolean (const pp_value& value) -> bool;

    private: /* Private Members ***********************************************/

        /**
         * @brief   A span of tokens representing the expression to be
         *          evaluated.
         */
        std::span<const token> m_tokens;

        /**
         * @brief   Reference to the macro table.
         */
        const pp_macro_table& m_macro_table;

        /**
         * @brief   Current position in the token list.
         */
        std::size_t m_current { 0 };

        /**
         * @brief   Indicates whether or not the provided expression was
         *          successfully evaluated.
         */
        bool m_good { false };

    };
}
