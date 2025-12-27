/**
 * @file    g10/program.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains definitions for the G10 CPU executable program file.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/object.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10
{
    /**
     * @brief   Magic number identifying a valid G10 program file.
     *          Corresponds to ASCII string "G10P" in little-endian.
     */
    constexpr std::uint32_t PROGRAM_MAGIC = 0x47313050;

    /**
     * @brief   Current version of the G10 program file format.
     *          Format: 0xMMmmPPPP (Major.Minor.Patch)
     */
    constexpr std::uint32_t PROGRAM_VERSION = 0x01000000;

    /**
     * @brief   Size of the program file header in bytes.
     */
    constexpr std::size_t PROGRAM_HEADER_SIZE = 0x40;

    /**
     * @brief   Size of a segment header in bytes.
     */
    constexpr std::size_t PROGRAM_SEGMENT_HEADER_SIZE = 16;

    /**
     * @brief   Size of the program info header in bytes (excluding string data).
     */
    constexpr std::size_t PROGRAM_INFO_HEADER_SIZE = 0x30;

    /**
     * @brief   Default entry point address for G10 programs.
     */
    constexpr std::uint32_t PROGRAM_DEFAULT_ENTRY = 0x00002000;

    /**
     * @brief   Default initial stack pointer for G10 programs.
     */
    constexpr std::uint32_t PROGRAM_DEFAULT_STACK = 0xFFFFFFFC;

    /**
     * @brief   Start of ROM region (includes metadata and interrupt table).
     */
    constexpr std::uint32_t PROGRAM_ROM_START = 0x00000000;

    /**
     * @brief   End of ROM region (inclusive).
     */
    constexpr std::uint32_t PROGRAM_ROM_END = 0x7FFFFFFF;

    /**
     * @brief   Start of code/data region within ROM.
     */
    constexpr std::uint32_t PROGRAM_CODE_START = 0x00002000;

    /**
     * @brief   Start of RAM region.
     */
    constexpr std::uint32_t PROGRAM_RAM_START = 0x80000000;

    /**
     * @brief   End of RAM region (inclusive).
     */
    constexpr std::uint32_t PROGRAM_RAM_END = 0xFFFFFFFF;

    /**
     * @brief   Strongly enumerates the file-level flags for a G10 program file.
     */
    enum class program_flags : std::uint32_t
    {
        none            = 0x00000000,
        has_entry       = 0x00000001,   /** @brief Program has a valid entry point */
        has_stack_init  = 0x00000002,   /** @brief Program specifies initial stack pointer */
        has_info        = 0x00000004,   /** @brief Program contains optional info section */
        debug_build     = 0x00000008,   /** @brief Program was built with debug info */
        double_speed    = 0x00000010    /** @brief Program requests double-speed CPU mode */
    };

    /**
     * @brief   Strongly enumerates the types of segments in a G10 program file.
     */
    enum class segment_type : std::uint16_t
    {
        null_       = 0x0000,   /** @brief Unused segment header entry */
        code        = 0x0001,   /** @brief Executable code (ROM region) */
        data        = 0x0002,   /** @brief Initialized read-only data (ROM region) */
        bss         = 0x0003,   /** @brief Uninitialized data (RAM region, zero-filled) */
        metadata    = 0x0004,   /** @brief Program metadata region */
        interrupt   = 0x0005    /** @brief Interrupt vector table */
    };

    /**
     * @brief   Strongly enumerates the segment attribute flags.
     */
    enum class segment_flags : std::uint16_t
    {
        none        = 0x0000,
        load        = 0x0001,   /** @brief Segment data is loaded from file */
        zero_fill   = 0x0002,   /** @brief Segment is zero-initialized (not loaded) */
        exec        = 0x0004,   /** @brief Segment contains executable code */
        write       = 0x0008    /** @brief Segment is writable at runtime */
    };

    /**
     * @brief   Strongly enumerates the flags for the program info section.
     */
    enum class program_info_flags : std::uint16_t
    {
        none            = 0x0000,
        has_name        = 0x0001,   /** @brief Program name string is present */
        has_version     = 0x0002,   /** @brief Version string is present */
        has_author      = 0x0004,   /** @brief Author string is present */
        has_desc        = 0x0008,   /** @brief Description string is present */
        has_checksum    = 0x0010    /** @brief Checksum field is valid */
    };

    G10_BIT_ENUM(program_flags)
    G10_BIT_ENUM(segment_flags)
    G10_BIT_ENUM(program_info_flags)
}

/* Public Unions and Structures ***********************************************/

namespace g10
{
    /**
     * @brief   Represents the file header of a G10 program file.
     * 
     * This structure matches the binary layout of the header (64 bytes).
     */
    struct program_header final
    {
        std::uint32_t   magic;              /** @brief Magic number (0x47313050) */
        std::uint32_t   version;            /** @brief Format version */
        std::uint32_t   flags;              /** @brief File-level flags */
        std::uint32_t   entry_point;        /** @brief Entry point address */
        std::uint32_t   stack_pointer;      /** @brief Initial stack pointer */
        std::uint32_t   segment_count;      /** @brief Number of segment headers */
        std::uint32_t   info_offset;        /** @brief Offset to program info section */
        std::uint32_t   info_size;          /** @brief Size of program info (bytes) */
        std::uint8_t    reserved[32];       /** @brief Reserved (must be zero) */
    };

    static_assert(sizeof(program_header) == PROGRAM_HEADER_SIZE,
        "program_header size mismatch");

    /**
     * @brief   Represents a segment header in a G10 program file.
     * 
     * This structure matches the binary layout of a segment header (16 bytes).
     */
    struct program_segment_header final
    {
        std::uint32_t   load_address;       /** @brief Target address in G10 memory */
        std::uint32_t   memory_size;        /** @brief Size in memory (bytes) */
        std::uint32_t   file_size;          /** @brief Size in file (bytes); 0 for BSS */
        std::uint16_t   type;               /** @brief Segment type identifier */
        std::uint16_t   flags;              /** @brief Segment attribute flags */
    };

    static_assert(sizeof(program_segment_header) == PROGRAM_SEGMENT_HEADER_SIZE,
        "program_segment_header size mismatch");

    /**
     * @brief   Represents the header of the program info section.
     * 
     * This structure matches the binary layout of the info header (48 bytes).
     */
    struct program_info_header final
    {
        std::uint16_t   info_version;       /** @brief Version of info structure */
        std::uint16_t   flags;              /** @brief Info section flags */
        std::uint32_t   name_offset;        /** @brief Offset to program name string */
        std::uint32_t   name_length;        /** @brief Length of program name (bytes) */
        std::uint32_t   version_offset;     /** @brief Offset to version string */
        std::uint32_t   version_length;     /** @brief Length of version string */
        std::uint32_t   author_offset;      /** @brief Offset to author string */
        std::uint32_t   author_length;      /** @brief Length of author string */
        std::uint32_t   desc_offset;        /** @brief Offset to description string */
        std::uint32_t   desc_length;        /** @brief Length of description string */
        std::uint32_t   build_date;         /** @brief Build timestamp (Unix epoch) */
        std::uint32_t   checksum;           /** @brief CRC-32 of segment data */
        std::uint32_t   reserved;           /** @brief Reserved (must be zero) */
    };

    static_assert(sizeof(program_info_header) == PROGRAM_INFO_HEADER_SIZE,
        "program_info_header size mismatch");

    /**
     * @brief   Represents a segment in the program file (in-memory representation).
     * 
     * This structure holds the segment metadata along with its data bytes.
     */
    struct program_segment final
    {
        std::uint32_t   load_address;       /** @brief Target address in G10 memory */
        std::uint32_t   memory_size;        /** @brief Size in memory (bytes) */
        segment_type    type;               /** @brief Segment type */
        segment_flags   flags;              /** @brief Segment attribute flags */
        std::vector<std::uint8_t> data;     /** @brief Segment data bytes */
    };

    /**
     * @brief   Represents optional program info (in-memory representation).
     */
    struct program_info final
    {
        std::string     name;               /** @brief Program name */
        std::string     version;            /** @brief Program version string */
        std::string     author;             /** @brief Program author */
        std::string     description;        /** @brief Program description */
        std::uint32_t   build_date { 0 };   /** @brief Build timestamp (Unix epoch) */
        std::uint32_t   checksum { 0 };     /** @brief CRC-32 of segment data */
    };
}

/* Public Classes *************************************************************/

namespace g10
{
    /**
     * @brief   Defines a class representing a program file executable by
     *          a virtual machine powered by the G10 CPU architecture.
     */
    class g10api program final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   The default constructor constructs a blank G10 program file.
         */
        program () = default;

        /**
         * @brief   Constructs a G10 program file by linking together the given
         *          G10 object files.
         * 
         * @param   objects     The vector of G10 object files to link.
         */
        explicit program (
            const std::vector<object>& objects
        );

        /**
         * @brief   Constructs a G10 program file by loading it from a file
         *          located at the given path.
         * 
         * @param   path    The path to the G10 program file to load.
         */
        explicit program (const fs::path& path);

        /**
         * @brief   The default destructor.
         */
        ~program () = default;

        /**
         * @brief   Loads the G10 program file from a file located at the given
         *          path.
         * 
         * @param   path    The path to the G10 program file to load.
         * 
         * @return  If loaded successfully and is valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto load_from_file (const fs::path& path) -> result<void>;

        /**
         * @brief   Generates a program file by linking together the given
         *          G10 object files.
         * 
         * @param   objects     The vector of G10 object files to link.
         * 
         * @return  If linked successfully and is valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto link_from_objects (
            const std::vector<object>& objects
        ) -> result<void>;

        /**
         * @brief   Saves the G10 program file to a file located at the given
         *          path.
         * 
         * @param   path    The path to save the G10 program file to.
         * 
         * @return  If saved successfully, returns `true`;
         *          If the program file is empty, returns `false` (a valid
         *          program file is still created and saved in this case);
         *          Otherwise, returns an error message describing the failure.
         */
        auto save_to_file (const fs::path& path) -> result<bool>;

        /**
         * @brief   Reads a byte from the program at the given address.
         * 
         * A G10 program file, once loaded, is flatly mapped into the ROM region
         * at addresses `0x00000000` to `0x7FFFFFFF`, with the following layout,
         * according to the G10 CPU architecture specification:
         * 
         * - `0x00000000` to `0x00000FFF`: Program Metadata (header, tables, etc.)
         * 
         * - `0x00001000` to `0x00001FFF`: Interrupt Vector Subroutines (32 
         *   interrupt vector subroutines, each `0x80` (128) bytes in size)
         * 
         * - `0x00002000` to `0x7FFFFFFF`: Program Code and Data Sections
         * 
         * @param   address     The address to read the byte from. Must be within
         *                      the ROM region (`0x00000000` to `0x7FFFFFFF`).
         * 
         * @return  If `address` is within the valid ROM region and the program's
         *          mapped data, returns the byte at the given address;
         *          Otherwise, returns an open-bus value of `0xFF`.
         */
        auto read_byte (std::uint32_t address) const -> std::uint8_t;

        /**
         * @brief   Retrieves whether or not a program file has been loaded and
         *          validated, or created and saved, successfully.
         * 
         * @return  `true` if the program file is good;
         *          `false` otherwise.
         */
        inline auto is_good () const -> bool
            { return m_good; }

        /**
         * @brief   Retrieves the program's flags.
         * 
         * @return  The program flags.
         */
        inline auto get_flags () const -> program_flags
            { return m_flags; }

        /**
         * @brief   Retrieves the program's entry point address.
         * 
         * @return  The entry point address.
         */
        inline auto get_entry_point () const -> std::uint32_t
            { return m_entry_point; }

        /**
         * @brief   Retrieves the program's initial stack pointer.
         * 
         * @return  The initial stack pointer.
         */
        inline auto get_stack_pointer () const -> std::uint32_t
            { return m_stack_pointer; }

        /**
         * @brief   Retrieves the segments contained in this program.
         * 
         * @return  A constant reference to the vector of segments.
         */
        inline auto get_segments () const -> const std::vector<program_segment>&
            { return m_segments; }

        /**
         * @brief   Retrieves the optional program info metadata.
         * 
         * @return  A constant reference to the program info structure.
         */
        inline auto get_info () const -> const program_info&
            { return m_info; }

        /**
         * @brief   Checks if this program has a valid entry point.
         * 
         * @return  `true` if the HAS_ENTRY flag is set; `false` otherwise.
         */
        inline auto has_entry () const -> bool
            { return (m_flags & program_flags::has_entry) != program_flags::none; }

        /**
         * @brief   Checks if this program specifies an initial stack pointer.
         * 
         * @return  `true` if the HAS_STACK_INIT flag is set; `false` otherwise.
         */
        inline auto has_stack_init () const -> bool
            { return (m_flags & program_flags::has_stack_init) != program_flags::none; }

        /**
         * @brief   Checks if this program contains an info section.
         * 
         * @return  `true` if the HAS_INFO flag is set; `false` otherwise.
         */
        inline auto has_info () const -> bool
            { return (m_flags & program_flags::has_info) != program_flags::none; }

        /**
         * @brief   Clears the program file, resetting it to a blank state.
         */
        auto clear () -> void;

    private: /* Private Methods ***********************************************/

        /**
         * @brief   Validates the G10 program file's internal structure and
         *          data.
         * 
         * @return  If valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto validate () -> result<void>;

    private: /* Private Methods - File Parsing ********************************/

        /**
         * @brief   Parses and validates the program file header from a buffer.
         * 
         * @param   buffer  The file contents buffer.
         * 
         * @return  If valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto parse_header (std::span<const std::uint8_t> buffer) -> result<void>;

        /**
         * @brief   Parses segment headers and data from a buffer.
         * 
         * @param   buffer  The file contents buffer.
         * 
         * @return  If valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto parse_segments (std::span<const std::uint8_t> buffer) -> result<void>;

        /**
         * @brief   Parses the program info section from a buffer.
         * 
         * @param   buffer  The file contents buffer.
         * 
         * @return  If valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto parse_info (std::span<const std::uint8_t> buffer) -> result<void>;

    private: /* Private Methods - Linking *************************************/

        /**
         * @brief   Represents a resolved symbol during linking.
         */
        struct resolved_symbol final
        {
            std::string     name;               /** @brief Symbol name */
            std::uint32_t   address;            /** @brief Final resolved address */
            symbol_type     type;               /** @brief Symbol type */
            symbol_binding  binding;            /** @brief Symbol binding (scope) */
            symbol_flags    flags;              /** @brief Additional symbol flags */
            std::size_t     object_index;       /** @brief Source object file index */
            std::size_t     section_index;      /** @brief Source section index */
        };

        /**
         * @brief   Represents a section during linking with tracking info.
         */
        struct link_section final
        {
            std::size_t             object_index;   /** @brief Source object index */
            std::size_t             section_index;  /** @brief Index within object */
            std::uint32_t           address;        /** @brief Final address */
            std::vector<std::uint8_t> data;         /** @brief Section data (copy) */
            section_type            type;           /** @brief Section type */
            section_flags           flags;          /** @brief Section flags */
        };

        /**
         * @brief   Collects and resolves all symbols from input object files.
         * 
         * @param   objects     The input object files.
         * @param   symbols     Output: resolved symbol table.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message.
         */
        auto collect_symbols (
            const std::vector<object>& objects,
            std::vector<resolved_symbol>& symbols
        ) -> result<void>;

        /**
         * @brief   Collects all sections from input object files for linking.
         * 
         * @param   objects     The input object files.
         * @param   sections    Output: collected sections.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message.
         */
        auto collect_sections (
            const std::vector<object>& objects,
            std::vector<link_section>& sections
        ) -> result<void>;

        /**
         * @brief   Applies relocations to all collected sections.
         * 
         * @param   objects     The input object files.
         * @param   symbols     The resolved symbol table.
         * @param   sections    The sections to patch.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message.
         */
        auto apply_relocations (
            const std::vector<object>& objects,
            const std::vector<resolved_symbol>& symbols,
            std::vector<link_section>& sections
        ) -> result<void>;

        /**
         * @brief   Generates program segments from linked sections.
         * 
         * @param   sections    The linked sections.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message.
         */
        auto generate_segments (
            const std::vector<link_section>& sections
        ) -> result<void>;

        /**
         * @brief   Finds the entry point symbol and sets the entry point address.
         * 
         * @param   symbols     The resolved symbol table.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message.
         */
        auto find_entry_point (
            const std::vector<resolved_symbol>& symbols
        ) -> result<void>;

    private: /* Private Members ***********************************************/

        /**
         * @brief   Indicates whether the program file has been loaded and
         *          validated successfully.
         */
        bool m_good { false };

        /**
         * @brief   The program's file-level flags.
         */
        program_flags m_flags { program_flags::none };

        /**
         * @brief   The program's entry point address.
         */
        std::uint32_t m_entry_point { PROGRAM_DEFAULT_ENTRY };

        /**
         * @brief   The program's initial stack pointer value.
         */
        std::uint32_t m_stack_pointer { PROGRAM_DEFAULT_STACK };

        /**
         * @brief   The segments contained in this program.
         */
        std::vector<program_segment> m_segments;

        /**
         * @brief   Optional program info metadata.
         */
        program_info m_info;

    };
}
