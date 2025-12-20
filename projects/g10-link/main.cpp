/**
 * @file    g10-link/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains the primary entry point for the G10 CPU's linker tool.
 */

/* Private Includes ***********************************************************/

#include <g10-link/common.hpp>
#include <g10-link/object_file.hpp>
#include <g10-link/executable_file.hpp>
#include <g10-link/object_reader.hpp>
#include <g10-link/linker.hpp>
#include <g10-link/executable_writer.hpp>

/* Private Static Variables ***************************************************/

namespace g10link
{
    static constexpr const char* PROGRAM_VERSION = "0.1.0";

    static bool s_help = false;
    static bool s_version = false;
    static bool s_verbose = false;
    static std::vector<std::string> s_input_files {};
    static std::string s_output_file = "";
}

/* Private Functions **********************************************************/

namespace g10link
{
    static auto parse_arguments (int argc, const char** argv) -> bool
    {
        // Usage: `g10-link [options] <input files>`
        //
        // Arguments:
        // `-o <file>`, `--output <file>`:
        //      Required. Specifies the output executable file path.
        // `-h`, `--help`:
        //      Displays help/usage information.
        // `-v`, `--version`:
        //      Displays version information.
    
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if ((arg == "-o") || (arg == "--output"))
            {
                if (i + 1 < argc)
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
            else if ((arg == "-h") || (arg == "--help"))
            {
                s_help = true;
            }
            else if ((arg == "-v") || (arg == "--version"))
            {
                s_version = true;
            }
            else if ((arg == "-V") || (arg == "--verbose"))
            {
                s_verbose = true;
            }
            else
            {
                s_input_files.emplace_back(arg);
            }
        }

        // Check if help or version was requested
        if (s_help == true || s_version == true)
        {
            return true;
        }

        // Validate `-o` / `--output` argument.
        if (s_output_file.empty())
        {
            std::println(stderr, "Error: No output file specified.");
            return false;
        }

        // Validate input files.
        if (s_input_files.empty())
        {
            std::println(stderr, "Error: No input files specified.");
            return false;
        }

        return true;
    }

    static auto display_version () -> void
    {
        std::println("G10 Linker Tool - Version {}", PROGRAM_VERSION);
        std::println("By: Dennis W. Griffin <dgdev1024@gmail.com>\n");
    }

    static auto display_help () -> void
    {
        std::println("Usage: g10-link [options] <input files>");
        std::println("Options:");
        std::println("  -o, --output <file>    Required. Specifies the output executable file path.");
        std::println("  -h, --help             Displays this help/usage information.");
        std::println("  -v, --version          Displays version information.\n");
    }
}

/* Main Function **************************************************************/

auto main (int argc, const char** argv) -> int
{
    if (g10link::parse_arguments(argc, argv) == false)
    {
        return 1;
    }

    if (g10link::s_help == true)
    {
        g10link::display_version();
        g10link::display_help();
        return 0;
    }
    else if (g10link::s_version == true)
    {
        g10link::display_version();
        return 0;
    }

    // Read all input object files
    std::vector<g10obj::loaded_object_file> object_files;
    object_files.reserve(g10link::s_input_files.size());

    for (const auto& input_file : g10link::s_input_files)
    {
        if (g10link::s_verbose)
        {
            std::println("Reading object file: {}", input_file);
        }

        g10link::object_reader reader { input_file };
        auto result = reader.read();
        if (!result)
        {
            std::println(stderr, "Error reading {}: {}", input_file, result.error());
            return 1;
        }

        const auto& obj = result.value();
        if (g10link::s_verbose)
        {
            std::println("  Source: {}", obj.source_filename);
            std::println("  Sections: {}", obj.sections.size());
            std::println("  Symbols: {}", obj.symbols.size());
            std::println("  Relocations: {}", obj.relocations.size());
            std::println("");
        }

        object_files.push_back(std::move(result.value()));
    }

    if (g10link::s_verbose)
    {
        std::println("Successfully read {} object file(s)", object_files.size());
    }

    // Link all object files
    if (g10link::s_verbose)
    {
        std::println("Linking...");
    }

    g10link::linker linker;
    for (const auto& obj : object_files)
    {
        linker.add_object_file(obj);
    }

    auto link_result = linker.link();
    if (!link_result)
    {
        std::println(stderr, "Linking failed: {}", link_result.error());
        return 1;
    }

    const auto& executable = link_result.value();
    if (g10link::s_verbose)
    {
        std::println("Linking successful!");
        std::println("  Entry point: ${:08X}", executable.entry_point);
        std::println("  Stack pointer: ${:08X}", executable.stack_pointer);
        std::println("  Segments: {}", executable.segments.size());
        std::println("  Total memory: {} bytes", executable.total_memory_size());
        std::println("");
    }

    // Write executable file
    if (g10link::s_verbose)
    {
        std::println("Writing executable: {}", g10link::s_output_file);
    }

    g10link::executable_writer writer { g10link::s_output_file };
    auto write_result = writer.write(executable);
    if (!write_result)
    {
        std::println(stderr, "Failed to write executable: {}", write_result.error());
        return 1;
    }

    if (g10link::s_verbose)
    {
        std::println("Executable written successfully!");
    }

    return 0;
}
