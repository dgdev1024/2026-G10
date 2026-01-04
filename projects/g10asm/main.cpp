/**
 * @file    g10asm/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-01
 * 
 * @brief   Contains the primary entry point for the G10 CPU assembler tool.
 */

/* Private Includes ***********************************************************/

#include <g10asm/lexer.hpp>
#include <g10asm/preprocessor.hpp>

/* Private Static Variables - Command-Line Arguments **************************/

namespace g10asm
{
    // Usage: `g10asm [options] -s <source file> -o <output file>`
    //
    // Options:
    //     -s <file>, --source <file>       Specify the source file to assemble. Required.
    //     -o <file>, --output <file>       Specify the output file for the assembled machine code.
    //                                      Required unless `--lex`, `--preprocess`, or `--parse` is used,
    //                                      in which case this is ignored.
    //     -i <dir>, --include <dir>        Specify an additional include directory for the preprocessor.
    //                                      Can be specified multiple times.
    //     --lex                            Perform initial lexical analysis only.
    //     --preprocess                     Perform preprocessing and postprocessing lexical analysis only.
    //                                      Ignored if `--lex` is also specified.
    //     --parse                          Perform AST parsing only.
    //                                      Ignored if `--lex` or `--preprocess` is also specified.
    //     --max-recursion-depth <n>        Set the maximum macro recursion depth (default, minimum: 256).
    //     --max-include-depth <n>          Set the maximum include depth (default, minimum: 64).
    //     -h, --help                       Show this help message and exit
    //     -v, --version                    Show version information and exit

    static std::string                  s_source_file   = "";
    static std::string                  s_output_file   = "";   
    static std::vector<std::string>     s_include_dirs  = {};
    static bool                         s_lex           = false;
    static bool                         s_preprocess    = false;
    static bool                         s_parse         = false;
    static std::size_t                  s_max_recursion_depth = g10asm::DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH;
    static std::size_t                  s_max_include_depth   = g10asm::DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH;
    static bool                         s_help          = false;
    static bool                         s_version       = false;
}

/* Private Functions **********************************************************/

namespace g10asm
{
    static auto parse_arguments (int argc, const char** argv) -> bool
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg = argv[i];

            if (arg == "-s" || arg == "--source")
            {
                if (i + 1 >= argc)
                {
                    std::println(stderr, 
                        "Error: Missing source file after '{}'\n", 
                        arg);
                    return false;
                }

                s_source_file = argv[++i];
            }
            else if (arg == "-o" || arg == "--output")
            {
                if (i + 1 >= argc)
                {
                    std::println(stderr, 
                        "Error: Missing output file after '{}'\n", 
                        arg);
                    return false;
                }

                s_output_file = argv[++i];
            }
            else if (arg == "-i" || arg == "--include")
            {
                if (i + 1 >= argc)
                {
                    std::println(stderr, 
                        "Error: Missing include directory after '{}'\n", 
                        arg);
                    return false;
                }

                s_include_dirs.emplace_back(argv[++i]);
            }
            else if (arg == "--preprocess")
            {
                s_preprocess = true;
            }
            else if (arg == "--parse")
            {
                s_parse = true;
            }
            else if (arg == "--lex")
            {
                s_lex = true;
            }
            else if (arg == "--max-recursion-depth")
            {
                if (i + 1 >= argc)
                {
                    std::println(stderr, 
                        "Error: Missing value after '{}'\n", 
                        arg);
                    return false;
                }

                try
                    { s_max_recursion_depth = std::stoul(argv[++i]); }
                catch (...)
                {
                    std::println(stderr,
                        "Error: Invalid value for '{}': '{}'\n",
                        arg,
                        argv[i]
                    );
                    return false;
                }
            }
            else if (arg == "--max-include-depth")
            {
                if (i + 1 >= argc)
                {
                    std::println(stderr, 
                        "Error: Missing value after '{}'\n", 
                        arg);
                    return false;
                }

                try
                    { s_max_include_depth = std::stoul(argv[++i]); }
                catch (...)
                {
                    std::println(stderr,
                        "Error: Invalid value for '{}': '{}'\n",
                        arg,
                        argv[i]
                    );
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
            else
            {
                std::println(stderr, "Unknown argument: '{}'\n", arg);
                return false;
            }
        }

        // - If `-h`/`--help` or `-v`/`--version` is specified, no further
        //   validation is necessary.
        if (
            s_help      == true ||
            s_version   == true
        )
        {
            return true;
        }

        // - Validate required arguments.
        if (s_source_file.empty() == true)
        {
            std::println(stderr,
                "Error: Source file not specified. "
                "Use '-s <file>' or '--source <file>' to specify a source file.\n"
            );
            return false;
        }

        // - If not performing lexing, preprocessing, or parsing only,
        //   the output file must be specified.
        if (
            s_lex                   == false &&
            s_preprocess            == false &&
            s_parse                 == false &&
            s_output_file.empty()   == true
        )
        {
            std::println(stderr,
                "Error: Output file not specified. "
                "Use '-o <file>' or '--output <file>' to specify an output file.\n"
            );
            return false;
        }

        // - Validate maximum recursion and include depths.
        if (s_lex == false && s_max_recursion_depth < g10asm::DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH)
        {
            std::println(stderr,
                "Error: Maximum recursion depth must be at least {}.\n",
                g10asm::DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH
            );
            return false;
        }
        else if (s_lex == false && s_max_include_depth < g10asm::DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH)
        {
            std::println(stderr,
                "Error: Maximum include depth must be at least {}.\n",
                g10asm::DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH
            );
            return false;
        }

        return true;
    }

    static auto display_version () -> void
    {
        std::println(
            "'g10asm' - G10 CPU Assembler Tool - Version {}\n"
            "By: Dennis W. Griffin <dgdev1024@gmail.com>\n",
            g10::VERSION_STRING
        );
    }

    static auto display_help (const char* arg0) -> void
    {
        std::println(
            "Usage: {} [options]\n\n"
            "Options:\n"
            "    -s <file>, --source <file>       Specify the source file to assemble. Required.\n"
            "    -o <file>, --output <file>       Specify the output file for the assembled machine code.\n"
            "                                     Required unless `--lex`, `--preprocess`, or `--parse` is used,\n"
            "                                     in which case this is ignored.\n"
            "    -i <dir>, --include <dir>        Specify an additional include directory for the preprocessor.\n"
            "                                     Can be specified multiple times.\n"
            "    --lex                            Perform initial lexical analysis only.\n"
            "    --preprocess                     Perform preprocessing and postprocessing lexical analysis only.\n"
            "                                     Ignored if `--lex` is also specified.\n"
            "    --parse                          Perform AST parsing only.\n"
            "                                     Ignored if `--lex` or `--preprocess` is also specified.\n"
            "    --max-recursion-depth <n>        Set the maximum macro recursion depth (default, minimum: 256).\n"
            "    --max-include-depth <n>          Set the maximum include depth (default, minimum: 64).\n"
            "    -h, --help                       Show this help message and exit\n"
            "    -v, --version                    Show version information and exit\n",

            arg0
        );
    }

    static auto print_lexer (const lexer& lex) -> void
    {
        const auto& tokens = lex.get_tokens();
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            const auto& token = tokens[i];
            std::println("{:08} | {}", i, token.to_string());
        }
    }
}

/* Main Function **************************************************************/

auto main (int argc, const char** argv) -> int
{
    // - Attempt to parse command-line arguments.
    if (g10asm::parse_arguments(argc, argv) == false)
    {
        g10asm::display_help(argv[0]);
        return 1;
    }

    // - Handle `-h`/`--help` and `-v`/`--version` options.
    if (g10asm::s_help == true)
    {
        g10asm::display_version();
        g10asm::display_help(argv[0]);
        return 0;
    }
    else if (g10asm::s_version == true)
    {
        g10asm::display_version();
        return 0;
    }

    // - Perform initial lexical analysis.
    g10asm::lexer lexer;
    auto lex_result = lexer.load_from_file(g10asm::s_source_file);
    if (lex_result.has_value() == false)
    {
        std::println(stderr,
            "Errors occured during initial lexical analysis."
        );
        return 1;
    }
    else if (g10asm::s_lex == true && g10asm::s_preprocess == false)
    {
        // Only --lex specified: print initial lexer output
        g10asm::print_lexer(lexer);
        return 0;
    }

    // - Configure and initialize the preprocessor.
    g10asm::preprocessor_config pp_config {
        .max_recursion_depth    = g10asm::s_max_recursion_depth,
        .max_include_depth      = g10asm::s_max_include_depth,
        .include_dirs           = g10asm::s_include_dirs
    };

    g10asm::preprocessor preprocessor { pp_config, lexer };
    if (preprocessor.is_good() == false)
    {
        std::println(stderr,
            "Errors occured during preprocessing."
        );
        return 1;
    }

    // - If only --preprocess is specified (without --lex), print the raw
    //   preprocessed string output.
    if (g10asm::s_preprocess == true && g10asm::s_lex == false)
    {
        std::println("{}", preprocessor.get_output());
        return 0;
    }

    // - Run lexical analysis on the string output by the preprocessor.
    lexer.clear();
    auto postprocess_lex_result = lexer.load_from_string(
        preprocessor.get_output()
    );
    if (postprocess_lex_result.has_value() == false)
    {
        std::println(stderr,
            "Errors occured during postprocessing lexical analysis."
        );
        return 1;
    }
    else if (g10asm::s_preprocess == true && g10asm::s_lex == true)
    {
        g10asm::print_lexer(lexer);
        return 0;
    }

    /*

    // - Perform AST parsing on the postprocessed tokens.
    g10asm::parser parser { lexer };
    if (parser.is_good() == false)
    {
        std::println(stderr,
            "Errors occured during AST parsing."
        );
        return 1;
    }
    else if (g10asm::s_parse == true)
    {
        g10asm::print_parser(parser);
        return 0;
    }

    // - Perform code generation on the parsed AST.
    g10asm::code_generator codegen { parser };
    if (codegen.is_good() == false)
    {
        std::println(stderr,
            "Errors occured during code generation."
        );
        return 1;
    }

    // - Write the generated machine code to the output file.
    if (codegen.save_to_file(g10asm::s_output_file) == false)
    {
        std::println(stderr,
            "Errors occured while writing machine code to output file."
        );
        return 1;
    }
    
    */

    return 0;
}
