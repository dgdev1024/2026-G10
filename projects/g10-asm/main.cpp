/**
 * @file    g10-asm/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains the primary entry point for the G10 CPU's assembler tool.
 */

/* Private Includes ***********************************************************/

#include <g10-asm/lexer.hpp>
#include <g10-asm/parser.hpp>
#include <g10-asm/codegen.hpp>
#include <g10-asm/object_writer.hpp>

/* Private Variables **********************************************************/

namespace g10asm
{
    static constexpr const char* PROGRAM_VERSION = "0.1.0";

    static bool s_help = false;
    static bool s_version = false;
    static bool s_list_tokens = false;
    static bool s_parse_only = false;
    static bool s_verbose = false;
    static std::string s_source_file = "";
    static std::string s_output_file = "";
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
        // `-o <file>`, `--output <file>`:
        //      Required. Specifies the output object file path.
        // `-l`, `--list-tokens`:
        //      If provided, the tool will print the list of tokens extracted
        //      from the provided source file after lexing. Only the tokens from
        //      this file will be printed; no included files are lexed or printed.
        // `-p`, `--parse`:
        //      If provided, the tool will parse the source file and print
        //      the parsed AST structure.
        // `-V`, `--verbose`:
        //      Enables verbose output during assembly.
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
            else if ((arg == "-o") || (arg == "--output"))
            {
                if ((i + 1) < argc)
                {
                    s_output_file = argv[i + 1];
                    ++i;
                }
                else
                {
                    std::println(stderr, "Error: Missing output file path.");
                    return false;
                }
            }
            else if ((arg == "-l") || (arg == "--list-tokens"))
            {
                s_list_tokens = true;
            }
            else if ((arg == "-p") || (arg == "--parse"))
            {
                s_parse_only = true;
            }
            else if ((arg == "-V") || (arg == "--verbose"))
            {
                s_verbose = true;
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

        // Validate `-o` / `--output` argument.
        if (s_output_file.empty())
        {
            std::println(stderr, "Error: No output file specified.");
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
        std::println("  -o, --output <file>    Required. Specifies the output object file path.");
        std::println("  -l, --list-tokens      Lists the tokens extracted from the source file after lexing.");
        std::println("  -p, --parse            Parses the source file and displays the AST structure.");
        std::println("  -V, --verbose          Enables verbose output during assembly.");
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

    static auto print_parse_tree (const program& prog) -> void
    {
        std::println("Statements Parsed: {}.", prog.statements.size());
        std::println("Labels: {}", prog.label_table.size());
        std::println("Global symbols: {}", prog.global_symbols.size());
        std::println("Extern symbols: {}", prog.extern_symbols.size());
        
        std::println("\nStatements:");
        for (std::size_t i = 0; i < prog.statements.size(); ++i)
        {
            const auto& stmt = prog.statements[i];
            std::print("  [{}] Line {}: ", i, stmt.line_number);
            
            switch (stmt.type)
            {
                case g10asm::statement_type::label_definition:
                    std::println("Label '{}'", stmt.label_name);
                    break;
                case g10asm::statement_type::instruction:
                    std::println("Instruction '{}' with {} operand(s)",
                        stmt.source_token.lexeme, stmt.operands.size());
                    break;
                case g10asm::statement_type::directive_org:
                    std::println(".ORG 0x{:X}", stmt.org_address);
                    break;
                case g10asm::statement_type::directive_byte:
                    std::println(".BYTE with {} value(s)", stmt.data_values.size());
                    break;
                case g10asm::statement_type::directive_word:
                    std::println(".WORD with {} value(s)", stmt.data_values.size());
                    break;
                case g10asm::statement_type::directive_dword:
                    std::println(".DWORD with {} value(s)", stmt.data_values.size());
                    break;
                case g10asm::statement_type::directive_global:
                    std::println(".GLOBAL with {} symbol(s)", stmt.symbol_names.size());
                    break;
                case g10asm::statement_type::directive_extern:
                    std::println(".EXTERN with {} symbol(s)", stmt.symbol_names.size());
                    break;
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

    // - Parse the source file.
    auto& lex = lex_result.value().get();
    g10asm::parser parse { lex };
    auto parse_result = parse.parse_program();
    if (parse_result.has_value() == false)
    {
        std::println(stderr, "{}", parse_result.error());
        return 1;
    }

    // - If requested, display parse results.
    if (g10asm::s_parse_only == true)
    {
        const auto& prog = parse_result.value().get();
        g10asm::print_parse_tree(prog);
        return 0;
    }

    // - Generate code from the parsed program.
    const auto& prog = parse_result.value().get();
    g10asm::codegen codegen_instance { prog, g10asm::s_source_file };
    auto codegen_result = codegen_instance.generate();
    if (codegen_result.has_value() == false)
    {
        std::println(stderr, "Code generation error: {}", codegen_result.error());
        return 1;
    }

    const auto& obj_file = codegen_result.value();
    if (g10asm::s_verbose == true)
    {
        std::println("Code generation successful!");
        std::println("  Sections: {}", obj_file.sections.size());
        std::println("  Symbols: {}", obj_file.symbols.size());
        std::println("  Relocations: {}", obj_file.relocations.size());
        std::println("  Code size: {} bytes", obj_file.total_code_size());
    }

    // - Write the object file
    g10asm::object_writer writer { obj_file };
    auto write_result = writer.write(g10asm::s_output_file);
    if (!write_result.has_value())
    {
        std::println(stderr, "Error writing object file: {}", write_result.error());
        return 1;
    }

    // - Display success message with file size
    if (g10asm::s_verbose == true)
    {
        std::println("Wrote object file: {}", g10asm::s_output_file);
        if (fs::exists(g10asm::s_output_file))
        {
            std::println("  File size: {} bytes", fs::file_size(g10asm::s_output_file));
        }
    }

    return 0;
}
