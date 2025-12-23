/**
 * @file    g10asm/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains the primary entry point for the G10 CPU Assembler Tool.
 */

/* Private Includes ***********************************************************/

#include <g10/common.hpp>
#include <g10asm/lexer.hpp>
#include <g10asm/parser.hpp>

/* Private Static Variables ***************************************************/

namespace g10asm
{
    // Command-Line Arguments
    static std::string s_source_file = "";  // `-s <file>`, `--source <file>` - Required: Source file to assemble
    static std::string s_output_file = "";  // `-o <file>`, `--output <file>` - Required: Output file name
    static bool s_help = false;             // `-h`, `--help` - Show help message
    static bool s_version = false;          // `-v`, `--version` - Show version info
    static bool s_verbose = false;          // `--verbose` - Enable verbose output
    static bool s_lex_only = false;         // `--lex-only` - Only perform lexical analysis on this file
    static bool s_parse_only = false;       // `--parse-only` - Only perform parsing on this file (and included files), and output the AST
}

/* Private Functions **********************************************************/

namespace g10asm
{
    static auto parse_arguments (int argc, const char** argv) -> bool
    {
        // - Iterate through command-line arguments and parse them.
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "-s" || arg == "--source")
            {
                if (i + 1 < argc)
                {
                    s_source_file = argv[++i];
                }
                else
                {
                    std::println(stderr, "Error: Missing source file after '{}'.", arg);
                    return false;
                }
            }
            else if (arg == "-o" || arg == "--output")
            {
                if (i + 1 < argc)
                {
                    s_output_file = argv[++i];
                }
                else
                {
                    std::println(stderr, "Error: Missing output file after '{}'.", arg);
                    return false;
                }
            }
            else if (arg == "-h" || arg == "--help")
            {
                s_help = true;
            }
            else if (arg == "-v" || arg == "--version")
            {
                s_version = true;
            }
            else if (arg == "--verbose")
            {
                s_verbose = true;
            }
            else if (arg == "--lex-only")
            {
                s_lex_only = true;
            }
            else if (arg == "--parse-only")
            {
                s_parse_only = true;
            }
            else
            {
                std::println(stderr, "Error: Unknown argument '{}'.", arg);
                return false;
            }
        }

        // - Check for `--help` or `--version` flags.
        if (s_help == true || s_version == true)
        {
            return true;
        }

        // - Validate required arguments.
        if (s_source_file.empty() == true)
        {
            std::println(stderr, 
                "Error: Source file is required. Use '-s <file>' or '--source <file>'.");
            return false;
        }
        else if (
            s_output_file.empty() == true &&
            s_lex_only == false &&
            s_parse_only == false
        )
        {
            std::println(stderr, 
                "Error: Output file is required. Use '-o <file>' or '--output <file>'.");
            return false;
        }

        return true;
    }

    static auto show_version () -> void
    {
        std::println(
            "'g10-asm' - G10 CPU Assembler Tool\n"
            "By: Dennis W. Griffin <dgdev1024@gmail.com>\n"
        );
    }

    static auto show_help () -> void
    {
        std::println(
            "Usage: g10-asm [options]\n\n"
            "Options:\n"
            "  -s, --source <file>     Specify the source file to assemble (required).\n"
            "  -o, --output <file>     Specify the output file name (required).\n"
            "  -h, --help              Show this help message and exit.\n"
            "  -v, --version           Show version information and exit.\n"
            "      --verbose           Enable verbose output during assembly.\n"
            "      --lex-only          Only perform lexical analysis on the source file and display the tokens.\n"
            "      --parse-only        Only perform parsing on the source file and display the AST.\n"
            "                          Ignored if '--lex-only' is also specified.\n"
        );
    }

    static auto show_lexer_output (const lexer& lex) -> void
    {   
        const auto& tokens = lex.get_tokens();
        
        std::println("Lexer output for file '{}':", s_source_file);
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            const auto& tok = tokens[i];
            std::println("{:04} | {}", i + 1, tok.to_string());
        }
    }

    static auto show_ast_output (const ast_module& ast_root) -> void
    {
        std::println("AST output for file '{}':", s_source_file);
        std::println("{}", g10asm::ast_to_string(ast_root));
    }
}

/* Main Function **************************************************************/

auto main (int argc, const char** argv) -> int
{
    // - Parse command-line arguments.
    if (g10asm::parse_arguments(argc, argv) == false)
    {
        return 1;
    }

    // - Handle `--help` and `--version` flags.
    if (g10asm::s_help == true)
    {
        g10asm::show_version();
        g10asm::show_help();
        return 0;
    }
    else if (g10asm::s_version == true)
    {
        g10asm::show_version();
        return 0;
    }

    // - Create a lexer for the source file.
    auto lex_result = g10asm::lexer::from_file(g10asm::s_source_file);
    if (lex_result.has_value() == false)
    {
        return 1;
    }

    // - Get the lexer. If `--lex-only` is specified, show the lexer output and
    //   exit early.
    auto& lex = lex_result.value().get();
    if (g10asm::s_lex_only == true)
    {
        g10asm::show_lexer_output(lex);
        return 0;
    }

    // - Parse the source file into an AST.
    auto parse_result = g10asm::parser::parse(lex);
    if (parse_result.has_value() == false)
    {
        return 1;
    }
    const auto& ast_root = parse_result.value();

    // - If `--parse-only` is specified, show the AST output and exit early.
    if (g10asm::s_parse_only == true)
    {
        g10asm::show_ast_output(ast_root);
        return 0;
    }

    return 0;
}
