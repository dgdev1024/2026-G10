/**
 * @file    g10-link/object_file.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains definitions for reading G10 Object File Format (.g10obj)
 *          files produced by the assembler.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-link/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10obj
{
    /**
     * @brief   The magic number identifying a G10 object file.
     *  
     * This is the ASCII string "G10A" (G10 Assembler) in little-endian format:
     * - Byte 0: `0x41` ('A')
     * - Byte 1: `0x30` ('0')
     * - Byte 2: `0x31` ('1')
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

/* Binary Structures (for file I/O) *******************************************/

namespace g10obj
{
    /**
     * @brief   Defines the binary structure of a G10 object file header.
     * 
     * This structure is exactly 32 bytes and is used for direct file I/O.
     */
    struct file_header final
    {
        std::uint32_t magic;              /** @brief Magic number (0x47313041 = "G10A") */
        std::uint16_t version;            /** @brief Format version (0x0001) */
        std::uint16_t flags;              /** @brief File flags */
        std::uint16_t section_count;      /** @brief Number of code sections */
        std::uint16_t symbol_count;       /** @brief Number of symbols */
        std::uint32_t relocation_count;   /** @brief Number of relocation entries */
        std::uint32_t string_table_size;  /** @brief Size of string table in bytes */
        std::uint32_t code_size;          /** @brief Total size of all code sections */
        std::uint32_t source_name_offset; /** @brief Offset into string table for source filename */
        std::uint32_t reserved;           /** @brief Reserved for future use (must be 0) */
    };

    static_assert(sizeof(file_header) == 32, "file_header must be exactly 32 bytes");

    /**
     * @brief   Defines the binary structure of a G10 object file section entry.
     * 
     * This structure is exactly 16 bytes and is used for direct file I/O.
     */
    struct section_entry final
    {
        std::uint32_t base_address;  /** @brief Base address from .ORG directive */
        std::uint32_t size;          /** @brief Size of this section in bytes */
        std::uint32_t offset;        /** @brief Offset in code data block */
        std::uint16_t flags;         /** @brief Section flags */
        std::uint16_t alignment;     /** @brief Required alignment (power of 2) */
    };

    static_assert(sizeof(section_entry) == 16, "section_entry must be exactly 16 bytes");

    /**
     * @brief   Defines the binary structure of a G10 object file symbol entry.
     * 
     * This structure is exactly 16 bytes and is used for direct file I/O.
     */
    struct symbol_entry final
    {
        std::uint32_t name_offset;   /** @brief Offset into string table */
        std::uint32_t value;         /** @brief Symbol value (address) */
        std::uint16_t section_index; /** @brief Section index (0xFFFF for extern) */
        std::uint8_t  type;          /** @brief Symbol type */
        std::uint8_t  binding;       /** @brief Symbol binding */
        std::uint32_t size;          /** @brief Symbol size */
    };

    static_assert(sizeof(symbol_entry) == 16, "symbol_entry must be exactly 16 bytes");

    /**
     * @brief   Defines the binary structure of a G10 object file relocation entry.
     * 
     * This structure is exactly 16 bytes and is used for direct file I/O.
     */
    struct relocation_entry final
    {
        std::uint32_t offset;        /** @brief Offset within section */
        std::uint16_t section_index; /** @brief Section containing relocation site */
        std::uint16_t symbol_index;  /** @brief Symbol to resolve */
        std::int32_t  addend;        /** @brief Signed value to add */
        std::uint8_t  type;          /** @brief Relocation type */
        std::uint8_t  reserved[3];   /** @brief Reserved (must be 0) */
    };

    static_assert(sizeof(relocation_entry) == 16, "relocation_entry must be exactly 16 bytes");

}

/* In-Memory Structures (for linker processing) *******************************/

namespace g10obj
{
    /**
     * @brief   Represents a loaded section with its code data.
     */
    struct loaded_section final
    {
        std::uint32_t              base_address;  /** @brief Base address */
        std::uint32_t              size;          /** @brief Size in bytes */
        std::uint16_t              flags;         /** @brief Section flags */
        std::uint16_t              alignment;     /** @brief Alignment requirement */
        std::vector<std::uint8_t>  data;          /** @brief Section code/data bytes */

        /**
         * @brief   Checks if this section is in the RAM region.
         */
        inline constexpr auto is_in_ram () const noexcept -> bool
        {
            return (flags & SECT_WRITABLE) != 0;
        }

        /**
         * @brief   Checks if this section is executable.
         */
        inline constexpr auto is_executable () const noexcept -> bool
        {
            return (flags & SECT_EXECUTABLE) != 0;
        }

        /**
         * @brief   Checks if this section is zero-initialized (BSS).
         */
        inline constexpr auto is_bss () const noexcept -> bool
        {
            return (flags & SECT_ZERO) != 0;
        }
    };

    /**
     * @brief   Represents a loaded symbol with its name resolved.
     */
    struct loaded_symbol final
    {
        std::string      name;           /** @brief Symbol name */
        std::uint32_t    value;          /** @brief Symbol value (address) */
        std::uint16_t    section_index;  /** @brief Section index (0xFFFF for extern) */
        symbol_type      type;           /** @brief Symbol type */
        symbol_binding   binding;        /** @brief Symbol binding */
        std::uint32_t    size;           /** @brief Symbol size */

        /**
         * @brief   Checks if this symbol is external (defined elsewhere).
         */
        inline constexpr auto is_external () const noexcept -> bool
        {
            return binding == symbol_binding::extern_;
        }

        /**
         * @brief   Checks if this symbol is global (exported).
         */
        inline constexpr auto is_global () const noexcept -> bool
        {
            return binding == symbol_binding::global;
        }

        /**
         * @brief   Checks if this symbol is local.
         */
        inline constexpr auto is_local () const noexcept -> bool
        {
            return binding == symbol_binding::local;
        }
    };

    /**
     * @brief   Represents a loaded relocation with symbol name resolved.
     */
    struct loaded_relocation final
    {
        std::uint32_t     offset;         /** @brief Offset in section */
        std::uint16_t     section_index;  /** @brief Section containing relocation */
        std::uint16_t     symbol_index;   /** @brief Original symbol index */
        std::string       symbol_name;    /** @brief Resolved symbol name */
        relocation_type   type;           /** @brief Relocation type */
        std::int32_t      addend;         /** @brief Value to add */
    };

    /**
     * @brief   Represents a complete loaded object file.
     */
    struct loaded_object_file final
    {
        std::string                     source_filename;  /** @brief Source file name */
        std::string                     object_filename;  /** @brief Object file path */
        std::uint16_t                   flags;            /** @brief File flags */
        std::vector<loaded_section>     sections;         /** @brief Loaded sections */
        std::vector<loaded_symbol>      symbols;          /** @brief Loaded symbols */
        std::vector<loaded_relocation>  relocations;      /** @brief Loaded relocations */

        /**
         * @brief   Finds a symbol by name.
         * 
         * @param   name    The symbol name to find.
         * 
         * @return  Pointer to the symbol if found, nullptr otherwise.
         */
        auto find_symbol (const std::string& name) const -> const loaded_symbol*
        {
            for (const auto& sym : symbols)
            {
                if (sym.name == name)
                {
                    return &sym;
                }
            }
            return nullptr;
        }

        /**
         * @brief   Finds a global symbol by name.
         * 
         * @param   name    The symbol name to find.
         * 
         * @return  Pointer to the symbol if found and global, nullptr otherwise.
         */
        auto find_global_symbol (const std::string& name) const -> const loaded_symbol*
        {
            for (const auto& sym : symbols)
            {
                if (sym.name == name && sym.is_global())
                {
                    return &sym;
                }
            }
            return nullptr;
        }
    };

}
