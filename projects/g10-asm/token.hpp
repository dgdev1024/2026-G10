/**
 * @file    g10-asm/token.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains definitions for tokens extracted by the G10 CPU assembler's
 *          lexical analyzer.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/keyword_table.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{
    /**
     * @brief   Strongly enumerates the types of tokens which can be extracted
     *          by the G10 assembler's lexical analyzer.
     */
    enum class token_type : std::uint8_t
    {
        unknown,                            /** @brief Not a recognized token. */

        // Keywords and Identifiers
        keyword,                            /** @brief A recognized keyword. See @a `keyword_type`. */
        identifier,                         /** @brief A user-defined symbol (label, variable name, etc.). */
        placeholder,                        /** @brief A placeholder symbol (starting with an `@` character). */

        // Literals
        integer_literal,                    /** @brief An integer literal (e.g., `123`, `0x7B`, `0b1111011`, etc.). */
        fixed_point_literal,                /** @brief A fixed-point numeric literal (e.g., `123.45`, `-0.75`, etc.). */
        character_literal,                  /** @brief A character literal (e.g., `'A'`, `'\n'`, etc.). */
        string_literal,                     /** @brief A string literal (e.g., `"Hello, World!"`, etc.). */

        // Arithmetic Operators
        plus,                               /** @brief Addition operator (`+`). */
        minus,                              /** @brief Subtraction operator (`-`). */
        times,                              /** @brief Multiplication operator (`*`). */
        divide,                             /** @brief Division operator (`/`). */
        modulo,                             /** @brief Modulus operator (`%`). */

        // Bitwise Logic Operators
        bitwise_and,                        /** @brief Bitwise AND operator (`&`). */
        bitwise_or,                         /** @brief Bitwise OR operator (`|`). */
        bitwise_xor,                        /** @brief Bitwise XOR operator (`^`). */
        bitwise_not,                        /** @brief Bitwise NOT operator (`~`). */
        left_shift,                         /** @brief Bitwise left shift operator (`<<`). */
        right_shift,                        /** @brief Bitwise right shift operator (`>>`). */

        // Assignment Operators
        assign_equal,                       /** @brief Assignment operator (`=`). */
        assign_plus,                        /** @brief Addition assignment operator (`+=`). */
        assign_minus,                       /** @brief Subtraction assignment operator (`-=`). */
        assign_times,                       /** @brief Multiplication assignment operator (`*=`). */
        assign_divide,                      /** @brief Division assignment operator (`/=`). */
        assign_modulo,                      /** @brief Modulus assignment operator (`%=`). */
        assign_and,                         /** @brief Bitwise AND assignment operator (`&=`). */
        assign_or,                          /** @brief Bitwise OR assignment operator (`|=`). */
        assign_xor,                         /** @brief Bitwise XOR assignment operator (`^=`). */
        assign_left_shift,                  /** @brief Bitwise left shift assignment operator (`<<=`). */
        assign_right_shift,                 /** @brief Bitwise right shift assignment operator (`>>=`). */

        // Comparison Operators
        compare_equal,                      /** @brief Equality comparison operator (`==`). */
        compare_not_equal,                  /** @brief Inequality comparison operator (`!=`). */
        compare_less_than,                  /** @brief Less-than comparison operator (`<`). */
        compare_less_equal,                 /** @brief Less-than-or-equal-to comparison operator (`<=`). */
        compare_greater_than,               /** @brief Greater-than comparison operator (`>`). */
        compare_greater_equal,              /** @brief Greater-than-or-equal-to comparison operator (`>=`). */

        // Logical Operators
        logical_and,                        /** @brief Logical AND operator (`&&`). */
        logical_or,                         /** @brief Logical OR operator (`||`). */
        logical_not,                        /** @brief Logical NOT operator (`!`). */

        // Grouping Operators
        left_parenthesis,                   /** @brief Left parenthesis (`(`). */
        right_parenthesis,                  /** @brief Right parenthesis (`)`). */
        left_brace,                         /** @brief Left brace (`{`). */
        right_brace,                        /** @brief Right brace (`}`). */
        left_bracket,                       /** @brief Left bracket (`[`). */
        right_bracket,                      /** @brief Right bracket (`]`). */
        
        // Punctuation
        comma,                              /** @brief Comma (`,`). */
        colon,                              /** @brief Colon (`:`). */

        // Control Tokens
        end_of_line,                        /** @brief End of line token. */
        end_of_file,                        /** @brief End of file token. */
    };
}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing a token extracted by the G10
     *          assembler's lexical analyzer.
     */
    struct token final
    {
        std::string_view    lexeme;             /** @brief The token's lexeme as it appears in source code. */
        token_type          type;               /** @brief The token's strongly-enumerated type. */
        keyword_type        kw_type;            /** @brief For keyword tokens, the specific keyword type. */
        std::string_view    source_file;        /** @brief The source file from which the token was extracted. */
        std::size_t         line_number { 0 };  /** @brief The line number in the source file where the token was found. */
    
        /**
         * @brief   If the token is an integer or fixed-point literal, this contains
         *          its parsed integer value; otherwise, it is `std::nullopt`.
         */
        std::optional<std::int64_t> int_value { std::nullopt };

        /**
         * @brief   If the token is an integer or fixed-point literal, this contains
         *          its parsed floating-point value; otherwise, it is `std::nullopt`.
         */
        std::optional<double> fixed_point_float { std::nullopt };

    public: /* Public Methods *************************************************/

        /**
         * @brief   Returns a string representation of the token's type.
         * 
         * @return  A string view representing the token's type.
         */
        inline constexpr auto stringify_type () const -> std::string_view
        {
            switch (type)
            {
                case token_type::unknown:                       return "Unknown";
                case token_type::keyword:                       return stringify_keyword_type();
                case token_type::identifier:                    return "Identifier";
                case token_type::placeholder:                   return "Placeholder";
                case token_type::integer_literal:               return "Integer Literal";
                case token_type::fixed_point_literal:           return "Fixed-Point Literal";
                case token_type::character_literal:             return "Character Literal";
                case token_type::string_literal:                return "String Literal";
                case token_type::plus:                          return "Plus";
                case token_type::minus:                         return "Minus";
                case token_type::times:                         return "Times";
                case token_type::divide:                        return "Divide";
                case token_type::modulo:                        return "Modulo";
                case token_type::bitwise_and:                   return "Bitwise AND";
                case token_type::bitwise_or:                    return "Bitwise OR";
                case token_type::bitwise_xor:                   return "Bitwise XOR";
                case token_type::bitwise_not:                   return "Bitwise NOT";
                case token_type::left_shift:                    return "Left Shift";
                case token_type::right_shift:                   return "Right Shift";
                case token_type::assign_equal:                  return "Assign Equal";
                case token_type::assign_plus:                   return "Assign Plus";
                case token_type::assign_minus:                  return "Assign Minus";
                case token_type::assign_times:                  return "Assign Times";
                case token_type::assign_divide:                 return "Assign Divide";
                case token_type::assign_modulo:                 return "Assign Modulo";
                case token_type::assign_and:                    return "Assign AND";
                case token_type::assign_or:                     return "Assign OR";
                case token_type::assign_xor:                    return "Assign XOR";
                case token_type::assign_left_shift:             return "Assign Left Shift";
                case token_type::assign_right_shift:            return "Assign Right Shift";
                case token_type::compare_equal:                 return "Compare Equal";
                case token_type::compare_not_equal:             return "Compare Not Equal";
                case token_type::compare_less_than:             return "Compare Less Than";
                case token_type::compare_less_equal:            return "Compare Less Equal";
                case token_type::compare_greater_than:          return "Compare Greater Than";
                case token_type::compare_greater_equal:         return "Compare Greater Equal";
                case token_type::logical_and:                   return "Logical AND";
                case token_type::logical_or:                    return "Logical OR";
                case token_type::logical_not:                   return "Logical NOT";
                case token_type::left_parenthesis:              return "Left Parenthesis";
                case token_type::right_parenthesis:             return "Right Parenthesis";
                case token_type::left_brace:                    return "Left Brace";
                case token_type::right_brace:                   return "Right Brace";
                case token_type::left_bracket:                  return "Left Bracket";
                case token_type::right_bracket:                 return "Right Bracket";
                case token_type::comma:                         return "Comma";
                case token_type::colon:                         return "Colon";
                case token_type::end_of_line:                   return "End of Line";
                case token_type::end_of_file:                   return "End of File";
                default:                                        return "Unknown";
            }
        }

        /**
         * @brief   Returns a string representation of the token's keyword
         *          type.
         * 
         * @return  A string view representing the token's keyword type.
         */
        inline constexpr auto stringify_keyword_type () const -> std::string_view
        {
            switch (kw_type)
            {
                case keyword_type::none:                        return "No Keyword";
                case keyword_type::instruction:                 return "Instruction";
                case keyword_type::register_name:               return "Register Name";
                case keyword_type::condition_code:              return "Condition Code";
                case keyword_type::directive:                   return "Directive";
                default:                                        return "Unknown Keyword";
            }
        }

    };
}
