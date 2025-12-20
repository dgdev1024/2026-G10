/**
 * @file    g10-link/executable_writer.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains implementations for writing G10 executable files (.g10).
 */

/* Private Includes ***********************************************************/

#include <g10-link/executable_writer.hpp>
#include <cstring>

/* Public Constructors ********************************************************/

namespace g10link
{
    executable_writer::executable_writer (const std::string& filename) :
        m_filename { filename },
        m_file {}
    {
    }
}

/* Public Methods *************************************************************/

namespace g10link
{
    auto executable_writer::write (const g10exe::executable_file& executable) -> g10::result<void>
    {
        // Validate the executable structure
        if (auto result = validate(executable); !result)
        {
            return g10::error("Validation failed: {}", result.error());
        }

        // Open the output file
        if (auto result = open_file(); !result)
        {
            return g10::error("Failed to open output file: {}", result.error());
        }

        // Write the file header
        if (auto result = write_header(executable); !result)
        {
            return g10::error("Failed to write header: {}", result.error());
        }

        // Write the segment table
        if (auto result = write_segment_table(executable); !result)
        {
            return g10::error("Failed to write segment table: {}", result.error());
        }

        // Write segment data
        if (auto result = write_segment_data(executable); !result)
        {
            return g10::error("Failed to write segment data: {}", result.error());
        }

        m_file.close();
        return {};
    }
}

/* Private Methods - File Writing *********************************************/

namespace g10link
{
    auto executable_writer::open_file () -> g10::result<void>
    {
        m_file.open(m_filename, std::ios::binary | std::ios::trunc);
        if (!m_file.is_open())
        {
            return g10::error("Cannot open file for writing: {}", m_filename);
        }

        return {};
    }

    auto executable_writer::validate (const g10exe::executable_file& executable) const -> g10::result<void>
    {
        // Check that at least one segment exists
        if (executable.segments.empty())
        {
            return g10::error("No segments to write");
        }

        // Validate entry point
        if (!executable.validate_entry_point())
        {
            return g10::error("Invalid entry point: ${:08X} (must be in ROM region $00002000-$7FFFFFFF)",
                executable.entry_point);
        }

        // Validate stack pointer
        if (!executable.validate_stack_pointer())
        {
            return g10::error("Invalid stack pointer: ${:08X} (must be 0 or in RAM region $80000000-$FFFFFFFF)",
                executable.stack_pointer);
        }

        // Check for overlapping segments
        if (executable.has_overlapping_segments())
        {
            return g10::error("Executable has overlapping segments");
        }

        return {};
    }

    auto executable_writer::write_header (const g10exe::executable_file& executable) -> g10::result<void>
    {
        // Calculate total file size
        // = header (64) + segment table (24 * segment_count) + all segment data
        std::uint32_t total_file_size = g10exe::HEADER_SIZE + 
            (g10exe::SEGMENT_ENTRY_SIZE * static_cast<std::uint32_t>(executable.segments.size()));
        
        for (const auto& seg : executable.segments)
        {
            total_file_size += seg.file_size();
        }

        // Create and populate header structure
        g10exe::file_header header;
        header.magic = g10exe::MAGIC;
        header.version = g10exe::VERSION;
        header.flags = executable.flags;
        header.entry_point = executable.entry_point;
        header.stack_pointer = executable.stack_pointer;
        header.segment_count = static_cast<std::uint16_t>(executable.segments.size());
        header.reserved1 = 0;
        header.total_file_size = total_file_size;
        header.total_memory_size = executable.total_memory_size();
        header.checksum = 0;  // Not computed for now

        // Copy program name (ensure null-termination)
        std::memset(header.program_name, 0, g10exe::PROGRAM_NAME_SIZE);
        if (!executable.program_name.empty())
        {
            std::size_t copy_len = std::min(executable.program_name.size(), 
                                           g10exe::PROGRAM_NAME_SIZE - 1);
            std::memcpy(header.program_name, executable.program_name.c_str(), copy_len);
        }

        // Write header to file
        return write_bytes(&header, sizeof(header));
    }

    auto executable_writer::write_segment_table (const g10exe::executable_file& executable) -> g10::result<void>
    {
        // Calculate file offset for first segment's data
        // = header size + segment table size
        std::uint32_t current_offset = g10exe::HEADER_SIZE + 
            (g10exe::SEGMENT_ENTRY_SIZE * static_cast<std::uint32_t>(executable.segments.size()));

        for (const auto& seg : executable.segments)
        {
            // Create segment entry
            g10exe::segment_entry entry;
            entry.load_address = seg.load_address;
            entry.memory_size = seg.memory_size;
            entry.file_size = seg.file_size();
            entry.file_offset = current_offset;
            entry.flags = seg.flags;
            entry.alignment = seg.alignment;
            entry.reserved = 0;

            // Write segment entry
            if (auto result = write_bytes(&entry, sizeof(entry)); !result)
            {
                return g10::error(result.error());
            }

            // Update offset for next segment
            current_offset += seg.file_size();
        }

        return {};
    }

    auto executable_writer::write_segment_data (const g10exe::executable_file& executable) -> g10::result<void>
    {
        for (const auto& seg : executable.segments)
        {
            // Skip BSS segments (no data to write)
            if (seg.is_bss() || seg.data.empty())
            {
                continue;
            }

            // Write segment data
            if (auto result = write_bytes(seg.data.data(), seg.data.size()); !result)
            {
                return g10::error("Failed to write segment data at ${:08X}: {}",
                    seg.load_address, result.error());
            }
        }

        return {};
    }
}

/* Private Methods - Binary Writing *******************************************/

namespace g10link
{
    auto executable_writer::write_u8 (std::uint8_t value) -> g10::result<void>
    {
        return write_bytes(&value, 1);
    }

    auto executable_writer::write_u16 (std::uint16_t value) -> g10::result<void>
    {
        // Little-endian conversion
        std::uint8_t bytes[2];
        bytes[0] = static_cast<std::uint8_t>(value & 0xFF);
        bytes[1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        return write_bytes(bytes, 2);
    }

    auto executable_writer::write_u32 (std::uint32_t value) -> g10::result<void>
    {
        // Little-endian conversion
        std::uint8_t bytes[4];
        bytes[0] = static_cast<std::uint8_t>(value & 0xFF);
        bytes[1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        bytes[2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
        bytes[3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
        return write_bytes(bytes, 4);
    }

    auto executable_writer::write_bytes (const void* buffer, std::size_t size) -> g10::result<void>
    {
        m_file.write(static_cast<const char*>(buffer), size);
        if (!m_file)
        {
            return g10::error("Failed to write {} bytes to file", size);
        }
        return {};
    }
}
