/**
 * @file    g10-asm/object_writer.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains declarations for the G10 CPU assembler's object file
 *          writer component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/object_file.hpp>

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing an object file writer for the G10
     *          Assembler.
     * 
     * This class is responsible for converting the code generator's output
     * into a valid G10 object file (.g10obj) that can be processed by the
     * linker. It writes the file in the format specified in g10obj.spec.md,
     * with all multi-byte values in little-endian byte order.
     */
    class object_writer final
    {
    public: /* Public Constructors ********************************************/

        /**
         * @brief   Constructs an object writer for the given object file.
         * 
         * @param   obj     Reference to the in-memory object file.
         */
        explicit object_writer (const g10obj::object_file& obj);

    public: /* Public Methods ************************************************/

        /**
         * @brief   Writes the object file to disk.
         * 
         * @param   filename    Path to the output file.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write (const std::string& filename) -> g10::result<void>;

        /**
         * @brief   Writes the object file to an output stream.
         * 
         * @param   out     Output stream (must be opened in binary mode).
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write (std::ostream& out) -> g10::result<void>;

    private: /* Private Methods - Preparation ********************************/

        /**
         * @brief   Prepares the object file data for writing.
         * 
         * This method builds the string table with proper offsets,
         * calculates section offsets, and validates the data.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto prepare () -> g10::result<void>;

        /**
         * @brief   Builds the string table from symbol names.
         * 
         * Adds all symbol names and the source filename to the string table,
         * recording their offsets for later use. Uses string pooling to avoid
         * duplicates.
         */
        auto build_string_table () -> void;

        /**
         * @brief   Calculates code data offsets for each section.
         * 
         * Sections are concatenated in the code data block; this method
         * calculates the offset of each section within that block.
         */
        auto calculate_section_offsets () -> void;

        /**
         * @brief   Adds a string to the string table and returns its offset.
         * 
         * If the string already exists in the table (string pooling), returns
         * the existing offset. Otherwise, appends the string with a null
         * terminator and returns the new offset.
         * 
         * @param   str     The string to add.
         * 
         * @return  The offset of the string in the string table.
         */
        auto add_string (const std::string& str) -> std::uint32_t;

    private: /* Private Methods - Writing ************************************/

        /**
         * @brief   Writes the file header (32 bytes).
         * 
         * @param   out     Output stream.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_header (std::ostream& out) -> g10::result<void>;

        /**
         * @brief   Writes the section table.
         * 
         * @param   out     Output stream.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_section_table (std::ostream& out) -> g10::result<void>;

        /**
         * @brief   Writes the symbol table.
         * 
         * @param   out     Output stream.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_symbol_table (std::ostream& out) -> g10::result<void>;

        /**
         * @brief   Writes the relocation table.
         * 
         * @param   out     Output stream.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_relocation_table (std::ostream& out) -> g10::result<void>;

        /**
         * @brief   Writes the string table.
         * 
         * @param   out     Output stream.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_string_table (std::ostream& out) -> g10::result<void>;

        /**
         * @brief   Writes the code data (all sections concatenated).
         * 
         * @param   out     Output stream.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_code_data (std::ostream& out) -> g10::result<void>;

    private: /* Private Methods - Binary Output ******************************/

        /**
         * @brief   Writes a single byte to the output stream.
         * 
         * @param   out     Output stream.
         * @param   value   Byte value to write.
         */
        auto write_u8 (std::ostream& out, std::uint8_t value) -> void;

        /**
         * @brief   Writes a 16-bit value in little-endian format.
         * 
         * @param   out     Output stream.
         * @param   value   16-bit value to write.
         */
        auto write_u16 (std::ostream& out, std::uint16_t value) -> void;

        /**
         * @brief   Writes a 32-bit value in little-endian format.
         * 
         * @param   out     Output stream.
         * @param   value   32-bit value to write.
         */
        auto write_u32 (std::ostream& out, std::uint32_t value) -> void;

        /**
         * @brief   Writes a signed 32-bit value in little-endian format.
         * 
         * @param   out     Output stream.
         * @param   value   Signed 32-bit value to write.
         */
        auto write_i32 (std::ostream& out, std::int32_t value) -> void;

    private: /* Private Member Variables *************************************/

        const g10obj::object_file&  m_obj;                  /** @brief Object file to write */
        std::string                 m_string_table;         /** @brief Built string table */
        std::vector<std::uint32_t>  m_section_offsets;      /** @brief Code offset per section */
        std::vector<std::uint32_t>  m_symbol_name_offsets;  /** @brief String offset per symbol */
        std::uint32_t               m_source_name_offset;   /** @brief String offset for source */
        
        /**
         * @brief   Maps symbol names to their indices in the symbol table.
         * 
         * Used for resolving symbol references in relocation entries.
         */
        std::unordered_map<std::string, std::uint16_t> m_symbol_index_map;
    };
}
