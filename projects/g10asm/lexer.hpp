/**
 * @file    g10asm/lexer.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-21
 * 
 * @brief   Contains definitions for the G10 assembler's lexical analysis
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/token.hpp>

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a static class representing the G10 assembler tool's
     *          lexical analysis component (henceforth, the "lexer").
     * 
     * The lexer is responsible for reading G10 assembly source code and
     * extracting tokens from it, which are then used by the parser to
     * construct the program's abstract syntax tree (AST).
     */
    class lexer final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Loads and lexically analyzes the specified source file,
         *          producing a stream of tokens.
         * 
         * @param   source_file     The path to the G10 assembly source
         *                          file to be lexically analyzed.
         * 
         * @return  If successful, returns an empty result;
         *          Otherwise, returns an error indicating the reason for
         *          the failure.
         */
        auto load_from_file (const fs::path& source_file) -> g10::result<void>;

        /**
         * @brief   Loads and lexically analyzes the specified source code
         *          string, producing a stream of tokens.
         * 
         * @param   source_code     The G10 assembly source code to be
         *                          lexically analyzed.
         * 
         * @return  If successful, returns an empty result;
         *          Otherwise, returns an error indicating the reason for
         *          the failure.
         */
        auto load_from_string (std::string_view source_code) -> g10::result<void>;

        /**
         * @brief   Clears the lexer's internal state, removing all loaded tokens
         *          and resetting its position.
         */
        auto clear () -> void;

        /**
         * @brief   Resets the lexer's current token position to the beginning
         *          of the token stream.
         */
        auto reset_position () -> void;

        /**
         * @brief   Peeks ahead (or back) in the token stream by the specified
         *          offset, returning a reference to the token found at that
         *          position.
         * 
         * @param   offset  The offset from the current token position. A value
         *                  of `0` returns the current token, a positive value
         *                  peeks ahead, and a negative value peeks back.
         * 
         * @return  If successful, returns a constant reference to the token
         *          found at the specified offset;
         *          Otherwise, returns an error indicating that the requested
         *          token is out of range.
         */
        auto peek_token (std::int64_t offset = 0) const -> g10::result_cref<token>;

        /**
         * @brief   Skips over the specified number of tokens in the token
         *          stream, advancing the lexer's current token position.
         * 
         * @param   count   The number of tokens to skip. Defaults to `1`.
         */
        auto skip_tokens (const std::size_t count = 1) const -> void;

        /**
         * @brief   Skips over tokens of the specified type in the token
         *          stream, advancing the lexer's current token position
         *          until a token of a different type is encountered.
         * 
         * @param   type    The type of tokens to skip.
         */
        auto skip_tokens (const token_type type) const -> void;

        /**
         * @brief   Consumes and retrieves the current token, advancing the
         *          lexer's current token position by one.
         * 
         * @return  If successful, returns a constant reference to the token
         *          that was consumed;
         *          Otherwise, returns an error indicating that there are no
         *          more tokens to consume.
         */
        auto consume_token () const -> g10::result_cref<token>;

        /**
         * @brief   Injects a list of tokens into the token stream at the
         *          current position.
         * 
         * @param   tokens              The list of tokens to inject.
         * @param   advance_position    If `true`, advances the current token
         *                              position past the injected tokens.
         */
        auto inject_tokens (const std::vector<token>& tokens,
            bool advance_position) -> void;

        /**
         * @brief   Loads and lexically analyzes the specified source file, then
         *          injects the resulting tokens into the token stream at the
         *          current position.
         * 
         * @param   source_file         The path to the G10 assembly source
         *                              file to be lexically analyzed and injected.
         * @param   advance_position    If `true`, advances the current token
         *                              position past the injected tokens.
         * 
         * @return  If the file is successfully loaded and tokenized, returns
         *          `void`;
         *          Otherwise, returns an error indicating the reason for the failure.
         */
        auto inject_tokens (const fs::path& source_file,
            bool advance_position) -> g10::result<void>;

        /**
         * @brief   Consumes and removes the current token from the token stream,
         *          then returns the erased token.
         * 
         * The lexer's current token position remains at the same index, which
         * will now point to the next token in the stream after the erased token.
         * 
         * @return  If successful, returns the token that was erased;
         *          Otherwise, returns an error indicating that there are no
         *          more tokens to erase.
         */
        auto erase_token () -> g10::result<token>;

        /**
         * @brief   Consumes and removes the specified number of tokens from
         *          the token stream, then returns the erased tokens as a list.
         * 
         * The lexer's current token position remains at the same index, which
         * will now point to the next token in the stream after the erased tokens.
         * 
         * @param   count   The number of tokens to erase.
         * 
         * @return  If successful, returns a list of the tokens that were erased;
         *          Otherwise, returns an error indicating that there are not
         *          enough tokens to erase.
         */
        auto erase_tokens (const std::size_t count) -> g10::result<std::vector<token>>;

        /**
         * @brief   Consumes and retrieves the current token, ensuring that
         *          it matches the expected token type.
         * 
         * If the consumed token does not match the expected type, an error
         * is returned with a formatted error message.
         * 
         * @tparam  Args    The types of any additional arguments used for
         *                  formatting the error message.
         * 
         * @param   expected    The expected token type.
         * @param   error_fmt   The format string for the error message if
         *                      the token type does not match.
         * @param   error_args  Additional arguments used for formatting
         *                      the error message.
         * 
         * @return  If successful, returns a constant reference to the token
         *          that was consumed;
         *          Otherwise, returns an error indicating that the token type
         *          did not match the expected type.
         */
        template <typename... Args>
        auto consume_token (const token_type expected, 
            const std::string& error_fmt, Args&&... error_args) const 
                -> g10::result_cref<token>
        {
            auto token_result = consume_token();
            if (token_result.has_value() == false)
            {
                return g10::error(token_result.error());
            }

            const token& tok = token_result.value();
            if (tok.type != expected)
            {
                return g10::error(
                    error_fmt,
                    std::forward<Args>(error_args)...
                );
            }

            return std::cref(tok);
        }

        /**
         * @brief   Consumes and retrieves the current token, ensuring that
         *          it is a keyword of the expected keyword type.
         * 
         * If the consumed token is not a keyword, or if its keyword type
         * does not match the expected type, an error is returned with a
         * formatted error message.
         * 
         * @tparam  Args    The types of any additional arguments used for
         *                  formatting the error message.
         * 
         * @param   expected    The expected keyword type.
         * @param   error_fmt   The format string for the error message if
         *                      the token is not a keyword or its type does
         *                      not match.
         * @param   error_args  Additional arguments used for formatting
         *                      the error message.
         * 
         * @return  If successful, returns a constant reference to the token
         *          that was consumed;
         *          Otherwise, returns an error indicating that the token is
         *          not a keyword or its type did not match the expected type.
         */
        template <typename... Args>
        auto consume_token (const keyword_type expected,
            const std::string& error_fmt, Args&&... error_args) const
                -> g10::result_cref<token>
        {
            auto token_result = consume_token();
            if (token_result.has_value() == false)
            {
                return g10::error(token_result.error());
            }

            const token& tok = token_result.value();
            if (
                (
                    tok.type != token_type::keyword &&
                    tok.type != token_type::placeholder_keyword
                ) ||
                tok.keyword_value.has_value() == false ||
                tok.keyword_value.value().get().type != expected
            )
            {
                return g10::error(
                    error_fmt,
                    std::forward<Args>(error_args)...
                );
            }

            return std::cref(tok);
        }

        /**
         * @brief   Consumes and removes the current token from the token stream,
         *          ensuring that it matches the expected token type, then
         *          returns the erased token.
         * 
         * If the consumed token does not match the expected type, an error
         * is returned with a formatted error message.
         * 
         * The lexer's current token position remains at the same index, which
         * will point to the next token in the stream after the erased token
         * if the operation is successful.
         * 
         * @tparam  Args    The types of any additional arguments used for
         *                  formatting the error message.
         * 
         * @param   expected    The expected token type.
         * @param   error_fmt   The format string for the error message if
         *                      the token type does not match.
         * @param   error_args  Additional arguments used for formatting
         *                      the error message.
         * 
         * @return  If successful, returns the token that was erased;
         *          Otherwise, returns an error indicating that the token type
         *          did not match the expected type.
         */
        template <typename... Args>
        auto erase_token (const token_type expected,
            const std::string& error_fmt, Args&&... error_args) -> g10::result<token>
        {
            // - For `erase_token`, we want to make sure that the current token
            //   matches the expected type *before* we erase it.
            auto token_result = peek_token();
            if (token_result.has_value() == false)
            {
                return g10::error(token_result.error());
            }

            const token& tok = token_result.value();
            if (tok.type != expected)
            {
                return g10::error(
                    error_fmt,
                    std::forward<Args>(error_args)...
                );
            }

            // - Now that we've verified the type, erase the token.
            return erase_token();
        }

        template <typename... Args>
        auto erase_token (const keyword_type expected,
            const std::string& error_fmt, Args&&... error_args)
                -> g10::result<token>
        {
            // - For `erase_token`, we want to make sure that the current token
            //   is a keyword of the expected type *before* we erase it.
            auto token_result = peek_token();
            if (token_result.has_value() == false)
            {
                return g10::error(token_result.error());
            }

            const token& tok = token_result.value();
            if (
                (
                    tok.type != token_type::keyword &&
                    tok.type != token_type::placeholder_keyword
                ) ||
                tok.keyword_value.has_value() == false ||
                tok.keyword_value.value().get().type != expected
            )
            {                
                return g10::error(
                    error_fmt,
                    std::forward<Args>(error_args)...
                );
            }

            // - Now that we've verified the type, erase the token.
            return erase_token();
        }

        /**
         * @brief   Retrieves the list of tokens produced by the lexer
         *          during tokenization.
         * 
         * @return  A constant reference to the vector of tokens.
         */
        inline auto get_tokens () -> const std::vector<token>&
            { return m_tokens; }
        inline auto get_tokens () const -> const std::vector<token>&
            { return m_tokens; }

        /**
         * @brief   Checks if the lexer is in a good state, meaning that
         *          tokenization was successful and no errors were encountered.
         * 
         * @return  `true` if the lexer is in a good state;
         *          Otherwise, `false`.
         */
        inline auto is_good () const -> bool
            { return m_good; }

        /**
         * @brief   Checks if the lexer has reached the end of the token stream.
         * 
         * @return  `true` if the end of the token stream has been reached;
         *          Otherwise, `false`.
         */
        inline auto is_at_end () const -> bool
            { return m_current_token >= m_tokens.size(); }

    private: /* Private Methods ***********************************************/

        /**
         * @brief   Upon construction, tokenizes the source code provided to the
         *          lexer, producing a list of tokens for further processing.
         */
        auto tokenize () -> void;

        /**
         * @brief   During tokenization, skips over any whitespace characters
         *          (spaces, tabs, newlines, etc.) in the source code.
         */
        auto skip_whitespace () -> void;

        /**
         * @brief   During tokenization, skips over comments in the source
         *          code. 
         * 
         * Comments start with a semicolon (`;`) and continue to the end of
         * the line.
         */
        auto skip_comment () -> void;

        /**
         * @brief   Scans an identifier or keyword from the source code at
         *          the current position.
         * 
         * Identifier tokens begin with a letter (`A-Z`, `a-z`), an underscore
         * (`_`), or a period (`.`), and may be followed by letters, digits
         * (`0-9`), underscores, or periods.
         * 
         * Identifiers are used to represent labels, variable names, and
         * other user-defined symbols in the source code. Certain identifiers
         * may also correspond to keywords in the G10 assembly language, such
         * as instruction mnemonics and assembler directives.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_identifier_or_keyword () -> g10::result<void>;

        /**
         * @brief   Scans a variable token from the source code at the
         *          current position.
         * 
         * Variable tokens begin with a dollar sign (`$`), followed by
         * letters, digits, or underscores.
         * 
         * In the G10 assembly language, variables and constants can be used
         * to represent values that may change during assembly or execution.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_variable () -> g10::result<void>; 

        /**
         * @brief   Scans a placeholder token from the source code at the
         *          current position.
         * 
         * Placeholder tokens begin with an "at" symbol (`@`), followed by letters,
         * digits, or underscores.
         * 
         * In the G10 assembly language's macro language, placeholders are used
         * to represent parameters, either named, positional or special-purpose,
         * within macro definitions and invocations.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_placeholder () -> g10::result<void>;

        /**
         * @brief   Scans a binary integer literal from the source code at the
         *          current position.
         * 
         * This is called internally by the @a `scan_integer_or_number_literal`
         * method when it encounters a binary integer literal, which begins with
         * the prefix `0b` or `0B`, followed by a sequence of binary digits
         * (`0` and `1`).
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_binary_integer_literal () -> g10::result<void>;

        /**
         * @brief   Scans an octal integer literal from the source code at the
         *          current position.
         * 
         * This is called internally by the @a `scan_integer_or_number_literal`
         * method when it encounters an octal integer literal, which begins with
         * the prefix `0o` or `0O`, followed by a sequence of octal digits
         * (`0` to `7`).
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_octal_integer_literal () -> g10::result<void>;

        /**
         * @brief   Scans a hexadecimal integer literal from the source code at
         *          the current position.
         * 
         * This is called internally by the @a `scan_integer_or_number_literal`
         * method when it encounters a hexadecimal integer literal, which begins
         * with the prefix `0x` or `0X`, followed by a sequence of hexadecimal
         * digits (`0` to `9` and `A` to `F`).
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_hexadecimal_integer_literal () -> g10::result<void>;

        /**
         * @brief   Scans an integer or floating-point number literal from the
         *          source code at the current position.
         * 
         * Integer literals consist of a sequence of digits (`0` to `9`). Note
         * that signs (`+` or `-`) are not part of the integer literal itself, 
         * but are treated as separate tokens, which will be handled during parsing.
         * 
         * Number literals (floating-point literals) consist of a sequence of digits
         * (`0` to `9`), a decimal point (`.`). Numbers in the G10 assembly language
         * are picked up by the lexer in floating-point format, and will be translated
         * into `32.32` fixed-point format during later stages of assembly.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_integer_or_number_literal () -> g10::result<void>;

        /**
         * @brief   Scans a character literal from the source code at the
         *          current position.
         * 
         * Character literals are enclosed in single quotes (`'`), and may
         * contain escape sequences (e.g., `'\n'` for newline, `'\t'` for
         * tab, etc.).
         * 
         * Unless an escape sequence is used, a character literal can only
         * contain a single character.
         * 
         * Character literals must be properly terminated with a closing single
         * quote; otherwise, an error is returned.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_character_literal () -> g10::result<void>;

        /**
         * @brief   Scans a string literal from the source code at the
         *          current position.
         * 
         * String literals are enclosed in double quotes (`"`), and may
         * contain escape sequences (e.g., `"\n"` for newline, `"\t"` for
         * tab, etc.).
         * 
         * String literals must be properly terminated with a closing double
         * quote; otherwise, an error is returned.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_string_literal () -> g10::result<void>;

        /**
         * @brief   Scans a symbol from the source code at the current position.
         * 
         * This is the catch-all method for scanning single- or multi-character
         * symbols that do not fall into the categories of identifiers, literals,
         * or placeholders.
         * 
         * Symbols include operators (arithmetic, bitwise, logical, comparison,
         * and assignment), punctuation (commas, semicolons, colons, parentheses,
         * brackets, braces), and other special characters.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns a string describing the error encountered.
         */
        auto scan_symbol () -> g10::result<void>;

    private: /* Private Members ***********************************************/

        /**
         * @file    If the source code processed by this lexer was read from a
         *          file, this is the absolute, lexically-normalized path to
         *          that file.
         */
        std::string m_source_file { "" };

        /**
         * @brief   The source code being processed by this lexer.
         */
        std::string m_source_code { "" };   

        /**
         * @brief   During lexical analysis, this contains the current position
         *          within the source code being processed.
         */
        std::size_t m_current_position { 0 };

        /**
         * @brief   During lexical analysis, this contains the current line
         *          number within the source code being processed (1-based).
         */
        std::size_t m_current_line { 1 };

        /**
         * @brief   During lexical analysis, this contains the current column
         *          number within the source code being processed (1-based).
         */
        std::size_t m_current_column { 1 };

        /**
         * @brief   During parsing, this contains the index of the current
         *          token being processed from the list of tokens produced
         *          by the lexer.
         */
        mutable std::size_t m_current_token { 0 };

        /**
         * @brief   The list of tokens produced by this lexer after processing
         *          the source code.
         */
        std::vector<token> m_tokens;

        /**
         * @brief   Indicates whether the lexer has successfully processed the
         *          source code without encountering any errors.
         */
        bool m_good { false };

    };
}
