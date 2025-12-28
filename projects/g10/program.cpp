/**
 * @file    g10/program.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains implementations for the G10 CPU executable program file.
 */

/* Private Includes ***********************************************************/

#include <g10/program.hpp>

/* Private Constants and Enumerations *****************************************/

namespace g10
{

}

/* Private Helper Functions ***************************************************/

namespace g10
{
    /**
     * @brief   Reads a string of specified length from a byte buffer.
     * 
     * @param   buffer  The buffer to read from.
     * @param   offset  The offset into the buffer.
     * @param   length  The number of bytes to read.
     * 
     * @return  The string at the given offset.
     */
    auto read_string_from_buffer (
        std::span<const std::uint8_t> buffer,
        std::size_t offset,
        std::size_t length
    ) -> std::string
    {
        if (offset + length > buffer.size())
        {
            return "";
        }

        return std::string(
            reinterpret_cast<const char*>(buffer.data() + offset),
            length
        );
    }
}

/* Public Methods *************************************************************/

namespace g10
{
    program::program (
        const std::vector<object>& objects
    )
    {
        link_from_objects(objects);
    }

    program::program (const fs::path& path)
    {
        load_from_file(path);
    }

    auto program::clear () -> void
    {
        m_good = false;
        m_flags = program_flags::none;
        m_entry_point = PROGRAM_DEFAULT_ENTRY;
        m_stack_pointer = PROGRAM_DEFAULT_STACK;
        m_segments.clear();
        m_info = program_info{};
    }

    auto program::link_from_objects (
        const std::vector<object>& objects
    ) -> result<void>
    {
        // Clear any existing data.
        clear();

        // Validate that we have at least one object file.
        if (objects.empty())
        {
            return error("No object files provided for linking");
        }

        // Validate that all object files are valid.
        for (std::size_t i = 0; i < objects.size(); ++i)
        {
            if (objects[i].is_good() == false)
            {
                return error("Object file {} is not valid", i);
            }
        }

        // Step 1: Collect and resolve all symbols.
        std::vector<resolved_symbol> symbols;
        auto collect_result = collect_symbols(objects, symbols);
        if (collect_result.has_value() == false)
        {
            return error(collect_result.error());
        }

        // Step 2: Collect all sections.
        std::vector<link_section> sections;
        auto sections_result = collect_sections(objects, sections);
        if (sections_result.has_value() == false)
        {
            return error(sections_result.error());
        }

        // Step 3: Apply relocations to patch section data.
        auto reloc_result = apply_relocations(objects, symbols, sections);
        if (reloc_result.has_value() == false)
        {
            return error(reloc_result.error());
        }

        // Step 4: Generate program segments from linked sections.
        auto segments_result = generate_segments(sections);
        if (segments_result.has_value() == false)
        {
            return error(segments_result.error());
        }

        // Handle BSS segment sizes (need to get from original sections).
        for (std::size_t i = 0; i < m_segments.size(); ++i)
        {
            auto& seg = m_segments[i];
            if (seg.type == segment_type::bss && seg.memory_size == 0)
            {
                // Find the corresponding link_section to get size.
                for (const auto& link_sec : sections)
                {
                    if (link_sec.address == seg.load_address &&
                        link_sec.type == section_type::bss)
                    {
                        // Get size from original object section.
                        const auto& obj = objects[link_sec.object_index];
                        const auto& obj_sec =
                            obj.get_sections()[link_sec.section_index];
                        seg.memory_size = static_cast<std::uint32_t>(
                            obj_sec.data.size() > 0 ? obj_sec.data.size() :
                            (obj_sec.virtual_address > 0 ? 4 : 0)
                        );
                        break;
                    }
                }
            }
        }

        // Step 5: Find and set the entry point.
        auto entry_result = find_entry_point(symbols);
        if (entry_result.has_value() == false)
        {
            return error(entry_result.error());
        }

        // Set the initial stack pointer to default.
        m_stack_pointer = PROGRAM_DEFAULT_STACK;
        m_flags = m_flags | program_flags::has_stack_init;

        // Validate the final program structure.
        auto validate_result = validate();
        if (validate_result.has_value() == false)
        {
            return error(validate_result.error());
        }

        m_good = true;
        return {};
    }

    auto program::load_from_file (const fs::path& path) -> result<void>
    {
        // Clear any existing data first.
        clear();

        // Check that the path points to a valid file.
        if (fs::exists(path) == false)
        {
            return error("File not found: '{}'", path.string());
        }

        if (fs::is_regular_file(path) == false)
        {
            return error("Not a regular file: '{}'", path.string());
        }

        // Determine the file size and check that it is large enough for the
        // header.
        const auto file_size = fs::file_size(path);
        if (file_size < PROGRAM_HEADER_SIZE)
        {
            return error(
                "File too small for program header: {} bytes (need at "
                "least {} bytes)",
                file_size, PROGRAM_HEADER_SIZE
            );
        }

        // Open the file for reading in binary mode.
        std::ifstream file(path, std::ios::binary);
        if (file.is_open() == false)
        {
            return error("Failed to open file for reading: '{}'",
                path.string());
        }

        // Read the entire file into a buffer.
        std::vector<std::uint8_t> buffer(file_size);
        file.read(reinterpret_cast<char*>(buffer.data()), file_size);
        if (file.good() == false)
        {
            return error("Failed to read file contents: '{}'", path.string());
        }
        file.close();

        // Parse and validate the header.
        auto header_result = parse_header(buffer);
        if (header_result.has_value() == false)
        {
            return error(header_result.error());
        }

        // Parse the segment headers and data.
        auto segments_result = parse_segments(buffer);
        if (segments_result.has_value() == false)
        {
            return error(segments_result.error());
        }

        // Parse the program info section if present.
        if (has_info())
        {
            auto info_result = parse_info(buffer);
            if (info_result.has_value() == false)
            {
                return error(info_result.error());
            }
        }

        // Validate the loaded program structure.
        auto validate_result = validate();
        if (validate_result.has_value() == false)
        {
            return error(validate_result.error());
        }

        m_good = true;
        return {};
    }

    auto program::save_to_file (const fs::path& path) -> result<bool>
    {
        // Calculate file size:
        // - Header: 64 bytes
        // - Segment headers: 16 bytes each
        // - Segment data: sum of all LOAD segment file sizes
        // - Program info: optional, not implemented yet

        const std::size_t header_size = PROGRAM_HEADER_SIZE;
        const std::size_t segment_headers_size =
            m_segments.size() * PROGRAM_SEGMENT_HEADER_SIZE;

        // Calculate total segment data size (only for LOAD segments).
        std::size_t segment_data_size = 0;
        for (const auto& seg : m_segments)
        {
            if ((seg.flags & segment_flags::load) != segment_flags::none)
            {
                segment_data_size += seg.data.size();
            }
        }

        const std::size_t total_size =
            header_size + segment_headers_size + segment_data_size;

        // Allocate the buffer.
        std::vector<std::uint8_t> buffer(total_size, 0);

        // Write the header.
        write_u32_le(buffer, 0x00, PROGRAM_MAGIC);
        write_u32_le(buffer, 0x04, PROGRAM_VERSION);
        write_u32_le(buffer, 0x08, static_cast<std::uint32_t>(m_flags));
        write_u32_le(buffer, 0x0C, m_entry_point);
        write_u32_le(buffer, 0x10, m_stack_pointer);
        write_u32_le(buffer, 0x14, static_cast<std::uint32_t>(m_segments.size()));
        write_u32_le(buffer, 0x18, 0);  // Program info offset (none).
        write_u32_le(buffer, 0x1C, 0);  // Program info size (none).
        // Reserved bytes (0x20 - 0x3F) are already zero-initialized.

        // Write segment headers and track data offset.
        std::size_t data_offset = header_size + segment_headers_size;

        for (std::size_t i = 0; i < m_segments.size(); ++i)
        {
            const auto& seg = m_segments[i];
            const std::size_t header_offset =
                header_size + (i * PROGRAM_SEGMENT_HEADER_SIZE);

            // Determine file size (0 for BSS/ZERO_FILL segments).
            const std::uint32_t file_size =
                ((seg.flags & segment_flags::load) != segment_flags::none)
                    ? static_cast<std::uint32_t>(seg.data.size())
                    : 0;

            // Write segment header.
            write_u32_le(buffer, header_offset + 0x00, seg.load_address);
            write_u32_le(buffer, header_offset + 0x04, seg.memory_size);
            write_u32_le(buffer, header_offset + 0x08, file_size);
            write_u16_le(buffer, header_offset + 0x0C,
                static_cast<std::uint16_t>(seg.type));
            write_u16_le(buffer, header_offset + 0x0E,
                static_cast<std::uint16_t>(seg.flags));

            // Write segment data if it has the LOAD flag.
            if (file_size > 0)
            {
                std::copy(seg.data.begin(), seg.data.end(),
                    buffer.begin() + data_offset);
                data_offset += file_size;
            }
        }

        // Open the file for writing.
        std::ofstream file(path, std::ios::binary);
        if (file.is_open() == false)
        {
            return error("Failed to open file for writing: '{}'", path.string());
        }

        // Write the buffer to the file.
        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        if (file.good() == false)
        {
            return error("Failed to write file contents: '{}'", path.string());
        }
        file.close();

        m_good = true;

        // Return true if non-empty, false if empty.
        return m_segments.empty() == false;
    }

    auto program::read_byte (std::uint32_t address) const -> std::uint8_t
    {
        // Only addresses in the ROM region (0x00000000 - 0x7FFFFFFF) can be read.
        // RAM addresses (bit 31 set) are managed by the virtual machine.
        if (address > PROGRAM_ROM_END)
        {
            return 0xFF;  // Open-bus value.
        }

        // Search for a segment containing this address.
        for (const auto& segment : m_segments)
        {
            // Skip BSS segments (no data stored in program file).
            if (segment.type == segment_type::bss)
            {
                continue;
            }

            const auto seg_start = segment.load_address;
            const auto seg_end = seg_start + segment.memory_size;

            // Check if address is within this segment's range.
            if (address >= seg_start && address < seg_end)
            {
                const auto offset = address - seg_start;

                // If offset is within the loaded data, return the byte.
                if (offset < segment.data.size())
                {
                    return segment.data[offset];
                }

                // If offset is beyond loaded data but within memory size,
                // return zero (zero-fill for partially initialized segments).
                return 0x00;
            }
        }

        // Address not covered by any segment - return open-bus value.
        return 0xFF;
    }
}

/* Private Methods ************************************************************/

namespace g10
{
    auto program::validate () -> result<void>
    {
        // Check that we have at least one segment.
        if (m_segments.empty())
        {
            return error("Program contains no segments");
        }

        // Validate entry point is within ROM region.
        if (has_entry())
        {
            if (m_entry_point > PROGRAM_ROM_END)
            {
                return error(
                    "Entry point 0x{:08X} is outside ROM region "
                    "(0x{:08X} - 0x{:08X})",
                    m_entry_point, PROGRAM_ROM_START, PROGRAM_ROM_END
                );
            }

            // Check that entry point falls within a CODE segment.
            bool entry_in_segment = false;
            for (const auto& segment : m_segments)
            {
                if (segment.type == segment_type::code ||
                    segment.type == segment_type::interrupt)
                {
                    const auto seg_start = segment.load_address;
                    const auto seg_end = seg_start + segment.memory_size;

                    if (m_entry_point >= seg_start && m_entry_point < seg_end)
                    {
                        entry_in_segment = true;
                        break;
                    }
                }
            }

            if (entry_in_segment == false)
            {
                return error(
                    "Entry point 0x{:08X} is not within any CODE segment",
                    m_entry_point
                );
            }
        }

        // Validate initial stack pointer is within RAM region.
        if (has_stack_init())
        {
            if (m_stack_pointer < PROGRAM_RAM_START)
            {
                return error(
                    "Initial stack pointer 0x{:08X} is outside RAM region "
                    "(0x{:08X} - 0x{:08X})",
                    m_stack_pointer, PROGRAM_RAM_START, PROGRAM_RAM_END
                );
            }
        }

        // Validate each segment's load address for its type.
        for (std::size_t i = 0; i < m_segments.size(); ++i)
        {
            const auto& segment = m_segments[i];
            const auto seg_start = segment.load_address;
            const auto seg_end = seg_start + segment.memory_size - 1;

            switch (segment.type)
            {
            case segment_type::code:
            case segment_type::data:
                // CODE and DATA must be in the program region of ROM.
                if (seg_start < PROGRAM_CODE_START || seg_end > PROGRAM_ROM_END)
                {
                    return error(
                        "Segment {}: CODE/DATA segment 0x{:08X}-0x{:08X} "
                        "is outside valid ROM region (0x{:08X}-0x{:08X})",
                        i, seg_start, seg_end,
                        PROGRAM_CODE_START, PROGRAM_ROM_END
                    );
                }
                break;

            case segment_type::bss:
                // BSS must be in RAM region.
                if (seg_start < PROGRAM_RAM_START)
                {
                    return error(
                        "Segment {}: BSS segment 0x{:08X}-0x{:08X} "
                        "is outside RAM region (0x{:08X}-0x{:08X})",
                        i, seg_start, seg_end,
                        PROGRAM_RAM_START, PROGRAM_RAM_END
                    );
                }
                break;

            case segment_type::metadata:
                // METADATA must be in metadata region.
                if (seg_start > 0x00000FFF || seg_end > 0x00000FFF)
                {
                    return error(
                        "Segment {}: METADATA segment 0x{:08X}-0x{:08X} "
                        "is outside metadata region (0x00000000-0x00000FFF)",
                        i, seg_start, seg_end
                    );
                }
                break;

            case segment_type::interrupt:
                // INTERRUPT must be in interrupt table region.
                if (seg_start < 0x00001000 || seg_end > 0x00001FFF)
                {
                    return error(
                        "Segment {}: INTERRUPT segment 0x{:08X}-0x{:08X} "
                        "is outside interrupt region (0x00001000-0x00001FFF)",
                        i, seg_start, seg_end
                    );
                }
                break;

            case segment_type::null_:
                // NULL segments should have been filtered out during parsing.
                break;
            }
        }

        // Check for segment overlaps.
        for (std::size_t i = 0; i < m_segments.size(); ++i)
        {
            const auto& seg_a = m_segments[i];
            const auto a_start = seg_a.load_address;
            const auto a_end = a_start + seg_a.memory_size;

            for (std::size_t j = i + 1; j < m_segments.size(); ++j)
            {
                const auto& seg_b = m_segments[j];
                const auto b_start = seg_b.load_address;
                const auto b_end = b_start + seg_b.memory_size;

                // Check if ranges overlap.
                if (a_start < b_end && b_start < a_end)
                {
                    return error(
                        "Segments {} and {} overlap: 0x{:08X}-0x{:08X} "
                        "and 0x{:08X}-0x{:08X}",
                        i, j, a_start, a_end - 1, b_start, b_end - 1
                    );
                }
            }
        }

        return {};
    }
}

/* Private Methods - File Parsing *********************************************/

namespace g10
{
    auto program::parse_header (std::span<const std::uint8_t> buffer)
        -> result<void>
    {
        // Read the magic number and verify it.
        const auto magic = read_u32_le(buffer, 0x00);
        if (magic != PROGRAM_MAGIC)
        {
            return error(
                "Invalid magic number: expected 0x{:08X}, got 0x{:08X}",
                PROGRAM_MAGIC, magic
            );
        }

        // Read and validate the version number.
        const auto version = read_u32_le(buffer, 0x04);
        const auto file_major = (version >> 24) & 0xFF;
        const auto current_major = (PROGRAM_VERSION >> 24) & 0xFF;

        if (file_major > current_major)
        {
            return error(
                "Incompatible program file version: file is v{}.x, "
                "but this library supports up to v{}.x",
                file_major, current_major
            );
        }

        // Read the flags field.
        m_flags = static_cast<program_flags>(read_u32_le(buffer, 0x08));

        // Read the entry point address.
        m_entry_point = read_u32_le(buffer, 0x0C);

        // Read the initial stack pointer.
        m_stack_pointer = read_u32_le(buffer, 0x10);

        // Read the segment count.
        const auto segment_count = read_u32_le(buffer, 0x14);
        if (segment_count == 0)
        {
            return error("Program file contains no segments");
        }

        // Calculate the minimum expected file size based on segment count.
        const auto min_size = PROGRAM_HEADER_SIZE +
            (segment_count * PROGRAM_SEGMENT_HEADER_SIZE);
        if (buffer.size() < min_size)
        {
            return error(
                "File too small for {} segment headers: {} bytes "
                "(need at least {} bytes)",
                segment_count, buffer.size(), min_size
            );
        }

        // Read and validate program info offset and size if HAS_INFO flag set.
        if (has_info())
        {
            const auto info_offset = read_u32_le(buffer, 0x18);
            const auto info_size = read_u32_le(buffer, 0x1C);

            if (info_offset == 0 || info_size == 0)
            {
                return error(
                    "HAS_INFO flag is set but info offset or size is zero"
                );
            }

            if (info_offset + info_size > buffer.size())
            {
                return error(
                    "Program info extends beyond file: offset {} + "
                    "size {} > file size {}",
                    info_offset, info_size, buffer.size()
                );
            }
        }

        return {};
    }

    auto program::parse_segments (std::span<const std::uint8_t> buffer)
        -> result<void>
    {
        // Read the segment count from the header.
        const auto segment_count = read_u32_le(buffer, 0x14);

        // Calculate where segment data begins (after all segment headers).
        std::size_t data_offset = PROGRAM_HEADER_SIZE +
            (segment_count * PROGRAM_SEGMENT_HEADER_SIZE);

        // Reserve space for the segments.
        m_segments.reserve(segment_count);

        // Parse each segment header.
        for (std::uint32_t i = 0; i < segment_count; ++i)
        {
            const std::size_t header_offset = PROGRAM_HEADER_SIZE +
                (i * PROGRAM_SEGMENT_HEADER_SIZE);

            // Read segment header fields.
            const auto load_address = read_u32_le(buffer, header_offset + 0x00);
            const auto memory_size = read_u32_le(buffer, header_offset + 0x04);
            const auto file_size = read_u32_le(buffer, header_offset + 0x08);
            const auto type_value = read_u16_le(buffer, header_offset + 0x0C);
            const auto flags_value = read_u16_le(buffer, header_offset + 0x0E);

            // Convert type and flags to their enum types.
            const auto type = static_cast<segment_type>(type_value);
            const auto flags = static_cast<segment_flags>(flags_value);

            // Skip NULL segment entries.
            if (type == segment_type::null_)
            {
                continue;
            }

            // Validate file size does not exceed memory size.
            if (file_size > memory_size)
            {
                return error(
                    "Segment {}: file size ({}) exceeds memory size ({})",
                    i, file_size, memory_size
                );
            }

            // Validate memory size is non-zero.
            if (memory_size == 0)
            {
                return error("Segment {}: memory size is zero", i);
            }

            // Validate segment data bounds for LOAD segments.
            if ((flags & segment_flags::load) != segment_flags::none &&
                file_size > 0)
            {
                if (data_offset + file_size > buffer.size())
                {
                    return error(
                        "Segment {}: data extends beyond file: offset {} "
                        "+ size {} > file size {}",
                        i, data_offset, file_size, buffer.size()
                    );
                }
            }

            // Create the segment.
            program_segment segment;
            segment.load_address = load_address;
            segment.memory_size = memory_size;
            segment.type = type;
            segment.flags = flags;

            // Load segment data if it has the LOAD flag and non-zero file size.
            if ((flags & segment_flags::load) != segment_flags::none &&
                file_size > 0)
            {
                segment.data.resize(file_size);
                std::copy(
                    buffer.begin() + data_offset,
                    buffer.begin() + data_offset + file_size,
                    segment.data.begin()
                );
                data_offset += file_size;
            }

            m_segments.push_back(std::move(segment));
        }

        return {};
    }

    auto program::parse_info (std::span<const std::uint8_t> buffer)
        -> result<void>
    {
        // Read the info offset and size from the header.
        const auto info_offset = read_u32_le(buffer, 0x18);
        const auto info_size = read_u32_le(buffer, 0x1C);

        // Validate minimum size for info header.
        if (info_size < PROGRAM_INFO_HEADER_SIZE)
        {
            return error(
                "Program info section too small: {} bytes "
                "(need at least {} bytes)",
                info_size, PROGRAM_INFO_HEADER_SIZE
            );
        }

        // Create a subspan for the info section.
        const auto info_buffer = buffer.subspan(info_offset, info_size);

        // Read the info header fields.
        const auto info_version = read_u16_le(info_buffer, 0x00);
        const auto info_flags = static_cast<program_info_flags>(
            read_u16_le(info_buffer, 0x02)
        );

        // We currently support info version 1.
        if (info_version != 1)
        {
            return error(
                "Unsupported program info version: {} (expected 1)",
                info_version
            );
        }

        // Read string offsets and lengths (relative to string data at 0x30).
        const auto name_offset = read_u32_le(info_buffer, 0x04);
        const auto name_length = read_u32_le(info_buffer, 0x08);
        const auto version_offset = read_u32_le(info_buffer, 0x0C);
        const auto version_length = read_u32_le(info_buffer, 0x10);
        const auto author_offset = read_u32_le(info_buffer, 0x14);
        const auto author_length = read_u32_le(info_buffer, 0x18);
        const auto desc_offset = read_u32_le(info_buffer, 0x1C);
        const auto desc_length = read_u32_le(info_buffer, 0x20);

        // Read build date and checksum.
        m_info.build_date = read_u32_le(info_buffer, 0x24);
        m_info.checksum = read_u32_le(info_buffer, 0x28);

        // The string data starts at offset 0x30 within the info section.
        const std::size_t string_data_start = PROGRAM_INFO_HEADER_SIZE;
        const std::size_t string_data_size = info_size - PROGRAM_INFO_HEADER_SIZE;

        // Helper lambda to read a string if present.
        auto read_info_string = [&](
            std::uint32_t offset,
            std::uint32_t length,
            program_info_flags flag
        ) -> std::string
        {
            if ((info_flags & flag) == program_info_flags::none || length == 0)
            {
                return "";
            }

            // String offsets are relative to the string data region (0x30).
            if (offset + length > string_data_size)
            {
                return "";  // String extends beyond info section.
            }

            return read_string_from_buffer(
                info_buffer,
                string_data_start + offset,
                length
            );
        };

        // Read the strings.
        m_info.name = read_info_string(
            name_offset, name_length, program_info_flags::has_name
        );
        m_info.version = read_info_string(
            version_offset, version_length, program_info_flags::has_version
        );
        m_info.author = read_info_string(
            author_offset, author_length, program_info_flags::has_author
        );
        m_info.description = read_info_string(
            desc_offset, desc_length, program_info_flags::has_desc
        );

        return {};
    }
}

/* Private Methods - Linking **************************************************/

namespace g10
{
    auto program::collect_symbols (
        const std::vector<object>& objects,
        std::vector<resolved_symbol>& symbols
    ) -> result<void>
    {
        // Map to track global symbols by name for duplicate detection and
        // extern resolution.
        std::unordered_map<std::string, std::size_t> global_symbol_map;

        // First pass: Collect all global and local symbols from each object.
        for (std::size_t obj_idx = 0; obj_idx < objects.size(); ++obj_idx)
        {
            const auto& obj = objects[obj_idx];
            const auto& obj_symbols = obj.get_symbols();
            const auto& obj_sections = obj.get_sections();

            for (std::size_t sym_idx = 0; sym_idx < obj_symbols.size(); ++sym_idx)
            {
                const auto& sym = obj_symbols[sym_idx];

                // Skip extern symbols for now - they'll be resolved in pass 2.
                if (sym.binding == symbol_binding::extern_)
                {
                    continue;
                }

                // The symbol value is already the absolute address in the
                // G10 assembler output, so we don't need to add the section
                // base address. The symbol value is set to the location counter
                // at the time of definition.
                std::uint32_t final_address = sym.value;

                // Check for duplicate global symbols.
                if (sym.binding == symbol_binding::global)
                {
                    auto it = global_symbol_map.find(sym.name);
                    if (it != global_symbol_map.end())
                    {
                        // Allow weak symbols to be overridden.
                        const auto& existing = symbols[it->second];
                        if (existing.binding != symbol_binding::weak)
                        {
                            return error(
                                "Duplicate global symbol '{}' defined in "
                                "object {} and object {}",
                                sym.name, existing.object_index, obj_idx
                            );
                        }
                    }
                }

                // Create the resolved symbol.
                resolved_symbol resolved;
                resolved.name = sym.name;
                resolved.address = final_address;
                resolved.type = sym.type;
                resolved.binding = sym.binding;
                resolved.flags = sym.flags;
                resolved.object_index = obj_idx;
                resolved.section_index = sym.section_index;

                // Track global symbols for extern resolution.
                if (sym.binding == symbol_binding::global ||
                    sym.binding == symbol_binding::weak)
                {
                    global_symbol_map[sym.name] = symbols.size();
                }

                symbols.push_back(std::move(resolved));
            }
        }

        // Second pass: Resolve extern symbols.
        for (std::size_t obj_idx = 0; obj_idx < objects.size(); ++obj_idx)
        {
            const auto& obj = objects[obj_idx];
            const auto& obj_symbols = obj.get_symbols();

            for (const auto& sym : obj_symbols)
            {
                if (sym.binding != symbol_binding::extern_)
                {
                    continue;
                }

                // Look for a global definition.
                auto it = global_symbol_map.find(sym.name);
                if (it == global_symbol_map.end())
                {
                    return error(
                        "Unresolved external symbol '{}' in object {}",
                        sym.name, obj_idx
                    );
                }

                // The extern symbol is resolved - we don't need to add it again
                // since relocations will reference the global definition.
            }
        }

        return {};
    }

    auto program::collect_sections (
        const std::vector<object>& objects,
        std::vector<link_section>& sections
    ) -> result<void>
    {
        // Collect all non-null sections from each object file.
        for (std::size_t obj_idx = 0; obj_idx < objects.size(); ++obj_idx)
        {
            const auto& obj = objects[obj_idx];
            const auto& obj_sections = obj.get_sections();

            for (std::size_t sec_idx = 0; sec_idx < obj_sections.size(); ++sec_idx)
            {
                const auto& sec = obj_sections[sec_idx];

                // Skip null sections.
                if (sec.type == section_type::null_)
                {
                    continue;
                }

                // Create a link section with a copy of the data.
                link_section link_sec;
                link_sec.object_index = obj_idx;
                link_sec.section_index = sec_idx;
                link_sec.address = sec.virtual_address;
                link_sec.type = sec.type;
                link_sec.flags = sec.flags;

                // Copy section data (BSS sections have no data).
                if (sec.type != section_type::bss)
                {
                    link_sec.data = sec.data;
                }
                else
                {
                    // BSS sections just reserve space; store size for reference.
                    link_sec.data.resize(0);
                }

                sections.push_back(std::move(link_sec));
            }
        }

        // Sort sections by address for segment generation.
        std::sort(sections.begin(), sections.end(),
            [](const link_section& a, const link_section& b)
            {
                return a.address < b.address;
            }
        );

        return {};
    }

    auto program::apply_relocations (
        const std::vector<object>& objects,
        const std::vector<resolved_symbol>& symbols,
        std::vector<link_section>& sections
    ) -> result<void>
    {
        // Build a map to quickly find resolved symbols by name.
        std::unordered_map<std::string, std::size_t> symbol_map;
        for (std::size_t i = 0; i < symbols.size(); ++i)
        {
            const auto& sym = symbols[i];
            // Only map global/weak symbols; locals are object-specific.
            if (sym.binding == symbol_binding::global ||
                sym.binding == symbol_binding::weak)
            {
                symbol_map[sym.name] = i;
            }
        }

        // Build a map to find link_section index from (obj_idx, sec_idx).
        std::unordered_map<std::uint64_t, std::size_t> section_map;
        for (std::size_t i = 0; i < sections.size(); ++i)
        {
            const auto& sec = sections[i];
            const std::uint64_t key =
                (static_cast<std::uint64_t>(sec.object_index) << 32) |
                sec.section_index;
            section_map[key] = i;
        }

        // Process relocations from each object file.
        for (std::size_t obj_idx = 0; obj_idx < objects.size(); ++obj_idx)
        {
            const auto& obj = objects[obj_idx];
            const auto& obj_relocs = obj.get_relocations();
            const auto& obj_symbols = obj.get_symbols();

            for (const auto& reloc : obj_relocs)
            {
                // Find the target section in our link sections.
                const std::uint64_t sec_key =
                    (static_cast<std::uint64_t>(obj_idx) << 32) |
                    reloc.section_index;
                auto sec_it = section_map.find(sec_key);
                if (sec_it == section_map.end())
                {
                    return error(
                        "Relocation references unknown section {} in object {}",
                        reloc.section_index, obj_idx
                    );
                }

                auto& target_section = sections[sec_it->second];

                // Get the symbol this relocation references.
                if (reloc.symbol_index >= obj_symbols.size())
                {
                    return error(
                        "Relocation references invalid symbol index {} in "
                        "object {}",
                        reloc.symbol_index, obj_idx
                    );
                }

                const auto& ref_sym = obj_symbols[reloc.symbol_index];

                // Resolve the symbol's final address.
                std::uint32_t symbol_address = 0;

                if (ref_sym.binding == symbol_binding::extern_)
                {
                    // Look up the global definition.
                    auto it = symbol_map.find(ref_sym.name);
                    if (it == symbol_map.end())
                    {
                        return error(
                            "Unresolved external '{}' for relocation in "
                            "object {}",
                            ref_sym.name, obj_idx
                        );
                    }
                    symbol_address = symbols[it->second].address;
                }
                else if (ref_sym.binding == symbol_binding::global ||
                         ref_sym.binding == symbol_binding::weak)
                {
                    // Use the global symbol map.
                    auto it = symbol_map.find(ref_sym.name);
                    if (it != symbol_map.end())
                    {
                        symbol_address = symbols[it->second].address;
                    }
                    else
                    {
                        // Calculate from local object data.
                        symbol_address = ref_sym.value;
                        if (ref_sym.section_index < obj.get_sections().size())
                        {
                            symbol_address +=
                                obj.get_sections()[ref_sym.section_index].virtual_address;
                        }
                    }
                }
                else
                {
                    // Local symbol - calculate address from object data.
                    symbol_address = ref_sym.value;
                    if (ref_sym.section_index < obj.get_sections().size())
                    {
                        symbol_address +=
                            obj.get_sections()[ref_sym.section_index].virtual_address;
                    }
                }

                // Add the relocation addend.
                const std::int32_t final_value =
                    static_cast<std::int32_t>(symbol_address) + reloc.addend;

                // Calculate PC-relative offset if needed.
                const std::uint32_t reloc_address =
                    target_section.address + reloc.offset;

                // Validate relocation offset is within section data.
                if (reloc.offset >= target_section.data.size())
                {
                    return error(
                        "Relocation offset {} exceeds section size {} in "
                        "object {}",
                        reloc.offset, target_section.data.size(), obj_idx
                    );
                }

                // Apply the relocation based on type.
                switch (reloc.type)
                {
                case relocation_type::abs32:
                    // 32-bit absolute address.
                    if (reloc.offset + 4 > target_section.data.size())
                    {
                        return error(
                            "ABS32 relocation at offset {} exceeds section "
                            "bounds in object {}",
                            reloc.offset, obj_idx
                        );
                    }
                    write_u32_le(target_section.data, reloc.offset,
                        static_cast<std::uint32_t>(final_value));
                    break;

                case relocation_type::abs16:
                    // 16-bit absolute address (truncated).
                    if (reloc.offset + 2 > target_section.data.size())
                    {
                        return error(
                            "ABS16 relocation at offset {} exceeds section "
                            "bounds in object {}",
                            reloc.offset, obj_idx
                        );
                    }
                    write_u16_le(target_section.data, reloc.offset,
                        static_cast<std::uint16_t>(final_value & 0xFFFF));
                    break;

                case relocation_type::abs8:
                    // 8-bit absolute address (truncated).
                    target_section.data[reloc.offset] =
                        static_cast<std::uint8_t>(final_value & 0xFF);
                    break;

                case relocation_type::rel32:
                {
                    // 32-bit PC-relative offset.
                    // Offset is calculated from the end of the relocation field.
                    const std::int32_t pc_offset =
                        final_value - static_cast<std::int32_t>(reloc_address + 4);
                    if (reloc.offset + 4 > target_section.data.size())
                    {
                        return error(
                            "REL32 relocation at offset {} exceeds section "
                            "bounds in object {}",
                            reloc.offset, obj_idx
                        );
                    }
                    write_u32_le(target_section.data, reloc.offset,
                        static_cast<std::uint32_t>(pc_offset));
                    break;
                }

                case relocation_type::rel16:
                {
                    // 16-bit PC-relative offset.
                    const std::int32_t pc_offset =
                        final_value - static_cast<std::int32_t>(reloc_address + 2);
                    if (reloc.offset + 2 > target_section.data.size())
                    {
                        return error(
                            "REL16 relocation at offset {} exceeds section "
                            "bounds in object {}",
                            reloc.offset, obj_idx
                        );
                    }
                    write_u16_le(target_section.data, reloc.offset,
                        static_cast<std::uint16_t>(pc_offset & 0xFFFF));
                    break;
                }

                case relocation_type::rel8:
                {
                    // 8-bit PC-relative offset.
                    const std::int32_t pc_offset =
                        final_value - static_cast<std::int32_t>(reloc_address + 1);
                    target_section.data[reloc.offset] =
                        static_cast<std::uint8_t>(pc_offset & 0xFF);
                    break;
                }

                case relocation_type::quick16:
                {
                    // 16-bit offset relative to $FFFF0000.
                    const std::int32_t quick_offset =
                        final_value - static_cast<std::int32_t>(0xFFFF0000);
                    if (reloc.offset + 2 > target_section.data.size())
                    {
                        return error(
                            "QUICK16 relocation at offset {} exceeds section "
                            "bounds in object {}",
                            reloc.offset, obj_idx
                        );
                    }
                    write_u16_le(target_section.data, reloc.offset,
                        static_cast<std::uint16_t>(quick_offset & 0xFFFF));
                    break;
                }

                case relocation_type::port8:
                {
                    // 8-bit offset relative to $FFFFFF00.
                    const std::int32_t port_offset =
                        final_value - static_cast<std::int32_t>(0xFFFFFF00);
                    target_section.data[reloc.offset] =
                        static_cast<std::uint8_t>(port_offset & 0xFF);
                    break;
                }

                case relocation_type::none:
                default:
                    // No relocation needed.
                    break;
                }
            }
        }

        return {};
    }

    auto program::generate_segments (
        const std::vector<link_section>& sections
    ) -> result<void>
    {
        // Clear existing segments.
        m_segments.clear();

        if (sections.empty())
        {
            return {};
        }

        // Group sections into segments by type and contiguity.
        // Sections are already sorted by address.

        for (const auto& sec : sections)
        {
            // Skip empty non-BSS sections.
            if (sec.type != section_type::bss && sec.data.empty())
            {
                continue;
            }

            // Determine the segment type based on section type.
            segment_type seg_type = segment_type::null_;
            segment_flags seg_flags = segment_flags::none;

            switch (sec.type)
            {
            case section_type::code:
                seg_type = segment_type::code;
                seg_flags = segment_flags::load | segment_flags::exec;
                break;

            case section_type::data:
                seg_type = segment_type::data;
                seg_flags = segment_flags::load;
                break;

            case section_type::bss:
                seg_type = segment_type::bss;
                seg_flags = segment_flags::zero_fill | segment_flags::write;
                break;

            case section_type::null_:
            default:
                continue;  // Skip null sections.
            }

            // Determine if this section is in a special region.
            // Note: sec.address is unsigned, so >= 0 is always true; we only
            // need to check the upper bound for the metadata region.
            if (sec.address <= 0x00000FFF)
            {
                seg_type = segment_type::metadata;
            }
            else if (sec.address >= 0x00001000 && sec.address <= 0x00001FFF)
            {
                seg_type = segment_type::interrupt;
                seg_flags = segment_flags::load | segment_flags::exec;
            }

            // Try to merge with the last segment if compatible.
            bool merged = false;
            if (m_segments.empty() == false)
            {
                auto& last = m_segments.back();
                const std::uint32_t last_end =
                    last.load_address + last.memory_size;

                // Check if this section is contiguous with and compatible with
                // the last segment.
                if (sec.address == last_end &&
                    last.type == seg_type &&
                    sec.type != section_type::bss)
                {
                    // Merge: append data and update size.
                    last.data.insert(last.data.end(),
                        sec.data.begin(), sec.data.end());
                    last.memory_size += static_cast<std::uint32_t>(sec.data.size());
                    merged = true;
                }
            }

            if (merged == false)
            {
                // Create a new segment.
                program_segment segment;
                segment.load_address = sec.address;
                segment.type = seg_type;
                segment.flags = seg_flags;

                if (sec.type == section_type::bss)
                {
                    // BSS segments have memory_size from the object but no data.
                    // We need to get the original section size from the object.
                    // For now, we'll use a placeholder - actual size comes from
                    // the object's section.
                    segment.memory_size = 0;  // Will be set from object data.
                    segment.data.clear();
                }
                else
                {
                    segment.memory_size = static_cast<std::uint32_t>(sec.data.size());
                    segment.data = sec.data;
                }

                m_segments.push_back(std::move(segment));
            }
        }

        return {};
    }

    auto program::find_entry_point (
        const std::vector<resolved_symbol>& symbols
    ) -> result<void>
    {
        // Look for a symbol marked with the entry flag, or named "main" or
        // "_start".
        const resolved_symbol* entry_symbol = nullptr;

        // First, look for a symbol with the entry flag set.
        for (const auto& sym : symbols)
        {
            if ((sym.flags & symbol_flags::entry) != symbol_flags::none)
            {
                entry_symbol = &sym;
                break;
            }
        }

        // If no explicit entry symbol, look for "main".
        if (entry_symbol == nullptr)
        {
            for (const auto& sym : symbols)
            {
                if (sym.name == "main" &&
                    (sym.binding == symbol_binding::global ||
                     sym.binding == symbol_binding::weak))
                {
                    entry_symbol = &sym;
                    break;
                }
            }
        }

        // If no "main", look for "_start".
        if (entry_symbol == nullptr)
        {
            for (const auto& sym : symbols)
            {
                if (sym.name == "_start" &&
                    (sym.binding == symbol_binding::global ||
                     sym.binding == symbol_binding::weak))
                {
                    entry_symbol = &sym;
                    break;
                }
            }
        }

        // Set the entry point if found.
        if (entry_symbol != nullptr)
        {
            m_entry_point = entry_symbol->address;
            m_flags = m_flags | program_flags::has_entry;
        }
        else
        {
            return error(
                "No entry point symbol found.\n"
                " - Are you missing a label named \"main\" or \"_start\"?"
            );
        }

        return {};
    }
}
