/**
 * @file    g10tmu/timer.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains declarations for the G10 Testbed Emulator's timer component.
 * 
 * This timer component is modeled after the Game Boy's timer hardware,
 * adapted to work with the G10's 32-bit architecture. The timer provides
 * a divider register (DIV), a timer counter (TIMA), a timer modulo (TMA),
 * and a timer control register (TAC).
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/common.hpp>

/* Public Forward Declarations ************************************************/

namespace g10tmu
{
    class bus;
}

/* Public Constants and Enumerations ******************************************/

namespace g10tmu
{
    /**
     * @brief   The interrupt vector number for the timer interrupt.
     * 
     * When the timer counter (TIMA) overflows, this interrupt is requested
     * by setting the corresponding bit in the CPU's IRQ register.
     */
    constexpr std::uint8_t TIMER_INTERRUPT_VECTOR = 3;

    /**
     * @brief   Enumerates the four possible clock speeds for the timer,
     *          selectable by bits 0-1 of the TAC register.
     */
    enum class timer_clock_speed : std::uint8_t
    {
        hz_4096     = 0b00,     /** @brief 4096 Hz (256 M-cycles per increment) */
        hz_262144   = 0b01,     /** @brief 262144 Hz (4 M-cycles per increment) */
        hz_65536    = 0b10,     /** @brief 65536 Hz (16 M-cycles per increment) */
        hz_16384    = 0b11      /** @brief 16384 Hz (64 M-cycles per increment) */
    };
}

/* Public Classes *************************************************************/

namespace g10tmu
{
    /**
     * @brief   Represents the G10 Testbed Emulator's timer component.
     * 
     * The timer provides the following hardware registers:
     * 
     * - `DIV` (`$FFFFFF09`): Timer Divider - Upper 8 bits of internal 16-bit counter.
     * 
     * - `TIMA` (`$FFFFFF0A`): Timer Counter - Incremented at rate set by TAC.
     * 
     * - `TMA` (`$FFFFFF0B`): Timer Modulo - Value loaded into TIMA on overflow.
     * 
     * - `TAC` (`$FFFFFF0C`): Timer Control - Enable flag and clock speed select.
     * 
     * The timer works by monitoring a specific bit in the internal divider
     * counter. When that bit transitions from 1 to 0 (falling edge) and the
     * timer is enabled, TIMA is incremented. When TIMA overflows from 0xFF
     * to 0x00, it is reset to the value in TMA and a timer interrupt is
     * requested.
     */
    class timer final
    {
    public:
        /**
         * @brief   Constructs a new timer component.
         * 
         * @param   parent_bus  A reference to the parent system bus that owns
         *                      this timer component.
         */
        explicit timer (bus& parent_bus);

        /**
         * @brief   The timer component's destructor.
         */
        ~timer ();

        /**
         * @brief   Resets the timer to its default, power-on state.
         * 
         * This sets the internal divider to 0, TIMA to 0, TMA to 0,
         * and TAC to 0xF8 (timer disabled, clock speed 4096 Hz).
         */
        auto reset () -> void;

        /**
         * @brief   Ticks the timer, advancing its internal state by one T-cycle.
         * 
         * This method increments the internal divider counter, checks for
         * falling edge on the selected divider bit (if timer is enabled),
         * increments TIMA if appropriate, and handles TIMA overflow by
         * resetting TIMA to TMA and requesting a timer interrupt.
         * 
         * @return  Returns `true` if the tick was successful; `false` if an
         *          error occurred.
         */
        auto tick () -> bool;

        /**
         * @brief   Checks for a falling edge on the selected divider bit.
         * 
         * A falling edge occurs when the bit transitions from 1 to 0 between
         * the old and new divider values.
         * 
         * @param   divider_bit     The bit index to check (0-15). Defaults to
         *                          the bit selected by the current TAC setting.
         * 
         * @return  `true` if a falling edge was detected; `false` otherwise.
         */
        auto check_falling_edge (std::uint8_t divider_bit = 0xFF) const -> bool;

        /**
         * @brief   Checks whether the timer is currently enabled.
         * 
         * @return  `true` if the timer is enabled (TAC bit 2 is set);
         *          `false` otherwise.
         */
        inline auto is_enabled () const -> bool
            { return (m_tac & 0b00000100) != 0; }

        /**
         * @brief   Gets the current clock speed setting.
         * 
         * @return  The timer_clock_speed value corresponding to bits 0-1 of TAC.
         */
        inline auto get_clock_speed () const -> timer_clock_speed
            { return static_cast<timer_clock_speed>(m_tac & 0b00000011); }

    public: /* Public Methods - Hardware Registers ****************************/

        /**
         * @brief   Reads the DIV (Timer Divider) register.
         * 
         * The DIV register exposes the upper 8 bits of the internal 16-bit
         * divider counter. The divider increments every T-cycle and is
         * automatically reset when the CPU is in STOP state.
         * 
         * @return  The current value of the DIV register.
         */
        auto read_div () const -> std::uint8_t;

        /**
         * @brief   Reads the TIMA (Timer Counter) register.
         * 
         * TIMA is incremented at the clock frequency specified by TAC.
         * When TIMA overflows (goes from 0xFF to 0x00), it is reset to
         * the value in TMA and a timer interrupt is requested.
         * 
         * @return  The current value of the TIMA register.
         */
        auto read_tima () const -> std::uint8_t;

        /**
         * @brief   Reads the TMA (Timer Modulo) register.
         * 
         * TMA contains the value that TIMA is reset to when it overflows.
         * 
         * @return  The current value of the TMA register.
         */
        auto read_tma () const -> std::uint8_t;

        /**
         * @brief   Reads the TAC (Timer Control) register.
         * 
         * TAC controls the timer's operation:
         * - Bit 2: Timer enable (0 = disabled, 1 = enabled)
         * - Bits 0-1: Clock speed selection (see timer_clock_speed enum)
         * - Bits 3-7: Unused, read as 1.
         * 
         * @return  The current value of the TAC register.
         */
        auto read_tac () const -> std::uint8_t;

        /**
         * @brief   Writes to the DIV (Timer Divider) register.
         * 
         * Writing any value to DIV resets the internal 16-bit divider
         * counter to 0x0000. The written value is ignored.
         * 
         * @param   value   The value to write (ignored; any write resets DIV).
         * 
         * @return  The actual value written to the register (always 0x00).
         */
        auto write_div (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes to the TIMA (Timer Counter) register.
         * 
         * @param   value   The new value for the TIMA register.
         * 
         * @return  The actual value written to the register.
         */
        auto write_tima (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes to the TMA (Timer Modulo) register.
         * 
         * @param   value   The new value for the TMA register.
         * 
         * @return  The actual value written to the register.
         */
        auto write_tma (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes to the TAC (Timer Control) register.
         * 
         * Only bits 0-2 are writable:
         * - Bit 2: Timer enable (0 = disabled, 1 = enabled)
         * - Bits 0-1: Clock speed selection
         * - Bits 3-7: Unused, always read/write as 1.
         * 
         * Note: Changing TAC may cause a spurious TIMA increment if the
         * currently-monitored divider bit goes from 1 to 0 as a result
         * of the clock speed change.
         * 
         * @param   value   The new value for the TAC register.
         * 
         * @return  The actual value written to the register.
         */
        auto write_tac (std::uint8_t value) -> std::uint8_t;

    private:
        /**
         * @brief   Gets the divider bit index to monitor based on the current
         *          clock speed setting.
         * 
         * The timer monitors a specific bit in the internal divider counter.
         * When this bit transitions from 1 to 0 (falling edge), TIMA is
         * incremented (if the timer is enabled).
         * 
         * Clock speed to divider bit mapping:
         * - 4096 Hz:   Bit 9 (divider >> 9, every 512 T-cycles = 256 M-cycles)
         * - 262144 Hz: Bit 3 (divider >> 3, every 8 T-cycles = 4 M-cycles)
         * - 65536 Hz:  Bit 5 (divider >> 5, every 32 T-cycles = 16 M-cycles)
         * - 16384 Hz:  Bit 7 (divider >> 7, every 128 T-cycles = 64 M-cycles)
         * 
         * @return  The bit index (0-15) of the divider bit to monitor.
         */
        auto get_divider_bit () const -> std::uint8_t;

    private:
        bus& m_parent_bus;              /** @brief Reference to parent system bus. */

        // Hardware Registers
        std::uint16_t m_div;            /** @brief Internal 16-bit divider counter. */
        std::uint8_t m_tima;            /** @brief Timer counter register. */
        std::uint8_t m_tma;             /** @brief Timer modulo register. */
        std::uint8_t m_tac;             /** @brief Timer control register. */

        // Internal State
        std::uint16_t m_old_div;        /** @brief Previous divider value for falling edge detection. */
    };
}
