/**
 * @file    g10asm/lexer.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-21
 * 
 * @brief   Contains definitions for the G10 assembler's lexical analysis
 *          component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/lexer.hpp>

/* Public Methods *************************************************************/

namespace g10asm
{

    auto lexer::load_from_file (const fs::path& source_file)
        -> g10::result<void>
    {
        // - Ensure the source file path is valid (not empty, exists and is a
        //   regular file).
        if (source_file.empty() == true)
        {
            std::println(stderr,
                "Lexer Initialization Error:\n"
                " - Source file path is empty."
            );
            return g10::error("Lexer Initialization Error");
        }

        m_source_file = fs::absolute(source_file).lexically_normal().string();
        if (fs::exists(m_source_file) == false)
        {
            std::println(stderr,
                "Lexer Initialization Error:\n"
                " - Source file '{}' does not exist.",
                m_source_file
            );
            return g10::error("Lexer Initialization Error");
        }
        else if (fs::is_regular_file(m_source_file) == false)
        {
            std::println(stderr,
                "Lexer Initialization Error:\n"
                " - Path '{}' does not resolve to a regular file.",
                m_source_file
            );
            return g10::error("Lexer Initialization Error");
        }

        // - Read the source file's contents into memory.
        std::fstream file { m_source_file, std::ios::in };
        if (file.is_open() == false)
        {
            std::println(stderr,
                "Lexer Initialization Error:\n"
                " - Could not open source file '{}' for reading.",
                m_source_file
            );
            return g10::error("Lexer Initialization Error");
        }

        // - Read the entire file content.
        m_source_code = std::string {
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
        file.close();

        // - Tokenize the source code.
        tokenize();

        if (m_good == false)
        {
            return g10::error("Lexer failed to tokenize source file.");
        }

        return {};
    }

    auto lexer::load_from_string (std::string_view source_code)
        -> g10::result<void>
    {
        m_source_code = std::string { source_code };

        // - Tokenize the source code.
        tokenize();

        if (m_good == false)
        {
            return g10::error("Lexer failed to tokenize source string.");
        }

        return {};
    }

    auto lexer::clear () -> void
    {
        m_tokens.clear();
        m_current_position = 0;
        m_current_line = 1;
        m_current_column = 1;
        m_current_token = 0;
        m_good = false;
    }

    auto lexer::reset_position () -> void
    {
        m_current_token = 0;
    }

    auto lexer::peek_token (std::int64_t offset) const
        -> g10::result_cref<token>
    {
        // - Calculate the target index.
        std::int64_t target_index = 
            static_cast<std::int64_t>(m_current_token) + offset;

        // - Check if the target index is within bounds.
        if (target_index < 0 || 
            static_cast<std::size_t>(target_index) >= m_tokens.size())
        {
            return g10::error(
                "Token peek offset {} from position {} is out of range.",
                offset, m_current_token
            );
        }

        return std::cref(m_tokens[static_cast<std::size_t>(target_index)]);
    }

    auto lexer::skip_tokens (const std::size_t count) const -> void
    {
        m_current_token += count;
        if (m_current_token > m_tokens.size())
        {
            m_current_token = m_tokens.size();
        }
    }

    auto lexer::skip_tokens (const token_type type) const -> void
    {
        while (m_current_token < m_tokens.size() &&
               m_tokens[m_current_token].type == type)
        {
            ++m_current_token;
        }
    }

    auto lexer::consume_token () const -> g10::result_cref<token>
    {
        // - Check if there are more tokens to consume.
        if (m_current_token >= m_tokens.size())
        {
            return g10::error(
                "No more tokens to consume; current token index {} is out of range.",
                m_current_token
            );
        }

        // - Return the current token and advance the token index.
        return std::cref(m_tokens[m_current_token++]);
    }

    auto lexer::inject_tokens (const std::vector<token>& tokens,
        bool advance_position) -> void
    {
        // - If there are no tokens to inject, do nothing.
        if (tokens.empty() == true)
        {
            return;
        }

        // - Insert the tokens at the current position.
        m_tokens.insert(
            m_tokens.begin() + m_current_token,
            tokens.begin(),
            tokens.end()
        );

        // - Advance the current token position if requested.
        if (advance_position == true)
        {
            m_current_token += tokens.size();
        }
    }

    auto lexer::inject_tokens (const fs::path& source_file,
        bool advance_position) -> g10::result<void>
    {
        // - Create a temporary lexer to tokenize the specified source file.
        lexer temp_lexer;
        auto result = temp_lexer.load_from_file(source_file);
        if (result.has_value() == false)
        {
            return g10::error(
                "Failed to inject tokens from source file '{}'.",
                source_file.string()
            );
        }

        // - Inject the tokens from the temporary lexer.
        inject_tokens(temp_lexer.get_tokens(), advance_position);

        return {};
    }

    auto lexer::erase_token () -> g10::result<token>
    {
        // - Check if there are more tokens to erase.
        if (m_current_token >= m_tokens.size())
        {
            return g10::error(
                "No more tokens to erase; current token index {} is out of range.",
                m_current_token
            );
        }

        // - Move the token out of the vector.
        token erased_token = std::move(m_tokens[m_current_token]);

        // - Erase the token from the vector.
        m_tokens.erase(m_tokens.begin() + m_current_token);

        // - Return the erased token.
        return erased_token;
    }

    auto lexer::erase_tokens (const std::size_t count)
        -> g10::result<std::vector<token>>
    {
        // - Check if there are enough tokens to erase.
        if (m_current_token + count > m_tokens.size())
        {
            return g10::error(
                "Not enough tokens to erase; "
                "requested {} tokens from position {}, "
                "but only {} tokens available.",
                count, m_current_token, m_tokens.size() - m_current_token
            );
        }

        // - Move the tokens out of the vector.
        std::vector<token> erased_tokens;
        erased_tokens.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            erased_tokens.emplace_back(std::move(m_tokens[m_current_token]));
            m_tokens.erase(m_tokens.begin() + m_current_token);
        }

        // - Return the erased tokens.
        return erased_tokens;
    }
}

/* Private Methods ************************************************************/

namespace g10asm
{
    auto lexer::tokenize () -> void
    {
        // - Clear the lexer's state before tokenization.
        clear();

        // - Main Tokenization Loop
        while (m_current_position < m_source_code.size())
        {
            // - Skip whitespace and comments.
            while (m_current_position < m_source_code.size())
            {
                skip_whitespace();
                if (m_current_position < m_source_code.size() && 
                    m_source_code[m_current_position] == ';')
                {
                    skip_comment();
                }
                else
                {
                    break;
                }
            }

            // - If, after skipping, we are at the end of the source code, break.
            if (m_current_position >= m_source_code.size())
            {
                break;
            }

            // - Determine the type of token to scan based on the current character.
            const char& ch = m_source_code[m_current_position];
            g10::result<void> scan {};

            if (std::isalpha(ch) || ch == '_' || ch == '.')
                { scan = scan_identifier_or_keyword(); }
            else if (ch == '$')
                { scan = scan_variable(); }
            else if (ch == '@')
                { scan = scan_placeholder(); }
            else if (std::isdigit(ch))
                { scan = scan_integer_or_number_literal(); }
            else if (ch == '\'')
                { scan = scan_character_literal(); }
            else if (ch == '"')
                { scan = scan_string_literal(); }
            else
                { scan = scan_symbol(); }

            // - If scanning failed, report the error and stop tokenization.
            if (scan.has_value() == false)
            {
                std::println(stderr,
                    "Lexical Analysis Error:\n"
                    " - {}\n"
                    " - In file '{}:{}:{}'",
                    scan.error(),
                    m_source_file, m_current_line, m_current_column
                );
                m_good = false;
                return;
            }
        }

        // - After tokenization, append an end-of-file token.
        m_tokens.emplace_back(
            token {
                .type = token_type::end_of_file,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = m_current_column
            }
        );

        // - Tokenization complete.
        m_good = true;
    }

    auto lexer::skip_whitespace () -> void
    {
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (std::isspace(ch))
            {
                // - Handle newlines for line and column tracking.
                if (ch == '\n')
                {
                    // - Emplace a newline token.
                    m_tokens.emplace_back(
                        token {
                            .type = token_type::new_line,
                            .lexeme = "\n",
                            .source_file = m_source_file,
                            .source_line = m_current_line,
                            .source_column = m_current_column
                        }
                    );

                    ++m_current_line;
                    m_current_column = 1;
                }
                else
                {
                    ++m_current_column;
                }

                // - Advance to the next character.
                ++m_current_position;
            }
            else
            {
                break;
            }
        }
    }

    auto lexer::skip_comment () -> void
    {
        const char& ch = m_source_code[m_current_position];
        if (ch == ';')
        {
            // - Advance until the end of the line or end of file.
            while (m_current_position < m_source_code.size() &&
                   m_source_code[m_current_position] != '\n')
            {
                ++m_current_position;
                ++m_current_column;
            }

            // - If we stopped at a newline, handle it.
            if (m_current_position < m_source_code.size() &&
                m_source_code[m_current_position] == '\n')
            {
                // - Emplace a newline token.
                m_tokens.emplace_back(
                    token {
                        .type = token_type::new_line,
                        .source_file = m_source_file,
                        .source_line = m_current_line,
                        .source_column = m_current_column
                    }
                );

                ++m_current_line;
                m_current_column = 1;
                ++m_current_position;
            }
        }
    }

    auto lexer::scan_identifier_or_keyword () -> g10::result<void>
    {
        // - Get the starting position and column.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Scan while the current character is valid for identifiers.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (std::isalnum(ch) || ch == '_' || ch == '.')
            {
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Look up the lexeme in the keyword table.
        auto keyword_result = keyword_table::lookup_keyword(lexeme);
        if (keyword_result.has_value() == true)
        {
            // - It's a keyword.
            m_tokens.emplace_back(
                token {
                    .type = token_type::keyword,
                    .lexeme = lexeme,
                    .source_file = m_source_file,
                    .source_line = m_current_line,
                    .source_column = start_column,
                    .keyword_value = keyword_result.value()
                }
            );
        }
        else
        {
            // - It's an identifier.
            m_tokens.emplace_back(
                token {
                    .type = token_type::identifier,
                    .lexeme = lexeme,
                    .source_file = m_source_file,
                    .source_line = m_current_line,
                    .source_column = start_column
                }
            );
        }

        return {};
    }

    auto lexer::scan_variable () -> g10::result<void>
    {
        // - Get the starting position and column. For variables, the `$` is included.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Advance past the `$`.
        ++m_current_position;
        ++m_current_column;

        // - Scan while the current character is valid for variables.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (std::isalnum(ch) || ch == '_')
            {
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Emplace the variable token.
        m_tokens.emplace_back(
            token {
                .type = token_type::variable,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        return {};
    }

    auto lexer::scan_placeholder () -> g10::result<void>
    {
        // - Get the starting position and column. For placeholders, the `@` is included.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Advance past the `@`.
        ++m_current_position;
        ++m_current_column;

        // - Scan while the current character is valid for placeholders.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (std::isalnum(ch) || ch == '_')
            {
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Certain placeholders can also be reserved keywords; check for that.
        auto keyword_result = keyword_table::lookup_keyword(lexeme);
        if (keyword_result.has_value() == true)
        {
            // - It's a keyword placeholder.
            m_tokens.emplace_back(
                token {
                    .type = token_type::placeholder_keyword,
                    .lexeme = lexeme,
                    .source_file = m_source_file,
                    .source_line = m_current_line,
                    .source_column = start_column,
                    .keyword_value = keyword_result.value()
                }
            );
        }
        else
        {
            // - It's a placeholder.
            m_tokens.emplace_back(
                token {
                    .type = token_type::placeholder,
                    .lexeme = lexeme,
                    .source_file = m_source_file,
                    .source_line = m_current_line,
                    .source_column = start_column
                }
            );
        }

        return {};
    }

    auto lexer::scan_binary_integer_literal () -> g10::result<void>
    {
        // - Get the starting position and column. For binary literals, the prefix is
        //   included.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Advance past the `0b` or `0B` prefix.
        m_current_position += 2;
        m_current_column += 2;

        // - Scan while the current character is a binary digit.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (ch == '0' || ch == '1')
            {
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Ensure at least one digit was found.
        if (lexeme.size() <= 2)
        {
            return g10::error("Expected binary digits after '{}' prefix.", lexeme);
        }

        // - Emplace the binary integer literal token, parsing its value.
        auto& emplaced = m_tokens.emplace_back(
            token {
                .type = token_type::integer_literal,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        // - Parse the binary value. This should not fail, as we have already
        //   validated the digits.
        emplaced.int_value = std::stoll(
            std::string { lexeme.substr(2) },
            nullptr,
            2
        );
        emplaced.number_value =
            static_cast<double>(emplaced.int_value.value());

        return {};
    }

    auto lexer::scan_octal_integer_literal () -> g10::result<void>
    {
        // - Get the starting position and column. For octal literals, the prefix is
        //   included.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Advance past the `0o` or `0O` prefix.
        m_current_position += 2;
        m_current_column += 2;

        // - Scan while the current character is an octal digit.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (ch >= '0' && ch <= '7')
            {
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Ensure at least one digit was found.
        if (lexeme.size() <= 2)
        {
            return g10::error("Expected octal digits after '{}' prefix.", lexeme);
        }

        // - Emplace the octal integer literal token, parsing its value.
        auto& emplaced = m_tokens.emplace_back(
            token {
                .type = token_type::integer_literal,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        // - Parse the octal value. This should not fail, as we have already
        //   validated the digits.
        emplaced.int_value = std::stoll(
            std::string { lexeme.substr(2) },
            nullptr,
            8
        );
        emplaced.number_value =
            static_cast<double>(emplaced.int_value.value());

        return {};
    }

    auto lexer::scan_hexadecimal_integer_literal () -> g10::result<void>
    {
        // - Get the starting position and column. For hexadecimal literals, the prefix is
        //   included.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Advance past the `0x` or `0X` prefix.
        m_current_position += 2;
        m_current_column += 2;

        // - Scan while the current character is a hexadecimal digit.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (std::isxdigit(ch))
            {
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Ensure at least one digit was found.
        if (lexeme.size() <= 2)
        {
            return g10::error("Expected hexadecimal digits after '{}' prefix.", lexeme);
        }

        // - Emplace the hexadecimal integer literal token, parsing its value.
        auto& emplaced = m_tokens.emplace_back(
            token {
                .type = token_type::integer_literal,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        // - Parse the hexadecimal value. This should not fail, as we have already
        //   validated the digits.
        emplaced.int_value = std::stoll(
            std::string { lexeme.substr(2) },
            nullptr,
            16
        );
        emplaced.number_value =
            static_cast<double>(emplaced.int_value.value());

        return {};
    }

    auto lexer::scan_integer_or_number_literal () -> g10::result<void>
    {
        // - Before we begin, check for a base prefix.
        const char& ch1 = m_source_code[m_current_position];
        if (ch1 == '0' && m_current_position + 1 < m_source_code.size())
        {
            const char& ch2 = m_source_code[m_current_position + 1];
            if (ch2 == 'b' || ch2 == 'B')
            {
                return scan_binary_integer_literal();
            }
            else if (ch2 == 'o' || ch2 == 'O')
            {
                return scan_octal_integer_literal();
            }
            else if (ch2 == 'x' || ch2 == 'X')
            {
                return scan_hexadecimal_integer_literal();
            }
        }

        // - Get the starting position and column.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;

        // - Keep track of whether we have seen a decimal point.
        bool seen_decimal_point = false;

        // - Scan while the current character is a digit or decimal point.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (std::isdigit(ch))
            {
                ++m_current_position;
                ++m_current_column;
            }
            else if (ch == '.' && seen_decimal_point == false)
            {
                seen_decimal_point = true;
                ++m_current_position;
                ++m_current_column;
            }
            else
            {
                break;
            }
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position,
            m_current_position - start_position
        };

        // - Emplace the appropriate token.
        auto& emplaced = m_tokens.emplace_back(
            token {
                .type = (seen_decimal_point == true) ?
                    token_type::number_literal :
                    token_type::integer_literal,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        // - Parse the value.
        emplaced.int_value = std::stoll(
            std::string { lexeme },
            nullptr,
            10
        );
        emplaced.number_value = std::stod(
            std::string { lexeme }
        );

        return {};
    }

    auto lexer::scan_character_literal () -> g10::result<void>
    {
        // - Skip over the opening single quote. It is not part of the lexeme.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;
        ++m_current_position;
        ++m_current_column;

        // - Scan until the closing single quote is found.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (ch == '\'')
            {
                break;
            }
            else
            {
                ++m_current_position;
                ++m_current_column;
            }
        }

        // - If we reached the end of the source code without finding a closing
        //   quote, return an error.
        if (m_current_position >= m_source_code.size())
        {
            return g10::error("Unterminated character literal; expected closing quote.");
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position + 1,
            m_current_position - (start_position + 1)
        };

        // - Advance past the closing single quote.
        ++m_current_position;
        ++m_current_column;

        // Validate the character literal:
        // - If the lexeme is empty, assume a null character.
        // - If the lexeme has more than one character, and is not an escape
        //   sequence, return an error.
        // - Handle escape sequences if present.
        char character_value = '\0';
        if (lexeme.empty() == true)
        {
            character_value = '\0';
        }
        else if (lexeme.size() == 1)
        {
            character_value = lexeme[0];
        }
        else if (lexeme[0] == '\\')
        {
            // Handle escape sequences. The following escape sequences are supported:
            // - \n : Newline
            // - \t : Tab
            // - \r : Carriage Return
            // - \\ : Backslash
            // - \' : Single Quote
            // - \" : Double Quote
            // - \0 : Null Character
            // - \xHH : Hexadecimal character (where HH are hex digits)
            // - \uHHHH : Unicode character (where HHHH are hex digits)
            switch (lexeme[1])
            {
                case 'n': character_value = '\n'; break;
                case 't': character_value = '\t'; break;
                case 'r': character_value = '\r'; break;
                case '\\': character_value = '\\'; break;
                case '\'': character_value = '\''; break;
                case '"': character_value = '\"'; break;
                case '0': character_value = '\0'; break;
                case 'x':
                {
                    // Hexadecimal escape sequence.
                    if (
                        lexeme.size() != 4 ||
                        !std::isxdigit(lexeme[2]) ||
                        !std::isxdigit(lexeme[3])
                    )
                    {
                        return g10::error(
                            "Invalid hexadecimal escape sequence '\\x{}' in character literal.", 
                            lexeme.substr(2)
                        );
                    }

                    std::string hex_digits { lexeme.substr(2, 2) };
                    character_value = static_cast<char>(
                        std::stoi(hex_digits, nullptr, 16)
                    );
                } break;
                case 'u':
                {
                    // Unicode escape sequence.
                    if (
                        lexeme.size() != 6 ||
                        !std::isxdigit(lexeme[2]) ||
                        !std::isxdigit(lexeme[3]) ||
                        !std::isxdigit(lexeme[4]) ||
                        !std::isxdigit(lexeme[5])
                    )
                    {
                        return g10::error(
                            "Invalid Unicode escape sequence '\\u{}' in character literal.", 
                            lexeme.substr(2)
                        );
                    }

                    std::string hex_digits { lexeme.substr(2, 4) };
                    character_value = static_cast<char>(
                        std::stoi(hex_digits, nullptr, 16)
                    );
                } break;
                default:
                    return g10::error(
                        "Invalid escape sequence '\\{}' in character literal.", 
                        lexeme[1]
                    );
            }
        }
        else
        {
            return g10::error("Invalid character literal '{}'; expected a single character or escape sequence.", lexeme);
        }

        // - Emplace the appropriate token.
        auto& emplaced = m_tokens.emplace_back(
            token {
                .type = token_type::character_literal,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        // - Set the character value.
        emplaced.int_value = static_cast<std::int64_t>(character_value);
        emplaced.number_value = static_cast<double>(emplaced.int_value.value());

        return {};
    }

    auto lexer::scan_string_literal () -> g10::result<void>
    {
        // - Skip over the opening double quote. It is not part of the lexeme.
        std::size_t start_position = m_current_position;
        std::size_t start_column = m_current_column;
        ++m_current_position;
        ++m_current_column;

        // - Scan until the closing double quote is found.
        while (m_current_position < m_source_code.size())
        {
            const char& ch = m_source_code[m_current_position];
            if (ch == '"')
            {
                break;
            }
            else
            {
                ++m_current_position;
                ++m_current_column;
            }
        }

        // - If we reached the end of the source code without finding a closing
        //   quote, return an error.
        if (m_current_position >= m_source_code.size())
        {
            return g10::error("Unterminated string literal; expected closing quote.");
        }

        // - Extract the lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_position + 1,
            m_current_position - (start_position + 1)
        };

        // - Advance past the closing double quote.
        ++m_current_position;
        ++m_current_column;

        // - Emplace the string literal token.
        m_tokens.emplace_back(
            token {
                .type = token_type::string_literal,
                .lexeme = lexeme,
                .source_file = m_source_file,
                .source_line = m_current_line,
                .source_column = start_column
            }
        );

        return {};
    } 

    auto lexer::scan_symbol () -> g10::result<void>
    {
        // - Symbols can be anywhere between one and three characters long.
        const char ch1 = m_source_code[m_current_position],
            ch2 = (m_current_position + 1 < m_source_code.size()) ?
                m_source_code[m_current_position + 1] : '\0',
            ch3 = (m_current_position + 2 < m_source_code.size()) ?
                m_source_code[m_current_position + 2] : '\0';

        // - Helper macro to shorten token emplacing and position advancing.
        #define et(sym, lex, skip) \
            do { \
                m_tokens.emplace_back( \
                    token { \
                        .type = sym, \
                        .lexeme = lex, \
                        .source_file = m_source_file, \
                        .source_line = m_current_line, \
                        .source_column = m_current_column \
                    } \
                ); \
                m_current_position += skip; \
                m_current_column += skip; \
                return {}; \
            } while (0)

        switch (ch1)
        {
            case '+':
                if (ch2 == '=') { et(token_type::assign_plus, "+=", 2); }
                else            { et(token_type::plus, "+", 1); }
                break;
            case '-':
                if (ch2 == '=') { et(token_type::assign_minus, "-=", 2); }
                else            { et(token_type::minus, "-", 1); }
                break;
            case '*':
                if (ch2 == '*' && ch3 == '=')   { et(token_type::assign_exponent, "**=", 3); }
                else if (ch2 == '*')            { et(token_type::exponent, "**", 2); }
                else if (ch2 == '=')            { et(token_type::assign_times, "*=", 2); }
                else                            { et(token_type::times, "*", 1); }
                break;
            case '/':
                if (ch2 == '=') { et(token_type::assign_divide, "/=", 2); }
                else            { et(token_type::divide, "/", 1); }
                break;
            case '%':
                if (ch2 == '=') { et(token_type::assign_modulo, "%=", 2); }
                else            { et(token_type::modulo, "%", 1); }
                break;
            case '&':
                if (ch2 == '&')         { et(token_type::logical_and, "&&", 2); }
                else if (ch2 == '=')    { et(token_type::assign_and, "&=", 2); }
                else                    { et(token_type::bitwise_and, "&", 1); }
                break;
            case '|':
                if (ch2 == '|')         { et(token_type::logical_or, "||", 2); }
                else if (ch2 == '=')    { et(token_type::assign_or, "|=", 2); }
                else                    { et(token_type::bitwise_or, "|", 1); }
                break;
            case '^':
                if (ch2 == '=') { et(token_type::assign_xor, "^=", 2); }
                else            { et(token_type::bitwise_xor, "^", 1); }
                break;
            case '<':
                if (ch2 == '<' && ch3 == '=')   { et(token_type::assign_shift_left, "<<=", 3); }
                else if (ch2 == '<')            { et(token_type::bitwise_shift_left, "<<", 2); }
                else if (ch2 == '=')            { et(token_type::compare_less_equal, "<=", 2); }
                else                            { et(token_type::compare_less, "<", 1); }
                break;
            case '>':
                if (ch2 == '>' && ch3 == '=')   { et(token_type::assign_shift_right, ">>=", 3); }
                else if (ch2 == '>')            { et(token_type::bitwise_shift_right, ">>", 2); }
                else if (ch2 == '=')            { et(token_type::compare_greater_equal, ">=", 2); }
                else                            { et(token_type::compare_greater, ">", 1); }
                break;
            case '=':
                if (ch2 == '=') { et(token_type::compare_equal, "==", 2); }
                else            { et(token_type::assign_equal, "=", 1); }
                break;
            case '!':
                if (ch2 == '=') { et(token_type::compare_not_equal, "!=", 2); }
                else            { et(token_type::logical_not, "!", 1); }
                break;
            case '~': et(token_type::bitwise_not, "~", 1); break;
            case '(': et(token_type::left_parenthesis, "(", 1); break;
            case ')': et(token_type::right_parenthesis, ")", 1); break;
            case '[': et(token_type::left_bracket, "[", 1); break;
            case ']': et(token_type::right_bracket, "]", 1); break;
            case '{': et(token_type::left_brace, "{", 1); break;
            case '}': et(token_type::right_brace, "}", 1); break;
            case ',': et(token_type::comma, ",", 1); break;
            case ':': et(token_type::colon, ":", 1); break;
            case '?': et(token_type::question_mark, "?", 1); break;
            case '`': et(token_type::backtick, "`", 1); break;
            case '\\': et(token_type::backslash, "\\", 1); break;
            case '#':
                if (ch2 == '#') { et(token_type::double_hash, "##", 2); }
                else            { et(token_type::hash, "#", 1); }
                break;
            default:
                return g10::error("Unrecognized character: '{}'.", ch1);
        }

        #undef et
    }
}
