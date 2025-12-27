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

}

/* Public Unions and Structures ***********************************************/

namespace g10
{

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

    private: /* Private Methods ***********************************************/

        /**
         * @brief   Validates the G10 program file's internal structure and
         *          data.
         * 
         * @return  If valid, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        auto validate () -> result<void>;

    private: /* Private Members ***********************************************/

        /**
         * @brief   Indicates whether the program file has been loaded and
         *          validated successfully.
         */
        bool m_good { false };

    };
}
