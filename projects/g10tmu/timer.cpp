/**
 * @file    g10tmu/timer.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains definitions for the G10 Testbed Emulator's timer component.
 */

/* Private Includes ***********************************************************/

#include <g10tmu/timer.hpp>
#include <g10tmu/bus.hpp>

/* Public Methods *************************************************************/

namespace g10tmu
{
    timer::timer (bus& parent_bus) :
        m_parent_bus    { parent_bus },
        m_div           { 0x0000 },
        m_tima          { 0x00 },
        m_tma           { 0x00 },
        m_tac           { 0xF8 },
        m_old_div       { 0x0000 }
    {
        // Constructor body intentionally empty.
        // Initial values are set in the initializer list above.
    }

    timer::~timer ()
    {
        // Destructor body intentionally empty.
        // No dynamic resources to release.
    }

    auto timer::reset () -> void
    {
        // - Reset internal divider counter to 0.
        m_div = 0x0000;
        m_old_div = 0x0000;

        // - Reset timer counter (TIMA) to 0.
        m_tima = 0x00;

        // - Reset timer modulo (TMA) to 0.
        m_tma = 0x00;

        // - Reset timer control (TAC) to default value.
        //   Bits 3-7 are unused and read as 1, bits 0-2 are cleared.
        m_tac = 0xF8;
    }

    auto timer::tick () -> bool
    {
        // - Save old divider value for falling edge detection.
        m_old_div = m_div;

        // - Increment the internal divider counter.
        //   The divider increments every T-cycle (tick).
        m_div++;

        // - If the timer is disabled, nothing more to do.
        if (is_enabled() == false)
        {
            return true;
        }

        // - Get the divider bit to monitor based on current clock speed.
        std::uint8_t divider_bit = get_divider_bit();

        // - Check for falling edge on the selected divider bit.
        if (check_falling_edge(divider_bit) == false)
        {
            // - No falling edge detected; nothing more to do.
            return true;
        }

        // - Falling edge detected; increment TIMA.
        m_tima++;

        // - Check for TIMA overflow (went from 0xFF to 0x00).
        if (m_tima == 0x00)
        {
            // - TIMA overflowed; reset it to the value in TMA.
            m_tima = m_tma;

            // - Request a timer interrupt by setting the corresponding bit
            //   in the CPU's IRQ (Interrupt Request) register.
            //   The timer interrupt is at vector 2, so we set bit 2 of IRQ.
            auto& cpu = m_parent_bus.get_cpu();
            cpu.request_interrupt(TIMER_INTERRUPT_VECTOR);
        }

        return true;
    }

    auto timer::check_falling_edge (std::uint8_t divider_bit) const -> bool
    {
        // - If the divider_bit is out of range, default to the bit selected
        //   by the `TAC` register.
        if (divider_bit > 15)
        {
            divider_bit = get_divider_bit();
        }

        // - Extract the specified bit from the old and new divider values.
        bool old_bit = ((m_old_div >> divider_bit) & 0x01) != 0;
        bool new_bit = ((m_div >> divider_bit) & 0x01) != 0;

        // - A falling edge occurs when the bit transitions from 1 to 0.
        return (old_bit == true) && (new_bit == false);
    }
}

/* Public Methods - Hardware Registers ****************************************/

namespace g10tmu
{
    auto timer::read_div () const -> std::uint8_t
    {
        // - DIV exposes the upper 8 bits of the internal 16-bit divider.
        return static_cast<std::uint8_t>((m_div >> 8) & 0xFF);
    }

    auto timer::read_tima () const -> std::uint8_t
    {
        // - All 8 bits of TIMA are readable.
        return m_tima;
    }

    auto timer::read_tma () const -> std::uint8_t
    {
        // - All 8 bits of TMA are readable.
        return m_tma;
    }

    auto timer::read_tac () const -> std::uint8_t
    {
        // - Bits 3-7 are unused and read as 1.
        // - Bits 0-2 contain the timer control bits.
        return 0b11111000 | (m_tac & 0b00000111);
    }

    auto timer::write_div (std::uint8_t value) -> std::uint8_t
    {
        // - Writing any value to DIV resets the internal divider to 0.
        //   The written value is ignored.
        
        // - Before resetting, check if this write causes a falling edge
        //   on the currently-monitored divider bit.
        if (is_enabled())
        {
            std::uint8_t divider_bit = get_divider_bit();
            std::uint16_t old_div = m_div;
            
            // - The new divider value will be 0.
            //   Check if the selected bit was 1 in the old value.
            bool old_bit = ((old_div >> divider_bit) & 0x01) != 0;
            
            // - In the new value (0), all bits are 0, so new_bit is always 0.
            //   If old_bit was 1, this is a falling edge.
            if (old_bit)
            {
                // - Increment TIMA due to falling edge caused by DIV reset.
                m_tima++;
                
                // - Check for overflow.
                if (m_tima == 0x00)
                {
                    m_tima = m_tma;
                    
                    auto& cpu = m_parent_bus.get_cpu();
                    std::uint8_t irq0 = cpu.read_irq0();
                    irq0 |= (1 << TIMER_INTERRUPT_VECTOR);
                    cpu.write_irq0(irq0);
                }
            }
        }
        
        m_div = 0x0000;
        m_old_div = 0x0000;
        
        return 0x00;
    }

    auto timer::write_tima (std::uint8_t value) -> std::uint8_t
    {
        // - All 8 bits of TIMA are writable.
        m_tima = value;
        return m_tima;
    }

    auto timer::write_tma (std::uint8_t value) -> std::uint8_t
    {
        // - All 8 bits of TMA are writable.
        m_tma = value;
        return m_tma;
    }

    auto timer::write_tac (std::uint8_t value) -> std::uint8_t
    {
        // - Save old TAC to check for clock speed changes that may cause
        //   a spurious TIMA increment.
        std::uint8_t old_tac = m_tac;
        bool old_enabled = is_enabled();
        std::uint8_t old_divider_bit = get_divider_bit();
        bool old_bit_value = ((m_div >> old_divider_bit) & 0x01) != 0;
        
        // - Bits 3-7 are unused; write as 1.
        // - Bits 0-2 are writable.
        m_tac = 0b11111000 | (value & 0b00000111);
        
        // - Check for "glitch" behavior: if the old configuration had a 1
        //   on the monitored bit, and either:
        //   a) The timer was just disabled, OR
        //   b) The new clock speed selects a bit that is now 0,
        //   then this causes a falling edge and TIMA should increment.
        
        bool new_enabled = is_enabled();
        std::uint8_t new_divider_bit = get_divider_bit();
        bool new_bit_value = ((m_div >> new_divider_bit) & 0x01) != 0;
        
        // - The "glitch" occurs when:
        //   (old_enabled AND old_bit_value) is true, AND
        //   (new_enabled AND new_bit_value) is false.
        //   This represents a falling edge on the "AND" of enable and divider bit.
        bool old_and = old_enabled && old_bit_value;
        bool new_and = new_enabled && new_bit_value;
        
        if (old_and && !new_and)
        {
            // - Spurious TIMA increment due to TAC write glitch.
            m_tima++;
            
            if (m_tima == 0x00)
            {
                m_tima = m_tma;
                
                auto& cpu = m_parent_bus.get_cpu();
                std::uint8_t irq0 = cpu.read_irq0();
                irq0 |= (1 << TIMER_INTERRUPT_VECTOR);
                cpu.write_irq0(irq0);
            }
        }
        
        return m_tac;
    }
}

/* Private Methods ************************************************************/

namespace g10tmu
{
    auto timer::get_divider_bit () const -> std::uint8_t
    {
        // - Map clock speed to the corresponding divider bit to monitor.
        //   The bit selected determines how often TIMA is incremented.
        //
        //   For the Game Boy (and G10 adaptation):
        //   - 4096 Hz:   Bit 9 (divider increments 512 times = 256 M-cycles)
        //   - 262144 Hz: Bit 3 (divider increments 8 times = 4 M-cycles)
        //   - 65536 Hz:  Bit 5 (divider increments 32 times = 16 M-cycles)
        //   - 16384 Hz:  Bit 7 (divider increments 128 times = 64 M-cycles)
        //
        //   Note: The G10 CPU uses 2 T-cycles per M-cycle (matching the
        //   Game Boy's relationship), so the bit mappings are the same.
        
        switch (get_clock_speed())
        {
            case timer_clock_speed::hz_4096:
                return 9;
            case timer_clock_speed::hz_262144:
                return 3;
            case timer_clock_speed::hz_65536:
                return 5;
            case timer_clock_speed::hz_16384:
                return 7;
            default:
                return 9;  // Default to 4096 Hz
        }
    }
}
