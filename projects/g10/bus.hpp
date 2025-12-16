/**
 * @file    g10/bus.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-14
 * 
 * @brief   Contains declarations for the G10 CPU's system bus base interface.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/common.hpp>

/* Public Classes *************************************************************/

namespace g10
{
    /**
     * @brief   Defines an interface representing the system bus of the G10 CPU.
     * 
     * The G10 CPU connects to a system bus, through which it accesses memory,
     * other virtual hardware components, and peripheral devices, and ticks those
     * devices' internal clocks whenever the CPU itself consumes T-cycles.
     */
    class g10api bus
    {
    public:

        /**
         * @brief   The base interface's virtual destructor.
         */
        virtual ~bus () = default;

        /**
         * @brief   Resets the CPU's connected system bus, setting all buffers,
         *          registers and internal states of all connected devices to
         *          their default, power-on values.
         * 
         * This method is called internally by the G10 CPU implementation whenever
         * the CPU is reset.
         */
        virtual auto reset () -> void = 0;

        /**
         * @brief   Ticks the CPU's connected system bus, advancing the internal
         *          clocks and states of all connected devices by one T-cycle.
         * 
         * This method is called internally by the G10 CPU implementation whenever
         * the CPU consumes T-cycles, to ensure that all connected devices remain
         * synchronized with the CPU's own internal clock.
         * 
         * @return  If all connected devices ticked without errors, returns `true`;
         *          Otherwise, returns `false`.
         */
        virtual auto tick () -> bool = 0;

        /**
         * @brief   Reads one byte of data from the specified address on the
         *          CPU's connected system bus.
         * 
         * @param   address     The absolute address of the byte to read.
         * 
         * @return  The byte read from the specified address.
         */
        virtual auto read (std::uint32_t address) -> std::uint8_t = 0;

        /**
         * @brief   Writes one byte of data to the specified address on the
         *          CPU's connected system bus.
         * 
         * @param   address     The absolute address to which to write the
         *                      specified byte.
         * @param   value       The byte to write to the specified address.
         * 
         * @return  The byte which was actually written to the specified address,
         *          which may differ from the requested value due to bus
         *          behavior.
         */
        virtual auto write (std::uint32_t address, std::uint8_t value)
            -> std::uint8_t = 0;

    protected:

        /**
         * @brief   The interface's default constructor, protected to prevent
         *          direct instantiation.
         */
        bus () = default;

    };
}
