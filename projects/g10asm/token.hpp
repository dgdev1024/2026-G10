/**
 * @file    g10asm/token.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains definitions for the G10 assembler lexer's token structure.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/keyword_table.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{
    /**
     * @brief   Strongly enumerates the different types of tokens produced
     *          by the G10 assembler's lexer.
     */
    enum class token_type
    {
        unknown,                            /** @brief An unknown or invalid token. */

        // Keywords and Identifiers
        keyword,                            /** @brief A recognized keyword (mnemonic, directive, register, etc.). */
        identifier,                         /** @brief A user-defined identifier (label, variable name, etc.). */
        variable,                           /** @brief A variable or constant token used during parsing (an identifier starting with a `$`). */
        placeholder,                        /** @brief A placeholder token used during parsing; an integer or identifier starting with a `$`. */
        placeholder_keyword,                /** @brief Certain placeholders can be reserved keywords, too. */

        // Literals
        integer_literal,                    /** @brief An integer literal (e.g., `123`, `0x7B`, `0b1111011`, etc.). */
        number_literal,                     /** @brief A floating-point number literal (e.g., `3.14`, `0.001`, etc.). */
        character_literal,                  /** @brief A character literal (e.g., `'A'`, `'\n'`, etc.). */
        string_literal,                     /** @brief A string literal (e.g., `"Hello, World!"`). */

        // Arithmetic and Bitwise Operators
        plus,                               /** @brief The addition operator (`+`). */
        minus,                              /** @brief The subtraction operator (`-`). */
        times,                              /** @brief The multiplication operator (`*`). */
        exponent,                           /** @brief The exponentiation operator (`**`). */
        divide,                             /** @brief The division operator (`/`). */
        modulo,                             /** @brief The modulo operator (`%`). */
        bitwise_and,                        /** @brief The bitwise AND operator (`&`). */
        bitwise_or,                         /** @brief The bitwise OR operator (`|`). */
        bitwise_xor,                        /** @brief The bitwise XOR operator (`^`). */
        bitwise_not,                        /** @brief The bitwise NOT operator (`~`). */
        bitwise_shift_left,                 /** @brief The bitwise shift left operator (`<<`). */
        bitwise_shift_right,                /** @brief The bitwise shift right operator (`>>`). */

        // Assignment Operators
        assign_equal,                       /** @brief The assignment operator (`=`). */
        assign_plus,                        /** @brief The addition assignment operator (`+=`). */
        assign_minus,                       /** @brief The subtraction assignment operator (`-=`). */
        assign_times,                       /** @brief The multiplication assignment operator (`*=`). */
        assign_exponent,                    /** @brief The exponentiation assignment operator (`**=`). */
        assign_divide,                      /** @brief The division assignment operator (`/=`). */
        assign_modulo,                      /** @brief The modulo assignment operator (`%=`). */
        assign_and,                         /** @brief The bitwise AND assignment operator (`&=`). */
        assign_or,                          /** @brief The bitwise OR assignment operator (`|=`). */
        assign_xor,                         /** @brief The bitwise XOR assignment operator (`^=`). */
        assign_shift_left,                  /** @brief The bitwise shift left assignment operator (`<<=`). */
        assign_shift_right,                 /** @brief The bitwise shift right assignment operator (`>>=`). */

        // Comparison Operators
        compare_equal,                      /** @brief The equality comparison operator (`==`). */
        compare_not_equal,                  /** @brief The inequality comparison operator (`!=`). */
        compare_less,                       /** @brief The less-than comparison operator (`<`). */
        compare_less_equal,                 /** @brief The less-than-or-equal-to comparison operator (`<=`). */
        compare_greater,                    /** @brief The greater-than comparison operator (`>`). */
        compare_greater_equal,              /** @brief The greater-than-or-equal-to comparison operator (`>=`). */

        // Logical Operators
        logical_and,                        /** @brief The logical AND operator (`&&`). */
        logical_or,                         /** @brief The logical OR operator (`||`). */
        logical_not,                        /** @brief The logical NOT operator (`!`). */

        // Grouping Operators
        left_parenthesis,                   /** @brief The left parenthesis (`(`). */
        right_parenthesis,                  /** @brief The right parenthesis (`)`). */
        left_bracket,                       /** @brief The left bracket (`[`). */
        right_bracket,                      /** @brief The right bracket (`]`). */
        left_brace,                         /** @brief The left brace (`{`). */
        right_brace,                        /** @brief The right brace (`}`). */

        // Punctuation
        comma,                              /** @brief The comma punctuation mark (`,`). */
        colon,                              /** @brief The colon punctuation mark (`:`). */
        question_mark,                      /** @brief The question mark (`?`). */
        backtick,                           /** @brief The backtick character (`\``). */
        backslash,                          /** @brief The backslash character (`\`). */
        hash,                               /** @brief The hash/pound character (`#`). */
        double_hash,                        /** @brief The double-hash/pound characters (`##`). */

        // Control Tokens
        new_line,                           /** @brief A newline token representing the end of a line. */
        end_of_file                         /** @brief An end-of-file token representing the end of the input stream. */
    };
}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing a token produced by the G10
     *          assembler's lexer.
     */
    struct token final
    {
        token_type          type;               /** @brief The type of token. */
        std::string_view    lexeme = "";        /** @brief The string contents of the token as found in the source code. */
        std::string_view    source_file = "";   /** @brief The source file from which the token was read. */
        std::size_t         source_line = 1;    /** @brief The line number in the source file where the token was found (1-based). */
        std::size_t         source_column = 1;  /** @brief The column number in the source file where the token starts (1-based). */

        /**
         * @brief   For integer and number literals, holds its integer value.
         */
        std::optional<std::int64_t> int_value = std::nullopt;

        /**
         * @brief   For integer and number literals, holds its floating-point value.
         */
        std::optional<double> number_value = std::nullopt;

        /**
         * @brief   For keyword tokens, holds a reference to the keyword entry.
         */
        g10::optional_cref<keyword> keyword_value = std::nullopt;

    public:

        static constexpr auto stringify_type (token_type type) -> std::string_view
        {
            switch (type)
            {
                case token_type::identifier:              return "identifier";
                case token_type::variable:                return "variable";
                case token_type::placeholder:             return "placeholder";
                case token_type::integer_literal:         return "integer_literal";
                case token_type::number_literal:          return "number_literal";
                case token_type::character_literal:       return "character_literal";
                case token_type::string_literal:          return "string_literal";
                case token_type::plus:                    return "plus";
                case token_type::minus:                   return "minus";
                case token_type::times:                   return "times";
                case token_type::exponent:                return "exponent";
                case token_type::divide:                  return "divide";
                case token_type::modulo:                  return "modulo";
                case token_type::bitwise_and:             return "bitwise_and";
                case token_type::bitwise_or:              return "bitwise_or";
                case token_type::bitwise_xor:             return "bitwise_xor";
                case token_type::bitwise_not:             return "bitwise_not";
                case token_type::bitwise_shift_left:      return "bitwise_shift_left";
                case token_type::bitwise_shift_right:     return "bitwise_shift_right";
                case token_type::assign_equal:            return "assign_equal";
                case token_type::assign_plus:             return "assign_plus";
                case token_type::assign_minus:            return "assign_minus";
                case token_type::assign_times:            return "assign_times";
                case token_type::assign_exponent:         return "assign_exponent";
                case token_type::assign_divide:           return "assign_divide";
                case token_type::assign_modulo:           return "assign_modulo";
                case token_type::assign_and:              return "assign_and";
                case token_type::assign_or:               return "assign_or";
                case token_type::assign_xor:              return "assign_xor";
                case token_type::assign_shift_left:       return "assign_shift_left";
                case token_type::assign_shift_right:      return "assign_shift_right";
                case token_type::compare_equal:           return "compare_equal";
                case token_type::compare_not_equal:       return "compare_not_equal";
                case token_type::compare_less:            return "compare_less";
                case token_type::compare_less_equal:      return "compare_less_equal";
                case token_type::compare_greater:         return "compare_greater";
                case token_type::compare_greater_equal:   return "compare_greater_equal";
                case token_type::logical_and:             return "logical_and";
                case token_type::logical_or:              return "logical_or";
                case token_type::logical_not:             return "logical_not";
                case token_type::left_parenthesis:        return "left_parenthesis";
                case token_type::right_parenthesis:       return "right_parenthesis";
                case token_type::left_bracket:            return "left_bracket";
                case token_type::right_bracket:           return "right_bracket";
                case token_type::left_brace:              return "left_brace";
                case token_type::right_brace:             return "right_brace";
                case token_type::comma:                   return "comma";
                case token_type::colon:                   return "colon";
                case token_type::question_mark:           return "question_mark";
                case token_type::backtick:                return "backtick";
                case token_type::backslash:               return "backslash";
                case token_type::hash:                    return "hash";
                case token_type::double_hash:             return "double_hash";
                case token_type::new_line:                return "new_line";
                case token_type::end_of_file:             return "end_of_file";
                case token_type::keyword:                 return "keyword";
                case token_type::placeholder_keyword:     return "placeholder_keyword";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief   Converts the token type to a human-readable string.
         * 
         * @return  A string view representing the token type.
         */
        constexpr auto type_to_string () const -> std::string_view
        {
            if (keyword_value.has_value() == true)
            {
                switch (keyword_value.value().get().type)
                {
                    case keyword_type::instruction_mnemonic: return "instruction_mnemonic";
                    case keyword_type::preprocessor_function: return "preprocessor_function";
                    case keyword_type::preprocessor_directive: return "preprocessor_directive";
                    case keyword_type::pragma: return "pragma";
                    case keyword_type::assembler_directive: return "assembler_directive";
                    case keyword_type::register_name: return "register_name";
                    case keyword_type::branching_condition: return "branching_condition";
                    default: break;
                }
            }

            return stringify_type(type);
        }

        /**
         * @brief   Converts the token to a human-readable string.
         * 
         * @return  A string representing the token.
         */
        auto to_string () const -> std::string
        {
            switch (type)
            {
                case token_type::identifier:
                case token_type::placeholder:
                case token_type::keyword:
                case token_type::placeholder_keyword:
                    return std::format("{} ('{}')", type_to_string(), lexeme);
                
                case token_type::integer_literal:
                    return std::format("{} ('{}', value = {})",
                        type_to_string(),
                        lexeme,
                        int_value.has_value() ? std::to_string(int_value.value()) : "null"
                    );

                case token_type::number_literal:
                    return std::format("{} ('{}', value = {})",
                        type_to_string(),
                        lexeme,
                        number_value.has_value() ? std::to_string(number_value.value()) : "null"
                    );

                case token_type::character_literal:
                    return std::format("{} ('{}', value = {})",
                        type_to_string(),
                        lexeme,
                        int_value.has_value() ? std::to_string(int_value.value()) : "null"
                    );

                case token_type::string_literal:
                    return std::format("{} ('{}')", type_to_string(), lexeme);

                default:
                    return std::format("{}", type_to_string());
            }
        }

    };
}
