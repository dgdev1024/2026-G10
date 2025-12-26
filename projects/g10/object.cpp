/**
 * @file    g10/object.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-26
 * 
 * @brief   Contains implementations for the G10 CPU object file.
 */

/* Private Includes ***********************************************************/

#include <g10/object.hpp>

/* Private Helper Functions ***************************************************/

namespace g10
{
    /**
     * @brief   Reads a little-endian 16-bit value from a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to read from.
     * @param   offset  The offset in bytes within the buffer.
     * 
     * @return  The 16-bit value in native byte order.
     */
    inline auto read_u16_le (
        std::span<const std::uint8_t> buffer,
        std::size_t offset
    ) -> std::uint16_t
    {
        return static_cast<std::uint16_t>(buffer[offset]) |
               (static_cast<std::uint16_t>(buffer[offset + 1]) << 8);
    }

    /**
     * @brief   Reads a little-endian 32-bit value from a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to read from.
     * @param   offset  The offset in bytes within the buffer.
     * 
     * @return  The 32-bit value in native byte order.
     */
    inline auto read_u32_le (
        std::span<const std::uint8_t> buffer,
        std::size_t offset
    ) -> std::uint32_t
    {
        return static_cast<std::uint32_t>(buffer[offset]) |
               (static_cast<std::uint32_t>(buffer[offset + 1]) << 8) |
               (static_cast<std::uint32_t>(buffer[offset + 2]) << 16) |
               (static_cast<std::uint32_t>(buffer[offset + 3]) << 24);
    }

    /**
     * @brief   Writes a little-endian 16-bit value to a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to write to.
     * @param   offset  The offset in bytes within the buffer.
     * @param   value   The 16-bit value to write in native byte order.
     */
    inline auto write_u16_le (
        std::span<std::uint8_t> buffer,
        std::size_t offset,
        std::uint16_t value
    ) -> void
    {
        buffer[offset] = static_cast<std::uint8_t>(value & 0xFF);
        buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    }

    /**
     * @brief   Writes a little-endian 32-bit value to a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to write to.
     * @param   offset  The offset in bytes within the buffer.
     * @param   value   The 32-bit value to write in native byte order.
     */
    inline auto write_u32_le (
        std::span<std::uint8_t> buffer,
        std::size_t offset,
        std::uint32_t value
    ) -> void
    {
        buffer[offset] = static_cast<std::uint8_t>(value & 0xFF);
        buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        buffer[offset + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
        buffer[offset + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
    }

    /**
     * @brief   Reads a null-terminated string from the string table.
     * 
     * @param   string_table    The string table data.
     * @param   offset          The offset into the string table.
     * 
     * @return  The string at the given offset, or empty if invalid.
     */
    auto read_string_from_table (
        const std::vector<std::uint8_t>& string_table,
        std::uint32_t offset
    ) -> std::string
    {
        if (offset >= string_table.size())
        {
            return "";
        }

        std::string result;
        for (std::size_t i = offset; i < string_table.size(); ++i)
        {
            if (string_table[i] == 0)
            {
                break;
            }
            result.push_back(static_cast<char>(string_table[i]));
        }

        return result;
    }
}

/* Public Methods *************************************************************/

namespace g10
{
    object::object (const fs::path& path)
    {
        load_from_file(path);
    }

    auto object::clear () -> void
    {
        m_good = false;
        m_flags = object_flags::none;
        m_sections.clear();
        m_symbols.clear();
        m_relocations.clear();
    }

    auto object::load_from_file (const fs::path& path) -> result<void>
    {
        // Clear any existing data.
        clear();

        // Open the file for binary reading.
        std::ifstream file { path, std::ios::binary | std::ios::ate };
        if (!file.is_open())
        {
            return error("Failed to open file '{}' for reading.",
                path.string());
        }

        // Get file size and read entire file into memory.
        const auto file_size = static_cast<std::size_t>(file.tellg());
        if (file_size < OBJECT_HEADER_SIZE)
        {
            return error("File '{}' is too small to be a valid object file"
                " ({} bytes, minimum {} bytes required).",
                path.string(), file_size, OBJECT_HEADER_SIZE);
        }

        file.seekg(0, std::ios::beg);
        std::vector<std::uint8_t> buffer(file_size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size))
        {
            return error("Failed to read file '{}'.", path.string());
        }
        file.close();

        // Create a span for safe, bounds-checked access.
        std::span<const std::uint8_t> file_data { buffer };

        // Read and validate the header.
        const std::uint32_t magic = read_u32_le(file_data, 0x00);
        if (magic != OBJECT_MAGIC)
        {
            return error("File '{}' has invalid magic number "
                "(expected 0x{:08X}, got 0x{:08X}).",
                path.string(), OBJECT_MAGIC, magic);
        }

        const std::uint32_t version = read_u32_le(file_data, 0x04);
        const std::uint8_t version_major = (version >> 24) & 0xFF;
        const std::uint8_t expected_major = (OBJECT_VERSION >> 24) & 0xFF;
        if (version_major != expected_major)
        {
            return error("File '{}' has incompatible major version "
                "(expected {}, got {}).",
                path.string(), expected_major, version_major);
        }

        m_flags = static_cast<object_flags>(read_u32_le(file_data, 0x08));
        const std::uint32_t section_count = read_u32_le(file_data, 0x0C);
        const std::uint32_t symbol_table_offset = read_u32_le(file_data, 0x10);
        const std::uint32_t symbol_table_size = read_u32_le(file_data, 0x14);
        const std::uint32_t string_table_offset = read_u32_le(file_data, 0x18);
        const std::uint32_t string_table_size = read_u32_le(file_data, 0x1C);
        const std::uint32_t reloc_table_offset = read_u32_le(file_data, 0x20);
        const std::uint32_t reloc_table_size = read_u32_le(file_data, 0x24);

        // Validate table offsets and sizes against file size.
        const std::size_t section_headers_end = OBJECT_HEADER_SIZE +
            (section_count * OBJECT_SECTION_HEADER_SIZE);
        if (section_headers_end > file_size)
        {
            return error("Section headers extend beyond file size.");
        }

        if (string_table_offset > 0)
        {
            if (string_table_offset + string_table_size > file_size)
            {
                return error("String table extends beyond file size.");
            }
        }

        if (symbol_table_offset > 0)
        {
            const std::size_t symbol_table_bytes = 
                symbol_table_size * OBJECT_SYMBOL_ENTRY_SIZE;
            if (symbol_table_offset + symbol_table_bytes > file_size)
            {
                return error("Symbol table extends beyond file size.");
            }
        }

        if (reloc_table_offset > 0)
        {
            const std::size_t reloc_table_bytes =
                reloc_table_size * OBJECT_RELOCATION_ENTRY_SIZE;
            if (reloc_table_offset + reloc_table_bytes > file_size)
            {
                return error("Relocation table extends beyond file size.");
            }
        }

        // Read the string table first (needed for section/symbol names).
        std::vector<std::uint8_t> string_table;
        if (string_table_offset > 0 && string_table_size > 0)
        {
            string_table.assign(
                buffer.begin() + string_table_offset,
                buffer.begin() + string_table_offset + string_table_size
            );
        }

        // Read section headers and data.
        std::size_t data_offset = section_headers_end;
        for (std::uint32_t i = 0; i < section_count; ++i)
        {
            const std::size_t header_offset = OBJECT_HEADER_SIZE +
                (i * OBJECT_SECTION_HEADER_SIZE);

            object_section section;
            const std::uint32_t name_offset = read_u32_le(file_data, header_offset + 0x00);
            section.name = read_string_from_table(string_table, name_offset);
            section.virtual_address = read_u32_le(file_data, header_offset + 0x04);
            const std::uint32_t size = read_u32_le(file_data, header_offset + 0x08);
            section.type = static_cast<section_type>(
                read_u16_le(file_data, header_offset + 0x0C));
            section.flags = static_cast<section_flags>(
                read_u16_le(file_data, header_offset + 0x0E));

            // Read section data if LOAD flag is set (non-BSS sections).
            if ((section.flags & section_flags::load) != section_flags::none)
            {
                if (data_offset + size > file_size)
                {
                    return error("Section {} data extends beyond file size.",
                        i);
                }
                section.data.assign(
                    buffer.begin() + data_offset,
                    buffer.begin() + data_offset + size
                );
                data_offset += size;
            }
            else
            {
                // BSS section: no data in file, but reserve size.
                section.data.resize(size, 0);
            }

            m_sections.push_back(std::move(section));
        }

        // Read symbols.
        if (symbol_table_offset > 0 && symbol_table_size > 0)
        {
            for (std::uint32_t i = 0; i < symbol_table_size; ++i)
            {
                const std::size_t entry_offset = symbol_table_offset +
                    (i * OBJECT_SYMBOL_ENTRY_SIZE);

                object_symbol symbol;
                const std::uint32_t name_offset = read_u32_le(file_data, entry_offset + 0x00);
                symbol.name = read_string_from_table(string_table, name_offset);
                symbol.value = read_u32_le(file_data, entry_offset + 0x04);
                symbol.section_index = read_u32_le(file_data, entry_offset + 0x08);
                symbol.type = static_cast<symbol_type>(file_data[entry_offset + 0x0C]);
                symbol.binding = static_cast<symbol_binding>(file_data[entry_offset + 0x0D]);
                symbol.flags = static_cast<symbol_flags>(
                    read_u16_le(file_data, entry_offset + 0x0E));

                m_symbols.push_back(std::move(symbol));
            }
        }

        // Read relocations.
        if (reloc_table_offset > 0 && reloc_table_size > 0)
        {
            for (std::uint32_t i = 0; i < reloc_table_size; ++i)
            {
                const std::size_t entry_offset = reloc_table_offset +
                    (i * OBJECT_RELOCATION_ENTRY_SIZE);

                object_relocation reloc;
                reloc.offset = read_u32_le(file_data, entry_offset + 0x00);
                reloc.symbol_index = read_u32_le(file_data, entry_offset + 0x04);
                reloc.section_index = read_u32_le(file_data, entry_offset + 0x08);
                reloc.type = static_cast<relocation_type>(
                    read_u16_le(file_data, entry_offset + 0x0C));
                reloc.addend = static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        read_u16_le(file_data, entry_offset + 0x0E)));

                m_relocations.push_back(std::move(reloc));
            }
        }

        // Validate the loaded data.
        return validate();
    }

    auto object::save_to_file (const fs::path& path) -> result<bool>
    {
        // Build the string table and get the offset map.
        auto [string_table, string_offsets] = build_string_table();

        // Calculate sizes and offsets.
        const std::uint32_t section_count = 
            static_cast<std::uint32_t>(m_sections.size());
        const std::uint32_t symbol_count =
            static_cast<std::uint32_t>(m_symbols.size());
        const std::uint32_t reloc_count =
            static_cast<std::uint32_t>(m_relocations.size());

        // Calculate section data size (only for LOAD sections).
        std::size_t section_data_size = 0;
        for (const auto& section : m_sections)
        {
            if ((section.flags & section_flags::load) != section_flags::none)
            {
                section_data_size += section.data.size();
            }
        }

        // Calculate offsets.
        const std::size_t section_headers_offset = OBJECT_HEADER_SIZE;
        const std::size_t section_data_offset = section_headers_offset +
            (section_count * OBJECT_SECTION_HEADER_SIZE);
        const std::size_t symbol_table_offset = section_data_offset +
            section_data_size;
        const std::size_t string_table_offset = symbol_table_offset +
            (symbol_count * OBJECT_SYMBOL_ENTRY_SIZE);
        const std::size_t reloc_table_offset = string_table_offset +
            string_table.size();
        const std::size_t total_size = reloc_table_offset +
            (reloc_count * OBJECT_RELOCATION_ENTRY_SIZE);

        // Allocate the output buffer.
        std::vector<std::uint8_t> buffer(total_size, 0);
        std::span<std::uint8_t> file_data { buffer };

        // Write the header.
        write_u32_le(file_data, 0x00, OBJECT_MAGIC);
        write_u32_le(file_data, 0x04, OBJECT_VERSION);
        write_u32_le(file_data, 0x08, static_cast<std::uint32_t>(m_flags));
        write_u32_le(file_data, 0x0C, section_count);
        write_u32_le(file_data, 0x10, symbol_count > 0 ?
            static_cast<std::uint32_t>(symbol_table_offset) : 0);
        write_u32_le(file_data, 0x14, symbol_count);
        write_u32_le(file_data, 0x18, !string_table.empty() ?
            static_cast<std::uint32_t>(string_table_offset) : 0);
        write_u32_le(file_data, 0x1C, static_cast<std::uint32_t>(string_table.size()));
        write_u32_le(file_data, 0x20, reloc_count > 0 ?
            static_cast<std::uint32_t>(reloc_table_offset) : 0);
        write_u32_le(file_data, 0x24, reloc_count);
        // Reserved bytes (0x28 - 0x3F) are already zero.

        // Write section headers.
        for (std::uint32_t i = 0; i < section_count; ++i)
        {
            const auto& section = m_sections[i];
            const std::size_t header_offset = section_headers_offset +
                (i * OBJECT_SECTION_HEADER_SIZE);

            const std::uint32_t name_offset = section.name.empty() ? 0 :
                string_offsets.at(section.name);
            write_u32_le(file_data, header_offset + 0x00, name_offset);
            write_u32_le(file_data, header_offset + 0x04, section.virtual_address);
            write_u32_le(file_data, header_offset + 0x08, 
                static_cast<std::uint32_t>(section.data.size()));
            write_u16_le(file_data, header_offset + 0x0C, 
                static_cast<std::uint16_t>(section.type));
            write_u16_le(file_data, header_offset + 0x0E, 
                static_cast<std::uint16_t>(section.flags));
        }

        // Write section data (for LOAD sections only).
        std::size_t current_data_offset = section_data_offset;
        for (const auto& section : m_sections)
        {
            if ((section.flags & section_flags::load) != section_flags::none)
            {
                std::memcpy(
                    buffer.data() + current_data_offset,
                    section.data.data(),
                    section.data.size()
                );
                current_data_offset += section.data.size();
            }
        }

        // Write symbol table.
        for (std::uint32_t i = 0; i < symbol_count; ++i)
        {
            const auto& symbol = m_symbols[i];
            const std::size_t entry_offset = symbol_table_offset +
                (i * OBJECT_SYMBOL_ENTRY_SIZE);

            const std::uint32_t name_offset = symbol.name.empty() ? 0 :
                string_offsets.at(symbol.name);
            write_u32_le(file_data, entry_offset + 0x00, name_offset);
            write_u32_le(file_data, entry_offset + 0x04, symbol.value);
            write_u32_le(file_data, entry_offset + 0x08, symbol.section_index);
            file_data[entry_offset + 0x0C] = static_cast<std::uint8_t>(symbol.type);
            file_data[entry_offset + 0x0D] = static_cast<std::uint8_t>(symbol.binding);
            write_u16_le(file_data, entry_offset + 0x0E, 
                static_cast<std::uint16_t>(symbol.flags));
        }

        // Write string table.
        if (!string_table.empty())
        {
            std::memcpy(
                buffer.data() + string_table_offset,
                string_table.data(),
                string_table.size()
            );
        }

        // Write relocation table.
        for (std::uint32_t i = 0; i < reloc_count; ++i)
        {
            const auto& reloc = m_relocations[i];
            const std::size_t entry_offset = reloc_table_offset +
                (i * OBJECT_RELOCATION_ENTRY_SIZE);

            write_u32_le(file_data, entry_offset + 0x00, reloc.offset);
            write_u32_le(file_data, entry_offset + 0x04, reloc.symbol_index);
            write_u32_le(file_data, entry_offset + 0x08, reloc.section_index);
            write_u16_le(file_data, entry_offset + 0x0C, 
                static_cast<std::uint16_t>(reloc.type));
            write_u16_le(file_data, entry_offset + 0x0E, 
                static_cast<std::uint16_t>(reloc.addend & 0xFFFF));
        }

        // Write the buffer to file.
        std::ofstream file { path, std::ios::binary | std::ios::trunc };
        if (!file.is_open())
        {
            return error("Failed to open file '{}' for writing.",
                path.string());
        }

        if (!file.write(reinterpret_cast<const char*>(buffer.data()), 
            buffer.size()))
        {
            return error("Failed to write to file '{}'.", path.string());
        }

        file.close();
        m_good = true;

        // Return true if object has content, false if empty (but still valid).
        return !m_sections.empty() || !m_symbols.empty();
    }
}

/* Public Methods - Object Building *******************************************/

namespace g10
{
    auto object::set_flags (object_flags flags) -> void
    {
        m_flags = flags;
    }

    auto object::add_section (const object_section& section) -> std::size_t
    {
        m_sections.push_back(section);
        return m_sections.size() - 1;
    }

    auto object::add_symbol (const object_symbol& symbol) -> result<std::size_t>
    {
        // Check for symbol scoping rule violations.
        for (std::size_t i = 0; i < m_symbols.size(); ++i)
        {
            const auto& existing = m_symbols[i];
            if (existing.name == symbol.name)
            {
                // Check if existing symbol is global - cannot redefine.
                if (existing.binding == symbol_binding::global)
                {
                    return error("Cannot redefine global symbol '{}'.",
                        symbol.name);
                }

                // Check if new symbol is trying to define an extern.
                if (existing.binding == symbol_binding::extern_)
                {
                    if (symbol.binding != symbol_binding::extern_)
                    {
                        return error("Cannot define extern symbol '{}' "
                            "within the same object file.", symbol.name);
                    }
                }

                // Check for global/extern mutual exclusivity.
                if ((existing.binding == symbol_binding::global &&
                     symbol.binding == symbol_binding::extern_) ||
                    (existing.binding == symbol_binding::extern_ &&
                     symbol.binding == symbol_binding::global))
                {
                    return error("Symbol '{}' cannot be both global and "
                        "extern.", symbol.name);
                }

                // Local symbols can be redefined - update existing.
                if (existing.binding == symbol_binding::local_ &&
                    symbol.binding == symbol_binding::local_)
                {
                    m_symbols[i] = symbol;
                    return i;
                }
            }
        }

        // New symbol being marked as both global and extern (different name).
        // This is checked by seeing if it's trying to define an extern.
        if (symbol.binding == symbol_binding::extern_ &&
            symbol.section_index != SECTION_INDEX_UNDEF)
        {
            return error("Extern symbol '{}' cannot have a section index "
                "(must be undefined).", symbol.name);
        }

        // Add the new symbol.
        m_symbols.push_back(symbol);
        return m_symbols.size() - 1;
    }

    auto object::add_relocation (const object_relocation& reloc)
        -> result<std::size_t>
    {
        // Validate section index.
        if (reloc.section_index >= m_sections.size())
        {
            return error("Relocation references invalid section index {}.",
                reloc.section_index);
        }

        // Validate symbol index.
        if (reloc.symbol_index >= m_symbols.size())
        {
            return error("Relocation references invalid symbol index {}.",
                reloc.symbol_index);
        }

        // Validate offset is within section bounds.
        const auto& section = m_sections[reloc.section_index];
        if (reloc.offset >= section.data.size())
        {
            return error("Relocation offset {} is beyond section size {}.",
                reloc.offset, section.data.size());
        }

        m_relocations.push_back(reloc);
        return m_relocations.size() - 1;
    }

    auto object::find_symbol (const std::string& name) const
        -> std::optional<std::size_t>
    {
        for (std::size_t i = 0; i < m_symbols.size(); ++i)
        {
            if (m_symbols[i].name == name)
            {
                return i;
            }
        }
        return std::nullopt;
    }

    auto object::find_section (const std::string& name) const
        -> std::optional<std::size_t>
    {
        for (std::size_t i = 0; i < m_sections.size(); ++i)
        {
            if (m_sections[i].name == name)
            {
                return i;
            }
        }
        return std::nullopt;
    }
}

/* Private Methods ************************************************************/

namespace g10
{
    auto object::build_string_table () const
        -> std::pair<std::vector<std::uint8_t>,
            std::unordered_map<std::string, std::uint32_t>>
    {
        std::vector<std::uint8_t> table;
        std::unordered_map<std::string, std::uint32_t> offsets;

        // String table must start with a null byte (empty string at offset 0).
        table.push_back(0);
        offsets[""] = 0;

        // Helper lambda to add a string if not already present.
        auto add_string = [&] (const std::string& str)
        {
            if (str.empty() || offsets.contains(str))
            {
                return;
            }

            const std::uint32_t offset = static_cast<std::uint32_t>(table.size());
            offsets[str] = offset;

            // Append string bytes and null terminator.
            for (char c : str)
            {
                table.push_back(static_cast<std::uint8_t>(c));
            }
            table.push_back(0);
        };

        // Add section names.
        for (const auto& section : m_sections)
        {
            add_string(section.name);
        }

        // Add symbol names.
        for (const auto& symbol : m_symbols)
        {
            add_string(symbol.name);
        }

        return { std::move(table), std::move(offsets) };
    }

    auto object::validate () -> result<void>
    {
        // Validate sections.
        for (std::size_t i = 0; i < m_sections.size(); ++i)
        {
            const auto& section = m_sections[i];

            // Check section type is valid.
            if (section.type != section_type::null_ &&
                section.type != section_type::code &&
                section.type != section_type::data &&
                section.type != section_type::bss)
            {
                return error("Section {} has invalid type 0x{:04X}.",
                    i, static_cast<std::uint16_t>(section.type));
            }

            // Check ROM/RAM region consistency.
            const bool is_rom_region = (section.virtual_address & 0x80000000) == 0;
            if (section.type == section_type::bss && is_rom_region)
            {
                return error("BSS section {} has address 0x{:08X} in ROM "
                    "region (should be >= 0x80000000).",
                    i, section.virtual_address);
            }
            if ((section.type == section_type::code ||
                 section.type == section_type::data) && !is_rom_region)
            {
                return error("CODE/DATA section {} has address 0x{:08X} in "
                    "RAM region (should be < 0x80000000).",
                    i, section.virtual_address);
            }
        }

        // Check for section overlaps.
        for (std::size_t i = 0; i < m_sections.size(); ++i)
        {
            const auto& sec_a = m_sections[i];
            if (sec_a.type == section_type::null_ || sec_a.data.empty())
            {
                continue;
            }

            const std::uint32_t a_start = sec_a.virtual_address;
            const std::uint32_t a_end = a_start + 
                static_cast<std::uint32_t>(sec_a.data.size());

            for (std::size_t j = i + 1; j < m_sections.size(); ++j)
            {
                const auto& sec_b = m_sections[j];
                if (sec_b.type == section_type::null_ || sec_b.data.empty())
                {
                    continue;
                }

                const std::uint32_t b_start = sec_b.virtual_address;
                const std::uint32_t b_end = b_start +
                    static_cast<std::uint32_t>(sec_b.data.size());

                // Check for overlap.
                if (a_start < b_end && b_start < a_end)
                {
                    return error("Sections {} and {} overlap "
                        "(0x{:08X}-0x{:08X} and 0x{:08X}-0x{:08X}).",
                        i, j, a_start, a_end, b_start, b_end);
                }
            }
        }

        // Validate symbols.
        std::unordered_set<std::string> global_symbols;
        for (std::size_t i = 0; i < m_symbols.size(); ++i)
        {
            const auto& symbol = m_symbols[i];

            // Check for duplicate global definitions.
            if (symbol.binding == symbol_binding::global)
            {
                if (global_symbols.contains(symbol.name))
                {
                    return error("Duplicate global symbol '{}'.",
                        symbol.name);
                }
                global_symbols.insert(symbol.name);
            }

            // Check section index validity (unless special index).
            if (symbol.section_index != SECTION_INDEX_UNDEF &&
                symbol.section_index != SECTION_INDEX_ABS &&
                symbol.section_index != SECTION_INDEX_COMMON)
            {
                if (symbol.section_index >= m_sections.size())
                {
                    return error("Symbol '{}' references invalid section "
                        "index {}.", symbol.name, symbol.section_index);
                }
            }

            // Extern symbols must have UNDEF section index.
            if (symbol.binding == symbol_binding::extern_ &&
                symbol.section_index != SECTION_INDEX_UNDEF)
            {
                return error("Extern symbol '{}' must have undefined "
                    "section index.", symbol.name);
            }

            // Global symbols must have valid section index (not UNDEF).
            if (symbol.binding == symbol_binding::global &&
                symbol.section_index == SECTION_INDEX_UNDEF)
            {
                return error("Global symbol '{}' must be defined "
                    "(cannot have undefined section index).", symbol.name);
            }
        }

        // Validate relocations.
        for (std::size_t i = 0; i < m_relocations.size(); ++i)
        {
            const auto& reloc = m_relocations[i];

            // Check section index.
            if (reloc.section_index >= m_sections.size())
            {
                return error("Relocation {} references invalid section "
                    "index {}.", i, reloc.section_index);
            }

            // Check symbol index.
            if (reloc.symbol_index >= m_symbols.size())
            {
                return error("Relocation {} references invalid symbol "
                    "index {}.", i, reloc.symbol_index);
            }

            // Check offset is within section.
            const auto& section = m_sections[reloc.section_index];
            if (reloc.offset >= section.data.size())
            {
                return error("Relocation {} offset {} is beyond section "
                    "size {}.", i, reloc.offset, section.data.size());
            }

            // Check relocation type is valid.
            if (reloc.type != relocation_type::none &&
                reloc.type != relocation_type::abs32 &&
                reloc.type != relocation_type::abs16 &&
                reloc.type != relocation_type::abs8 &&
                reloc.type != relocation_type::rel32 &&
                reloc.type != relocation_type::rel16 &&
                reloc.type != relocation_type::rel8 &&
                reloc.type != relocation_type::quick16 &&
                reloc.type != relocation_type::port8)
            {
                return error("Relocation {} has invalid type 0x{:04X}.",
                    i, static_cast<std::uint16_t>(reloc.type));
            }
        }

        m_good = true;
        return {};
    }
}
