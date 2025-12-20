/**
 * @file    g10-asm/object_writer.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains implementations for the G10 CPU assembler's object file
 *          writer component.
 */

/* Private Includes ***********************************************************/

#include <g10-asm/object_writer.hpp>

/* Public Constructors ********************************************************/

namespace g10asm
{
    object_writer::object_writer (const g10obj::object_file& obj) :
        m_obj { obj },
        m_string_table {},
        m_section_offsets {},
        m_symbol_name_offsets {},
        m_source_name_offset { 0 },
        m_symbol_index_map {}
    {
    }
}

/* Private Methods - Preparation *********************************************/

namespace g10asm
{
    auto object_writer::prepare () -> g10::result<void>
    {
        build_string_table();
        calculate_section_offsets();
        return {};
    }

    auto object_writer::build_string_table () -> void
    {
        // String table starts with a null byte (offset 0 = empty string)
        m_string_table.clear();
        m_string_table.push_back('\0');
        
        // Add source filename
        m_source_name_offset = add_string(m_obj.source_filename);
        
        // Add all symbol names and build symbol index map
        m_symbol_name_offsets.resize(m_obj.symbols.size());
        for (std::size_t i = 0; i < m_obj.symbols.size(); ++i)
        {
            m_symbol_name_offsets[i] = add_string(m_obj.symbols[i].name);
            m_symbol_index_map[m_obj.symbols[i].name] = static_cast<std::uint16_t>(i);
        }
    }

    auto object_writer::calculate_section_offsets () -> void
    {
        m_section_offsets.resize(m_obj.sections.size());
        
        std::uint32_t current_offset = 0;
        for (std::size_t i = 0; i < m_obj.sections.size(); ++i)
        {
            m_section_offsets[i] = current_offset;
            current_offset += m_obj.sections[i].size();
        }
    }

    auto object_writer::add_string (const std::string& str) -> std::uint32_t
    {
        if (str.empty())
        {
            return 0;  // Offset 0 is the empty string
        }
        
        // Check if string already exists (string pooling)
        // Search for the string followed by a null terminator
        std::size_t pos = 0;
        while (pos < m_string_table.size())
        {
            // Check if we found a matching string
            if (m_string_table.compare(pos, str.size(), str) == 0)
            {
                // Verify it's followed by a null terminator
                if (pos + str.size() < m_string_table.size() &&
                    m_string_table[pos + str.size()] == '\0')
                {
                    return static_cast<std::uint32_t>(pos);
                }
            }
            
            // Move to next null terminator
            pos = m_string_table.find('\0', pos);
            if (pos == std::string::npos)
            {
                break;
            }
            ++pos;  // Skip past the null terminator
        }
        
        // String not found - add it
        std::uint32_t offset = static_cast<std::uint32_t>(m_string_table.size());
        m_string_table.append(str);
        m_string_table.push_back('\0');
        return offset;
    }
}

/* Private Methods - Writing *************************************************/

namespace g10asm
{
    auto object_writer::write_header (std::ostream& out) -> g10::result<void>
    {
        // Magic number
        write_u32(out, g10obj::MAGIC);
        
        // Version
        write_u16(out, g10obj::VERSION);
        
        // Flags (currently none set)
        write_u16(out, 0);
        
        // Section count
        write_u16(out, static_cast<std::uint16_t>(m_obj.sections.size()));
        
        // Symbol count
        write_u16(out, static_cast<std::uint16_t>(m_obj.symbols.size()));
        
        // Relocation count
        write_u32(out, static_cast<std::uint32_t>(m_obj.relocations.size()));
        
        // String table size
        write_u32(out, static_cast<std::uint32_t>(m_string_table.size()));
        
        // Total code size
        write_u32(out, m_obj.total_code_size());
        
        // Source filename offset in string table
        write_u32(out, m_source_name_offset);
        
        // Reserved (must be 0)
        write_u32(out, 0);
        
        return {};
    }

    auto object_writer::write_section_table (std::ostream& out) -> g10::result<void>
    {
        for (std::size_t i = 0; i < m_obj.sections.size(); ++i)
        {
            const auto& section = m_obj.sections[i];
            
            // Base address from .ORG directive
            write_u32(out, section.base_address);
            
            // Size of this section
            write_u32(out, section.size());
            
            // Offset in code data block
            write_u32(out, m_section_offsets[i]);
            
            // Section flags
            write_u16(out, section.flags);
            
            // Alignment (default to 1)
            write_u16(out, 1);
        }
        
        return {};
    }

    auto object_writer::write_symbol_table (std::ostream& out) -> g10::result<void>
    {
        for (std::size_t i = 0; i < m_obj.symbols.size(); ++i)
        {
            const auto& sym = m_obj.symbols[i];
            
            // Name offset in string table
            write_u32(out, m_symbol_name_offsets[i]);
            
            // Symbol value (address)
            write_u32(out, sym.value);
            
            // Section index (0xFFFF for external)
            std::uint16_t section_idx = (sym.binding == g10obj::symbol_binding::extern_)
                ? 0xFFFF
                : static_cast<std::uint16_t>(sym.section_index);
            write_u16(out, section_idx);
            
            // Symbol type
            write_u8(out, static_cast<std::uint8_t>(sym.type));
            
            // Symbol binding
            write_u8(out, static_cast<std::uint8_t>(sym.binding));
            
            // Symbol size (0 for labels)
            write_u32(out, sym.size);
        }
        
        return {};
    }

    auto object_writer::write_relocation_table (std::ostream& out) -> g10::result<void>
    {
        for (const auto& reloc : m_obj.relocations)
        {
            // Offset within section
            write_u32(out, reloc.offset);
            
            // Section index
            write_u16(out, static_cast<std::uint16_t>(reloc.section_index));
            
            // Symbol index (look up by name)
            auto it = m_symbol_index_map.find(reloc.symbol_name);
            if (it == m_symbol_index_map.end())
            {
                return g10::error("Relocation references unknown symbol: {}",
                    reloc.symbol_name);
            }
            write_u16(out, it->second);
            
            // Addend
            write_i32(out, reloc.addend);
            
            // Relocation type
            write_u8(out, static_cast<std::uint8_t>(reloc.type));
            
            // Reserved bytes (3 bytes, must be 0)
            write_u8(out, 0);
            write_u8(out, 0);
            write_u8(out, 0);
        }
        
        return {};
    }

    auto object_writer::write_string_table (std::ostream& out) -> g10::result<void>
    {
        out.write(m_string_table.data(), m_string_table.size());
        
        if (!out.good())
        {
            return g10::error("Failed to write string table");
        }
        
        return {};
    }

    auto object_writer::write_code_data (std::ostream& out) -> g10::result<void>
    {
        for (const auto& section : m_obj.sections)
        {
            out.write(reinterpret_cast<const char*>(section.code.data()),
                      section.code.size());
            
            if (!out.good())
            {
                return g10::error("Failed to write code data");
            }
        }
        
        return {};
    }
}

/* Private Methods - Binary Output *******************************************/

namespace g10asm
{
    auto object_writer::write_u8 (std::ostream& out, std::uint8_t value) -> void
    {
        out.put(static_cast<char>(value));
    }

    auto object_writer::write_u16 (std::ostream& out, std::uint16_t value) -> void
    {
        // Little-endian: low byte first
        out.put(static_cast<char>(value & 0xFF));
        out.put(static_cast<char>((value >> 8) & 0xFF));
    }

    auto object_writer::write_u32 (std::ostream& out, std::uint32_t value) -> void
    {
        // Little-endian: low byte first
        out.put(static_cast<char>(value & 0xFF));
        out.put(static_cast<char>((value >> 8) & 0xFF));
        out.put(static_cast<char>((value >> 16) & 0xFF));
        out.put(static_cast<char>((value >> 24) & 0xFF));
    }

    auto object_writer::write_i32 (std::ostream& out, std::int32_t value) -> void
    {
        write_u32(out, static_cast<std::uint32_t>(value));
    }
}

/* Public Methods *************************************************************/

namespace g10asm
{
    auto object_writer::write (const std::string& filename) -> g10::result<void>
    {
        std::ofstream out(filename, std::ios::binary);
        if (!out.is_open())
        {
            return g10::error("Failed to open output file: {}", filename);
        }
        
        return write(out);
    }

    auto object_writer::write (std::ostream& out) -> g10::result<void>
    {
        // Prepare data (build string table, calculate offsets)
        auto prep_result = prepare();
        if (!prep_result.has_value())
        {
            return prep_result;
        }
        
        // Write all sections in order
        auto header_result = write_header(out);
        if (!header_result.has_value()) return header_result;
        
        auto section_result = write_section_table(out);
        if (!section_result.has_value()) return section_result;
        
        auto symbol_result = write_symbol_table(out);
        if (!symbol_result.has_value()) return symbol_result;
        
        auto reloc_result = write_relocation_table(out);
        if (!reloc_result.has_value()) return reloc_result;
        
        auto string_result = write_string_table(out);
        if (!string_result.has_value()) return string_result;
        
        auto code_result = write_code_data(out);
        if (!code_result.has_value()) return code_result;
        
        return {};
    }
}
