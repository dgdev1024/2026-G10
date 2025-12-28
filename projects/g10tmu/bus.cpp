/**
 * @file    g10tmu/bus.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains definitions for the G10 Testbed Emulator's system bus.
 */

/* Private Includes ***********************************************************/

#include <g10tmu/bus.hpp>

/* Public Methods *************************************************************/

namespace g10tmu
{
    bus::bus (const fs::path& program_path, const std::size_t ram_size) :
        m_ram       (ram_size, 0),
        m_program   { program_path },
        m_cpu       { *this },
        m_timer     { *this }
    {
        m_cpu.reset();  // Call CPU's `reset` to initialize the bus as well.
    }

    bus::~bus ()
    {
        // Nothing to do here.
    }

    auto bus::reset () -> void
    {
        // - Clear RAM
        std::fill(m_ram.begin(), m_ram.end(), 0);

        // - Reset the timer component
        m_timer.reset();

        m_cpu.set_pc(m_program.get_entry_point());
        m_cpu.set_sp(0x80000000 + m_ram.size());
    }

    auto bus::tick () -> bool
    {
        // - Tick the timer component.
        //   The timer is ticked every T-cycle.
        if (m_timer.tick() == false)
        {
            return false;
        }

        return true;   
    }

    auto bus::read (std::uint32_t address) -> std::uint8_t
    {
        // - `$00000000` to `$7FFFFFFF`: Program ROM region
        if (address <= g10::PROGRAM_ROM_END)
        {
            return m_program.read_byte(address);
        }
        // - `$80000000` to `$FFFFFFFF`: System RAM region
        else if (
            address >= g10::PROGRAM_RAM_START &&
            address < (g10::PROGRAM_RAM_START + m_ram.size())
        )
        {
            return m_ram[address - g10::PROGRAM_RAM_START];
        }
        else
        {
            // G10TMU Hardware Registers:
            // - `$FFFFFF00`: `IRQ0` - Interrupt Request Register - Byte 0
            // - `$FFFFFF01`: `IRQ1` - Interrupt Request Register - Byte 1
            // - `$FFFFFF02`: `IRQ2` - Interrupt Request Register - Byte 2
            // - `$FFFFFF03`: `IRQ3` - Interrupt Request Register - Byte 3
            // - `$FFFFFF04`: `IE0`  - Interrupt Enable Register - Byte 0
            // - `$FFFFFF05`: `IE1`  - Interrupt Enable Register - Byte 1
            // - `$FFFFFF06`: `IE2`  - Interrupt Enable Register - Byte 2
            // - `$FFFFFF07`: `IE3`  - Interrupt Enable Register - Byte 3
            // - `$FFFFFF08`: `SPD` - CPU Speed Switch Register
            // - `$FFFFFF09`: `DIV` - Timer Divider
            // - `$FFFFFF0A`: `TIMA` - Timer Counter
            // - `$FFFFFF0B`: `TMA` - Timer Modulo
            // - `$FFFFFF0C`: `TAC` - Timer Control

            // - Check for port registers, hardware devices, etc.
            switch (address)
            {
                case 0xFFFFFF00: return m_cpu.read_irq0();
                case 0xFFFFFF01: return m_cpu.read_irq1();
                case 0xFFFFFF02: return m_cpu.read_irq2();
                case 0xFFFFFF03: return m_cpu.read_irq3();
                case 0xFFFFFF04: return m_cpu.read_ie0();
                case 0xFFFFFF05: return m_cpu.read_ie1();
                case 0xFFFFFF06: return m_cpu.read_ie2();
                case 0xFFFFFF07: return m_cpu.read_ie3();
                case 0xFFFFFF08: return m_cpu.read_spd();
                case 0xFFFFFF09: return m_timer.read_div();
                case 0xFFFFFF0A: return m_timer.read_tima();
                case 0xFFFFFF0B: return m_timer.read_tma();
                case 0xFFFFFF0C: return m_timer.read_tac();
                default:
                    return 0xFF;  // Unmapped address
            }
        }
    }

    auto bus::write (std::uint32_t address, std::uint8_t value)
        -> std::uint8_t
    {
        // - `$80000000` to `$FFFFFFFF`: System RAM region
        if (
            address >= g10::PROGRAM_RAM_START &&
            address < (g10::PROGRAM_RAM_START + m_ram.size())
        )
        {
            m_ram[address - g10::PROGRAM_RAM_START] = value;
            return value;
        }
        else
        {
            // - Check for port registers, hardware devices, etc.
            switch (address)
            {
                case 0xFFFFFF00: return m_cpu.write_irq0(value);
                case 0xFFFFFF01: return m_cpu.write_irq1(value);
                case 0xFFFFFF02: return m_cpu.write_irq2(value);
                case 0xFFFFFF03: return m_cpu.write_irq3(value);
                case 0xFFFFFF04: return m_cpu.write_ie0(value);
                case 0xFFFFFF05: return m_cpu.write_ie1(value);
                case 0xFFFFFF06: return m_cpu.write_ie2(value);
                case 0xFFFFFF07: return m_cpu.write_ie3(value);
                case 0xFFFFFF08: return m_cpu.write_spd(value);
                case 0xFFFFFF09: return m_timer.write_div(value);
                case 0xFFFFFF0A: return m_timer.write_tima(value);
                case 0xFFFFFF0B: return m_timer.write_tma(value);
                case 0xFFFFFF0C: return m_timer.write_tac(value);
                default:
                    return 0xFF;  // Unmapped address
            }
        }
    }

    auto bus::start () -> std::int32_t
    {
        // - Main emulation loop
        while (m_cpu.is_stopped() == false)
        {
            m_cpu.tick();
            if (m_cpu.get_ec() != g10::EC_OK)
            {
                std::println("CPU exception occurred: 0x{:02X}",
                    m_cpu.get_ec());
                break;
            }
        }

        // - CPU has stopped execution.
        if (m_cpu.get_ec() != g10::EC_OK)
        {
            // - Emulation ended due to an exception.
            std::println("Emulation ended with exception code: 0x{:02X}",
                m_cpu.get_ec());
            return static_cast<std::int32_t>(m_cpu.get_ec());
        }
        
        // - Emulation ended successfully.
        return 0;
    }
}
