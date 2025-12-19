/**
 * @file    g10-asm/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains the primary entry point for the G10 CPU's assembler tool.
 */

/* Private Includes ***********************************************************/

#include <g10-asm/lexer.hpp>

/* Private Variables **********************************************************/

namespace g10asm
{
    static constexpr const char* PROGRAM_VERSION = "0.1.0";

    static bool s_help = false;
    static bool s_version = false;
    static bool s_list_tokens = false;
    static std::string s_source_file = "";
}

/* Private Functions **********************************************************/

namespace g10asm
{
    static auto parse_arguments (int argc, const char** argv) -> bool
    {
        // Arguments:
        // `-s <file>`, `--source <file>`:
        //      Required. Specifies the path to the assembly source file to be
        //      assembled.
        // `-l`, `--list-tokens`:
        //      If provided, the tool will print the list of tokens extracted
        //      from the provided source file after lexing. Only the tokens from
        //      this file will be printed; no included files are lexed or printed.
        // `-h`, `--help`:
        //      Displays help/usage information.
        // `-v`, `--version`:
        //      Displays version information.
    
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if ((arg == "-s") || (arg == "--source"))
            {
                if ((i + 1) < argc)
                {
                    s_source_file = argv[i + 1];
                    ++i;
                }
                else
                {
                    std::println(stderr, "Error: Missing source file path.");
                    return false;
                }
            }
            else if ((arg == "-l") || (arg == "--list-tokens"))
            {
                s_list_tokens = true;
            }
            else if ((arg == "-h") || (arg == "--help"))
            {
                s_help = true;
            }
            else if ((arg == "-v") || (arg == "--version"))
            {
                s_version = true;
            }
            else
            {
                std::println(stderr, "Error: Unrecognized argument '{}'.", arg);
                return false;
            }
        }

        // Validate `-s` / `--source` argument.
        if (s_source_file.empty())
        {
            std::println(stderr, "Error: No source file specified.");
            return false;
        }

        return true;
    }

    static auto display_version () -> void
    {
        std::println("G10 Assembler Tool - Version {}", PROGRAM_VERSION);
        std::println("By: Dennis W. Griffin <dgdev1024@gmail.com>\n");
    }

    static auto display_help () -> void
    {
        std::println("Usage: g10-asm [options]");
        std::println("Options:");
        std::println("  -s, --source <file>    Required. Specifies the path to the assembly source file to be assembled.");
        std::println("  -l, --list-tokens      Lists the tokens extracted from the source file after lexing.");
        std::println("  -h, --help             Displays this help/usage information.");
        std::println("  -v, --version          Displays version information.\n");
    }

    static auto list_tokens (const lexer& lex) -> void
    {

        const auto& tokens = lex.get_tokens();
        for (const auto& tk : tokens)
        {
            if (tk.lexeme.empty() == false)
            {
                if (tk.type == token_type::string_literal)
                {
                    std::println("Line {:<4} | Type: {:<22} | Lexeme: \"{}\"",
                        tk.line_number,
                        tk.stringify_type(),
                        tk.lexeme);
                }
                else if (
                    tk.type == token_type::integer_literal ||
                    tk.type == token_type::fixed_point_literal ||
                    tk.type == token_type::character_literal
                )
                {
                    std::println("Line {:<4} | Type: {:<22} | Lexeme: '{}' ({})",
                        tk.line_number,
                        tk.stringify_type(),
                        tk.lexeme,
                        (tk.type != token_type::fixed_point_literal) ?
                            std::to_string(tk.int_value.value()) :
                            std::to_string(tk.fixed_point_float.value())
                    );
                }
                else
                {
                    std::println("Line {:<4} | Type: {:<22} | Lexeme: {}",
                        tk.line_number,
                        tk.stringify_type(),
                        tk.lexeme);
                }
            }
            else
            {
                std::println("Line {:<4} | Type: {:<22}",
                    tk.line_number,
                    tk.stringify_type());
            }
        }
    }
}

/* Main Function **************************************************************/

auto main (int argc, const char** argv) -> int
{
    if (g10asm::parse_arguments(argc, argv) == false)
    {
        return 1;
    }

    if (g10asm::s_help == true)
    {
        g10asm::display_version();
        g10asm::display_help();
        return 0;
    }
    else if (g10asm::s_version == true)
    {
        g10asm::display_version();
        return 0;
    }

    // - Lex the specified source file.
    auto lex_result = g10asm::lexer::from_file(g10asm::s_source_file);
    if (lex_result.has_value() == false)
    {
        std::println(stderr, "{}", lex_result.error());
        return 1;
    }

    // - If requested, print the list of lexed tokens.
    if (g10asm::s_list_tokens == true)
    {
        g10asm::list_tokens(lex_result.value().get());
        return 0;
    }

    return 0;
}
