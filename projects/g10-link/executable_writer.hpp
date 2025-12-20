/**
 * @file    g10-link/executable_writer.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains declarations for writing G10 executable files (.g10).
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-link/common.hpp>
#include <g10-link/executable_file.hpp>

/* Public Classes *************************************************************/

namespace g10link
{
    /**
     * @brief   Defines a class for writing G10 executable files.
     * 
     * The executable writer takes an in-memory `executable_file` structure
     * and writes it as a binary `.g10` file to disk.
     */
    class executable_writer final
    {
    public: /* Public Constructors ********************************************/

        /**
         * @brief   Constructs an executable writer for the given file path.
         * 
         * @param   filename    Path to the executable file to write.
         */
        explicit executable_writer (const std::string& filename);

    public: /* Public Methods ************************************************/

        /**
         * @brief   Writes the executable file to disk.
         * 
         * This method performs the complete write operation:
         * - Validates the executable structure
         * - Opens the output file
         * - Writes the file header
         * - Writes the segment table
         * - Writes all segment data
         * 
         * @param   executable  The executable file structure to write.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write (const g10exe::executable_file& executable) -> g10::result<void>;

    private: /* Private Methods - File Writing ********************************/

        /**
         * @brief   Opens the output file for writing.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto open_file () -> g10::result<void>;

        /**
         * @brief   Validates the executable structure before writing.
         * 
         * @param   executable  The executable file to validate.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto validate (const g10exe::executable_file& executable) const -> g10::result<void>;

        /**
         * @brief   Writes the file header.
         * 
         * @param   executable  The executable file structure.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_header (const g10exe::executable_file& executable) -> g10::result<void>;

        /**
         * @brief   Writes the segment table.
         * 
         * @param   executable  The executable file structure.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_segment_table (const g10exe::executable_file& executable) -> g10::result<void>;

        /**
         * @brief   Writes all segment data.
         * 
         * @param   executable  The executable file structure.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_segment_data (const g10exe::executable_file& executable) -> g10::result<void>;

    private: /* Private Methods - Binary Writing ******************************/

        /**
         * @brief   Writes a single unsigned 8-bit value.
         */
        auto write_u8 (std::uint8_t value) -> g10::result<void>;

        /**
         * @brief   Writes a single unsigned 16-bit value (little-endian).
         */
        auto write_u16 (std::uint16_t value) -> g10::result<void>;

        /**
         * @brief   Writes a single unsigned 32-bit value (little-endian).
         */
        auto write_u32 (std::uint32_t value) -> g10::result<void>;

        /**
         * @brief   Writes raw bytes from a buffer.
         * 
         * @param   buffer  Pointer to the source buffer.
         * @param   size    Number of bytes to write.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto write_bytes (const void* buffer, std::size_t size) -> g10::result<void>;

    private: /* Private Member Variables **************************************/

        std::string     m_filename;     /** @brief Output file path */
        std::ofstream   m_file;         /** @brief Output file stream */
    };

}
