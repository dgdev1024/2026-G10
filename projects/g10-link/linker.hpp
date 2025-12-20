/**
 * @file    g10-link/linker.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains declarations for the G10 linker core functionality.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-link/common.hpp>
#include <g10-link/object_file.hpp>
#include <g10-link/executable_file.hpp>

/* Public Classes *************************************************************/

namespace g10link
{
    /**
     * @brief   Defines a class for linking multiple object files into an executable.
     * 
     * The linker performs the following operations:
     * 1. Builds a global symbol table from all object files
     * 2. Merges sections with the same base address
     * 3. Assigns final addresses to all symbols
     * 4. Processes and applies all relocations
     * 5. Creates program segments from merged sections
     * 6. Selects an entry point for the program
     */
    class linker final
    {
    public: /* Public Constructors ********************************************/

        /**
         * @brief   Constructs a linker instance.
         */
        linker ();

    public: /* Public Methods ************************************************/

        /**
         * @brief   Adds an object file to the linker.
         * 
         * @param   obj     The loaded object file to add.
         */
        auto add_object_file (const g10obj::loaded_object_file& obj) -> void;

        /**
         * @brief   Performs the complete linking process.
         * 
         * This method executes all linking phases in sequence:
         * - Symbol table building
         * - Section merging
         * - Address assignment
         * - Relocation processing
         * - Segment creation
         * - Entry point selection
         * 
         * @return  If successful, returns the executable file;
         *          otherwise, returns an error description.
         */
        auto link () -> g10::result<g10exe::executable_file>;

    private: /* Private Types *************************************************/

        /**
         * @brief   Represents a symbol in the global symbol table.
         */
        struct global_symbol
        {
            std::string      name;           /** @brief Symbol name */
            std::uint32_t    address;        /** @brief Final resolved address */
            g10obj::symbol_type      type;   /** @brief Symbol type */
            g10obj::symbol_binding   binding;/** @brief Symbol binding */
            std::size_t      object_index;   /** @brief Source object file index */
            std::size_t      section_index;  /** @brief Source section index */
        };

        /**
         * @brief   Represents a merged section.
         */
        struct merged_section
        {
            std::uint32_t              base_address;  /** @brief Base address */
            std::uint16_t              flags;         /** @brief Section flags */
            std::uint16_t              alignment;     /** @brief Alignment requirement */
            std::vector<std::uint8_t>  data;          /** @brief Combined section data */
            
            // Track which object file and section each byte came from
            struct source_info
            {
                std::size_t object_index;
                std::size_t section_index;
                std::uint32_t offset_in_section;
            };
            std::vector<source_info>   sources;       /** @brief Source tracking */
        };

    private: /* Private Methods - Linking Phases ******************************/

        /**
         * @brief   Builds the global symbol table.
         * 
         * This phase creates a unified symbol table from all object files,
         * checking for duplicate definitions and undefined externals.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto build_symbol_table () -> g10::result<void>;

        /**
         * @brief   Merges sections with the same base address.
         * 
         * Sections from different object files with the same base address
         * are combined into a single merged section.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto merge_sections () -> g10::result<void>;

        /**
         * @brief   Assigns final addresses to all symbols.
         * 
         * After sections are merged, symbol addresses are updated to reflect
         * their final positions in the merged sections.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto assign_addresses () -> g10::result<void>;

        /**
         * @brief   Processes and applies all relocations.
         * 
         * For each relocation, looks up the target symbol and patches the
         * code bytes based on the relocation type.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto process_relocations () -> g10::result<void>;

        /**
         * @brief   Creates program segments from merged sections.
         * 
         * Converts the merged sections into executable segments with
         * appropriate flags and data.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto create_segments () -> g10::result<void>;

        /**
         * @brief   Selects the program entry point.
         * 
         * Searches for entry point symbols (main, _start, start) in order
         * of preference.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto select_entry_point () -> g10::result<void>;

    private: /* Private Methods - Helpers *************************************/

        /**
         * @brief   Finds a symbol in the global symbol table.
         * 
         * @param   name    The symbol name to find.
         * 
         * @return  Pointer to the symbol if found, nullptr otherwise.
         */
        auto find_symbol (const std::string& name) const -> const global_symbol*;

        /**
         * @brief   Applies a relocation to section data.
         * 
         * @param   reloc           The relocation to apply.
         * @param   section_data    The section data to patch.
         * @param   section_base    The section's base address.
         * @param   symbol_addr     The target symbol's address.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto apply_relocation (
            const g10obj::loaded_relocation& reloc,
            std::vector<std::uint8_t>& section_data,
            std::uint32_t section_base,
            std::uint32_t symbol_addr
        ) -> g10::result<void>;

        /**
         * @brief   Writes a 32-bit value to section data (little-endian).
         */
        auto write_u32 (std::vector<std::uint8_t>& data, std::size_t offset, std::uint32_t value) -> void;

        /**
         * @brief   Writes a 16-bit value to section data (little-endian).
         */
        auto write_u16 (std::vector<std::uint8_t>& data, std::size_t offset, std::uint16_t value) -> void;

        /**
         * @brief   Writes an 8-bit value to section data.
         */
        auto write_u8 (std::vector<std::uint8_t>& data, std::size_t offset, std::uint8_t value) -> void;

    private: /* Private Member Variables **************************************/

        std::vector<g10obj::loaded_object_file>     m_objects;          /** @brief Input object files */
        std::unordered_map<std::string, global_symbol> m_symbol_table; /** @brief Global symbol table */
        std::vector<merged_section>                 m_merged_sections;  /** @brief Merged sections */
        g10exe::executable_file                     m_executable;       /** @brief Output executable */
    };

}
