/**
 * @file    g10-link/linker.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains implementations for the G10 linker core functionality.
 */

/* Private Includes ***********************************************************/

#include <g10-link/linker.hpp>

/* Public Constructors ********************************************************/

namespace g10link
{
    linker::linker () :
        m_objects {},
        m_symbol_table {},
        m_merged_sections {},
        m_executable {}
    {
    }
}

/* Public Methods *************************************************************/

namespace g10link
{
    auto linker::add_object_file (const g10obj::loaded_object_file& obj) -> void
    {
        m_objects.push_back(obj);
    }

    auto linker::link () -> g10::result<g10exe::executable_file>
    {
        if (m_objects.empty())
        {
            return g10::error("No object files to link");
        }

        // Phase 1: Build global symbol table
        if (auto result = build_symbol_table(); !result)
        {
            return g10::error("Symbol table build failed: {}", result.error());
        }

        // Phase 2: Merge sections
        if (auto result = merge_sections(); !result)
        {
            return g10::error("Section merging failed: {}", result.error());
        }

        // Phase 3: Assign final addresses
        if (auto result = assign_addresses(); !result)
        {
            return g10::error("Address assignment failed: {}", result.error());
        }

        // Phase 4: Process relocations
        if (auto result = process_relocations(); !result)
        {
            return g10::error("Relocation processing failed: {}", result.error());
        }

        // Phase 5: Create segments
        if (auto result = create_segments(); !result)
        {
            return g10::error("Segment creation failed: {}", result.error());
        }

        // Phase 6: Select entry point
        if (auto result = select_entry_point(); !result)
        {
            return g10::error("Entry point selection failed: {}", result.error());
        }

        return m_executable;
    }
}

/* Private Methods - Linking Phases *******************************************/

namespace g10link
{
    auto linker::build_symbol_table () -> g10::result<void>
    {
        // First pass: Add all global symbols
        for (std::size_t obj_idx = 0; obj_idx < m_objects.size(); ++obj_idx)
        {
            const auto& obj = m_objects[obj_idx];
            
            for (std::size_t sym_idx = 0; sym_idx < obj.symbols.size(); ++sym_idx)
            {
                const auto& sym = obj.symbols[sym_idx];
                
                // Only process global symbols in first pass
                if (sym.binding != g10obj::symbol_binding::global)
                {
                    continue;
                }

                // Check for duplicate definition
                if (m_symbol_table.find(sym.name) != m_symbol_table.end())
                {
                    return g10::error("Duplicate symbol definition: '{}' in {}",
                        sym.name, obj.object_filename);
                }

                // Add to global symbol table
                global_symbol gsym;
                gsym.name = sym.name;
                gsym.address = sym.value;  // Will be updated during address assignment
                gsym.type = sym.type;
                gsym.binding = sym.binding;
                gsym.object_index = obj_idx;
                gsym.section_index = sym.section_index;

                m_symbol_table[sym.name] = gsym;
            }
        }

        // Second pass: Check that all external symbols are defined
        for (const auto& obj : m_objects)
        {
            for (const auto& sym : obj.symbols)
            {
                if (sym.binding == g10obj::symbol_binding::extern_)
                {
                    if (m_symbol_table.find(sym.name) == m_symbol_table.end())
                    {
                        return g10::error("Undefined external symbol: '{}' referenced in {}",
                            sym.name, obj.object_filename);
                    }
                }
            }
        }

        return {};
    }

    auto linker::merge_sections () -> g10::result<void>
    {
        // Group sections by base address
        std::unordered_map<std::uint32_t, std::vector<std::pair<std::size_t, std::size_t>>> 
            sections_by_address;

        for (std::size_t obj_idx = 0; obj_idx < m_objects.size(); ++obj_idx)
        {
            const auto& obj = m_objects[obj_idx];
            for (std::size_t sec_idx = 0; sec_idx < obj.sections.size(); ++sec_idx)
            {
                const auto& sec = obj.sections[sec_idx];
                sections_by_address[sec.base_address].emplace_back(obj_idx, sec_idx);
            }
        }

        // Merge sections with the same base address
        for (const auto& [base_addr, section_list] : sections_by_address)
        {
            merged_section merged;
            merged.base_address = base_addr;
            merged.flags = 0;
            merged.alignment = 1;

            // Combine all sections at this address
            for (const auto& [obj_idx, sec_idx] : section_list)
            {
                const auto& obj = m_objects[obj_idx];
                const auto& sec = obj.sections[sec_idx];

                // Merge flags (union of all flags)
                merged.flags |= sec.flags;
                
                // Take maximum alignment
                if (sec.alignment > merged.alignment)
                {
                    merged.alignment = sec.alignment;
                }

                // Track where each byte came from
                std::uint32_t offset_in_merged = static_cast<std::uint32_t>(merged.data.size());
                
                for (std::size_t i = 0; i < sec.data.size(); ++i)
                {
                    merged_section::source_info src;
                    src.object_index = obj_idx;
                    src.section_index = sec_idx;
                    src.offset_in_section = static_cast<std::uint32_t>(i);
                    merged.sources.push_back(src);
                }

                // Append section data
                merged.data.insert(merged.data.end(), sec.data.begin(), sec.data.end());
            }

            m_merged_sections.push_back(std::move(merged));
        }

        // Sort merged sections by base address
        std::sort(m_merged_sections.begin(), m_merged_sections.end(),
            [](const merged_section& a, const merged_section& b)
            {
                return a.base_address < b.base_address;
            });

        return {};
    }

    auto linker::assign_addresses () -> g10::result<void>
    {
        // For each symbol in the global symbol table, update its address
        // based on its position in the merged section
        
        for (auto& [name, gsym] : m_symbol_table)
        {
            // Find which merged section this symbol belongs to
            const auto& obj = m_objects[gsym.object_index];
            
            if (gsym.section_index >= obj.sections.size())
            {
                // External symbol or constant - address is already set
                continue;
            }

            const auto& orig_section = obj.sections[gsym.section_index];
            std::uint32_t base_addr = orig_section.base_address;

            // Find the merged section with this base address
            auto it = std::find_if(m_merged_sections.begin(), m_merged_sections.end(),
                [base_addr](const merged_section& ms) { return ms.base_address == base_addr; });

            if (it == m_merged_sections.end())
            {
                return g10::error("Cannot find merged section for symbol '{}'", name);
            }

            // Calculate offset within the merged section
            // The symbol's original value is an offset within its original section
            std::uint32_t offset_in_original = gsym.address;

            // Find where the original section starts in the merged section
            std::uint32_t offset_in_merged = 0;
            for (const auto& [obj_idx, sec_idx] : 
                 std::vector<std::pair<std::size_t, std::size_t>>{})
            {
                if (obj_idx == gsym.object_index && sec_idx == gsym.section_index)
                {
                    break;
                }
                
                const auto& other_obj = m_objects[obj_idx];
                const auto& other_sec = other_obj.sections[sec_idx];
                if (other_sec.base_address == base_addr)
                {
                    offset_in_merged += other_sec.size;
                }
            }

            // Recalculate offset by looking at source tracking
            offset_in_merged = 0;
            for (std::size_t i = 0; i < it->sources.size(); ++i)
            {
                const auto& src = it->sources[i];
                if (src.object_index == gsym.object_index && 
                    src.section_index == gsym.section_index &&
                    src.offset_in_section == 0)
                {
                    offset_in_merged = static_cast<std::uint32_t>(i);
                    break;
                }
            }

            // Final address = base address + offset in merged + offset in original
            gsym.address = base_addr + offset_in_merged + offset_in_original;
        }

        return {};
    }

    auto linker::process_relocations () -> g10::result<void>
    {
        for (std::size_t obj_idx = 0; obj_idx < m_objects.size(); ++obj_idx)
        {
            const auto& obj = m_objects[obj_idx];
            
            for (const auto& reloc : obj.relocations)
            {
                // Find the target symbol - first check global, then local
                std::uint32_t symbol_addr = 0;
                
                const global_symbol* global_target = find_symbol(reloc.symbol_name);
                if (global_target)
                {
                    symbol_addr = global_target->address;
                }
                else
                {
                    // Try to find as a local symbol in this object file
                    bool found = false;
                    for (const auto& sym : obj.symbols)
                    {
                        if (sym.name == reloc.symbol_name)
                        {
                            // Calculate the local symbol's address
                            if (sym.section_index < obj.sections.size())
                            {
                                const auto& sec = obj.sections[sym.section_index];
                                symbol_addr = sec.base_address + sym.value;
                                found = true;
                            }
                            break;
                        }
                    }
                    
                    if (!found)
                    {
                        return g10::error("Relocation references undefined symbol: '{}'",
                            reloc.symbol_name);
                    }
                }

                // Find the section containing the relocation
                if (reloc.section_index >= obj.sections.size())
                {
                    return g10::error("Invalid section index {} in relocation",
                        reloc.section_index);
                }

                const auto& section = obj.sections[reloc.section_index];
                std::uint32_t section_base = section.base_address;

                // Find the merged section
                auto it = std::find_if(m_merged_sections.begin(), m_merged_sections.end(),
                    [section_base](const merged_section& ms) 
                    { return ms.base_address == section_base; });

                if (it == m_merged_sections.end())
                {
                    return g10::error("Cannot find merged section for relocation");
                }

                // Apply the relocation using the resolved symbol address
                if (auto result = apply_relocation(reloc, it->data, section_base, symbol_addr);
                    !result)
                {
                    return g10::error("Failed to apply relocation: {}", result.error());
                }
            }
        }

        return {};
    }

    auto linker::create_segments () -> g10::result<void>
    {
        // Convert each merged section to a program segment
        for (const auto& section : m_merged_sections)
        {
            g10exe::program_segment segment;
            segment.load_address = section.base_address;
            segment.flags = g10exe::section_flags_to_segment_flags(section.flags);
            segment.alignment = section.alignment;
            
            // For BSS sections, memory_size > 0 but data is empty
            if (section.flags & g10obj::SECT_ZERO)
            {
                segment.memory_size = static_cast<std::uint32_t>(section.data.size());
                // Don't copy data for BSS
            }
            else
            {
                segment.memory_size = static_cast<std::uint32_t>(section.data.size());
                segment.data = section.data;
            }

            m_executable.segments.push_back(std::move(segment));
        }

        // Sort segments by load address
        m_executable.sort_segments();

        return {};
    }

    auto linker::select_entry_point () -> g10::result<void>
    {
        // Try to find entry point symbols in global symbol table first
        const char* entry_names[] = { "main", "_start", "start" };
        
        for (const char* name : entry_names)
        {
            const global_symbol* sym = find_symbol(name);
            if (sym)
            {
                m_executable.entry_point = sym->address;
                return {};
            }
        }

        // If not found in global symbols, search all symbols in all object files
        for (const auto& obj : m_objects)
        {
            for (const auto& sym : obj.symbols)
            {
                for (const char* name : entry_names)
                {
                    if (sym.name == name && 
                        sym.type == g10obj::symbol_type::label &&
                        sym.section_index < obj.sections.size())
                    {
                        // Calculate the symbol's final address
                        const auto& sec = obj.sections[sym.section_index];
                        m_executable.entry_point = sec.base_address + sym.value;
                        return {};
                    }
                }
            }
        }

        // If still nothing found, use the lowest address in a ROM segment
        for (const auto& seg : m_executable.segments)
        {
            if (!seg.is_writable() && seg.load_address < 0x80000000)
            {
                m_executable.entry_point = seg.load_address;
                return {};
            }
        }

        return g10::error("No entry point found");
    }
}

/* Private Methods - Helpers **************************************************/

namespace g10link
{
    auto linker::find_symbol (const std::string& name) const -> const global_symbol*
    {
        auto it = m_symbol_table.find(name);
        if (it != m_symbol_table.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    auto linker::apply_relocation (
        const g10obj::loaded_relocation& reloc,
        std::vector<std::uint8_t>& section_data,
        std::uint32_t section_base,
        std::uint32_t symbol_addr
    ) -> g10::result<void>
    {
        std::uint32_t reloc_site = section_base + reloc.offset;
        std::int32_t addend = reloc.addend;

        switch (reloc.type)
        {
            case g10obj::relocation_type::abs32:
            {
                // Write 32-bit absolute address
                std::uint32_t value = symbol_addr + addend;
                write_u32(section_data, reloc.offset, value);
                break;
            }

            case g10obj::relocation_type::abs16:
            {
                // Write 16-bit absolute address (truncated)
                std::uint32_t value = symbol_addr + addend;
                write_u16(section_data, reloc.offset, static_cast<std::uint16_t>(value & 0xFFFF));
                break;
            }

            case g10obj::relocation_type::abs8:
            {
                // Write 8-bit absolute address (truncated)
                std::uint32_t value = symbol_addr + addend;
                write_u8(section_data, reloc.offset, static_cast<std::uint8_t>(value & 0xFF));
                break;
            }

            case g10obj::relocation_type::rel32:
            {
                // Write 32-bit PC-relative offset
                std::int32_t offset = static_cast<std::int32_t>(symbol_addr) - 
                                     static_cast<std::int32_t>(reloc_site) + addend;
                write_u32(section_data, reloc.offset, static_cast<std::uint32_t>(offset));
                break;
            }

            case g10obj::relocation_type::rel16:
            {
                // Write 16-bit PC-relative offset
                std::int32_t offset = static_cast<std::int32_t>(symbol_addr) - 
                                     static_cast<std::int32_t>(reloc_site) + addend;
                write_u16(section_data, reloc.offset, static_cast<std::uint16_t>(offset & 0xFFFF));
                break;
            }

            case g10obj::relocation_type::rel8:
            {
                // Write 8-bit PC-relative offset
                std::int32_t offset = static_cast<std::int32_t>(symbol_addr) - 
                                     static_cast<std::int32_t>(reloc_site) + addend;
                write_u8(section_data, reloc.offset, static_cast<std::uint8_t>(offset & 0xFF));
                break;
            }

            case g10obj::relocation_type::hi16:
            {
                // Write high 16 bits of address
                std::uint32_t value = symbol_addr + addend;
                write_u16(section_data, reloc.offset, static_cast<std::uint16_t>((value >> 16) & 0xFFFF));
                break;
            }

            case g10obj::relocation_type::lo16:
            {
                // Write low 16 bits of address
                std::uint32_t value = symbol_addr + addend;
                write_u16(section_data, reloc.offset, static_cast<std::uint16_t>(value & 0xFFFF));
                break;
            }

            default:
                return g10::error("Unknown relocation type: {}", 
                    static_cast<int>(reloc.type));
        }

        return {};
    }

    auto linker::write_u32 (std::vector<std::uint8_t>& data, std::size_t offset, std::uint32_t value) -> void
    {
        if (offset + 4 <= data.size())
        {
            data[offset + 0] = static_cast<std::uint8_t>(value & 0xFF);
            data[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
            data[offset + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
            data[offset + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
        }
    }

    auto linker::write_u16 (std::vector<std::uint8_t>& data, std::size_t offset, std::uint16_t value) -> void
    {
        if (offset + 2 <= data.size())
        {
            data[offset + 0] = static_cast<std::uint8_t>(value & 0xFF);
            data[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        }
    }

    auto linker::write_u8 (std::vector<std::uint8_t>& data, std::size_t offset, std::uint8_t value) -> void
    {
        if (offset < data.size())
        {
            data[offset] = value;
        }
    }
}
