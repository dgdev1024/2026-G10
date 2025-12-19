/**
 * @file    g10-asm/lexer.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains declarations for the G10 assembler tool's lexical analyzer
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/token.hpp>

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing a lexical analyzer used by the
     *          G10 assembler tool to tokenize assembly source code.
     */
    class lexer final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a new @a `lexer` instance, initializing it with
         *          the provided assembly source code and optional source file
         *          path.
         * 
         * @param   source_code     A string containing the assembly source code
         *                          to be tokenized.
         * @param   source_path     An optional filesystem path representing the
         *                          location of the source file. Defaults to an
         *                          empty path if not provided. If it is
         *                          provided, it should be an absolute, lexically
         *                          normalized path. 
         */
        explicit lexer (
            const std::string&  source_code,
            const fs::path&     source_path = ""
        );

        /**
         * @brief   Factory method that creates a new @a `lexer` instance from
         *          the provided assembly source code string.
         * 
         * @param   source_code     A string containing the assembly source code
         *                          to be tokenized.
         * 
         * @return  If successful, returns a unique pointer to the newly-created
         *          and tokenized @a `lexer` instance;
         *          otherwise, returns an error describing the failure.
         */
        static auto from_string (const std::string& source_code)
            -> result_ref<lexer>;

        /**
         * @brief   Factory method that creates a new @a `lexer` instance from
         *          the assembly source code contained in the specified file.
         * 
         * @param   source_path     A filesystem path representing the location
         *                          of the source file. It should be an absolute,
         *                          lexically normalized path.
         *
         * @return  If successful, returns a unique pointer to the newly-created
         *          and tokenized @a `lexer` instance;
         *          otherwise, returns an error describing the failure.
         */
        static auto from_file (const fs::path& source_path)
            -> result_ref<lexer>;

        /**
         * @brief   Retrieves the next token from the lexer's token stream.
         *
         * @return  If successful, returns a reference to the next @a `token` in
         *          the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto next_token () -> g10::result_ref<token>;

        /**
         * @brief   Peeks at the next token in the lexer's token stream without
         *          advancing the internal token pointer.
         *
         * @return  If successful, returns a reference to the next @a `token` in
         *          the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto next_token () const -> g10::result_cref<token>;

        /**
         * @brief   Peeks at a token in the lexer's token stream at the
         *          specified offset from the current token pointer, without
         *          advancing the internal token pointer.
         * 
         * @param   offset  The number of tokens ahead of the current token
         *                  pointer to peek at. Defaults to `1`, which peeks at
         *                  the next token.
         * 
         * @return  If successful, returns a reference to the @a `token` at the
         *          specified offset in the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto peek_token (std::int64_t offset = 1) -> g10::result_ref<token>;

        /**
         * @brief   Peeks at a token in the lexer's token stream at the
         *          specified offset from the current token pointer, without
         *          advancing the internal token pointer.
         * 
         * @param   offset  The number of tokens ahead of the current token
         *                  pointer to peek at. Defaults to `1`, which peeks at
         *                  the next token.
         * 
         * @return  If successful, returns a reference to the @a `token` at the
         *          specified offset in the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto peek_token (std::int64_t offset = 1) const -> g10::result_cref<token>;

        /**
         * @brief   Retrieves the list of tokens generated by the lexer.
         * 
         * @return  A reference to the vector containing all tokens produced
         *          during tokenization.
         */
        inline auto get_tokens () -> std::vector<token>&
            { return m_tokens; }
        inline auto get_tokens () const -> const std::vector<token>&
            { return m_tokens; }

        /**
         * @brief   Indicates whether the lexer is in a good state.
         * 
         * The lexer is considered "good" if it has successfully tokenized the
         * source code without encountering any errors.
         *
         * @return  `true` if the lexer is in a good state; 
         *          Otherwise, `false`.
         */
        inline constexpr auto is_good () const -> bool
            { return m_good; }

        /**
         * @brief   Indicates whether the lexer has reached the end of its token
         *          stream.
         * 
         * @return  `true` if the end of the token stream has been reached;
         *          Otherwise, `false`.
         */
        inline auto is_at_end () const -> bool
            { return m_token_index >= m_tokens.size(); }

    private: /* Private Methods ***********************************************/

        /**
         * @brief   Tokenizes the assembly source code contained within the
         *          lexer instance.
         */
        auto tokenize () -> void;

        /**
         * @brief   Scans and tokenizes an identifier from the source code
         *          during tokenization.
         * 
         * Identifiers may begin with a letter (`A-Z`, `a-z`), an underscore
         * (`_`), or a dot (`.`), and may contain alphanumeric characters (`A-Z`,
         * `a-z`, `0-9`), underscores (`_`), or dots (`.`).
         * 
         * This method also checks if the scanned identifier matches any
         * known keywords in the keyword table; if a match is found, the token
         * is classified as a keyword token.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_identifier () -> result<void>;

        /**
         * @brief   Scans and tokenizes a number from the source code during
         *          tokenization.
         * 
         * Number tokens may be in any one of the following formats:
         * 
         * - Decimal (e.g., `123`, `0`, `4567`)
         * 
         * - Fixed-Point Decimal (`32.32` format; e.g., `123.456`, `0.0`)
         * 
         * - Hexadecimal (prefixed with `0x` or `0X`; e.g., `0x1A3F`, `0X0`)
         * 
         * - Octal (prefixed with `0o` or `0O`; e.g., `0o755`, `0O0`)
         * 
         * - Binary (prefixed with `0b` or `0B`; e.g., `0b1101`, `0B0`)
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_number () -> result<void>;

        /**
         * @brief   Scans and tokenizes a binary number from the source code
         *          during tokenization.
         * 
         * Called internally by @a `scan_number()` when a binary number is
         * encountered.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_binary_number () -> result<void>;

        /**
         * @brief   Scans and tokenizes an octal number from the source
         *          code during tokenization.
         * 
         * Called internally by @a `scan_number()` when an octal number is
         * encountered.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_octal_number () -> result<void>;

        /**
         * @brief   Scans and tokenizes a hexadecimal number from the source
         *          code during tokenization.
         * 
         * Called internally by @a `scan_number()` when a hexadecimal number is
         * encountered.
         * 
         * @param   single_char_prefix  Indicates whether the hexadecimal number
         *                              token being scanned uses a single-character
         *                              prefix (`$`), as opposed to the standard
         *                              two-character prefix (`0x` or `0X`).   
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_hexadecimal_number (bool single_char_prefix = false) -> result<void>;

        /**
         * @brief   Scans and tokenizes a character literal from the source
         *          code during tokenization.
         * 
         * Character literals are enclosed in single quotes (e.g., `'A'`,
         * `'\n'`, `'\x41'`) and may include escape sequences. Character
         * literals which do not include escape sequences may only contain a
         * single character.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_character_literal () -> result<void>;

        /**
         * @brief   Scans and tokenizes a string literal from the source code
         *          during tokenization.
         * 
         * String literals are enclosed in double quotes (e.g., `"Hello, World!"`)
         * and may include escape sequences.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_string_literal () -> result<void>;

        /**
         * @brief   Scans and tokenizes a placeholder from the source code
         *          during tokenization.
         * 
         * Placeholders are just like identifiers, except they begin with an "at"
         * symbol (`@`) (e.g., `@my_placeholder`, `@value1`).
         * 
         * Placeholder tokens are used to indicate named and positional parameters,
         * as well as other important mechanics, in the G10 assembly language's
         * macro system.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_placeholder () -> result<void>;

        /**
         * @brief   Scans and tokenizes a symbol from the source code during
         *          tokenization.
         * 
         * Symbols include operators, grouping characters, and punctuation.
         * 
         * @return  If successful, returns an empty result;
         *          otherwise, returns an error describing the failure.
         */
        auto scan_symbol () -> result<void>;

        /**
         * @brief   Skips over whitespace characters, as well as characters that
         *          are part of comments, in the source code during tokenization.
         * 
         * Whitespace characters include spaces, tabs, and newline characters.
         * Comments begin with a semicolon (`;`) and continue to the end of the
         * line.
         * 
         * If, during skipping, a newline character is encountered, the lexer's
         * current line number is incremented accordingly and a newline token is
         * emplaced in the token list.
         */
        auto skip_chars () -> void;

        /**
         * @brief   Retrieves the current character being examined from the
         *          source code string during tokenization.
         * 
         * @return  The current character being examined from the source code
         *          string.
         */
        auto current_char () const -> char;

        /**
         * @brief   Peeks ahead (or looks behind) in the source code string by
         *          the specified offset without modifying the current character
         *          position.
         * 
         * @param   offset      The number of characters to look ahead (positive
         *                      offset) or behind (negative offset) from the
         *                      current character position.
         * 
         * @return  The character at the specified offset from the current
         *          character position.
         */
        auto peek_char (std::int64_t offset) const -> char;

        /**
         * @brief   Advances the current character position in the source code
         *          string by the specified count and returns the new current
         *          character.
         * 
         * @param   count       The number of characters to advance. Defaults to
         *                      `1`.
         * 
         * @return  If the new current position is within the bounds of the
         *          source code string, returns the character at that position;
         *          Otherwise, returns the null character (`'\0'`).
         */
        auto advance_char (std::size_t count = 1) -> char;

        /**
         * @brief   Attempts to match the character at the current position in
         *          the source code, offset by the specified count, against the
         *          expected character.
         * 
         * @param   expected    The character to match against.
         * @param   count       The number of characters to offset from the
         *                      current position. Defaults to `0`.
         * 
         * @return  `true` if the character at the offset position matches the
         *          expected character;
         *          Otherwise, `false`.
         */
        auto match_char (char expected, std::int64_t count = 0) -> bool;

        /**
         * @brief   Emplaces a new token at the end of the lexer's token list.
         * 
         * @param   lexeme      A string view representing the lexeme of the new
         *                      token.
         * @param   tk_type     The strongly-enumerated type of the new token.
         * @param   kw_type     For keyword tokens, the specific keyword type of
         *                      the new token; otherwise, `keyword_type::none`.
         *                      Defaults to `keyword_type::none`.
         * 
         * @return  A reference to the newly-emplaced token.
         */
        auto emplace_token (std::string_view lexeme, token_type tk_type,
            keyword_type kw_type = keyword_type::none)
                -> token&;

    private: /* Private Members ***********************************************/

        /**
         * @brief   A cache mapping source file paths to their corresponding
         *          @a `lexer` instances to avoid redundant lexing of the same
         *          source files.
         */
        static std::unordered_map<fs::path, std::unique_ptr<lexer>> s_cache;

        /**
         * @brief   A string containing the assembly source code to be tokenized.
         */
        std::string m_source_code;

        /**
         * @brief   The absolute, lexically normalized filesystem path of the
         *          source file from which the assembly source code was read.
         * 
         * If the source code was provided directly from a string rather than
         * a file, this will be an empty path.
         */
        std::string m_source_path;

        /**
         * @brief   Indicates whether the lexer is in a good state.
         * 
         * The lexer is considered "good" if it has successfully tokenized the
         * source code without encountering any errors.
         */
        bool m_good { false };

        /**
         * @brief   Contains the tokens extracted from the assembly source code.
         */
        std::vector<token> m_tokens;

        /**
         * @brief   The position of the next token to be read from the token
         *          stream.
         */
        mutable std::size_t m_token_index { 0 };

        /**
         * @brief   During tokenization, this tracks the current line number
         *          within the source code string.
         */
        std::size_t m_line_number { 1 };

        /**
         * @brief   During tokenization, this tracks the current character index
         *          within the source code string.
         */
        std::size_t m_char_index { 0 };

        /**
         * @brief   During tokenization, this holds the current character being
         *          examined from the source code string.
         */
        char m_char { '\0' };

    };
}
