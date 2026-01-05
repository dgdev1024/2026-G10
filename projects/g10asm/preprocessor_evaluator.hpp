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
         * @param   value       The value to convert.
         * @param   quote_strings   If true, wrap string values in double quotes.
         *                          Defaults to true for standalone expressions,
         *                          but should be false for interpolation contexts.
         * 
         * @return  The string representation of the value.
         */
        static auto value_to_string (
            const pp_value& value, 
            bool quote_strings = true
        ) -> std::string;

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
         * @brief   Parses a primary expression (literals, identifiers, parens,
         *          function calls).
         */
        auto parse_primary () -> g10::result<pp_value>;

        /**
         * @brief   Parses a function call.
         * 
         * @param   func_name   The name of the function being called.
         * 
         * @return  If successful, returns the result of the function call;
         *          Otherwise, returns an error.
         */
        auto parse_function_call (const std::string& func_name) 
            -> g10::result<pp_value>;

        /**
         * @brief   Dispatches a built-in function call.
         * 
         * @param   func_name   The name of the function being called.
         * @param   args        The arguments passed to the function.
         * 
         * @return  If successful, returns the result of the function call;
         *          Otherwise, returns an error.
         */
        auto dispatch_function (
            const std::string& func_name,
            const std::vector<pp_value>& args
        ) -> g10::result<pp_value>;

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

    public: /* Public Methods - Value Operations ******************************/

        /**
         * @brief   Converts a value to a boolean.
         * 
         * @param   value   The value to convert.
         * 
         * @return  `true` if the value is truthy; `false` otherwise.
         */
        static auto to_boolean (const pp_value& value) -> bool;

    private: /* Private Methods - Value Operations ****************************/

        /**
         * @brief   Converts a value to a fixed-point number.
         */
        static auto to_number (const pp_value& value) 
            -> g10::result<pp_number>;

        /**
         * @brief   Converts a value to a string.
         */
        static auto to_string_value (const pp_value& value) 
            -> g10::result<pp_string>;

    private: /* Private Methods - Built-in Functions *************************/

        // Integer Functions
        auto fn_high (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_low (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_bitwidth (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_abs (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_min (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_max (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_clamp (const std::vector<pp_value>& args) -> g10::result<pp_value>;

        // Fixed-Point Arithmetic Functions
        auto fn_fmul (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_fdiv (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_fmod (const std::vector<pp_value>& args) -> g10::result<pp_value>;

        // Fixed-Point Conversion Functions
        auto fn_fint (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_ffrac (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_round (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_ceil (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_floor (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_trunc (const std::vector<pp_value>& args) -> g10::result<pp_value>;

        // Fixed-Point Math Functions
        auto fn_pow (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_sqrt (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_exp (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_ln (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_log2 (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_log10 (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_log (const std::vector<pp_value>& args) -> g10::result<pp_value>;

        // Trigonometric Functions
        auto fn_sin (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_cos (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_tan (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_asin (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_acos (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_atan (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_atan2 (const std::vector<pp_value>& args) -> g10::result<pp_value>;

        // String Functions
        auto fn_strlen (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_strcmp (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_substr (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_indexof (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_toupper (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_tolower (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_concat (const std::vector<pp_value>& args) -> g10::result<pp_value>;

        // Miscellaneous Functions
        auto fn_defined (const std::vector<pp_value>& args) -> g10::result<pp_value>;
        auto fn_typeof (const std::vector<pp_value>& args) -> g10::result<pp_value>;

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

        /**
         * @brief   Stores the last parsed identifier for special functions
         *          like `defined()` that need raw identifier access.
         */
        std::string m_last_identifier {};

    };
}
