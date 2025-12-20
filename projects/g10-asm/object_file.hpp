/**
 * @file    g10-asm/object_file.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains definitions for the G10 Object File Format (.g10obj),
 *          the output format produced by the code generator.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10obj
{
    /**
     * @brief   The magic number identifying an object file output by the G10
     *          Assembler and processed by the G10 Linker.
     *  
     * This is the ASCII string "G10A" (G10 Assembler) in little-endian format:
     * 
     * - Byte 0: `0x41` ('A')
     * 
     * - Byte 1: `0x30` ('0')
     * 
     * - Byte 2: `0x31` ('1')
     * 
     * - Byte 3: `0x47` ('G')
     */
    constexpr std::uint32_t MAGIC = 0x47313041;

    /**
     * @brief   The version of the G10 object file format.
     */
    constexpr std::uint16_t VERSION = 0x0001;

    /**
     * @brief   The size, in bytes, of the object file header.
     */
    constexpr std::size_t HEADER_SIZE = 32;

    /**
     * @brief   The size, in bytes, of a single section entry.
     */
    constexpr std::size_t SECTION_ENTRY_SIZE = 16;

    /**
     * @brief   The size, in bytes, of a single symbol table entry.
     */
    constexpr std::size_t SYMBOL_ENTRY_SIZE = 16;

    /**
     * @brief   The size, in bytes, of a single relocation entry.
     */
    constexpr std::size_t RELOCATION_ENTRY_SIZE = 16;

    /**
     * @brief   Weakly enumerates the flags that can appear in a G10 object file's
     *          header.
     */
    enum file_flags : std::uint16_t
    {
        FLAG_HAS_ENTRY  = 0x0001,   /** @brief File defines an entry point symbol */
        FLAG_DEBUG      = 0x0002,   /** @brief Debug information is included */
    };

    /**
     * @brief   Weakly enumerates the flags that can appear in a G10 object file's
     *          section entries.
     */
    enum section_flags : std::uint16_t
    {
        SECT_EXECUTABLE  = 0x0001,  /** @brief Section contains executable code */
        SECT_WRITABLE    = 0x0002,  /** @brief Section is writable (RAM region) */
        SECT_INITIALIZED = 0x0004,  /** @brief Section contains initialized data */
        SECT_ZERO        = 0x0008,  /** @brief Section is zero-initialized (BSS) */
    };

    /**
     * @brief   Strongly enumerates the types of symbols which can appear in a
     *          G10 object file's symbol table.
     */
    enum class symbol_type : std::uint8_t
    {
        undefined  = 0x00,  /** @brief Undefined symbol (forward reference) */
        label      = 0x01,  /** @brief Code label (instruction address) */
        data       = 0x02,  /** @brief Data label (data address) */
        constant   = 0x03,  /** @brief Constant value (not an address) */
        section    = 0x04,  /** @brief Section name */
    };

    /**
     * @brief   Strongly enumerates the binding scopes of a G10 object file's
     *          symbol table entries.
     */
    enum class symbol_binding : std::uint8_t
    {
        local   = 0x00,  /** @brief Local symbol (not visible outside file) */
        global  = 0x01,  /** @brief Global symbol (exported, defined here) */
        extern_ = 0x02,  /** @brief External symbol (imported, defined elsewhere) */
        weak    = 0x03,  /** @brief Weak symbol (can be overridden) */
    };

    /**
     * @brief   Strongly enumerates the types of relocations that can appear in a
     *          G10 object file's relocation table.
     */
    enum class relocation_type : std::uint8_t
    {
        none   = 0x00,  /** @brief No relocation (placeholder) */
        abs32  = 0x01,  /** @brief 32-bit absolute address */
        abs16  = 0x02,  /** @brief 16-bit absolute address (truncated) */
        abs8   = 0x03,  /** @brief 8-bit absolute address (truncated) */
        rel32  = 0x04,  /** @brief 32-bit PC-relative offset */
        rel16  = 0x05,  /** @brief 16-bit PC-relative offset */
        rel8   = 0x06,  /** @brief 8-bit PC-relative offset (for JPB/JR) */
        hi16   = 0x07,  /** @brief High 16 bits of 32-bit address */
        lo16   = 0x08,  /** @brief Low 16 bits of 32-bit address */
    };

}

/* Public Structures **********************************************************/

namespace g10obj
{
    /**
     * @brief   Defines a structure representing the header of a G10 object file.
     * 
     * The file header is exactly 32 bytes and contains metadata about the
     * object file, including the magic number, version, and counts/sizes of
     * various sections.
     */
    struct file_header final
    {
        std::uint32_t magic;              /** @brief Magic number (0x47313041 = "G10A") */
        std::uint16_t version;            /** @brief Format version (0x0001) */
        std::uint16_t flags;              /** @brief File flags (see file_flags enum) */
        std::uint16_t section_count;      /** @brief Number of code sections */
        std::uint16_t symbol_count;       /** @brief Number of symbols in symbol table */
        std::uint32_t relocation_count;   /** @brief Number of relocation entries */
        std::uint32_t string_table_size;  /** @brief Size of string table in bytes */
        std::uint32_t code_size;          /** @brief Total size of all code sections */
        std::uint32_t source_name_offset; /** @brief Offset into string table for source filename */
        std::uint32_t reserved;           /** @brief Reserved for future use (must be 0) */
    };

    static_assert(sizeof(file_header) == 32, "file_header must be exactly 32 bytes");

    /**
     * @brief   Defines a structure representing a section entry in a G10 object
     *          file.
     * 
     * Each section entry is 16 bytes and describes a contiguous block of
     * code at a specific base address.
     */
    struct section_entry final
    {
        std::uint32_t base_address;  /** @brief Base address from .ORG directive */
        std::uint32_t size;          /** @brief Size of this section in bytes */
        std::uint32_t offset;        /** @brief Offset in code data block where this section begins */
        std::uint16_t flags;         /** @brief Section flags (see section_flags enum) */
        std::uint16_t alignment;     /** @brief Required alignment (power of 2, minimum 1) */
    };

    static_assert(sizeof(section_entry) == 16, "section_entry must be exactly 16 bytes");

    /**
     * @brief   Defines a structure representing a symbol table entry in a G10
     *          object file.
     * 
     * Each symbol entry is 16 bytes and describes a label, data symbol, or
     * external reference.
     */
    struct symbol_entry final
    {
        std::uint32_t name_offset;   /** @brief Offset into string table for symbol name */
        std::uint32_t value;         /** @brief Symbol value (address for defined symbols, 0 for extern) */
        std::uint16_t section_index; /** @brief Index of section containing symbol (0xFFFF for extern) */
        std::uint8_t  type;          /** @brief Symbol type (see symbol_type enum) */
        std::uint8_t  binding;       /** @brief Symbol binding (see symbol_binding enum) */
        std::uint32_t size;          /** @brief Size of symbol (0 for labels, size for data) */
    };

    static_assert(sizeof(symbol_entry) == 16, "symbol_entry must be exactly 16 bytes");

    /**
     * @brief   Defines a structure representing a relocation entry in a G10
     *          object file.
     * 
     * Each relocation entry is 16 bytes and describes a location in the code
     * that must be fixed up by the linker when the final address of a symbol
     * is determined.
     */
    struct relocation_entry final
    {
        std::uint32_t offset;        /** @brief Offset within section where relocation is needed */
        std::uint16_t section_index; /** @brief Index of section containing relocation site */
        std::uint16_t symbol_index;  /** @brief Index of symbol to resolve */
        std::int32_t  addend;        /** @brief Signed value to add after symbol resolution */
        std::uint8_t  type;          /** @brief Relocation type (see relocation_type enum) */
        std::uint8_t  reserved[3];   /** @brief Reserved (must be 0) */
    };

    static_assert(sizeof(relocation_entry) == 16, "relocation_entry must be exactly 16 bytes");

    /**
     * @brief   Defines a structure representing a code section in the in-memory
     *          object file structure.
     * 
     * This is an in-memory representation of a code section, used by the
     * code generator during assembly.
     */
    struct code_section final
    {
        std::uint32_t              base_address;  /** @brief Base address from .ORG directive */
        std::vector<std::uint8_t>  code;          /** @brief Generated machine code bytes */
        std::uint16_t              flags;         /** @brief Section flags */

        /**
         * @brief   Returns the size of this section's code in bytes.
         */
        inline auto size () const noexcept -> std::uint32_t
        {
            return static_cast<std::uint32_t>(code.size());
        }
    };

    /**
     * @brief   Defines a structure representing a symbol in the in-memory symbol
     *          table.
     * 
     * This is an in-memory representation of a symbol, used by the code
     * generator during assembly.
     */
    struct symbol final
    {
        std::string      name;           /** @brief Symbol name */
        std::uint32_t    value;          /** @brief Symbol value (address) */
        std::size_t      section_index;  /** @brief Index of section containing symbol */
        symbol_type      type;           /** @brief Symbol type */
        symbol_binding   binding;        /** @brief Symbol binding */
        std::uint32_t    size;           /** @brief Symbol size (0 for labels) */
    };

    /**
     * @brief   Defines a structure representing a relocation in the in-memory
     *          relocation table.
     * 
     * This is an in-memory representation of a relocation, used by the code
     * generator during assembly.
     */
    struct relocation final
    {
        std::uint32_t     offset;        /** @brief Offset in section where relocation is needed */
        std::size_t       section_index; /** @brief Index of section containing relocation site */
        std::string       symbol_name;   /** @brief Name of symbol to resolve */
        relocation_type   type;          /** @brief Relocation type */
        std::int32_t      addend;        /** @brief Value to add after symbol resolution */
    };

    /**
     * @brief   Defines a structure representing a G10 object file in memory.
     * 
     * This structure is populated by the code generator and later serialized
     * to disk as a binary object file.
     */
    struct object_file final
    {
        std::vector<code_section>  sections;           /** @brief Code sections */
        std::vector<symbol>        symbols;            /** @brief Symbol table */
        std::vector<relocation>    relocations;        /** @brief Relocations */
        std::string                string_table;       /** @brief String table (null-terminated strings) */
        std::string                source_filename;    /** @brief Source filename for debugging */
        
        /**
         * @brief   Returns the total size of all code sections combined.
         */
        inline auto total_code_size () const noexcept -> std::uint32_t
        {
            std::uint32_t total = 0;
            for (const auto& section : sections)
            {
                total += section.size();
            }
            return total;
        }

        /**
         * @brief   Adds a string to the string table and returns its offset.
         * 
         * @param   str     The string to add (will be null-terminated).
         * 
         * @return  The offset of the string in the string table.
         */
        auto add_string (const std::string& str) -> std::uint32_t
        {
            // Empty string always has offset 0
            if (str.empty())
            {
                return 0;
            }

            // Check if string already exists (string pooling)
            auto pos = string_table.find(str + '\0');
            if (pos != std::string::npos)
            {
                return static_cast<std::uint32_t>(pos);
            }

            // Add string to table
            std::uint32_t offset = static_cast<std::uint32_t>(string_table.size());
            string_table.append(str);
            string_table.push_back('\0');
            return offset;
        }

        /**
         * @brief   Returns the string at the given offset in the string table.
         * 
         * @param   offset  The offset in the string table.
         * 
         * @return  The string at the given offset, or an empty string if invalid.
         */
        auto get_string (std::uint32_t offset) const -> std::string
        {
            if (offset >= string_table.size())
            {
                return "";
            }

            std::size_t end = string_table.find('\0', offset);
            if (end == std::string::npos)
            {
                return "";
            }

            return string_table.substr(offset, end - offset);
        }
    };

}
