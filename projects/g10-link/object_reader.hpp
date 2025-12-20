/**
 * @file    g10-link/object_reader.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains declarations for reading G10 object files (.g10obj).
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-link/common.hpp>
#include <g10-link/object_file.hpp>

/* Public Classes *************************************************************/

namespace g10link
{
    /**
     * @brief   Defines a class for reading G10 object files.
     * 
     * The object reader parses binary `.g10obj` files produced by the assembler
     * and loads them into memory as `loaded_object_file` structures that can be
     * processed by the linker.
     */
    class object_reader final
    {
    public: /* Public Constructors ********************************************/

        /**
         * @brief   Constructs an object reader for the given file path.
         * 
         * @param   filename    Path to the object file to read.
         */
        explicit object_reader (const std::string& filename);

    public: /* Public Methods ************************************************/

        /**
         * @brief   Reads and parses the object file.
         * 
         * This method performs the complete read operation:
         * - Opens the file
         * - Reads and validates the header
         * - Reads all tables (sections, symbols, relocations)
         * - Reads the string table and code data
         * - Resolves symbol names
         * 
         * @return  If successful, returns the loaded object file;
         *          otherwise, returns an error description.
         */
        auto read () -> g10::result<g10obj::loaded_object_file>;

    private: /* Private Methods - File Reading ********************************/

        /**
         * @brief   Opens the object file for reading.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto open_file () -> g10::result<void>;

        /**
         * @brief   Reads and validates the file header.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto read_header () -> g10::result<void>;

        /**
         * @brief   Reads the section table.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto read_sections () -> g10::result<void>;

        /**
         * @brief   Reads the symbol table.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto read_symbols () -> g10::result<void>;

        /**
         * @brief   Reads the relocation table.
         * 
         * @return  If successful, returns an error description.
         */
        auto read_relocations () -> g10::result<void>;

        /**
         * @brief   Reads the string table.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto read_string_table () -> g10::result<void>;

        /**
         * @brief   Reads the code data block.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto read_code_data () -> g10::result<void>;

        /**
         * @brief   Resolves symbol and source names from the string table.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto resolve_names () -> g10::result<void>;

    private: /* Private Methods - Binary Reading ******************************/

        /**
         * @brief   Reads a single unsigned 8-bit value.
         */
        auto read_u8 () -> g10::result<std::uint8_t>;

        /**
         * @brief   Reads a single unsigned 16-bit value (little-endian).
         */
        auto read_u16 () -> g10::result<std::uint16_t>;

        /**
         * @brief   Reads a single unsigned 32-bit value (little-endian).
         */
        auto read_u32 () -> g10::result<std::uint32_t>;

        /**
         * @brief   Reads a single signed 32-bit value (little-endian).
         */
        auto read_i32 () -> g10::result<std::int32_t>;

        /**
         * @brief   Reads raw bytes into a buffer.
         * 
         * @param   buffer  Pointer to the destination buffer.
         * @param   size    Number of bytes to read.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto read_bytes (void* buffer, std::size_t size) -> g10::result<void>;

    private: /* Private Methods - String Table Helpers ************************/

        /**
         * @brief   Extracts a null-terminated string from the string table.
         * 
         * @param   offset  Offset into the string table.
         * 
         * @return  If successful, returns the string;
         *          otherwise, returns an error description.
         */
        auto get_string (std::uint32_t offset) const -> g10::result<std::string>;

    private: /* Private Member Variables **************************************/

        std::string                     m_filename;         /** @brief Object file path */
        std::ifstream                   m_file;             /** @brief Input file stream */
        g10obj::file_header             m_header;           /** @brief Parsed file header */
        g10obj::loaded_object_file      m_object;           /** @brief Loaded object file */
        std::vector<std::uint8_t>       m_string_table;     /** @brief String table buffer */
        std::vector<std::uint8_t>       m_code_data;        /** @brief Code data buffer */
    };

}
