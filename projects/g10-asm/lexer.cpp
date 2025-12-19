/**
 * @file    g10-asm/lexer.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains definitions for the G10 assembler tool's lexical analyzer
 *          component.
 */

/* Private Includes ***********************************************************/

#include <g10-asm/keyword_table.hpp>
#include <g10-asm/lexer.hpp>

/* Private Static Members *****************************************************/

namespace g10asm
{
    std::unordered_map<fs::path, std::unique_ptr<lexer>> lexer::s_cache;
}

/* Public Methods *************************************************************/

namespace g10asm
{
    lexer::lexer (
        const std::string&  source_code,
        const fs::path&     source_path
    ) :
        m_source_code   { source_code },
        m_source_path   { fs::absolute(source_path).lexically_normal().string() }
    {
        tokenize();
    }

    auto lexer::from_string (const std::string& source_code)
        -> result_ref<lexer>
    {
        // - Create a new lexer instance from the provided source code string.
        auto lex = std::make_unique<lexer>(source_code);
        if (lex->is_good() == false)
        {
            return error("Could not lex provided source code string.");
        }

        // - Return the newly-created lexer instance. Lexers created directly
        //   from strings are not cached.
        return std::ref(*lex);
    }

    auto lexer::from_file (const fs::path& source_path)
        -> result_ref<lexer>
    {
        // - Determine the absolute, lexically normalized path of the source
        //   file. Make sure it hasn't already been lexed and cached.
        const auto absolute_path = fs::absolute(source_path).lexically_normal();
        if (auto it = s_cache.find(absolute_path); it != s_cache.end())
        {
            return std::ref(*(it->second));
        }

        // - Ensure the source file exists, is a regular file, and is readable.
        if (fs::exists(absolute_path) == false)
        {
            return error("Source file '{}' not found.", absolute_path.string());
        }
        else if (fs::is_regular_file(absolute_path) == false)
        {
            return error("Source path '{}' is not a regular file.",
                absolute_path.string());
        }

        // - Read the contents of the source file.
        std::fstream file { absolute_path, std::ios::in };
        if (file.is_open() == false)
        {
            return error("Failed to open source file '{}' for reading.",
                absolute_path.string());
        }

        // - Read the entire file contents into a string.
        std::string source_code {
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
        file.close();

        // - Create a new lexer instance from the source code string.
        auto lex = std::make_unique<lexer>(source_code, absolute_path);
        if (lex->is_good() == false)
        {
            return error("Could not lex source file '{}'.",
                absolute_path.string());
        }
        
        // - Cache and return the newly-created lexer instance.
        auto emplaced = s_cache.emplace(
            absolute_path,
            std::move(lex)
        );
        return std::ref(*(emplaced.first->second));
    }

    auto lexer::next_token () -> g10::result_ref<token>
    {
        if (m_token_index >= m_tokens.size())
        {
            return error("No more tokens available in the token stream.");
        }

        return std::ref(m_tokens[m_token_index++]);
    }

    auto lexer::next_token () const -> g10::result_cref<token>
    {
        if (m_token_index >= m_tokens.size())
        {
            return error("No more tokens available in the token stream.");
        }

        return std::cref(m_tokens[m_token_index++]);
    }

    auto lexer::peek_token (std::int64_t offset) -> g10::result_ref<token>
    {
        std::size_t peek_index = m_token_index + offset;
        if (peek_index >= m_tokens.size())
        {
            return error("Peek offset {} exceeds token stream bounds.", offset);
        }

        return std::ref(m_tokens[peek_index]);
    }

    auto lexer::peek_token (std::int64_t offset) const -> g10::result_cref<token>
    {
        std::size_t peek_index = m_token_index + offset;
        if (peek_index >= m_tokens.size())
        {
            return error("Peek offset {} exceeds token stream bounds.", offset);
        }

        return std::cref(m_tokens[peek_index]);
    }
}

/* Private Methods ************************************************************/

namespace g10asm
{
    auto lexer::tokenize () -> void
    {
        // - Tokenization state variables.
        bool in_comment = false;

        // - Main tokenization loop.
        while (m_char_index < m_source_code.size())
        {
            // - Skip whitespace characters and comments.
            skip_chars();

            // - If, after skipping, we've reached the end of the source code,
            //   break out of the loop.
            if (m_char_index >= m_source_code.size())
                { break; }

            // - Scan for the next token.
            result<void> scan_result;
            char c = current_char();
            if (std::isalpha(c) || c == '.' || c == '_')
                { scan_result = scan_identifier(); }
            else if (std::isdigit(c))
                { scan_result = scan_number(); }
            else if (c == '\'')
                { scan_result = scan_character_literal(); }
            else if (c == '"')
                { scan_result = scan_string_literal(); }
            else if (c == '$')
                { scan_result = scan_hexadecimal_number(true); }
            else if (c == '@')
                { scan_result = scan_placeholder(); }
            else
                { scan_result = scan_symbol(); }

            // - If an error occurred during scanning, stop tokenization.
            if (scan_result.has_value() == false)
            {
                std::println(stderr, "Lexer error in file '{}:{}':\n"
                    " - {}",
                    (m_source_path.empty() ? "<input>" : m_source_path),
                    m_line_number,
                    scan_result.error());
                return;
            }
        }

        // - If we reach this point, tokenization was successful.
        emplace_token("", token_type::end_of_file);
        m_good = true;
    }

    auto lexer::scan_identifier () -> result<void>
    {
        // - Start building the identifier lexeme.
        std::size_t start_index = m_char_index;

        // - Continue consuming characters while they are valid identifier
        //   characters (alphanumeric, underscore, period).
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (std::isalnum(c) || c == '_' || c == '.')
            {
                advance_char();
            }
            else
            {
                break;
            }
        }

        // - Extract the identifier lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };

        // - Check if the lexeme is a keyword.
        auto kw_result = keyword_table::lookup(lexeme);
        emplace_token(
            lexeme,
            (kw_result.has_value() == true) ?
                token_type::keyword :
                token_type::identifier,
            (kw_result.has_value() == true) ?
                kw_result.value().get().type :
                keyword_type::none
        );

        return {};
    }

    auto lexer::scan_number () -> result<void>
    {
        // - Get the current character. If it's a zero, this may be a base
        //   prefix.
        char c1 = current_char(), c2 = peek_char(1);
        if (c1 == '0')
        {
            if (c2 == 'b' || c2 == 'B')
            {
                advance_char(2);
                return scan_binary_number();
            }
            else if (c2 == 'o' || c2 == 'O')
            {
                advance_char(2);
                return scan_octal_number();
            }
            else if (c2 == 'x' || c2 == 'X')
            {
                advance_char(2);
                return scan_hexadecimal_number();
            }
        }

        // - Start building the number lexeme.
        std::size_t start_index = m_char_index;

        // - Keep track of whether we've seen a decimal point, indicating a
        //   fixed-point literal.
        bool has_decimal_point = false;

        // - Continue consuming characters while they are digits or a decimal
        //   point.
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (std::isdigit(c))
            {
                advance_char();
            }
            else if (c == '.' && has_decimal_point == false)
            {
                has_decimal_point = true;
                advance_char();
            }
            else
            {
                break;
            }
        }

        // - Extract the number lexeme.
        // - We will also need a hard string copy for parsing.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };
        std::string lexeme_str { lexeme };

        // - Emplace the appropriate token type based on whether it's an
        //   integer or fixed-point literal.
        auto& emplaced_token = emplace_token(
            lexeme,
            has_decimal_point ?
                token_type::fixed_point_literal :
                token_type::integer_literal
        );

        // - Parse and store the numeric value.
        emplaced_token.int_value = std::stoll(lexeme_str);
        emplaced_token.fixed_point_float = std::stod(lexeme_str);

        return {};
    }

    auto lexer::scan_binary_number () -> result<void>
    {
        // - Start building the binary number lexeme.
        std::size_t start_index = m_char_index;

        // - Continue consuming characters while they are valid binary digits.
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (c == '0' || c == '1')
            {
                advance_char();
            }
            else
            {
                break;
            }
        }

        // - Extract the binary number lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };

        // - We'll also need a hard string copy for parsing.
        std::string lexeme_str { lexeme };

        // - Emplace the integer literal token.
        auto& emplaced_token = emplace_token(
            lexeme,
            token_type::integer_literal
        );

        // - Parse and store the integer value.
        emplaced_token.int_value = std::stoll(lexeme_str, nullptr, 2);
        emplaced_token.fixed_point_float = 
            static_cast<double>(emplaced_token.int_value.value());

        return {};
    }

    auto lexer::scan_octal_number () -> result<void>
    {
        // - Start building the octal number lexeme.
        std::size_t start_index = m_char_index;

        // - Continue consuming characters while they are valid octal digits.
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (c >= '0' && c <= '7')
            {
                advance_char();
            }
            else
            {
                break;
            }
        }

        // - Extract the octal number lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };

        // - We'll also need a hard string copy for parsing.
        std::string lexeme_str { lexeme };

        // - Emplace the integer literal token.
        auto& emplaced_token = emplace_token(
            lexeme,
            token_type::integer_literal
        );

        // - Parse and store the integer value.
        emplaced_token.int_value = std::stoll(lexeme_str, nullptr, 8);
        emplaced_token.fixed_point_float = 
            static_cast<double>(emplaced_token.int_value.value());

        return {};
    }

    auto lexer::scan_hexadecimal_number (bool single_char_prefix) -> result<void>
    {
        // - If using a single-character prefix (`$`), we haven't skipped it yet.
        //   Do so now.
        if (single_char_prefix == true)
        {
            advance_char();
        }

        // - Start building the hexadecimal number lexeme.
        std::size_t start_index = m_char_index;

        // - Continue consuming characters while they are valid hexadecimal
        //   digits.
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (std::isxdigit(c))
            {
                advance_char();
            }
            else
            {
                break;
            }
        }

        // - Extract the hexadecimal number lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };

        // - We'll also need a hard string copy for parsing.
        std::string lexeme_str { lexeme };

        // - Emplace the integer literal token.
        auto& emplaced_token = emplace_token(
            lexeme,
            token_type::integer_literal
        );

        // - Parse and store the integer value.
        emplaced_token.int_value = std::stoll(lexeme_str, nullptr, 16);
        emplaced_token.fixed_point_float = 
            static_cast<double>(emplaced_token.int_value.value());

        return {};
    }

    auto lexer::scan_character_literal () -> result<void>
    {
        // - Skip the opening single quote.
        advance_char();

        // - Start building the character literal lexeme.
        std::size_t start_index = m_char_index;

        // - Indicate whether we've just encountered an escape character.
        bool found_escape = false;

        // - Continue consuming characters until we reach the closing single
        //   quote.
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (c == '\\' && found_escape == false)
            {
                found_escape = true;
                advance_char();
            }
            else if (c == '\'' && found_escape == false)
            {
                break;
            }
            else
            {
                found_escape = false;
                advance_char();
            }
        }

        // - If we reached the end of the source code without finding a closing
        //   single quote, return an error.
        if (m_char_index >= m_source_code.size())
            { return error("Unterminated character literal."); }

        // - Extract the character literal lexeme. Get a hard string copy for
        //   parsing especially for escape sequences.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };
        std::string lexeme_str { lexeme };

        // - Check for escape sequences and parse the character value.
        if (lexeme.starts_with('\\'))
        {
            char parsed_char = '\0';
            if (lexeme.size() == 2)
            {
                switch (lexeme_str[1])
                {
                    case 'n':   parsed_char = '\n'; break;
                    case 't':   parsed_char = '\t'; break;
                    case 'r':   parsed_char = '\r'; break;
                    case '\\':  parsed_char = '\\'; break;
                    case '\'':  parsed_char = '\''; break;
                    case '\"':  parsed_char = '\"'; break;
                    case '0':   parsed_char = '\0'; break;
                    default:
                        return error("Invalid escape sequence in character literal: '\\{}'.",
                            lexeme_str[1]);
                }
            }
            else if (lexeme.size() == 4 && lexeme_str[1] == 'x')
            {
                // - Hexadecimal escape sequence (e.g., '\x41').
                std::string hex_digits = lexeme_str.substr(2, 2);
                try
                {
                    parsed_char = static_cast<char>(std::stoi(hex_digits, nullptr, 16));
                }
                catch (const std::exception&)
                {
                    return error("Invalid hexadecimal escape sequence in character literal: '{}'.",
                        lexeme);
                }
            }
            else
            {
                return error("Invalid escape sequence in character literal: '{}'.",
                    lexeme);
            }

            // - Emplace the character literal token.
            auto& emplaced_token = emplace_token(
                lexeme,
                token_type::character_literal
            );

            // - Store the integer value of the character.
            emplaced_token.int_value = static_cast<std::int64_t>(parsed_char);
            emplaced_token.fixed_point_float = 
                static_cast<double>(emplaced_token.int_value.value());
        }
        else if (lexeme.size() == 1)
        {
            // - Emplace the character literal token.
            auto& emplaced_token = emplace_token(
                lexeme,
                token_type::character_literal
            );

            // - Store the integer value of the character.
            emplaced_token.int_value = static_cast<std::int64_t>(lexeme_str[0]);
            emplaced_token.fixed_point_float = 
                static_cast<double>(emplaced_token.int_value.value());
        }
        else
        {
            return error("Invalid character literal: '{}'.", lexeme);
        }
        
        // - Skip the closing single quote.
        advance_char();

        return {};
    }

    auto lexer::scan_string_literal () -> result<void>
    {
        // - Skip the opening double quote.
        advance_char();

        // - Start building the string literal lexeme.
        std::size_t start_index = m_char_index;

        // - Continue consuming characters until we reach the closing double
        //   quote.
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (c == '\"')
                { break; }
            else
                { advance_char(); }
        }

        // - If we reached the end of the source code without finding a closing
        //   double quote, return an error.
        if (m_char_index >= m_source_code.size())
            { return error("Unterminated string literal."); }

        // - Extract the string literal lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };

        // - Emplace the string literal token.
        emplace_token(
            lexeme,
            token_type::string_literal
        );

        // - Skip the closing double quote.
        advance_char();

        return {};
    }

    auto lexer::scan_placeholder () -> result<void>
    {
        // - Skip the opening '@' character.
        advance_char();

        // - Start building the placeholder lexeme.
        std::size_t start_index = m_char_index;

        // - Continue consuming characters while they are valid identifier
        //   characters (alphanumeric, underscore, period).
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();
            if (std::isalnum(c) || c == '_' || c == '.')
            {
                advance_char();
            }
            else
            {
                break;
            }
        }

        // - Extract the placeholder lexeme.
        std::string_view lexeme {
            m_source_code.data() + start_index,
            m_char_index - start_index
        };

        // - Emplace the placeholder token.
        auto& emplaced_token = emplace_token(
            lexeme,
            token_type::placeholder
        );

        // - If the lexeme contains only decimal digits (`0-9`), then this
        //   indicates a positional parameter placeholder. Parse and store its
        //   index.
        bool is_positional = true;
        for (char c : lexeme)
        {
            if (std::isdigit(c) == false)
            {
                is_positional = false;
                break;
            }
        }

        if (is_positional == true)
        {
            emplaced_token.int_value = std::stoll(std::string { lexeme });
        }

        return {};
    }

    auto lexer::scan_symbol () -> result<void>
    {
        // - Get the current character.
        char c1 = current_char(), c2 = peek_char(1), c3 = peek_char(2);

        // - Handle single-character symbols.
        switch (c1)
        {
            case '+':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::assign_plus); }
                else            { advance_char(); emplace_token("", token_type::plus); }
                break;
            case '-':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::assign_minus); }
                else            { advance_char(); emplace_token("", token_type::minus); }
                break;
            case '*':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::assign_times); }
                else            { advance_char(); emplace_token("", token_type::times); }
                break;
            case '/':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::assign_divide); }
                else            { advance_char(); emplace_token("", token_type::divide); }
                break;
            case '%':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::assign_modulo); }
                else            { advance_char(); emplace_token("", token_type::modulo); }
                break;
            case '&':
                if (c2 == '&')      { advance_char(2); emplace_token("", token_type::logical_and); }
                else if (c2 == '=') { advance_char(2); emplace_token("", token_type::assign_and); }
                else                { advance_char(); emplace_token("", token_type::bitwise_and); }
                break;
            case '|':
                if (c2 == '|')      { advance_char(2); emplace_token("", token_type::logical_or); }
                else if (c2 == '=') { advance_char(2); emplace_token("", token_type::assign_or); }
                else                { advance_char(); emplace_token("", token_type::bitwise_or); }
                break;
            case '^':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::assign_xor); }
                else            { advance_char(); emplace_token("", token_type::bitwise_xor); }
                break;
            case '~':
                advance_char();
                emplace_token("~", token_type::bitwise_not);
                break;
            case '<':
                if (c2 == '<' && c3 == '=') { advance_char(3); emplace_token("", token_type::assign_left_shift); }
                else if (c2 == '<')         { advance_char(2); emplace_token("", token_type::left_shift); }
                else if (c2 == '=')         { advance_char(2); emplace_token("", token_type::compare_less_equal); }
                else                        { advance_char(); emplace_token("", token_type::compare_less_than); }
                break;
            case '>':
                if (c2 == '>' && c3 == '=') { advance_char(3); emplace_token("", token_type::assign_right_shift); }
                else if (c2 == '>')         { advance_char(2); emplace_token("", token_type::right_shift); }
                else if (c2 == '=')         { advance_char(2); emplace_token("", token_type::compare_greater_equal); }
                else                        { advance_char(); emplace_token("", token_type::compare_greater_than); }
                break;
            case '=':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::compare_equal); }
                else            { advance_char(); emplace_token("", token_type::assign_equal); }
                break;
            case '!':
                if (c2 == '=')  { advance_char(2); emplace_token("", token_type::compare_not_equal); }
                else            { advance_char(); emplace_token("", token_type::logical_not); }
                break;
            case '(':
                advance_char();
                emplace_token("", token_type::left_parenthesis);
                break;
            case ')':
                advance_char();
                emplace_token("", token_type::right_parenthesis);
                break;
            case '{':
                advance_char();
                emplace_token("", token_type::left_brace);
                break;
            case '}':
                advance_char();
                emplace_token("", token_type::right_brace);
                break;
            case '[':
                advance_char();
                emplace_token("", token_type::left_bracket);
                break;
            case ']':
                advance_char();
                emplace_token("", token_type::right_bracket);
                break;
            case ',':
                advance_char();
                emplace_token("", token_type::comma);
                break;
            case ':':
                advance_char();
                emplace_token("", token_type::colon);
                break;
            default:
                return error("Unrecognized character: '{}'.", c1);
        }

        return {};
    }

    auto lexer::skip_chars () -> void
    {
        while (m_char_index < m_source_code.size())
        {
            char c = current_char();

            // - Handle whitespace characters.
            if (std::isspace(static_cast<unsigned char>(c)))
            {
                if (c == '\n')
                {
                    // - Newline encountered; emplace a newline token, then
                    //   increment the line number.
                    emplace_token("", token_type::end_of_line);
                    m_line_number++;
                }

                advance_char();
            }
            // - Handle comments (starting with ';' and continuing to end of line).
            else if (c == ';')
            {
                // - Skip to the end of the line.
                while (
                    m_char_index < m_source_code.size() &&
                    current_char() != '\n'
                )
                {
                    advance_char();
                }

                // - If we stopped at a newline, it will be handled in the next
                //   iteration of the outer loop.
            }
            else
            {
                // - Non-whitespace, non-comment character found; stop skipping.
                break;
            }
        }
    }

    auto lexer::current_char () const -> char
    {
        return (m_char_index < m_source_code.size()) ?
            m_source_code[m_char_index] : 
            '\0';
    }

    auto lexer::peek_char (std::int64_t offset) const -> char
    {
        const auto index = m_char_index + offset;
        return (index < m_source_code.size()) ?
            m_source_code[index] :
            '\0';
    }

    auto lexer::advance_char (std::size_t count) -> char
    {
        m_char_index = (m_char_index + count <= m_source_code.size()) ?
            m_char_index + count :
            m_source_code.size();

        return current_char();
    }

    auto lexer::match_char (char expected, std::int64_t count) -> bool
    {
        return peek_char(count) == expected;
    }

    auto lexer::emplace_token (std::string_view lexeme, token_type tk_type,
        keyword_type kw_type) -> token&
    {
        return m_tokens.emplace_back(
            token {
                .lexeme         = lexeme,
                .type           = tk_type,
                .kw_type        = kw_type,
                .source_file    = m_source_path,
                .line_number    = m_line_number
            }
        );
    }
}
