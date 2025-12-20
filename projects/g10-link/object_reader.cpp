/**
 * @file    g10-link/object_reader.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains implementations for reading G10 object files (.g10obj).
 */

/* Private Includes ***********************************************************/

#include <g10-link/object_reader.hpp>

/* Public Constructors ********************************************************/

namespace g10link
{
    object_reader::object_reader (const std::string& filename) :
        m_filename { filename },
        m_file {},
        m_header {},
        m_object {},
        m_string_table {},
        m_code_data {}
    {
    }
}

/* Public Methods *************************************************************/

namespace g10link
{
    auto object_reader::read () -> g10::result<g10obj::loaded_object_file>
    {
        // Open the file
        if (auto result = open_file(); !result)
        {
            return g10::error(result.error());
        }

        // Read and validate header
        if (auto result = read_header(); !result)
        {
            return g10::error(result.error());
        }

        // Read section table
        if (auto result = read_sections(); !result)
        {
            return g10::error(result.error());
        }

        // Read symbol table
        if (auto result = read_symbols(); !result)
        {
            return g10::error(result.error());
        }

        // Read relocation table
        if (auto result = read_relocations(); !result)
        {
            return g10::error(result.error());
        }

        // Read string table
        if (auto result = read_string_table(); !result)
        {
            return g10::error(result.error());
        }

        // Read code data
        if (auto result = read_code_data(); !result)
        {
            return g10::error(result.error());
        }

        // Resolve names from string table
        if (auto result = resolve_names(); !result)
        {
            return g10::error(result.error());
        }

        // Store the object filename
        m_object.object_filename = m_filename;

        return m_object;
    }
}

/* Private Methods - File Reading *********************************************/

namespace g10link
{
    auto object_reader::open_file () -> g10::result<void>
    {
        m_file.open(m_filename, std::ios::binary);
        if (!m_file.is_open())
        {
            return g10::error("Failed to open object file: {}", m_filename);
        }

        return {};
    }

    auto object_reader::read_header () -> g10::result<void>
    {
        // Read 32 bytes into header structure
        if (auto result = read_bytes(&m_header, g10obj::HEADER_SIZE); !result)
        {
            return g10::error("Failed to read file header: {}", result.error());
        }

        // Validate magic number
        if (m_header.magic != g10obj::MAGIC)
        {
            return g10::error("Invalid magic number: expected 0x{:08X}, got 0x{:08X}",
                g10obj::MAGIC, m_header.magic);
        }

        // Validate version
        if (m_header.version != g10obj::VERSION)
        {
            return g10::error("Unsupported object file version: 0x{:04X}", m_header.version);
        }

        // Store file flags
        m_object.flags = m_header.flags;

        return {};
    }

    auto object_reader::read_sections () -> g10::result<void>
    {
        m_object.sections.reserve(m_header.section_count);

        for (std::uint16_t i = 0; i < m_header.section_count; ++i)
        {
            g10obj::section_entry entry;
            if (auto result = read_bytes(&entry, g10obj::SECTION_ENTRY_SIZE); !result)
            {
                return g10::error("Failed to read section {}: {}", i, result.error());
            }

            g10obj::loaded_section section;
            section.base_address = entry.base_address;
            section.size = entry.size;
            section.flags = entry.flags;
            section.alignment = entry.alignment;
            section.data.reserve(entry.size);

            m_object.sections.push_back(std::move(section));
        }

        return {};
    }

    auto object_reader::read_symbols () -> g10::result<void>
    {
        m_object.symbols.reserve(m_header.symbol_count);

        for (std::uint16_t i = 0; i < m_header.symbol_count; ++i)
        {
            g10obj::symbol_entry entry;
            if (auto result = read_bytes(&entry, g10obj::SYMBOL_ENTRY_SIZE); !result)
            {
                return g10::error("Failed to read symbol {}: {}", i, result.error());
            }

            g10obj::loaded_symbol symbol;
            // Name will be resolved later from string table
            symbol.value = entry.value;
            symbol.section_index = entry.section_index;
            symbol.type = static_cast<g10obj::symbol_type>(entry.type);
            symbol.binding = static_cast<g10obj::symbol_binding>(entry.binding);
            symbol.size = entry.size;

            // Store the name offset temporarily in the name field (will be resolved later)
            symbol.name = std::to_string(entry.name_offset);

            m_object.symbols.push_back(std::move(symbol));
        }

        return {};
    }

    auto object_reader::read_relocations () -> g10::result<void>
    {
        m_object.relocations.reserve(m_header.relocation_count);

        for (std::uint32_t i = 0; i < m_header.relocation_count; ++i)
        {
            g10obj::relocation_entry entry;
            if (auto result = read_bytes(&entry, g10obj::RELOCATION_ENTRY_SIZE); !result)
            {
                return g10::error("Failed to read relocation {}: {}", i, result.error());
            }

            g10obj::loaded_relocation reloc;
            reloc.offset = entry.offset;
            reloc.section_index = entry.section_index;
            reloc.symbol_index = entry.symbol_index;
            reloc.type = static_cast<g10obj::relocation_type>(entry.type);
            reloc.addend = entry.addend;
            // Symbol name will be resolved later

            m_object.relocations.push_back(std::move(reloc));
        }

        return {};
    }

    auto object_reader::read_string_table () -> g10::result<void>
    {
        m_string_table.resize(m_header.string_table_size);

        if (auto result = read_bytes(m_string_table.data(), m_header.string_table_size); !result)
        {
            return g10::error("Failed to read string table: {}", result.error());
        }

        return {};
    }

    auto object_reader::read_code_data () -> g10::result<void>
    {
        m_code_data.resize(m_header.code_size);

        if (auto result = read_bytes(m_code_data.data(), m_header.code_size); !result)
        {
            return g10::error("Failed to read code data: {}", result.error());
        }

        // Distribute code data to sections based on their offsets
        for (auto& section : m_object.sections)
        {
            // Find this section's data in the code data block
            // Note: section.size is stored but we need to find the offset
            // We need to track offsets - they're in the original section_entry
        }

        // Re-read sections to get offsets and distribute code data
        m_file.seekg(g10obj::HEADER_SIZE, std::ios::beg);
        for (std::size_t i = 0; i < m_object.sections.size(); ++i)
        {
            g10obj::section_entry entry;
            if (auto result = read_bytes(&entry, g10obj::SECTION_ENTRY_SIZE); !result)
            {
                return g10::error("Failed to re-read section {} for offset: {}", i, result.error());
            }

            // Copy section data from code data block
            auto& section = m_object.sections[i];
            if (entry.size > 0 && entry.offset + entry.size <= m_code_data.size())
            {
                section.data.assign(
                    m_code_data.begin() + entry.offset,
                    m_code_data.begin() + entry.offset + entry.size
                );
            }
        }

        return {};
    }

    auto object_reader::resolve_names () -> g10::result<void>
    {
        // Resolve source filename
        auto source_name = get_string(m_header.source_name_offset);
        if (!source_name)
        {
            return g10::error("Failed to resolve source filename: {}", source_name.error());
        }
        m_object.source_filename = source_name.value();

        // Resolve symbol names
        for (auto& symbol : m_object.symbols)
        {
            // The name field temporarily holds the string offset as a string
            std::uint32_t offset = std::stoul(symbol.name);
            auto name = get_string(offset);
            if (!name)
            {
                return g10::error("Failed to resolve symbol name at offset {}: {}",
                    offset, name.error());
            }
            symbol.name = name.value();
        }

        // Resolve relocation symbol names
        for (auto& reloc : m_object.relocations)
        {
            if (reloc.symbol_index < m_object.symbols.size())
            {
                reloc.symbol_name = m_object.symbols[reloc.symbol_index].name;
            }
            else
            {
                return g10::error("Invalid symbol index {} in relocation",
                    reloc.symbol_index);
            }
        }

        return {};
    }
}

/* Private Methods - Binary Reading *******************************************/

namespace g10link
{
    auto object_reader::read_u8 () -> g10::result<std::uint8_t>
    {
        std::uint8_t value;
        if (auto result = read_bytes(&value, 1); !result)
        {
            return g10::error(result.error());
        }
        return value;
    }

    auto object_reader::read_u16 () -> g10::result<std::uint16_t>
    {
        std::uint8_t bytes[2];
        if (auto result = read_bytes(bytes, 2); !result)
        {
            return g10::error(result.error());
        }
        // Little-endian conversion
        return static_cast<std::uint16_t>(bytes[0]) |
               (static_cast<std::uint16_t>(bytes[1]) << 8);
    }

    auto object_reader::read_u32 () -> g10::result<std::uint32_t>
    {
        std::uint8_t bytes[4];
        if (auto result = read_bytes(bytes, 4); !result)
        {
            return g10::error(result.error());
        }
        // Little-endian conversion
        return static_cast<std::uint32_t>(bytes[0]) |
               (static_cast<std::uint32_t>(bytes[1]) << 8) |
               (static_cast<std::uint32_t>(bytes[2]) << 16) |
               (static_cast<std::uint32_t>(bytes[3]) << 24);
    }

    auto object_reader::read_i32 () -> g10::result<std::int32_t>
    {
        auto result = read_u32();
        if (!result)
        {
            return g10::error(result.error());
        }
        return static_cast<std::int32_t>(result.value());
    }

    auto object_reader::read_bytes (void* buffer, std::size_t size) -> g10::result<void>
    {
        m_file.read(static_cast<char*>(buffer), size);
        if (!m_file)
        {
            return g10::error("Failed to read {} bytes from file", size);
        }
        return {};
    }
}

/* Private Methods - String Table Helpers *************************************/

namespace g10link
{
    auto object_reader::get_string (std::uint32_t offset) const -> g10::result<std::string>
    {
        if (offset >= m_string_table.size())
        {
            return g10::error("String offset {} exceeds string table size {}",
                offset, m_string_table.size());
        }

        // Find null terminator
        std::size_t end = offset;
        while (end < m_string_table.size() && m_string_table[end] != '\0')
        {
            ++end;
        }

        if (end >= m_string_table.size())
        {
            return g10::error("String at offset {} is not null-terminated", offset);
        }

        // Extract string
        return std::string(
            reinterpret_cast<const char*>(m_string_table.data() + offset),
            end - offset
        );
    }
}
