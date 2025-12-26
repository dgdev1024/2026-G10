/**
 * @file    g10/object.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-26
 * 
 * @brief   Contains definitions for the G10 CPU object file.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10
{
    /**
     * @brief   Magic number identifying a valid G10 object file.
     *          Corresponds to ASCII string "G10O" in little-endian.
     */
    constexpr std::uint32_t OBJECT_MAGIC = 0x4731304F;

    /**
     * @brief   Current version of the G10 object file format.
     *          Format: 0xMMmmPPPP (Major.Minor.Patch)
     */
    constexpr std::uint32_t OBJECT_VERSION = 0x01000000;

    /**
     * @brief   Size of the object file header in bytes.
     */
    constexpr std::size_t OBJECT_HEADER_SIZE = 0x40;

    /**
     * @brief   Size of a section header in bytes.
     */
    constexpr std::size_t OBJECT_SECTION_HEADER_SIZE = 16;

    /**
     * @brief   Size of a symbol table entry in bytes.
     */
    constexpr std::size_t OBJECT_SYMBOL_ENTRY_SIZE = 16;

    /**
     * @brief   Size of a relocation table entry in bytes.
     */
    constexpr std::size_t OBJECT_RELOCATION_ENTRY_SIZE = 16;

    /**
     * @brief   Strongly enumerates the file-level flags for a G10 object file.
     */
    enum class object_flags : std::uint32_t
    {
        none        = 0x00000000,
        has_entry   = 0x00000001,   /** @brief File contains an entry point symbol */
        debug_info  = 0x00000002,   /** @brief File contains debug information */
        relocatable = 0x00000004    /** @brief File is relocatable (not absolute) */
    };

    /**
     * @brief   Strongly enumerates the types of sections in a G10 object file.
     */
    enum class section_type : std::uint16_t
    {
        null_   = 0x0000,   /** @brief Unused section header entry */
        code    = 0x0001,   /** @brief Executable code (ROM region) */
        data    = 0x0002,   /** @brief Initialized data (ROM region, read-only) */
        bss     = 0x0003    /** @brief Uninitialized data reservation (RAM region) */
    };

    /**
     * @brief   Strongly enumerates the section attribute flags.
     */
    enum class section_flags : std::uint16_t
    {
        none    = 0x0000,
        alloc   = 0x0001,   /** @brief Section occupies memory at runtime */
        load    = 0x0002,   /** @brief Section data should be loaded from file */
        exec    = 0x0004,   /** @brief Section contains executable code */
        write   = 0x0008,   /** @brief Section is writable at runtime */
        merge   = 0x0010,   /** @brief Section can be merged with similar sections */
        strings = 0x0020    /** @brief Section contains null-terminated strings */
    };

    /**
     * @brief   Strongly enumerates the types of symbols in the symbol table.
     */
    enum class symbol_type : std::uint8_t
    {
        none    = 0x00,     /** @brief Unspecified type */
        label   = 0x01,     /** @brief Code label (jump/call target) */
        data    = 0x02,     /** @brief Data label (memory location) */
        section = 0x03,     /** @brief Section name symbol */
        file    = 0x04      /** @brief Source file name */
    };

    /**
     * @brief   Strongly enumerates the symbol binding (visibility/scope).
     */
    enum class symbol_binding : std::uint8_t
    {
        local_  = 0x00,     /** @brief Symbol is local to this object file */
        global  = 0x01,     /** @brief Symbol is visible to other object files */
        extern_ = 0x02,     /** @brief Symbol is defined in another object file */
        weak    = 0x03      /** @brief Symbol can be overridden by global def. */
    };

    /**
     * @brief   Strongly enumerates the symbol flags.
     */
    enum class symbol_flags : std::uint16_t
    {
        none     = 0x0000,
        entry    = 0x0001,  /** @brief Symbol is the program entry point */
        absolute = 0x0002,  /** @brief Symbol value is absolute (no relocation) */
        common   = 0x0004   /** @brief Symbol is a common block */
    };

    /**
     * @brief   Special section index values for symbols.
     */
    constexpr std::uint32_t SECTION_INDEX_UNDEF  = 0xFFFFFFFF;
    constexpr std::uint32_t SECTION_INDEX_ABS    = 0xFFFFFFFE;
    constexpr std::uint32_t SECTION_INDEX_COMMON = 0xFFFFFFFD;

    /**
     * @brief   Strongly enumerates the relocation types.
     */
    enum class relocation_type : std::uint16_t
    {
        none    = 0x0000,   /** @brief No relocation (placeholder) */
        abs32   = 0x0001,   /** @brief 32-bit absolute address */
        abs16   = 0x0002,   /** @brief 16-bit absolute address (truncated) */
        abs8    = 0x0003,   /** @brief 8-bit absolute address (truncated) */
        rel32   = 0x0004,   /** @brief 32-bit PC-relative offset */
        rel16   = 0x0005,   /** @brief 16-bit PC-relative offset */
        rel8    = 0x0006,   /** @brief 8-bit PC-relative offset */
        quick16 = 0x0007,   /** @brief 16-bit offset relative to $FFFF0000 */
        port8   = 0x0008    /** @brief 8-bit offset relative to $FFFFFF00 */
    };

    G10_BIT_ENUM(object_flags)
    G10_BIT_ENUM(section_flags)
    G10_BIT_ENUM(symbol_flags)
}

/* Public Unions and Structures ***********************************************/

namespace g10
{
    /**
     * @brief   Represents the file header of a G10 object file.
     * 
     * This structure matches the binary layout of the header (64 bytes).
     */
    struct object_header final
    {
        std::uint32_t   magic;              /** @brief Magic number (0x4731304F) */
        std::uint32_t   version;            /** @brief Format version */
        std::uint32_t   flags;              /** @brief File-level flags */
        std::uint32_t   section_count;      /** @brief Number of section headers */
        std::uint32_t   symbol_table_offset;/** @brief Offset to symbol table */
        std::uint32_t   symbol_table_size;  /** @brief Number of symbol entries */
        std::uint32_t   string_table_offset;/** @brief Offset to string table */
        std::uint32_t   string_table_size;  /** @brief Size of string table (bytes) */
        std::uint32_t   reloc_table_offset; /** @brief Offset to relocation table */
        std::uint32_t   reloc_table_size;   /** @brief Number of relocation entries */
        std::uint8_t    reserved[24];       /** @brief Reserved (must be zero) */
    };

    static_assert(sizeof(object_header) == OBJECT_HEADER_SIZE,
        "object_header size mismatch");

    /**
     * @brief   Represents a section header in a G10 object file.
     * 
     * This structure matches the binary layout of a section header (16 bytes).
     */
    struct object_section_header final
    {
        std::uint32_t   name_offset;        /** @brief Offset into string table */
        std::uint32_t   virtual_address;    /** @brief Target address in memory */
        std::uint32_t   size;               /** @brief Size of section data (bytes) */
        std::uint16_t   type;               /** @brief Section type identifier */
        std::uint16_t   flags;              /** @brief Section attribute flags */
    };

    static_assert(sizeof(object_section_header) == OBJECT_SECTION_HEADER_SIZE,
        "object_section_header size mismatch");

    /**
     * @brief   Represents a symbol table entry in a G10 object file.
     * 
     * This structure matches the binary layout of a symbol entry (16 bytes).
     */
    struct object_symbol_entry final
    {
        std::uint32_t   name_offset;        /** @brief Offset into string table */
        std::uint32_t   value;              /** @brief Symbol value (address/size) */
        std::uint32_t   section_index;      /** @brief Associated section index */
        std::uint8_t    type;               /** @brief Symbol type */
        std::uint8_t    binding;            /** @brief Symbol binding (scope) */
        std::uint16_t   flags;              /** @brief Additional symbol flags */
    };

    static_assert(sizeof(object_symbol_entry) == OBJECT_SYMBOL_ENTRY_SIZE,
        "object_symbol_entry size mismatch");

    /**
     * @brief   Represents a relocation table entry in a G10 object file.
     * 
     * This structure matches the binary layout of a relocation entry (16 bytes).
     */
    struct object_relocation_entry final
    {
        std::uint32_t   offset;             /** @brief Byte offset within section */
        std::uint32_t   symbol_index;       /** @brief Index into symbol table */
        std::uint32_t   section_index;      /** @brief Section containing relocation */
        std::uint16_t   type;               /** @brief Relocation type */
        std::uint16_t   addend;             /** @brief Low 16 bits of addend */
    };

    static_assert(sizeof(object_relocation_entry) == OBJECT_RELOCATION_ENTRY_SIZE,
        "object_relocation_entry size mismatch");

    /**
     * @brief   Represents a section in the object file (in-memory representation).
     * 
     * This structure holds the section metadata along with its data bytes.
     */
    struct object_section final
    {
        std::string     name;               /** @brief Section name */
        std::uint32_t   virtual_address;    /** @brief Target address in memory */
        section_type    type;               /** @brief Section type */
        section_flags   flags;              /** @brief Section attribute flags */
        std::vector<std::uint8_t> data;     /** @brief Section data bytes */
    };

    /**
     * @brief   Represents a symbol in the object file (in-memory representation).
     */
    struct object_symbol final
    {
        std::string     name;               /** @brief Symbol name */
        std::uint32_t   value;              /** @brief Symbol value (address/size) */
        std::uint32_t   section_index;      /** @brief Associated section index */
        symbol_type     type;               /** @brief Symbol type */
        symbol_binding  binding;            /** @brief Symbol binding (scope) */
        symbol_flags    flags;              /** @brief Additional symbol flags */
    };

    /**
     * @brief   Represents a relocation in the object file (in-memory repr.).
     */
    struct object_relocation final
    {
        std::uint32_t       offset;         /** @brief Byte offset within section */
        std::uint32_t       symbol_index;   /** @brief Index into symbol table */
        std::uint32_t       section_index;  /** @brief Section containing relocation */
        relocation_type     type;           /** @brief Relocation type */
        std::int32_t        addend;         /** @brief Full addend value */
    };
}

/* Public Classes *************************************************************/

namespace g10
{
    /**
     * @brief   Defines a class representing an object file, created and output
     *          by the G10 assembler tool (`g10asm`), and input and processed by
     *          the G10 linker tool (`g10link`).
     * 
     * For more information on the G10 object file format, see the specification
     * document located at `docs/g10obj.spec.md`.
     */
    class g10api object final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   The default constructor constructs a blank G10 object file.
         */
        object () = default;

        /**
         * @brief   Constructs a G10 object file by loading it from a file
         *          located at the given path.
         * 
         * @param   path    The path to the G10 object file to load.
         */
        explicit object (const fs::path& path);

        /**
         * @brief   The default destructor.
         */
        ~object () = default;

        /**
         * @brief   Loads the G10 object file from a file located at the given
         *          path.
         * 
         * @param   path    The path to the G10 object file to load.
         * 
         * @return  If loaded successfully and is valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto load_from_file (const fs::path& path) -> result<void>;

        /**
         * @brief   Saves the G10 object file to a file located at the given
         *          path.
         * 
         * @param   path    The path to save the G10 object file to.
         * 
         * @return  If saved successfully, returns `true`;
         *          If the object file is empty, returns `false` (a valid
         *          object file is still created and saved in this case);
         *          Otherwise, returns an error message describing the failure.
         */
        auto save_to_file (const fs::path& path) -> result<bool>;

        /**
         * @brief   Clears all data from the object file, resetting it to an
         *          empty state.
         */
        auto clear () -> void;

        /**
         * @brief   Indicates whether the object file was loaded and validated
         *          successfully; or if a new object file was saved successfully.
         * 
         * @return  `true` if the object file is good;
         *          `false` otherwise.
         */
        inline auto is_good () const noexcept -> bool
            { return m_good; }

        /**
         * @brief   Indicates whether the object file was loaded and validated
         *          successfully; or if a new object file was saved successfully.
         * 
         * @return  `true` if the object file is good;
         *          `false` otherwise.
         */
        inline operator bool () const noexcept
            { return is_good(); }

    public: /* Public Methods - Accessors *************************************/

        /**
         * @brief   Retrieves the file-level flags.
         */
        inline auto get_flags () const noexcept -> object_flags
            { return m_flags; }

        /**
         * @brief   Retrieves a constant reference to the sections vector.
         */
        inline auto get_sections () const noexcept
            -> const std::vector<object_section>&
            { return m_sections; }

        /**
         * @brief   Retrieves a constant reference to the symbols vector.
         */
        inline auto get_symbols () const noexcept
            -> const std::vector<object_symbol>&
            { return m_symbols; }

        /**
         * @brief   Retrieves a constant reference to the relocations vector.
         */
        inline auto get_relocations () const noexcept
            -> const std::vector<object_relocation>&
            { return m_relocations; }

    public: /* Public Methods - Object Building *******************************/

        /**
         * @brief   Sets the file-level flags.
         * 
         * @param   flags   The flags to set.
         */
        auto set_flags (object_flags flags) -> void;

        /**
         * @brief   Adds a new section to the object file.
         * 
         * @param   section The section to add.
         * 
         * @return  The index of the newly-added section.
         */
        auto add_section (const object_section& section) -> std::size_t;

        /**
         * @brief   Adds a new symbol to the object file.
         * 
         * @param   symbol  The symbol to add.
         * 
         * @return  If successful, returns the index of the newly-added symbol;
         *          Otherwise, returns an error message.
         */
        auto add_symbol (const object_symbol& symbol) -> result<std::size_t>;

        /**
         * @brief   Adds a new relocation to the object file.
         * 
         * @param   reloc   The relocation to add.
         * 
         * @return  If successful, returns the index of the newly-added reloc.;
         *          Otherwise, returns an error message.
         */
        auto add_relocation (const object_relocation& reloc)
            -> result<std::size_t>;

        /**
         * @brief   Finds a symbol by name.
         * 
         * @param   name    The name of the symbol to find.
         * 
         * @return  If found, returns the symbol's index;
         *          Otherwise, returns `std::nullopt`.
         */
        auto find_symbol (const std::string& name) const
            -> std::optional<std::size_t>;

        /**
         * @brief   Finds a section by name.
         * 
         * @param   name    The name of the section to find.
         * 
         * @return  If found, returns the section's index;
         *          Otherwise, returns `std::nullopt`.
         */
        auto find_section (const std::string& name) const
            -> std::optional<std::size_t>;
        
    private: /* Private Methods ***********************************************/

        /**
         * @brief   Validates the G10 object file's internal structure and
         *          data.
         * 
         * @return  If valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto validate () -> result<void>;

        /**
         * @brief   Builds the string table from the current sections, symbols,
         *          and relocations.
         * 
         * @return  A pair containing the string table data and a map of string
         *          offsets keyed by string content.
         */
        auto build_string_table () const
            -> std::pair<std::vector<std::uint8_t>,
                std::unordered_map<std::string, std::uint32_t>>;

    private: /* Private Members ***********************************************/

        /**
         * @brief   Indicates whether an object file was loaded and validated
         *          successfully; or if a new object file was saved successfully.
         */
        bool m_good { false };

        /**
         * @brief   The file-level flags.
         */
        object_flags m_flags { object_flags::none };

        /**
         * @brief   The list of sections in this object file.
         */
        std::vector<object_section> m_sections;

        /**
         * @brief   The list of symbols in this object file.
         */
        std::vector<object_symbol> m_symbols;

        /**
         * @brief   The list of relocations in this object file.
         */
        std::vector<object_relocation> m_relocations;

    };
}