/**
 * @file    g10/cpu.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-14
 * 
 * @brief   Contains definitions for the G10 CPU context class.
 */

/* Private Includes ***********************************************************/

#include <g10/bus.hpp>
#include <g10/cpu.hpp>

/* Private Constants and Enumerations *****************************************/

namespace g10
{
    /**
     * @brief   The starting address of the CPU's Interrupt Vector Table (IVT).
     */
    static constexpr std::uint32_t IVT_START = 0x00001000;

    /**
     * @brief   The default power-on value for the CPU's program counter (`PC`)
     *          register.
     */
    static constexpr std::uint32_t DEFAULT_PC = 0x00002000;

    /**
     * @brief   The default power-on value for the CPU's stack pointer (`SP`)
     *          register.
     */
    static constexpr std::uint32_t DEFAULT_SP = 0xFFFFFFFF;
}

/* Public Functions ***********************************************************/

namespace g10
{
    cpu::cpu (bus& bus) :
        m_bus   { bus }
    {
        reset();
    }

    auto cpu::reset () -> void
    {
        // Reset all general-purpose registers to 0
        for (auto& reg : m_regs.gp)
        {
            reg = 0;
        }

        // Reset special-purpose registers to their default power-on values
        m_regs.pc = DEFAULT_PC;
        m_regs.sp = DEFAULT_SP;
        m_regs.ie = 0b1;                // IV #0 always enabled
        m_regs.irq = 0;
        m_regs.flags.raw = 0b10000000;  // Set Zero flag to 1
        m_regs.ec = 0;

        // Reset hardware registers.
        m_speed_switch_reg.raw = 0;

        // Clear fetch state
        m_fetch_address = 0;
        m_fetch_data = 0;
        m_opcode = 0;
        m_opcode_address = 0;

        // Clear internal CPU state flags
        m_stopped = false;
        m_double_fault = false;
        m_halted = false;
        m_ime = false;
        m_imp = false;
        m_handling_exception = false;
        m_speed_switching = false;

        // Reset the connected system bus
        m_bus.reset();
    }

    auto cpu::wake () -> void
    {
        // - If the CPU is not currently stopped, or if it is double-faulted,
        //   do nothing.
        if (m_stopped == false || m_double_fault == true)
            { return; }

        // - Clear the stopped state.
        m_stopped = false;
    }

    auto cpu::tick () -> bool
    {
        // - If the CPU is stopped, do nothing and return `true`.
        if (is_stopped() == true)
            { return true; }

        // - If the CPU is halted, check for any enabled and pending interrupts.
        //   If an interrupt is found, exit the halted state and proceed with the
        //   step. Otherwise, consume one M-cycle on the bus.
        if (is_halted() == true)
        {
            if (is_any_interrupt_pending() == true)
            {
                m_halted = false;
            }
            else
            {
                return consume_machine_cycles(1);
            }
        }

        // - Service the highest-priority pending interrupt, if any.
        if (service_interrupt() == false)
            { return false; }

        // - Clear fetch state.
        m_fetch_address = 0;
        m_fetch_data = 0;
        m_opcode = 0;
        m_opcode_address = 0;

        // - Fetch the next opcode from the instruction stream.
        if (fetch_opcode() == false)
            { return false; }

        // - Decode and execute the fetched opcode.
        bool ok = false;
        switch ((m_opcode >> 8) & 0xFF)
        {
            // `0x0***` - CPU Control Instructions
            case 0x00: ok = nop(); break;
            case 0x01: ok = stop(); break;
            case 0x02: ok = halt(); break;
            case 0x03: ok = di(); break;
            case 0x04: ok = ei(); break;
            case 0x05: ok = eii(); break;
            case 0x06: ok = daa(); break;
            case 0x07: ok = scf(); break;
            case 0x08: ok = ccf(); break;
            case 0x09: ok = clv(); break;
            case 0x0A: ok = sev(); break;

            // `0x1***` - 8-Bit Load/Store/Move Instructions
            case 0x10: ok = fetch_imm8() && ld_lx_imm8(); break;
            case 0x11: ok = fetch_imm32() && ld_lx_addr32(); break;
            case 0x12: ok = ld_lx_pdy(); break;
            case 0x13: ok = fetch_imm16() && ldq_lx_addr16(); break;
            case 0x14: ok = ldq_lx_pwy(); break;
            case 0x15: ok = fetch_imm8() && ldp_lx_addr8(); break;
            case 0x16: ok = ldp_lx_ply(); break;
            case 0x17: ok = fetch_imm32() && st_addr32_ly(); break;
            case 0x18: ok = st_pdx_ly(); break;
            case 0x19: ok = fetch_imm16() && stq_addr16_ly(); break;
            case 0x1A: ok = stq_pwx_ly(); break;
            case 0x1B: ok = fetch_imm8() && stp_addr8_ly(); break;
            case 0x1C: ok = stp_plx_ly(); break;
            case 0x1D: ok = mv_lx_ly(); break;
            case 0x1E: ok = mv_hx_ly(); break;
            case 0x1F: ok = mv_lx_hy(); break;

            // `0x2***` - 16-Bit Load/Store/Move Instructions
            case 0x20: ok = fetch_imm16() && ld_wx_imm16(); break;
            case 0x21: ok = fetch_imm32() && ld_wx_addr32(); break;
            case 0x22: ok = ld_wx_pdy(); break;
            case 0x23: ok = fetch_imm16() && ldq_wx_addr16(); break;
            case 0x24: ok = ldq_wx_pwy(); break;
            case 0x27: ok = fetch_imm32() && st_addr32_wy(); break;
            case 0x28: ok = st_pdx_wy(); break;
            case 0x29: ok = fetch_imm16() && stq_addr16_wy(); break;
            case 0x2A: ok = stq_pwx_wy(); break;
            case 0x2D: ok = mv_wx_wy(); break;
            case 0x2E: ok = mwh_dx_wy(); break;
            case 0x2F: ok = mwl_wx_dy(); break;

            // `0x3***` - 32-Bit Load/Store/Move Instructions
            case 0x30: ok = fetch_imm32() && ld_dx_imm32(); break;
            case 0x31: ok = fetch_imm32() && ld_dx_addr32(); break;
            case 0x32: ok = ld_dx_pdy(); break;
            case 0x33: ok = fetch_imm16() && ldq_dx_addr16(); break;
            case 0x34: ok = ldq_dx_pwy(); break;
            case 0x35: ok = fetch_imm32() && lsp_imm32(); break;
            case 0x36: ok = pop_dx(); break;
            case 0x37: ok = fetch_imm32() && st_addr32_dy(); break;
            case 0x38: ok = st_pdx_dy(); break;
            case 0x39: ok = fetch_imm16() && stq_addr16_dy(); break;
            case 0x3A: ok = stq_pwx_dy(); break;
            case 0x3B: ok = fetch_imm32() && ssp_addr32(); break;
            case 0x3C: ok = push_dy(); break;
            case 0x3D: ok = mv_dx_dy(); break;
            case 0x3E: ok = spo_dx(); break;
            case 0x3F: ok = spi_dy(); break;

            // `0x4***` - Branching Instructions
            case 0x40: ok = fetch_imm32() && jmp_x_imm32(); break;
            case 0x41: ok = jmp_x_dy(); break;
            case 0x42: ok = fetch_imm16() && jpb_x_simm16(); break;
            case 0x43: ok = fetch_imm32() && call_x_imm32(); break;
            case 0x44: ok = int_xx(); break;
            case 0x45: ok = ret_x(); break;
            case 0x46: ok = reti(); break;

            // `0x5***` - 8-Bit Arithmetic Instructions
            case 0x50: ok = fetch_imm8() && add_l0_imm8(); break;
            case 0x51: ok = add_l0_ly(); break;
            case 0x52: ok = add_l0_pdy(); break;
            case 0x53: ok = fetch_imm8() && adc_l0_imm8(); break;
            case 0x54: ok = adc_l0_ly(); break;
            case 0x55: ok = adc_l0_pdy(); break;
            case 0x56: ok = fetch_imm8() && sub_l0_imm8(); break;
            case 0x57: ok = sub_l0_ly(); break;
            case 0x58: ok = sub_l0_pdy(); break;
            case 0x59: ok = fetch_imm8() && sbc_l0_imm8(); break;
            case 0x5A: ok = sbc_l0_ly(); break;
            case 0x5B: ok = sbc_l0_pdy(); break;
            case 0x5C: ok = inc_lx(); break;
            case 0x5D: ok = inc_pdx(); break;
            case 0x5E: ok = dec_lx(); break;
            case 0x5F: ok = dec_pdx(); break;

            // `0x6***` - 16-Bit and 32-Bit Arithmetic Instructions
            case 0x60: ok = fetch_imm16() && add_w0_imm16(); break;
            case 0x61: ok = add_w0_wy(); break;
            case 0x62: ok = fetch_imm32() && add_d0_imm32(); break;
            case 0x63: ok = add_d0_dy(); break;
            case 0x64: ok = fetch_imm16() && sub_w0_imm16(); break;
            case 0x65: ok = sub_w0_wy(); break;
            case 0x66: ok = fetch_imm32() && sub_d0_imm32(); break;
            case 0x67: ok = sub_d0_dy(); break;
            case 0x6C: ok = inc_wx(); break;
            case 0x6D: ok = inc_dx(); break;
            case 0x6E: ok = dec_wx(); break;
            case 0x6F: ok = dec_dx(); break;

            // `0x7***` - 8-Bit Bitwise and Logical Instructions
            case 0x70: ok = fetch_imm8() && and_l0_imm8(); break;
            case 0x71: ok = and_l0_ly(); break;
            case 0x72: ok = and_l0_pdy(); break;
            case 0x73: ok = fetch_imm8() && or_l0_imm8(); break;
            case 0x74: ok = or_l0_ly(); break;
            case 0x75: ok = or_l0_pdy(); break;
            case 0x76: ok = fetch_imm8() && xor_l0_imm8(); break;
            case 0x77: ok = xor_l0_ly(); break;
            case 0x78: ok = xor_l0_pdy(); break;
            case 0x79: ok = not_lx(); break;
            case 0x7A: ok = not_pdx(); break;
            case 0x7D: ok = fetch_imm8() && cmp_l0_imm8(); break;
            case 0x7E: ok = cmp_l0_ly(); break;
            case 0x7F: ok = cmp_l0_pdy(); break;

            // `0x8***` - Bit Shift and Swap Instructions
            case 0x80: ok = sla_lx(); break;
            case 0x81: ok = sla_pdx(); break;
            case 0x82: ok = sra_lx(); break;
            case 0x83: ok = sra_pdx(); break;
            case 0x84: ok = srl_lx(); break;
            case 0x85: ok = srl_pdx(); break;
            case 0x86: ok = swap_lx(); break;
            case 0x87: ok = swap_pdx(); break;
            case 0x88: ok = swap_wx(); break;
            case 0x89: ok = swap_dx(); break;

            // `0x9***` - Bit Rotate Instructions
            case 0x90: ok = rla(); break;
            case 0x91: ok = rl_lx(); break;
            case 0x92: ok = rl_pdx(); break;
            case 0x93: ok = rlca(); break;
            case 0x94: ok = rlc_lx(); break;
            case 0x95: ok = rlc_pdx(); break;
            case 0x96: ok = rra(); break;
            case 0x97: ok = rr_lx(); break;
            case 0x98: ok = rr_pdx(); break;
            case 0x99: ok = rrca(); break;
            case 0x9A: ok = rrc_lx(); break;
            case 0x9B: ok = rrc_pdx(); break;

            // `0xA***` - Bit Test and Manipulation Instructions
            case 0xA0: ok = bit_y_lx(); break;
            case 0xA1: ok = bit_y_pdx(); break;
            case 0xA2: ok = set_y_lx(); break;
            case 0xA3: ok = set_y_pdx(); break;
            case 0xA4: ok = res_y_lx(); break;
            case 0xA5: ok = res_y_pdx(); break;
            case 0xA6: ok = tog_y_lx(); break;
            case 0xA7: ok = tog_y_pdx(); break;

            default:
                return raise_exception(EC_INVALID_INSTRUCTION);
        }

        // - Early exit if the instruction execution failed.
        if (ok == false)
            { return false; }

        // - At this point, if `IMP` is set, set `IME` and clear `IMP`.
        if (m_imp == true)
        {
            m_ime = true;
            m_imp = false;
        }

        return true;
    }

    auto cpu::raise_exception (exception_code code) -> bool
    {
        // - If `code` is `EC_OK`, do nothing and return `true`.
        if (code == EC_OK)
            { return true; }

        // - Check for a Double Fault condition.
        if (m_regs.ec != EC_OK || code == EC_DOUBLE_FAULT)
        {
            m_double_fault = true;
            m_stopped = true;
        }

        // - Set the `EC` register to the specified exception code.
        m_regs.ec = static_cast<std::uint8_t>(code);

        // - If a Double Fault condition has not occured, call interrupt vector #0
        //   to handle the exception. If this fails, double fault the CPU.
        if (m_double_fault == false)
        {
            if (call_interrupt(0) == false)
            {
                m_double_fault = true;
                m_stopped = true;
            }
        }

        // - Print a message indicating the exception.
        switch (code)
        {
            case EC_INVALID_INSTRUCTION:
                std::println("Illegal instruction '0x{:04X}' at address ${:08X}",
                    m_opcode, m_opcode_address);
                break;
            case EC_INVALID_ARGUMENT:
                std::println("Instruction '0x{:04X}' at address ${:08X} "
                    "received an invalid argument",
                    m_opcode, m_opcode_address);
                break;
            case EC_INVALID_READ_ACCESS:
                std::println("Instruction '0x{:04X}' at address ${:08X} "
                    "attempted reading from invalid memory address '${:08X}'",
                    m_opcode, m_opcode_address, m_fetch_address);
                break;
            case EC_INVALID_WRITE_ACCESS:
                std::println("Instruction '0x{:04X}' at address ${:08X} "
                    "attempted writing to invalid memory address '${:08X}'",
                    m_opcode, m_opcode_address, m_fetch_address);   
                break;
            case EC_INVALID_EXECUTE_ACCESS:
                std::println("Attempted to execute instruction at invalid address "
                    "'${:08X}'", m_fetch_address);
                break;
            case EC_DIVIDE_BY_ZERO:
                std::println("Divide by zero error in instruction '0x{:04X}' "
                    "at address ${:08X}",
                    m_opcode, m_opcode_address);
                break;
            case EC_STACK_OVERFLOW:
                std::println("Stack overflow in instruction '0x{:04X}' "
                    "at address ${:08X}",
                    m_opcode, m_opcode_address);
                break;
            case EC_STACK_UNDERFLOW:
                std::println("Stack underflow in instruction '0x{:04X}' "
                    "at address ${:08X}",
                    m_opcode, m_opcode_address);
                break;
            case EC_HARDWARE_ERROR:
                std::println("Hardware error occurred during instruction "
                    "'0x{:04X}' at address ${:08X}",
                    m_opcode, m_opcode_address);
                break;
            case EC_DOUBLE_FAULT:
                std::println("Double Fault exception explicitly raised.");
                break;
            default:
                std::println("CPU Exception Raised: EC={:02X}", static_cast<std::uint8_t>(code));
                break;
        }

        if (m_double_fault == true)
        {
            std::println("Double Fault condition encountered. System halted.");
        }

        return false;
    }

    auto cpu::consume_tick_cycles (std::uint32_t t_cycles) -> bool
    {
        for (std::uint32_t i = 0; i < t_cycles; ++i)
        {
            if (m_bus.tick() == false)
            {
                return raise_exception(EC_HARDWARE_ERROR);
            }
        }

        return true;
    }

    auto cpu::consume_machine_cycles (std::uint32_t m_cycles) -> bool
    {
        return consume_tick_cycles(m_cycles * 4);
    }

    auto cpu::request_interrupt (std::uint8_t vector) -> void
    {
        // - Set the corresponding bit in the `IRQ` register to request the
        //   interrupt.
        if (vector < 32)
        {
            m_regs.irq |= (1 << vector);
        }
    }
}

/* Public Methods - Hardware Registers ****************************************/

namespace g10
{
    auto cpu::read_irq0 () const -> std::uint8_t
    {
        // - Reading `IRQ0` reads the low byte of the 32-bit `IRQ` register.
        // - All 8 bits are readable.
        return m_regs.irq & 0xFF;
    }

    auto cpu::read_irq1 () const -> std::uint8_t
    {
        // - Reading `IRQ1` reads bits 8-15 of the 32-bit `IRQ` register.
        // - All 8 bits are readable.
        return (m_regs.irq >> 8) & 0xFF;
    }

    auto cpu::read_irq2 () const -> std::uint8_t
    {
        // - Reading `IRQ2` reads bits 16-23 of the 32-bit `IRQ` register.
        // - All 8 bits are readable.
        return (m_regs.irq >> 16) & 0xFF;
    }

    auto cpu::read_irq3 () const -> std::uint8_t
    {
        // - Reading `IRQ3` reads bits 24-31 of the 32-bit `IRQ` register.
        // - All 8 bits are readable.
        return (m_regs.irq >> 24) & 0xFF;
    }

    auto cpu::read_ie0 () const -> std::uint8_t
    {
        // - Reading `IE0` reads the low byte of the 32-bit `IE` register.
        // - All 8 bits are readable.
        return m_regs.ie & 0xFF;
    }

    auto cpu::read_ie1 () const -> std::uint8_t
    {
        // - Reading `IE1` reads bits 8-15 of the 32-bit `IE` register.
        // - All 8 bits are readable.
        return (m_regs.ie >> 8) & 0xFF;
    }

    auto cpu::read_ie2 () const -> std::uint8_t
    {
        // - Reading `IE2` reads bits 16-23 of the 32-bit `IE` register.
        // - All 8 bits are readable.
        return (m_regs.ie >> 16) & 0xFF;
    }

    auto cpu::read_ie3 () const -> std::uint8_t
    {
        // - Reading `IE3` reads bits 24-31 of the 32-bit `IE` register.
        // - All 8 bits are readable.
        return (m_regs.ie >> 24) & 0xFF;
    }

    auto cpu::read_spd () const -> std::uint8_t
    {
        return
            0b01111110 |                            // - Bits 1-6 unused; read `1`
            (m_speed_switch_reg.raw & 0b10000001);  // - Bits 0 and 7 readable
    }

    auto cpu::write_irq0 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IRQ0` writes to the low byte of the 32-bit `IRQ` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IRQ` register are unaffected.
        m_regs.irq =
            (m_regs.irq & 0xFFFFFF00) | 
            (static_cast<std::uint32_t>(value) & 0x000000FF);

        return value;
    }

    auto cpu::write_irq1 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IRQ1` writes to bits 8-15 of the 32-bit `IRQ` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IRQ` register are unaffected.
        m_regs.irq =
            (m_regs.irq & 0xFFFF00FF) |
            ((static_cast<std::uint32_t>(value) << 8) & 0x0000FF00);

        return value;
    }

    auto cpu::write_irq2 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IRQ2` writes to bits 16-23 of the 32-bit `IRQ` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IRQ` register are unaffected.
        m_regs.irq =
            (m_regs.irq & 0xFF00FFFF) |
            ((static_cast<std::uint32_t>(value) << 16) & 0x00FF0000);

        return value;
    }

    auto cpu::write_irq3 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IRQ3` writes to bits 24-31 of the 32-bit `IRQ` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IRQ` register are unaffected.
        m_regs.irq =
            (m_regs.irq & 0x00FFFFFF) |
            ((static_cast<std::uint32_t>(value) << 24) & 0xFF000000);

        return value;
    }

    auto cpu::write_ie0 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IE0` writes to the low byte of the 32-bit `IE` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IE` register are unaffected.
        m_regs.ie =
            (m_regs.ie & 0xFFFFFF00) | 
            (static_cast<std::uint32_t>(value) & 0x000000FF);

        return value;
    }

    auto cpu::write_ie1 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IE1` writes to bits 8-15 of the 32-bit `IE` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IE` register are unaffected.
        m_regs.ie =
            (m_regs.ie & 0xFFFF00FF) |
            ((static_cast<std::uint32_t>(value) << 8) & 0x0000FF00);

        return value;
    }

    auto cpu::write_ie2 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IE2` writes to bits 16-23 of the 32-bit `IE` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IE` register are unaffected.
        m_regs.ie =
            (m_regs.ie & 0xFF00FFFF) |
            ((static_cast<std::uint32_t>(value) << 16) & 0x00FF0000);

        return value;
    }

    auto cpu::write_ie3 (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `IE3` writes to bits 24-31 of the 32-bit `IE` register.
        // - All 8 bits are writable.
        // - The other 24 bits of the `IE` register are unaffected.
        m_regs.ie =
            (m_regs.ie & 0x00FFFFFF) |
            ((static_cast<std::uint32_t>(value) << 24) & 0xFF000000);

        return value;
    }

    auto cpu::write_spd (std::uint8_t value) -> std::uint8_t
    {
        // - Writing `SPD` updates bits 0 and 7 of the Speed Switch Register.
        // - Bit 7 is read-only and retains its previous value.
        // - Bits 1-6 are unused and write `1`
        // - Only bit 0 is writable.
        m_speed_switch_reg.raw =
            (m_speed_switch_reg.raw & 0b10000000) |     // - Bit 7 read-only; retain original value
            0b01111110 |                                // - Bits 1-6 unused; write `1`
            (value & 0b00000001);                       // - Bit 0 writable

        return m_speed_switch_reg.raw;
    }
}

/* Private Methods - Register and Flag Access *********************************/

namespace g10
{
    auto cpu::read_register (register_type reg) const -> std::uint32_t
    {
        switch (reg)
        {
            // General-Purpose Full Registers (`Dn`)
            case register_type::d0:    return m_regs.gp[0];
            case register_type::d1:    return m_regs.gp[1];
            case register_type::d2:    return m_regs.gp[2];
            case register_type::d3:    return m_regs.gp[3];
            case register_type::d4:    return m_regs.gp[4];
            case register_type::d5:    return m_regs.gp[5];
            case register_type::d6:    return m_regs.gp[6];
            case register_type::d7:    return m_regs.gp[7];
            case register_type::d8:    return m_regs.gp[8];
            case register_type::d9:    return m_regs.gp[9];
            case register_type::d10:   return m_regs.gp[10];
            case register_type::d11:   return m_regs.gp[11];
            case register_type::d12:   return m_regs.gp[12];
            case register_type::d13:   return m_regs.gp[13];
            case register_type::d14:   return m_regs.gp[14];
            case register_type::d15:   return m_regs.gp[15];

            // General-Purpose Word Registers (`Wn`, lower 16 bits of `Dn`)
            case register_type::w0:    return m_regs.gp[0] & 0xFFFF;
            case register_type::w1:    return m_regs.gp[1] & 0xFFFF;
            case register_type::w2:    return m_regs.gp[2] & 0xFFFF;
            case register_type::w3:    return m_regs.gp[3] & 0xFFFF;
            case register_type::w4:    return m_regs.gp[4] & 0xFFFF;
            case register_type::w5:    return m_regs.gp[5] & 0xFFFF;
            case register_type::w6:    return m_regs.gp[6] & 0xFFFF;
            case register_type::w7:    return m_regs.gp[7] & 0xFFFF;
            case register_type::w8:    return m_regs.gp[8] & 0xFFFF;
            case register_type::w9:    return m_regs.gp[9] & 0xFFFF;
            case register_type::w10:   return m_regs.gp[10] & 0xFFFF;
            case register_type::w11:   return m_regs.gp[11] & 0xFFFF;
            case register_type::w12:   return m_regs.gp[12] & 0xFFFF;
            case register_type::w13:   return m_regs.gp[13] & 0xFFFF;
            case register_type::w14:   return m_regs.gp[14] & 0xFFFF;
            case register_type::w15:   return m_regs.gp[15] & 0xFFFF;

            // General-Purpose High Byte Registers (`Hn`, upper 8 bits of `Wn`)
            case register_type::h0:    return (m_regs.gp[0] >> 8) & 0xFF;
            case register_type::h1:    return (m_regs.gp[1] >> 8) & 0xFF;
            case register_type::h2:    return (m_regs.gp[2] >> 8) & 0xFF;
            case register_type::h3:    return (m_regs.gp[3] >> 8) & 0xFF;
            case register_type::h4:    return (m_regs.gp[4] >> 8) & 0xFF;
            case register_type::h5:    return (m_regs.gp[5] >> 8) & 0xFF;
            case register_type::h6:    return (m_regs.gp[6] >> 8) & 0xFF;
            case register_type::h7:    return (m_regs.gp[7] >> 8) & 0xFF;
            case register_type::h8:    return (m_regs.gp[8] >> 8) & 0xFF;
            case register_type::h9:    return (m_regs.gp[9] >> 8) & 0xFF;
            case register_type::h10:   return (m_regs.gp[10] >> 8) & 0xFF;
            case register_type::h11:   return (m_regs.gp[11] >> 8) & 0xFF;
            case register_type::h12:   return (m_regs.gp[12] >> 8) & 0xFF;
            case register_type::h13:   return (m_regs.gp[13] >> 8) & 0xFF;
            case register_type::h14:   return (m_regs.gp[14] >> 8) & 0xFF;
            case register_type::h15:   return (m_regs.gp[15] >> 8) & 0xFF;

            // General-Purpose Low Byte Registers (`Ln`, lower 8 bits of `Wn`)
            case register_type::l0:    return m_regs.gp[0] & 0xFF;
            case register_type::l1:    return m_regs.gp[1] & 0xFF;
            case register_type::l2:    return m_regs.gp[2] & 0xFF;
            case register_type::l3:    return m_regs.gp[3] & 0xFF;
            case register_type::l4:    return m_regs.gp[4] & 0xFF;
            case register_type::l5:    return m_regs.gp[5] & 0xFF;
            case register_type::l6:    return m_regs.gp[6] & 0xFF;
            case register_type::l7:    return m_regs.gp[7] & 0xFF;
            case register_type::l8:    return m_regs.gp[8] & 0xFF;
            case register_type::l9:    return m_regs.gp[9] & 0xFF;
            case register_type::l10:   return m_regs.gp[10] & 0xFF;
            case register_type::l11:   return m_regs.gp[11] & 0xFF;
            case register_type::l12:   return m_regs.gp[12] & 0xFF;
            case register_type::l13:   return m_regs.gp[13] & 0xFF;
            case register_type::l14:   return m_regs.gp[14] & 0xFF;
            case register_type::l15:   return m_regs.gp[15] & 0xFF;

            // Special-Purpose Registers
            case register_type::pc:        return m_regs.pc;
            case register_type::sp:        return m_regs.sp;
            case register_type::flags:     return static_cast<std::uint32_t>(m_regs.flags.raw);
            case register_type::ec:        return static_cast<std::uint32_t>(m_regs.ec);

            default:
                return 0;
        }
    }

    auto cpu::write_register (register_type reg, std::uint32_t value) -> void
    {
        switch (reg)
        {
            // General-Purpose Full Registers (`Dn`)
            case register_type::d0:  m_regs.gp[0] = value;  break;
            case register_type::d1:  m_regs.gp[1] = value;  break;
            case register_type::d2:  m_regs.gp[2] = value;  break;
            case register_type::d3:  m_regs.gp[3] = value;  break;
            case register_type::d4:  m_regs.gp[4] = value;  break;
            case register_type::d5:  m_regs.gp[5] = value;  break;
            case register_type::d6:  m_regs.gp[6] = value;  break;
            case register_type::d7:  m_regs.gp[7] = value;  break;
            case register_type::d8:  m_regs.gp[8] = value;  break;
            case register_type::d9:  m_regs.gp[9] = value;  break;
            case register_type::d10: m_regs.gp[10] = value; break;
            case register_type::d11: m_regs.gp[11] = value; break;
            case register_type::d12: m_regs.gp[12] = value; break;
            case register_type::d13: m_regs.gp[13] = value; break;
            case register_type::d14: m_regs.gp[14] = value; break;
            case register_type::d15: m_regs.gp[15] = value; break;

            // General-Purpose Word Registers (`Wn`, lower 16 bits of `Dn`)
            case register_type::w0:
                m_regs.gp[0] = (m_regs.gp[0] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w1:
                m_regs.gp[1] = (m_regs.gp[1] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w2:
                m_regs.gp[2] = (m_regs.gp[2] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w3:
                m_regs.gp[3] = (m_regs.gp[3] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w4:
                m_regs.gp[4] = (m_regs.gp[4] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w5:
                m_regs.gp[5] = (m_regs.gp[5] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w6:
                m_regs.gp[6] = (m_regs.gp[6] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w7:
                m_regs.gp[7] = (m_regs.gp[7] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w8:
                m_regs.gp[8] = (m_regs.gp[8] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w9:
                m_regs.gp[9] = (m_regs.gp[9] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w10:
                m_regs.gp[10] = (m_regs.gp[10] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w11:
                m_regs.gp[11] = (m_regs.gp[11] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w12:
                m_regs.gp[12] = (m_regs.gp[12] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w13:
                m_regs.gp[13] = (m_regs.gp[13] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w14:
                m_regs.gp[14] = (m_regs.gp[14] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;
            case register_type::w15:
                m_regs.gp[15] = (m_regs.gp[15] & 0xFFFF0000) | (value & 0x0000FFFF);
                break;

            // General-Purpose High Byte Registers (`Hn`, upper 8 bits of `Wn`)
            case register_type::h0:
                m_regs.gp[0] = (m_regs.gp[0] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h1:
                m_regs.gp[1] = (m_regs.gp[1] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h2:
                m_regs.gp[2] = (m_regs.gp[2] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h3:
                m_regs.gp[3] = (m_regs.gp[3] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h4:
                m_regs.gp[4] = (m_regs.gp[4] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h5:
                m_regs.gp[5] = (m_regs.gp[5] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h6:
                m_regs.gp[6] = (m_regs.gp[6] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h7:
                m_regs.gp[7] = (m_regs.gp[7] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h8:
                m_regs.gp[8] = (m_regs.gp[8] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h9:
                m_regs.gp[9] = (m_regs.gp[9] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h10:
                m_regs.gp[10] = (m_regs.gp[10] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h11:
                m_regs.gp[11] = (m_regs.gp[11] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h12:
                m_regs.gp[12] = (m_regs.gp[12] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h13:
                m_regs.gp[13] = (m_regs.gp[13] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h14:
                m_regs.gp[14] = (m_regs.gp[14] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;
            case register_type::h15:
                m_regs.gp[15] = (m_regs.gp[15] & 0xFFFF00FF) | ((value & 0x000000FF) << 8);
                break;

            // General-Purpose Low Byte Registers (`Ln`, lower 8 bits of `Wn`)
            case register_type::l0:
                m_regs.gp[0] = (m_regs.gp[0] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l1:
                m_regs.gp[1] = (m_regs.gp[1] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l2:
                m_regs.gp[2] = (m_regs.gp[2] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l3:
                m_regs.gp[3] = (m_regs.gp[3] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l4:
                m_regs.gp[4] = (m_regs.gp[4] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l5:
                m_regs.gp[5] = (m_regs.gp[5] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l6:
                m_regs.gp[6] = (m_regs.gp[6] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l7:
                m_regs.gp[7] = (m_regs.gp[7] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l8:
                m_regs.gp[8] = (m_regs.gp[8] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l9:
                m_regs.gp[9] = (m_regs.gp[9] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l10:
                m_regs.gp[10] = (m_regs.gp[10] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l11:
                m_regs.gp[11] = (m_regs.gp[11] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l12:
                m_regs.gp[12] = (m_regs.gp[12] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l13:
                m_regs.gp[13] = (m_regs.gp[13] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l14:
                m_regs.gp[14] = (m_regs.gp[14] & 0xFFFFFF00) | (value & 0x000000FF);
                break;
            case register_type::l15:
                m_regs.gp[15] = (m_regs.gp[15] & 0xFFFFFF00) | (value & 0x000000FF);
                break;

            // Special-Purpose Registers
            case register_type::pc:    m_regs.pc = value;        break;
            case register_type::sp:    m_regs.sp = value;        break;
            case register_type::flags: 
                m_regs.flags.raw = static_cast<std::uint8_t>(value & 0xFF);
                break;

            case register_type::ec: // `EC` is read-only.
            default:
                break;
        }
    }
}

/* Private Methods - Internal Bus Access **************************************/

namespace g10
{
    auto cpu::fetch_opcode () -> bool
    {
        std::uint8_t bytes[2] = { 0 };

        for (std::uint8_t i = 0; i < 2; ++i)
        {
            bytes[i] = m_bus.read(m_regs.pc++);

            // - If an exception is raised during the read or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        m_opcode_address = m_regs.pc - 2;
        m_opcode = (static_cast<std::uint16_t>(bytes[0])      ) |
                   (static_cast<std::uint16_t>(bytes[1]) << 8 );

        return true;
    }

    auto cpu::fetch_imm8 () -> bool
    {
        std::uint8_t byte = m_bus.read(m_regs.pc++);

        // - If any exception is raised during the read or cycle consumption, 
        //   return `false`.
        if (
            m_regs.ec != EC_OK ||
            consume_machine_cycles(1) == false
        )
        {
            return false;
        }

        m_fetch_address = m_regs.pc - 1;
        m_fetch_data = byte;

        return true;
    }

    auto cpu::fetch_imm16 () -> bool
    {
        std::uint8_t bytes[2] = { 0 };

        for (std::uint8_t i = 0; i < 2; ++i)
        {
            bytes[i] = m_bus.read(m_regs.pc++);

            // - If an exception is raised during the read or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        m_fetch_address = m_regs.pc - 2;
        m_fetch_data = (static_cast<std::uint16_t>(bytes[0])      ) |
                       (static_cast<std::uint16_t>(bytes[1]) << 8 );

        return true;
    }

    auto cpu::fetch_imm32 () -> bool
    {
        std::uint8_t bytes[4] = { 0 };

        for (std::uint8_t i = 0; i < 4; ++i)
        {
            bytes[i] = m_bus.read(m_regs.pc++);

            // - If an exception is raised during the read or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        m_fetch_address = m_regs.pc - 4;
        m_fetch_data = (static_cast<std::uint32_t>(bytes[0])      ) |
                       (static_cast<std::uint32_t>(bytes[1]) << 8 ) |
                       (static_cast<std::uint32_t>(bytes[2]) << 16) |
                       (static_cast<std::uint32_t>(bytes[3]) << 24);

        return true;
    }

    auto cpu::read_byte (std::uint32_t address, std::uint8_t& out_value) -> bool
    {
        std::uint8_t byte = m_bus.read(address);
        if (
            m_regs.ec != EC_OK ||
            consume_machine_cycles(1) == false
        )
        {
            return false;
        }
        
        out_value = byte;
        return true;
    }

    auto cpu::read_word (std::uint32_t address, std::uint16_t& out_value) -> bool
    {
        std::uint8_t bytes[2] = { 0 };

        for (std::uint8_t i = 0; i < 2; ++i)
        {
            bytes[i] = m_bus.read(address + i);

            // - If an exception is raised during the read or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        out_value = (static_cast<std::uint16_t>(bytes[0])      ) |
                    (static_cast<std::uint16_t>(bytes[1]) << 8 );

        return true;
    }

    auto cpu::read_dword (std::uint32_t address, std::uint32_t& out_value) -> bool
    {
        std::uint8_t bytes[4] = { 0 };

        for (std::uint8_t i = 0; i < 4; ++i)
        {
            bytes[i] = m_bus.read(address + i);

            // - If an exception is raised during the read or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        out_value = (static_cast<std::uint32_t>(bytes[0])      ) |
                    (static_cast<std::uint32_t>(bytes[1]) << 8 ) |
                    (static_cast<std::uint32_t>(bytes[2]) << 16) |
                    (static_cast<std::uint32_t>(bytes[3]) << 24);

        return true;
    }

    auto cpu::write_byte (std::uint32_t address, std::uint8_t value) -> bool
    {
        m_bus.write(address, value);

        // - If an exception is raised during the write or cycle consumption,
        //   return `false`.
        if (
            m_regs.ec != EC_OK ||
            consume_machine_cycles(1) == false
        )
        {
            return false;
        }

        return true;
    }

    auto cpu::write_word (std::uint32_t address, std::uint16_t value) -> bool
    {
        std::uint8_t bytes[2] = {
            static_cast<std::uint8_t>(value & 0xFF),
            static_cast<std::uint8_t>((value >> 8) & 0xFF)
        };

        for (std::uint8_t i = 0; i < 2; ++i)
        {
            m_bus.write(address + i, bytes[i]);

            // - If an exception is raised during the write or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        return true;
    }

    auto cpu::write_dword (std::uint32_t address, std::uint32_t value) -> bool
    {
        std::uint8_t bytes[4] = {
            static_cast<std::uint8_t>(value & 0xFF),
            static_cast<std::uint8_t>((value >> 8) & 0xFF),
            static_cast<std::uint8_t>((value >> 16) & 0xFF),
            static_cast<std::uint8_t>((value >> 24) & 0xFF)
        };

        for (std::uint8_t i = 0; i < 4; ++i)
        {
            m_bus.write(address + i, bytes[i]);

            // - If an exception is raised during the write or cycle consumption,
            //   return `false`.
            if (
                m_regs.ec != EC_OK ||
                consume_machine_cycles(1) == false
            )
            {
                return false;
            }
        }

        return true;
    }

    auto cpu::pop_dword (std::uint32_t& out_value) -> bool
    {
        // - Read four bytes from the stack, starting with the least-significant
        //   byte.
        std::uint8_t bytes[4] = { 0 };

        for (std::uint8_t i = 0; i < 4; ++i)
        {
            bytes[i] = m_bus.read(m_regs.sp++);

            // - If an exception is raised during the stack read, return `false`.
            if (m_regs.ec != EC_OK)
            {
                // - If the exception is an `EC_INVALID_READ_ACCESS`, change it
                //   to `EC_STACK_UNDERFLOW` (but don't raise a new exception;
                //   this is not a double fault scenario).
                if (m_regs.ec == EC_INVALID_READ_ACCESS)
                    { m_regs.ec = EC_STACK_UNDERFLOW; }

                return false;
            }

            if (consume_machine_cycles(1) == false)
                { return false; }
        }

        // - Combine the four bytes into a double word.
        out_value = (static_cast<std::uint32_t>(bytes[0])      ) |
                    (static_cast<std::uint32_t>(bytes[1]) << 8 ) |
                    (static_cast<std::uint32_t>(bytes[2]) << 16) |
                    (static_cast<std::uint32_t>(bytes[3]) << 24);

        // - Because the stack pointer was modified during the read operations,
        //   an additional M-cycle is consumed here to account for the final
        //   state of the stack pointer.
        return consume_machine_cycles(1);
    }

    auto cpu::push_dword (std::uint32_t value) -> bool
    {
        // - Break the double word into four bytes, starting with the
        //   most-significant byte.
        std::uint8_t bytes[4] = {
            static_cast<std::uint8_t>((value >> 24) & 0xFF),
            static_cast<std::uint8_t>((value >> 16) & 0xFF),
            static_cast<std::uint8_t>((value >> 8) & 0xFF),
            static_cast<std::uint8_t>(value & 0xFF)
        };

        // - Write each byte to the stack and consume one M-cycle for each write.
        for (std::uint8_t i = 0; i < 4; ++i)
        {
            m_bus.write(--m_regs.sp, bytes[i]);

            // - If an exception is raised during the stack write, return `false`.
            if (m_regs.ec != EC_OK)
            {
                // - If the exception is an `EC_INVALID_WRITE_ACCESS`, change it
                //   to `EC_STACK_OVERFLOW` (but don't raise a new exception;
                //   this is not a double fault scenario).
                if (m_regs.ec == EC_INVALID_WRITE_ACCESS)
                    { m_regs.ec = EC_STACK_OVERFLOW; }

                return false;
            }

            if (consume_machine_cycles(1) == false)
                { return false; }
        }

        // - Because the stack pointer was modified during the write operations,
        //   an additional M-cycle is consumed here to account for the final
        //   state of the stack pointer.
        return consume_machine_cycles(1);
    }
}

/* Private Methods - Interrupts ***********************************************/

namespace g10
{
    auto cpu::is_any_interrupt_pending () const -> bool
    {
        // - Note: The `IME` does not have to be checked here; This method only
        //   checks whether any enabled interrupts are pending, regardless of whether
        //   interrupts are globally enabled or disabled.
        return (m_regs.ie & m_regs.irq) != 0;
    }

    auto cpu::disable_interrupts () -> void
    {
        m_ime = false;
        m_imp = false;
    }

    auto cpu::enable_interrupts (bool immediately) -> void
    {
        if (immediately == true)
            { m_imp = false; m_ime = true; }
        else
            { m_imp = true; }
    }

    auto cpu::call_interrupt (std::uint8_t vector) -> bool
    {
        // - Acknowledge the interrupt by clearing its bit in the `IRQ` register,
        //   the `IME` and `IMP` flags, and the `HALT` state.
        m_regs.irq &= ~(1 << vector);
        m_ime = false;
        m_imp = false;
        m_halted = false;

        // - Wait two M-cycles before servicing the interrupt.
        if (consume_machine_cycles(2) == false)
            { return false; }

        // - Push the current `PC` onto the stack.
        if (push_dword(m_regs.pc) == false)
            { return false; }

        // - Move the `PC` to the interrupt handler address.
        //   The handler address is calculated as IVT_START + (vector * 0x80).
        //   Consume 1 M-cycle for the jump.
        m_regs.pc = IVT_START + (static_cast<std::uint32_t>(vector) * 0x80);
        return consume_machine_cycles(1);
    }

    auto cpu::service_interrupt () -> bool
    {
        // - If `IME` is disabled, do nothing and return `true`.
        if (m_ime == false)
            { return true; }

        // - If `IMP` is set, clear it and return `true`.
        if (m_imp == true)
        {
            m_imp = false;
            return true;
        }

        // - Find the highest-priority pending interrupt.
        for (std::uint8_t i = 0; i < 32; ++i)
        {
            if ((m_regs.ie & m_regs.irq) & (1 << i))
            {
                return call_interrupt(i);
            }
        }

        return true;
    }
}

/* Private Methods - Halt and Stop ********************************************/

namespace g10
{
    auto cpu::enter_halt_state () -> void
    {
        m_halted = true;
    }

    auto cpu::enter_stop_state () -> void
    {
        if (m_speed_switch_reg.armed == 1)
        {
            m_speed_switch_reg.armed = 0;
            m_speed_switching = true;

            if (consume_machine_cycles(2050) == false)
                { return; }

            m_speed_switch_reg.double_speed = 
                (m_speed_switch_reg.double_speed == 0) ? 1 : 0;
            m_speed_switching = false;

            return;
        }

        m_stopped = true;
    }

    auto cpu::exit_halt_state () -> void
    {
        m_halted = false;
    }
}
