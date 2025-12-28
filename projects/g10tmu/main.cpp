/**
 * @file    g10tmu/main.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains the primary entry point for the G10 Testbed Emulator
 *          application.
 */

/* Private Includes ***********************************************************/

#include <g10tmu/bus.hpp>

/* Private Static Variables ***************************************************/

namespace g10tmu
{
    static constexpr std::size_t MIN_RAM_SIZE = 0x10;           // Minimum RAM size: 16 bytes
    static constexpr std::size_t MAX_RAM_SIZE = 0x80000000;     // Maximum RAM size: 2 GB

    // Usage: `g10tmu [options] <input file>`
    // - `<input file>` - The program file to load into the emulator (required)
    // - `-h`, `--help` - Show help message
    // - `-v`, `--version` - Show version info
    static std::string s_input_file = "";           // Input program file to load
    static std::size_t s_ram_size = 0x10;           // `-r <size>`, `--ram <size>` - RAM size in bytes.
                                                    //      Minimum: 16 bytes (0x10, default)
                                                    //      Maximum: 2 GB (0x80000000, 2,147,483,648 bytes)
    static std::string s_dump_ram = "";             // `-d <file>`, `--dump-ram <file>` - Dump RAM contents to file on exit
    static bool s_help = false;                     // `-h`, `--help` - Show help message
    static bool s_version = false;                  // `-v`, `--version` - Show version info
}

/* Private Functions **********************************************************/

namespace g10tmu
{
    static auto parse_arguments (int argc, const char** argv) -> bool
    {
        // - Iterate through command-line arguments and parse them.
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help")
            {
                s_help = true;
            }
            else if (arg == "-v" || arg == "--version")
            {
                s_version = true;
            }
            else if ((arg == "-r" || arg == "--ram") && (i + 1 < argc))
            {
                // - Parse RAM size argument.
                std::string ram_size_str = argv[++i];
                try
                {
                    std::size_t ram_size = std::stoull(ram_size_str, nullptr, 0);
                    if (ram_size < MIN_RAM_SIZE || ram_size > MAX_RAM_SIZE)
                    {
                        std::println(stderr, "Error: RAM size must be between {} and {} bytes.", 
                            MIN_RAM_SIZE, MAX_RAM_SIZE);
                        return false;
                    }
                    s_ram_size = ram_size;
                }
                catch (const std::exception& e)
                {
                    std::println(stderr, "Error: Invalid RAM size '{}'.", ram_size_str);
                    return false;
                }
            }
            else if ((arg == "-d" || arg == "--dump-ram") && (i + 1 < argc))
            {
                // - Parse RAM dump file argument.
                s_dump_ram = argv[++i];
            }
            else if (arg.starts_with("-"))
            {
                std::println(stderr, "Error: Unknown option '{}'.", arg);
                return false;
            }
            else
            {
                // - Assume any non-option argument is the input file.
                s_input_file = arg;
            }
        }

        // - Validate required arguments.
        if (s_input_file.empty() && s_help == false && s_version == false)
        {
            std::println(stderr, "Error: No input file specified.");
            return false;
        }

        return true;
    }

    static auto show_version () -> void
    {
        std::println(
            "'g10tmu' - G10 Testbed Emulator\n"
            "By: Dennis W. Griffin <dgdev1024@gmail.com>\n"
        );
    }

    static auto show_help () -> void
    {
        std::println(
            "Usage: g10tmu [options] <input file>\n\n"
            "Options:\n"
            "  -h, --help              Show this help message and exit.\n"
            "  -v, --version           Show version information and exit.\n"
            "  -r, --ram <size>        Specify the size of system RAM in bytes.\n"
            "                          Minimum, Default: 16 (0x10, 16 bytes)\n"
            "                          Maximum: 2147483648 (0x80000000, 2 GiB)\n"
            "  -d, --dump-ram <file>   Dump the contents of RAM to the specified\n"
            "                          file upon emulator exit.\n"
        );
    }

    static auto dump_ram_to_file (const bus& system_bus, 
        const std::string& file_path) -> bool
    {
        try
        {
            std::ofstream ofs(file_path, std::ios::binary);
            if (!ofs)
            {
                std::println(stderr, "Error: Unable to open file '{}' for writing.", file_path);
                return false;
            }

            const auto& ram = system_bus.get_ram();
            ofs.write(reinterpret_cast<const char*>(ram.data()), ram.size());
            ofs.close();

            std::println("RAM dumped to file '{}'.", file_path);
            return true;
        }
        catch (const std::exception& e)
        {
            std::println(stderr, "Error: Exception occurred while dumping RAM to file '{}': {}", 
                file_path, e.what());
            return false;
        }
    }
}

/* Main Function **************************************************************/

auto main (int argc, const char** argv) -> int
{
    // - Parse command-line arguments.
    if (g10tmu::parse_arguments(argc, argv) == false)
    {
        g10tmu::show_help();
        return 1;
    }

    // - Handle `--help` or `--version` flags.
    if (g10tmu::s_help == true)
    {
        g10tmu::show_version();
        g10tmu::show_help();
        return 0;
    }
    else if (g10tmu::s_version == true)
    {
        g10tmu::show_version();
        return 0;
    }

    // - Create the system bus and start the emulator.
    g10tmu::bus system_bus { g10tmu::s_input_file, g10tmu::s_ram_size };
    auto exit_code = system_bus.start();

    // - Dump RAM to file if requested.
    if (!g10tmu::s_dump_ram.empty())
    {
        g10tmu::dump_ram_to_file(system_bus, g10tmu::s_dump_ram);
    }

    return exit_code;
}
