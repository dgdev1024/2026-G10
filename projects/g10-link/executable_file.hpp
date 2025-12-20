/**
 * @file    g10-link/executable_file.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains definitions for writing G10 Executable Program File Format
 *          (.g10) files.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-link/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10exe
{
    /**
     * @brief   The magic number identifying a G10 executable file.
     *  
     * This is the ASCII string "G10X" (G10 eXecutable) in little-endian format:
     * - Byte 0: `0x58` ('X')
     * - Byte 1: `0x30` ('0')
     * - Byte 2: `0x31` ('1')
     * - Byte 3: `0x47` ('G')
     */
    constexpr std::uint32_t MAGIC = 0x47313058;

    /**
     * @brief   The version of the G10 executable file format.
     */
    constexpr std::uint16_t VERSION = 0x0001;

    /**
     * @brief   The size, in bytes, of the executable file header.
     */
    constexpr std::size_t HEADER_SIZE = 64;

    /**
     * @brief   The size, in bytes, of a single segment entry.
     */
    constexpr std::size_t SEGMENT_ENTRY_SIZE = 24;

    /**
     * @brief   The maximum length of the program name (including null terminator).
     */
    constexpr std::size_t PROGRAM_NAME_SIZE = 32;

    /**
     * @brief   The default entry point address if none is specified.
     */
    constexpr std::uint32_t DEFAULT_ENTRY_POINT = 0x00002000;

    /**
     * @brief   The default stack pointer value if none is specified.
     */
    constexpr std::uint32_t DEFAULT_STACK_POINTER = 0xFFFFFFFC;

    /**
     * @brief   Weakly enumerates the flags that can appear in a G10 executable
     *          file's header.
     */
    enum file_flags : std::uint16_t
    {
        FLAG_HAS_SYMBOLS = 0x0001,  /** @brief Optional symbol table appended */
        FLAG_COMPRESSED  = 0x0002,  /** @brief Segment data is compressed */
        FLAG_SIGNED      = 0x0004,  /** @brief File includes signature */
    };

    /**
     * @brief   Weakly enumerates the flags that can appear in a G10 executable
     *          file's segment entries.
     */
    enum segment_flags : std::uint16_t
    {
        SEG_READ  = 0x0001,  /** @brief Segment is readable */
        SEG_WRITE = 0x0002,  /** @brief Segment is writable */
        SEG_EXEC  = 0x0004,  /** @brief Segment is executable */
        SEG_ZERO  = 0x0008,  /** @brief Segment is zero-initialized (BSS) */
    };

}

/* Binary Structures (for file I/O) *******************************************/

namespace g10exe
{
    /**
     * @brief   Defines the binary structure of a G10 executable file header.
     * 
     * This structure is exactly 64 bytes and is used for direct file I/O.
     */
    struct file_header final
    {
        std::uint32_t magic;              /** @brief Magic number (0x47313058 = "G10X") */
        std::uint16_t version;            /** @brief Format version (0x0001) */
        std::uint16_t flags;              /** @brief File flags */
        std::uint32_t entry_point;        /** @brief Address where execution begins */
        std::uint32_t stack_pointer;      /** @brief Initial stack pointer value */
        std::uint16_t segment_count;      /** @brief Number of program segments */
        std::uint16_t reserved1;          /** @brief Reserved (must be 0) */
        std::uint32_t total_file_size;    /** @brief Total size of this file */
        std::uint32_t total_memory_size;  /** @brief Total memory required */
        std::uint32_t checksum;           /** @brief CRC-32 checksum (0 = not computed) */
        char          program_name[32];   /** @brief Null-terminated program name */
    };

    static_assert(sizeof(file_header) == 64, "file_header must be exactly 64 bytes");

    /**
     * @brief   Defines the binary structure of a G10 executable segment entry.
     * 
     * This structure is exactly 24 bytes and is used for direct file I/O.
     */
    struct segment_entry final
    {
        std::uint32_t load_address;  /** @brief Memory address where segment loads */
        std::uint32_t memory_size;   /** @brief Size in memory (may include BSS) */
        std::uint32_t file_size;     /** @brief Size in file (0 for pure BSS) */
        std::uint32_t file_offset;   /** @brief Offset in file to segment data */
        std::uint16_t flags;         /** @brief Segment flags */
        std::uint16_t alignment;     /** @brief Alignment requirement */
        std::uint32_t reserved;      /** @brief Reserved (must be 0) */
    };

    static_assert(sizeof(segment_entry) == 24, "segment_entry must be exactly 24 bytes");

}

/* In-Memory Structures (for linker processing) *******************************/

namespace g10exe
{
    /**
     * @brief   Represents a program segment with its data.
     */
    struct program_segment final
    {
        std::uint32_t              load_address;  /** @brief Load address in memory */
        std::uint32_t              memory_size;   /** @brief Size in memory */
        std::uint16_t              flags;         /** @brief Segment flags */
        std::uint16_t              alignment;     /** @brief Alignment requirement */
        std::vector<std::uint8_t>  data;          /** @brief Segment data (may be empty for BSS) */

        /**
         * @brief   Returns the file size (actual data bytes).
         */
        inline auto file_size () const noexcept -> std::uint32_t
        {
            return static_cast<std::uint32_t>(data.size());
        }

        /**
         * @brief   Checks if this segment is in the RAM region.
         */
        inline constexpr auto is_writable () const noexcept -> bool
        {
            return (flags & SEG_WRITE) != 0;
        }

        /**
         * @brief   Checks if this segment is executable.
         */
        inline constexpr auto is_executable () const noexcept -> bool
        {
            return (flags & SEG_EXEC) != 0;
        }

        /**
         * @brief   Checks if this segment is BSS (zero-initialized).
         */
        inline constexpr auto is_bss () const noexcept -> bool
        {
            return (flags & SEG_ZERO) != 0;
        }

        /**
         * @brief   Checks if this segment overlaps with another in memory.
         * 
         * @param   other   The other segment to check.
         * 
         * @return  True if segments overlap, false otherwise.
         */
        inline auto overlaps_with (const program_segment& other) const noexcept -> bool
        {
            std::uint32_t this_end = load_address + memory_size;
            std::uint32_t other_end = other.load_address + other.memory_size;

            return (load_address < other_end) && (other.load_address < this_end);
        }
    };

    /**
     * @brief   Represents a complete executable file in memory.
     */
    struct executable_file final
    {
        std::string                    program_name;    /** @brief Program name */
        std::uint32_t                  entry_point;     /** @brief Entry point address */
        std::uint32_t                  stack_pointer;   /** @brief Initial stack pointer */
        std::uint16_t                  flags;           /** @brief File flags */
        std::vector<program_segment>   segments;        /** @brief Program segments */

        /**
         * @brief   Constructs an executable file with default values.
         */
        executable_file () :
            program_name {},
            entry_point { DEFAULT_ENTRY_POINT },
            stack_pointer { DEFAULT_STACK_POINTER },
            flags { 0 },
            segments {}
        {
        }

        /**
         * @brief   Returns the total memory size required by all segments.
         */
        auto total_memory_size () const noexcept -> std::uint32_t
        {
            std::uint32_t total = 0;
            for (const auto& seg : segments)
            {
                total += seg.memory_size;
            }
            return total;
        }

        /**
         * @brief   Returns the total file size of all segment data.
         */
        auto total_data_size () const noexcept -> std::uint32_t
        {
            std::uint32_t total = 0;
            for (const auto& seg : segments)
            {
                total += seg.file_size();
            }
            return total;
        }

        /**
         * @brief   Validates the entry point address.
         * 
         * The entry point must be in the ROM region ($00002000 - $7FFFFFFF).
         * 
         * @return  True if valid, false otherwise.
         */
        auto validate_entry_point () const noexcept -> bool
        {
            return (entry_point >= 0x00002000) && (entry_point < 0x80000000);
        }

        /**
         * @brief   Validates the stack pointer address.
         * 
         * The stack pointer must be 0 or in the RAM region ($80000000 - $FFFFFFFF).
         * 
         * @return  True if valid, false otherwise.
         */
        auto validate_stack_pointer () const noexcept -> bool
        {
            return (stack_pointer == 0) || (stack_pointer >= 0x80000000);
        }

        /**
         * @brief   Checks if any segments overlap in memory.
         * 
         * @return  True if any segments overlap, false otherwise.
         */
        auto has_overlapping_segments () const noexcept -> bool
        {
            for (std::size_t i = 0; i < segments.size(); ++i)
            {
                for (std::size_t j = i + 1; j < segments.size(); ++j)
                {
                    if (segments[i].overlaps_with(segments[j]))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        /**
         * @brief   Sorts segments by load address.
         */
        auto sort_segments () -> void
        {
            std::sort(segments.begin(), segments.end(),
                [](const program_segment& a, const program_segment& b)
                {
                    return a.load_address < b.load_address;
                });
        }
    };

}

/* Utility Functions **********************************************************/

namespace g10exe
{
    /**
     * @brief   Converts object file section flags to executable segment flags.
     * 
     * @param   section_flags   The section flags from an object file.
     * 
     * @return  The corresponding segment flags.
     */
    inline auto section_flags_to_segment_flags (std::uint16_t section_flags) -> std::uint16_t
    {
        std::uint16_t result = SEG_READ;  // All segments are readable

        // Check for executable
        if (section_flags & 0x0001)  // SECT_EXECUTABLE
        {
            result |= SEG_EXEC;
        }

        // Check for writable (RAM)
        if (section_flags & 0x0002)  // SECT_WRITABLE
        {
            result |= SEG_WRITE;
        }

        // Check for BSS
        if (section_flags & 0x0008)  // SECT_ZERO
        {
            result |= SEG_ZERO;
        }

        return result;
    }

}
