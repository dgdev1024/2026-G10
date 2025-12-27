/**
 * @file    g10link/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains the primary entry point for the G10 CPU Linker Tool.
 */

/* Private Includes ***********************************************************/

#include <g10/common.hpp>

/* Private Static Variables ***************************************************/

namespace g10link
{
    // Usage: `g10link [options] <input files> -o <output file>`
    // - `<input files>` - One or more object files to link (required)
    // - `-o <output file>`, `--output <output file>` - Specify the output file name (required)
    // - `-h`, `--help` - Show help message
    // - `-v`, `--version` - Show version info
    static std::vector<std::string> s_input_files;  // Input object files to link
    static std::string s_output_file = "";          // `-o <file>`, `--output <file>` - Output file name
    static bool s_help = false;                     // `-h`, `--help` - Show help message
    static bool s_version = false;                  // `-v`, `--version` - Show version info
}

/* Private Functions **********************************************************/

namespace g10link
{
    static auto parse_arguments (int argc, const char** argv) -> bool
    {
        // - Iterate through command-line arguments and parse them.
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "-o" || arg == "--output")
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
            else if (arg.starts_with("-"))
            {
                std::println(stderr, "Error: Unknown argument '{}'.", arg);
                return false;
            }
            else
            {
                s_input_files.push_back(arg);
            }
        }

        // - Check for `--help` or `--version` flags.
        if (s_help == true || s_version == true)
        {
            return true;
        }

        // - Validate required arguments.
        if (s_input_files.empty() == true)
        {
            std::println(stderr, 
                "Error: At least one input file is required.");
            return false;
        }
        else if (s_output_file.empty() == true)
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
            "'g10link' - G10 CPU Linker Tool\n"
            "By: Dennis W. Griffin <dgdev1024@gmail.com>\n"
        );
    }

    static auto show_help () -> void
    {
        std::println(
            "Usage: g10link [options] <input files> -o <output file>\n\n"
            "Options:\n"
            "  -o, --output <file>     Specify the output file name (required).\n"
            "  -h, --help              Show this help message and exit.\n"
            "  -v, --version           Show version information and exit.\n"
        );
    }
}

/* Main Function **************************************************************/

auto main (int argc, const char** argv) -> int
{
    // - Parse command-line arguments.
    if (g10link::parse_arguments(argc, argv) == false)
    {
        return 1;
    }

    // - Handle `--help` and `--version` flags.
    if (g10link::s_help == true)
    {
        g10link::show_version();
        g10link::show_help();
        return 0;
    }
    else if (g10link::s_version == true)
    {
        g10link::show_version();   
        return 0;
    }

    // - Linking functionality to be implemented here.

    return 0;
}
