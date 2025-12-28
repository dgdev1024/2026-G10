/**
 * @file   g10/cpu_inst.cpp
 * @author Dennis W. Griffin <dgdev1024@gmail.com>
 * @date   2025-12-15
 * 
 * @brief  Contains definitions for G10 CPU instruction execution methods.
 */

/* Private Includes ***********************************************************/

#include <g10/bus.hpp>
#include <g10/cpu.hpp>

/* Private Methods - CPU Control Instructions *********************************/

namespace g10
{
    auto cpu::nop () -> bool
    {
        return true;
    }

    auto cpu::stop () -> bool
    {
        enter_stop_state();
        return true;
    }

    auto cpu::halt () -> bool
    {
        enter_halt_state();
        return true;
    }

    auto cpu::di () -> bool
    {
        disable_interrupts();
        return true;
    }

    auto cpu::ei () -> bool
    {
        enable_interrupts(false);
        return true;
    }

    auto cpu::eii () -> bool
    {
        enable_interrupts(true);
        return true;
    }

    auto cpu::daa () -> bool
    {
        // - Read `L0`.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the negative, half-carry, and carry flags.
        bool negative       = m_regs.flags.negative;
        bool half_carry     = m_regs.flags.half_carry;
        bool carry          = m_regs.flags.carry;

        // Adjust `L0` based on the DAA rules:
        // - If the half-carry flag is set or the lower nibble of `L0` is greater
        //   than 9, add 0x06 to the correction value.
        // - If the carry flag is set or `L0` is greater than 0x99, add 0x60 to
        //   the correction value and set the carry flag. Otherwise, clear the
        //   carry flag.
        // - If the negative flag is clear, add the correction value to `L0`.
        //   Otherwise, subtract the correction value from `L0`.
        std::uint8_t correction = 0;
        if (half_carry == true || (negative == false && (l0 & 0x0F) > 9))
            { correction += 0x06; }
        if (carry == true || (negative == false && l0 > 0x99))
            { correction += 0x60; m_regs.flags.carry = 1; }
        else
            { m_regs.flags.carry = 0; }

        if (negative == false)
            { l0 += correction; }
        else
            { l0 -= correction; }

        // - Write the adjusted value back to `L0`.
        // - Update flags as appropriate.
        write_register(register_type::l0, l0);
        m_regs.flags.zero = (l0 == 0);
        m_regs.flags.half_carry = 0;

        return true;
    }

    auto cpu::scf () -> bool
    {
        m_regs.flags.carry = 1;
        m_regs.flags.half_carry = 0;
        m_regs.flags.negative = 0;
        return true;
    }

    auto cpu::ccf () -> bool
    {
        m_regs.flags.carry = !m_regs.flags.carry;
        m_regs.flags.half_carry = 0;
        m_regs.flags.negative = 0;
        return true;
    }

    auto cpu::clv () -> bool
    {
        m_regs.flags.overflow = 0;
        return true;
    }

    auto cpu::sev () -> bool
    {
        m_regs.flags.overflow = 1;
        return true;
    }
}

/* Private Methods - 8-Bit Load/Store/Move Instructions ***********************/

namespace g10
{
    auto cpu::ld_lx_imm8 () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        write_register(dest_reg, m_fetch_data & 0xFF);
        return true;
    }

    auto cpu::ld_lx_addr32 () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t value = 0;

        if (read_byte(m_fetch_data, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ld_lx_pdy () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t address = read_register(src_reg);
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldq_lx_addr16 () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        std::uint32_t address = 0xFFFF0000 | (m_fetch_data & 0x0000FFFF);
        std::uint8_t value = 0;

        if (read_byte(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldq_lx_pwy () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        auto src_reg = word_reg(m_opcode);
        std::uint32_t address = read_register(src_reg);
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldp_lx_addr8 () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        std::uint32_t address = 0xFFFFFF00 | (m_fetch_data & 0x000000FF);
        std::uint8_t value = 0;

        if (read_byte(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldp_lx_ply () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        auto src_reg = low_byte_reg(m_opcode);
        std::uint32_t address = read_register(src_reg);
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::st_addr32_ly () -> bool
    {
        auto src_reg = low_byte_reg(m_opcode);
        std::uint8_t value = read_register(src_reg);

        if (write_byte(m_fetch_data, value) == false)
            { return false; }

        return true;
    }

    auto cpu::st_pdx_ly () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        auto src_reg = low_byte_reg(m_opcode);
        std::uint32_t address = read_register(dest_reg);
        std::uint8_t value = read_register(src_reg);

        if (write_byte(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stq_addr16_ly () -> bool
    {
        auto src_reg = low_byte_reg(m_opcode);
        std::uint8_t value = read_register(src_reg);
        std::uint32_t address = 0xFFFF0000 | (m_fetch_data & 0x0000FFFF);

        if (write_byte(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stq_pwx_ly () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = low_byte_reg(m_opcode);
        std::uint32_t address = read_register(dest_reg);
        std::uint8_t value = read_register(src_reg);

        if (write_byte(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stp_addr8_ly () -> bool
    {
        auto src_reg = low_byte_reg(m_opcode);
        std::uint8_t value = read_register(src_reg);
        std::uint32_t address = 0xFFFFFF00 | (m_fetch_data & 0x000000FF);

        if (write_byte(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stp_plx_ly () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode);
        auto src_reg = low_byte_reg(m_opcode);
        std::uint32_t address = read_register(dest_reg);
        std::uint8_t value = read_register(src_reg);

        if (write_byte(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::mv_lx_ly () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        auto src_reg = low_byte_reg(m_opcode);
        std::uint8_t value = read_register(src_reg);
        write_register(dest_reg, value);
        return true;
    }

    auto cpu::mv_hx_ly () -> bool
    {
        auto dest_reg = high_byte_reg(m_opcode >> 4);
        auto src_reg = low_byte_reg(m_opcode);
        std::uint8_t value = read_register(src_reg);
        write_register(dest_reg, value);
        return true;
    }

    auto cpu::mv_lx_hy () -> bool
    {
        auto dest_reg = low_byte_reg(m_opcode >> 4);
        auto src_reg = high_byte_reg(m_opcode);
        std::uint8_t value = read_register(src_reg);
        write_register(dest_reg, value);
        return true;
    }
}

/* Private Methods - 16-Bit Load/Store/Move Instructions **********************/

namespace g10
{
    auto cpu::ld_wx_imm16 () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        write_register(dest_reg, m_fetch_data & 0xFFFF);
        return true;
    }

    auto cpu::ld_wx_addr32 () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        std::uint16_t value = 0;

        if (read_word(m_fetch_data, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ld_wx_pdy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t address = read_register(src_reg);
        std::uint16_t value = 0;
        if (read_word(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldq_wx_addr16 () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        std::uint32_t address = 0xFFFF0000 | (m_fetch_data & 0x0000FFFF);
        std::uint16_t value = 0;

        if (read_word(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldq_wx_pwy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = word_reg(m_opcode);
        std::uint32_t address = 0xFFFF0000 | read_register(src_reg);
        std::uint16_t value = 0;
        if (read_word(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::st_addr32_wy () -> bool
    {
        auto src_reg = word_reg(m_opcode);
        std::uint16_t value = read_register(src_reg);

        if (write_word(m_fetch_data, value) == false)
            { return false; }

        return true;
    }

    auto cpu::st_pdx_wy () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        auto src_reg = word_reg(m_opcode);
        std::uint32_t address = read_register(dest_reg);
        std::uint16_t value = read_register(src_reg);

        if (write_word(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stq_addr16_wy () -> bool
    {
        auto src_reg = word_reg(m_opcode);
        std::uint16_t value = read_register(src_reg);
        std::uint32_t address = 0xFFFF0000 | (m_fetch_data & 0x0000FFFF);

        if (write_word(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stq_pwx_wy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = word_reg(m_opcode);
        std::uint32_t address = 0xFFFF0000 | read_register(dest_reg);
        std::uint16_t value = read_register(src_reg);

        if (write_word(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::mv_wx_wy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = word_reg(m_opcode);
        std::uint16_t value = read_register(src_reg);
        write_register(dest_reg, value);
        return true;
    }

    auto cpu::mwh_dx_wy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t dest_value = read_register(dest_reg);
        std::uint16_t src_value = read_register(src_reg);
        dest_value =
            (dest_value & 0x0000FFFF) |
            (static_cast<std::uint32_t>(src_value) << 16);

        write_register(dest_reg, dest_value);
        return true;
    }

    auto cpu::mwl_wx_dy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t src_value = read_register(src_reg);
        std::uint16_t dest_value =
            static_cast<std::uint16_t>(
                (src_value >> 16) & 0x0000FFFF
            );

        write_register(dest_reg, dest_value);
        return true;
    }
}

/* Private Methods - 32-Bit Load/Store/Move Instructions **********************/

namespace g10
{
    auto cpu::ld_dx_imm32 () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        write_register(dest_reg, m_fetch_data);
        return true;
    }

    auto cpu::ld_dx_addr32 () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        std::uint32_t value = 0;

        if (read_dword(m_fetch_data, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ld_dx_pdy () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t address = read_register(src_reg);
        std::uint32_t value = 0;
        if (read_dword(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldq_dx_addr16 () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = 0xFFFF0000 | (m_fetch_data & 0x0000FFFF);
        std::uint32_t value = 0;

        if (read_dword(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::ldq_dx_pwy () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        auto src_reg = word_reg(m_opcode);
        std::uint32_t address = 0xFFFF0000 | read_register(src_reg);
        std::uint32_t value = 0;
        if (read_dword(address, value) == false)
            { return false; }

        write_register(dest_reg, value);
        return true;
    }

    auto cpu::lsp_imm32 ()-> bool
    {
        write_register(register_type::sp, m_fetch_data);
        return true;
    }

    auto cpu::pop_dx () -> bool
    {
        std::uint32_t value = 0;

        if (pop_dword(value) == false)
            { return false; }

        auto dest_reg = full_reg(m_opcode >> 4);
        write_register(dest_reg, value);
        return true;
    }

    auto cpu::st_addr32_dy () -> bool
    {
        auto src_reg = full_reg(m_opcode);
        std::uint32_t value = read_register(src_reg);

        if (write_dword(m_fetch_data, value) == false)
            { return false; }

        return true;
    }

    auto cpu::st_pdx_dy () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t address = read_register(dest_reg);
        std::uint32_t value = read_register(src_reg);

        if (write_dword(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stq_addr16_dy () -> bool
    {
        auto src_reg = full_reg(m_opcode);
        std::uint32_t value = read_register(src_reg);
        std::uint32_t address = 0xFFFF0000 | (m_fetch_data & 0x0000FFFF);

        if (write_dword(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::stq_pwx_dy () -> bool
    {
        auto dest_reg = word_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t address = 0xFFFF0000 | read_register(dest_reg);
        std::uint32_t value = read_register(src_reg);

        if (write_dword(address, value) == false)
            { return false; }

        return true;
    }

    auto cpu::ssp_addr32 () -> bool
    {
        std::uint32_t value = read_register(register_type::sp);

        if (write_dword(m_fetch_data, value) == false)
            { return false; }

        return true;
    }

    auto cpu::push_dy () -> bool
    {
        auto src_reg = full_reg(m_opcode >> 4);
        std::uint32_t value = read_register(src_reg);

        if (push_dword(value) == false)
            { return false; }

        return true;
    }

    auto cpu::mv_dx_dy () -> bool
    {
        auto dest_reg = full_reg(m_opcode >> 4);
        auto src_reg = full_reg(m_opcode);
        std::uint32_t value = read_register(src_reg);
        write_register(dest_reg, value);
        return true;
    }

    auto cpu::spo_dx () -> bool
    {
        auto sp = read_register(register_type::sp);
        auto dest_reg = full_reg(m_opcode >> 4);
        write_register(dest_reg, sp);
        return true;
    }

    auto cpu::spi_dy () -> bool
    {
        auto src_reg = full_reg(m_opcode >> 4);
        std::uint32_t value = read_register(src_reg);
        write_register(register_type::sp, value);
        return consume_machine_cycles(1);   // - `SP` modified. Consume 1 M-cycle.
    }
}

/* Private Methods - Branching Instructions ***********************************/

namespace g10
{
    /**
     * @brief   Helper function for evaluating condition codes for the branching
     *          instructions.
     * 
     * @param   flags   The current state of the CPU flags register.
     * @param   code    The condition code to evaluate.
     * 
     * @return  If the condition is met, or if there is no condition, returns
     *          `true`;
     *          Otherwise, returns `false`.
     */
    static auto check_condition (const flags_register& flags,
        condition_code code) -> bool
    {
        switch (code)
        {
            case CC_NO_CONDITION:       return true;
            case CC_ZERO_SET:           return (flags.zero == 1);
            case CC_ZERO_CLEAR:         return (flags.zero == 0);
            case CC_CARRY_SET:          return (flags.carry == 1);
            case CC_CARRY_CLEAR:        return (flags.carry == 0);
            case CC_OVERFLOW_SET:       return (flags.overflow == 1);
            case CC_OVERFLOW_CLEAR:     return (flags.overflow == 0);
            default:                    return false;
        }
    }

    auto cpu::jmp_x_imm32 () -> bool
    {
        auto condition = cond(m_opcode);
        if (check_condition(m_regs.flags, condition) == true)
        {
            m_regs.pc = m_fetch_data;
            return consume_machine_cycles(1);   // - Jump taken. Consume 1 M-cycle.
        }

        return true;    // - Jump not taken.
    }

    auto cpu::jmp_x_dy () -> bool
    {
        auto condition = cond(m_opcode);
        if (check_condition(m_regs.flags, condition) == true)
        {
            auto src_reg = full_reg(m_opcode);
            m_regs.pc = read_register(src_reg);
            return consume_machine_cycles(1);   // - Jump taken. Consume 1 M-cycle.
        }

        return true;    // - Jump not taken.
    }

    auto cpu::jpb_x_simm16 () -> bool
    {
        auto condition = cond(m_opcode);
        if (check_condition(m_regs.flags, condition) == true)
        {
            // - Sign-extend the 16-bit immediate value to 32 bits.
            std::int32_t offset = static_cast<std::int16_t>(m_fetch_data & 0xFFFF);
            m_regs.pc = static_cast<std::uint32_t>(
                static_cast<std::int32_t>(m_regs.pc) + offset
            );
            return consume_machine_cycles(1);   // - Jump taken. Consume 1 M-cycle.
        }

        return true;    // - Jump not taken.
    }

    auto cpu::call_x_imm32 () -> bool
    {
        auto condition = cond(m_opcode);
        if (check_condition(m_regs.flags, condition) == true)
        {
            // - Push the current PC onto the stack.
            // - This consumes 5 of the 6 extra M-cycles for a taken call (4
            //   for the memory write, 1 for the stack pointer update).
            if (push_dword(m_regs.pc) == false)
                { return false; }

            // - Set the PC to the target address.
            // - This consumes the remaining 1 M-cycle for a taken call.
            m_regs.pc = m_fetch_data;
            return consume_machine_cycles(1);
        }

        return true;    // - Call not taken.
    }

    auto cpu::int_xx () -> bool
    { 
        // - Validate `XX`.
        std::uint8_t int_num = static_cast<std::uint8_t>(m_opcode & 0xFF);
        if (int_num >= 32)
        {
            return raise_exception(EC_INVALID_ARGUMENT);
        }

        // - Call the subroutine, just like a normal CALL instruction.
        if (push_dword(m_regs.pc) == false)
            { return false; }

        m_regs.pc = 0x1000 + (static_cast<std::uint32_t>(int_num) * 0x80);
        return consume_machine_cycles(1);   // - Call taken. Consume 1 M-cycle.
    }

    auto cpu::ret_x () -> bool
    {
        // - Get condition code. Consume 1 M-cycle if conditional.
        auto condition = cond(m_opcode);
        if (condition != CC_NO_CONDITION)
        {
            if (consume_machine_cycles(1) == false)
                { return false; }
        }

        // - Check condition.
        if (check_condition(m_regs.flags, condition) == true)
        {
            // - Pop return address from stack.
            std::uint32_t return_address = 0;
            if (pop_dword(return_address) == false)
                { return false; }

            m_regs.pc = return_address;
            return consume_machine_cycles(1);
        }

        return true;
    }

    auto cpu::reti () -> bool
    {
        // - Re-enable interrupts.
        enable_interrupts(true);

        // - Pop return address from stack.
        std::uint32_t return_address = 0;
        if (pop_dword(return_address) == false)
            { return false; }

        m_regs.pc = return_address;
        return consume_machine_cycles(1);
    }
}

/* Private Methods - 8-Bit Arithmetic Instructions ****************************/

namespace g10
{
    /**
     * @brief   Helper function for computing the flags for 8-bit addition
     *          operations.
     * 
     * @param   a       The first operand.
     * @param   b       The second operand.
     * @param   carry   The carry flag value to add.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the addition.
     */
    static auto add8_with_flags (std::uint8_t a, std::uint8_t b, 
        std::uint8_t carry, flags_register& flags) -> std::uint8_t
    {
        std::uint16_t result = static_cast<std::uint16_t>(a) + 
                               static_cast<std::uint16_t>(b) + 
                               static_cast<std::uint16_t>(carry);
        std::uint8_t result8 = static_cast<std::uint8_t>(result & 0xFF);

        // `Z`: Set if result is zero
        flags.zero = (result8 == 0) ? 1 : 0;

        // `N`: Cleared for addition
        flags.negative = 0;

        // `H`: Set if carry from bit 3 to bit 4
        flags.half_carry = (((a & 0x0F) + (b & 0x0F) + carry) > 0x0F) ? 1 : 0;

        // `C`: Set if carry from bit 7
        flags.carry = (result > 0xFF) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        // - Overflow occurs if both operands have the same sign and the result
        //   has a different sign
        std::int8_t sa = static_cast<std::int8_t>(a);
        std::int8_t sb = static_cast<std::int8_t>(b);
        std::int8_t sr = static_cast<std::int8_t>(result8);
        flags.overflow = 
            (((sa >= 0) == (sb >= 0)) && ((sa >= 0) != (sr >= 0))) ? 1 : 0;

        return result8;
    }

    /**
     * @brief   Helper function for computing the flags for 8-bit subtraction
     *          operations.
     * 
     * @param   a       The first operand (minuend).
     * @param   b       The second operand (subtrahend).
     * @param   carry   The carry/borrow flag value to subtract.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the subtraction.
     */
    static auto sub8_with_flags (std::uint8_t a, std::uint8_t b, 
        std::uint8_t carry, flags_register& flags) -> std::uint8_t
    {
        std::int16_t result = static_cast<std::int16_t>(a) - 
                              static_cast<std::int16_t>(b) - 
                              static_cast<std::int16_t>(carry);
        std::uint8_t result8 = static_cast<std::uint8_t>(result & 0xFF);

        // `Z`: Set if result is zero
        flags.zero = (result8 == 0) ? 1 : 0;

        // `N`: Set for subtraction
        flags.negative = 1;

        // `H`: Set if borrow from bit 4 to bit 3
        flags.half_carry = ((a & 0x0F) < ((b & 0x0F) + carry)) ? 1 : 0;

        // `C`: Set if borrow from bit 8
        flags.carry = (result < 0) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        // - Overflow occurs if operands have different signs and the result
        //   has a different sign from the minuend
        std::int8_t sa = static_cast<std::int8_t>(a);
        std::int8_t sb = static_cast<std::int8_t>(b);
        std::int8_t sr = static_cast<std::int8_t>(result8);
        flags.overflow = 
            (((sa >= 0) != (sb >= 0)) && ((sa >= 0) != (sr >= 0))) ? 1 : 0;

        return result8;
    }

    /**
     * @brief   Helper function for computing the flags for 8-bit increment
     *          operations.
     * 
     * @param   a       The operand to increment.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the increment.
     */
    static auto inc8_with_flags (std::uint8_t a, flags_register& flags) 
        -> std::uint8_t
    {
        std::uint8_t result = a + 1;

        // `Z`: Set if result is zero
        flags.zero = (result == 0) ? 1 : 0;

        // `N`: Cleared for increment
        flags.negative = 0;

        // `H`: Set if carry from bit 3 to bit 4
        flags.half_carry = ((a & 0x0F) == 0x0F) ? 1 : 0;

        // `C`: Unchanged for increment

        // `V`: Set if signed overflow occurred (`0x7F -> 0x80`)
        flags.overflow = (a == 0x7F) ? 1 : 0;

        return result;
    }

    /**
     * @brief   Helper function for computing the flags for 8-bit decrement
     *          operations.
     * 
     * @param   a       The operand to decrement.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the decrement.
     */
    static auto dec8_with_flags (std::uint8_t a, flags_register& flags) 
        -> std::uint8_t
    {
        std::uint8_t result = a - 1;

        // `Z`: Set if result is zero
        flags.zero = (result == 0) ? 1 : 0;

        // `N`: Set for decrement
        flags.negative = 1;

        // `H`: Set if borrow from bit 4 to bit 3
        flags.half_carry = ((a & 0x0F) == 0x00) ? 1 : 0;

        // `C`: Unchanged for decrement

        // `V`: Set if signed overflow occurred (`0x80 -> 0x7F`)
        flags.overflow = (a == 0x80) ? 1 : 0;

        return result;
    }

    auto cpu::add_l0_imm8 () -> bool
    {
        // - Read L0 and the immediate value.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);

        // - Perform the addition and update flags.
        std::uint8_t result = add8_with_flags(l0, imm, 0, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::add_l0_ly () -> bool
    {
        // - Read L0 and LY.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));

        // - Perform the addition and update flags.
        std::uint8_t result = add8_with_flags(l0, ly, 0, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::add_l0_pdy () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the addition and update flags.
        std::uint8_t result = add8_with_flags(l0, value, 0, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::adc_l0_imm8 () -> bool
    {
        // - Read L0, the immediate value, and the carry flag.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);
        std::uint8_t carry = m_regs.flags.carry;

        // - Perform the addition with carry and update flags.
        std::uint8_t result = add8_with_flags(l0, imm, carry, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::adc_l0_ly () -> bool
    {
        // - Read L0, LY, and the carry flag.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));
        std::uint8_t carry = m_regs.flags.carry;

        // - Perform the addition with carry and update flags.
        std::uint8_t result = add8_with_flags(l0, ly, carry, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::adc_l0_pdy () -> bool
    {
        // - Read L0 and the carry flag.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t carry = m_regs.flags.carry;

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the addition with carry and update flags.
        std::uint8_t result = add8_with_flags(l0, value, carry, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::sub_l0_imm8 () -> bool
    {
        // - Read L0 and the immediate value.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);

        // - Perform the subtraction and update flags.
        std::uint8_t result = sub8_with_flags(l0, imm, 0, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::sub_l0_ly () -> bool
    {
        // - Read L0 and LY.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));

        // - Perform the subtraction and update flags.
        std::uint8_t result = sub8_with_flags(l0, ly, 0, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::sub_l0_pdy () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the subtraction and update flags.
        std::uint8_t result = sub8_with_flags(l0, value, 0, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::sbc_l0_imm8 () -> bool
    {
        // - Read L0, the immediate value, and the carry flag.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);
        std::uint8_t carry = m_regs.flags.carry;

        // - Perform the subtraction with carry and update flags.
        std::uint8_t result = sub8_with_flags(l0, imm, carry, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::sbc_l0_ly () -> bool
    {
        // - Read L0, LY, and the carry flag.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));
        std::uint8_t carry = m_regs.flags.carry;

        // - Perform the subtraction with carry and update flags.
        std::uint8_t result = sub8_with_flags(l0, ly, carry, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::sbc_l0_pdy () -> bool
    {
        // - Read L0 and the carry flag.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t carry = m_regs.flags.carry;

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the subtraction with carry and update flags.
        std::uint8_t result = sub8_with_flags(l0, value, carry, m_regs.flags);

        // - Write the result back to L0.
        write_register(register_type::l0, result);
        return true;
    }

    auto cpu::inc_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Perform the increment and update flags.
        std::uint8_t result = inc8_with_flags(lx, m_regs.flags);

        // - Write the result back to LX.
        write_register(lx_reg, result);
        return true;
    }

    auto cpu::inc_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the increment and update flags.
        std::uint8_t result = inc8_with_flags(value, m_regs.flags);

        // - Write the result back to memory.
        return write_byte(address, result);
    }

    auto cpu::dec_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Perform the decrement and update flags.
        std::uint8_t result = dec8_with_flags(lx, m_regs.flags);

        // - Write the result back to LX.
        write_register(lx_reg, result);
        return true;
    }

    auto cpu::dec_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the decrement and update flags.
        std::uint8_t result = dec8_with_flags(value, m_regs.flags);

        // - Write the result back to memory.
        return write_byte(address, result);
    }
}

/* Private Methods - 16-Bit and 32-Bit Arithmetic Instructions ****************/

namespace g10
{
    /**
     * @brief   Helper function for computing the flags for 16-bit addition
     *          operations.
     * 
     * @param   a       The first operand.
     * @param   b       The second operand.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the addition.
     */
    static auto add16_with_flags (std::uint16_t a, std::uint16_t b, 
        flags_register& flags) -> std::uint16_t
    {
        std::uint32_t result = static_cast<std::uint32_t>(a) + 
                               static_cast<std::uint32_t>(b);
        std::uint16_t result16 = static_cast<std::uint16_t>(result & 0xFFFF);

        // `Z`: Set if result is zero
        flags.zero = (result16 == 0) ? 1 : 0;

        // `N`: Cleared for addition
        flags.negative = 0;

        // `H`: Set if carry from bit 11 to bit 12
        flags.half_carry = (((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF) ? 1 : 0;

        // `C`: Set if carry from bit 15
        flags.carry = (result > 0xFFFF) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        std::int16_t sa = static_cast<std::int16_t>(a);
        std::int16_t sb = static_cast<std::int16_t>(b);
        std::int16_t sr = static_cast<std::int16_t>(result16);
        flags.overflow = 
            (((sa >= 0) == (sb >= 0)) && ((sa >= 0) != (sr >= 0))) ? 1 : 0;

        return result16;
    }

    /**
     * @brief   Helper function for computing the flags for 32-bit addition
     *          operations.
     * 
     * @param   a       The first operand.
     * @param   b       The second operand.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the addition.
     */
    static auto add32_with_flags (std::uint32_t a, std::uint32_t b, 
        flags_register& flags) -> std::uint32_t
    {
        std::uint64_t result = static_cast<std::uint64_t>(a) + 
                               static_cast<std::uint64_t>(b);
        std::uint32_t result32 = static_cast<std::uint32_t>(result & 0xFFFFFFFF);

        // `Z`: Set if result is zero
        flags.zero = (result32 == 0) ? 1 : 0;

        // `N`: Cleared for addition
        flags.negative = 0;

        // `H`: Set if carry from bit 27 to bit 28
        flags.half_carry = 
            (((a & 0x0FFFFFFF) + (b & 0x0FFFFFFF)) > 0x0FFFFFFF) ? 1 : 0;

        // `C`: Set if carry from bit 31
        flags.carry = (result > 0xFFFFFFFF) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        std::int32_t sa = static_cast<std::int32_t>(a);
        std::int32_t sb = static_cast<std::int32_t>(b);
        std::int32_t sr = static_cast<std::int32_t>(result32);
        flags.overflow = (((sa >= 0) == (sb >= 0)) && ((sa >= 0) != (sr >= 0))) ? 1 : 0;

        return result32;
    }

    /**
     * @brief   Helper function for computing the flags for 16-bit subtraction
     *          operations.
     * 
     * @param   a       The first operand (minuend).
     * @param   b       The second operand (subtrahend).
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the subtraction.
     */
    static auto sub16_with_flags (std::uint16_t a, std::uint16_t b, 
        flags_register& flags) -> std::uint16_t
    {
        std::int32_t result = static_cast<std::int32_t>(a) - 
                              static_cast<std::int32_t>(b);
        std::uint16_t result16 = static_cast<std::uint16_t>(result & 0xFFFF);

        // `Z`: Set if result is zero
        flags.zero = (result16 == 0) ? 1 : 0;

        // `N`: Set for subtraction
        flags.negative = 1;

        // `H`: Set if borrow from bit 12 to bit 11
        flags.half_carry = ((a & 0x0FFF) < (b & 0x0FFF)) ? 1 : 0;

        // `C`: Set if borrow from bit 16
        flags.carry = (result < 0) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        std::int16_t sa = static_cast<std::int16_t>(a);
        std::int16_t sb = static_cast<std::int16_t>(b);
        std::int16_t sr = static_cast<std::int16_t>(result16);
        flags.overflow = 
            (((sa >= 0) != (sb >= 0)) && ((sa >= 0) != (sr >= 0))) ? 1 : 0;

        return result16;
    }

    /**
     * @brief   Helper function for computing the flags for 32-bit subtraction
     *          operations.
     * 
     * @param   a       The first operand (minuend).
     * @param   b       The second operand (subtrahend).
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the subtraction.
     */
    static auto sub32_with_flags (std::uint32_t a, std::uint32_t b, 
        flags_register& flags) -> std::uint32_t
    {
        std::int64_t result = static_cast<std::int64_t>(a) - 
                              static_cast<std::int64_t>(b);
        std::uint32_t result32 = static_cast<std::uint32_t>(result & 0xFFFFFFFF);

        // `Z`: Set if result is zero
        flags.zero = (result32 == 0) ? 1 : 0;

        // `N`: Set for subtraction
        flags.negative = 1;

        // `H`: Set if borrow from bit 28 to bit 27
        flags.half_carry = ((a & 0x0FFFFFFF) < (b & 0x0FFFFFFF)) ? 1 : 0;

        // `C`: Set if borrow from bit 32
        flags.carry = (result < 0) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        std::int32_t sa = static_cast<std::int32_t>(a);
        std::int32_t sb = static_cast<std::int32_t>(b);
        std::int32_t sr = static_cast<std::int32_t>(result32);
        flags.overflow = 
            (((sa >= 0) != (sb >= 0)) && ((sa >= 0) != (sr >= 0))) ? 1 : 0;

        return result32;
    }

    /**
     * @brief   Helper function for computing the flags for 16-bit increment
     *          operations. Does not affect C or V flags.
     * 
     * @param   a       The operand to increment.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the increment.
     */
    static auto inc16_with_flags (std::uint16_t a, flags_register& flags) 
        -> std::uint16_t
    {
        std::uint16_t result = a + 1;

        // `Z`: Set if result is zero
        flags.zero = (result == 0) ? 1 : 0;

        // `N`: Cleared for increment
        flags.negative = 0;

        // `H`: Set if carry from bit 11 to bit 12
        flags.half_carry = ((a & 0x0FFF) == 0x0FFF) ? 1 : 0;

        // `C`: Unchanged
        // `V`: Unchanged

        return result;
    }

    /**
     * @brief   Helper function for computing the flags for 32-bit increment
     *          operations. Does not affect C or V flags.
     * 
     * @param   a       The operand to increment.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the increment.
     */
    static auto inc32_with_flags (std::uint32_t a, flags_register& flags) 
        -> std::uint32_t
    {
        std::uint32_t result = a + 1;

        // `Z`: Set if result is zero
        flags.zero = (result == 0) ? 1 : 0;

        // `N`: Cleared for increment
        flags.negative = 0;

        // `H`: Set if carry from bit 27 to bit 28
        flags.half_carry = ((a & 0x0FFFFFFF) == 0x0FFFFFFF) ? 1 : 0;

        // `C`: Unchanged
        // `V`: Unchanged

        return result;
    }

    /**
     * @brief   Helper function for computing the flags for 16-bit decrement
     *          operations. Does not affect C or V flags.
     * 
     * @param   a       The operand to decrement.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the decrement.
     */
    static auto dec16_with_flags (std::uint16_t a, flags_register& flags) -> std::uint16_t
    {
        std::uint16_t result = a - 1;

        // `Z`: Set if result is zero
        flags.zero = (result == 0) ? 1 : 0;

        // `N`: Set for decrement
        flags.negative = 1;

        // `H`: Set if borrow from bit 12 to bit 11
        flags.half_carry = ((a & 0x0FFF) == 0x0000) ? 1 : 0;

        // `C`: Unchanged
        // `V`: Unchanged

        return result;
    }

    /**
     * @brief   Helper function for computing the flags for 32-bit decrement
     *          operations. Does not affect C or V flags.
     * 
     * @param   a       The operand to decrement.
     * @param   flags   A reference to the flags register to update.
     * 
     * @return  The result of the decrement.
     */
    static auto dec32_with_flags (std::uint32_t a, flags_register& flags) 
        -> std::uint32_t
    {
        std::uint32_t result = a - 1;

        // `Z`: Set if result is zero
        flags.zero = (result == 0) ? 1 : 0;

        // `N`: Set for decrement
        flags.negative = 1;

        // `H`: Set if borrow from bit 28 to bit 27
        flags.half_carry = ((a & 0x0FFFFFFF) == 0x00000000) ? 1 : 0;

        // `C`: Unchanged
        // `V`: Unchanged

        return result;
    }

    auto cpu::add_w0_imm16 () -> bool
    {
        // - Read W0 and the immediate value.
        std::uint16_t w0 = read_register(register_type::w0);
        std::uint16_t imm = static_cast<std::uint16_t>(m_fetch_data & 0xFFFF);

        // - Perform the addition and update flags.
        std::uint16_t result = add16_with_flags(w0, imm, m_regs.flags);

        // - Write the result back to W0.
        write_register(register_type::w0, result);

        // - Consume the extra M-cycle for 16-bit operation.
        return consume_machine_cycles(1);
    }

    auto cpu::add_w0_wy () -> bool
    {
        // - Read W0 and WY.
        std::uint16_t w0 = read_register(register_type::w0);
        std::uint16_t wy = read_register(word_reg(m_opcode));

        // - Perform the addition and update flags.
        std::uint16_t result = add16_with_flags(w0, wy, m_regs.flags);

        // - Write the result back to W0.
        write_register(register_type::w0, result);

        // - Consume the extra M-cycle for 16-bit operation.
        return consume_machine_cycles(1);
    }

    auto cpu::add_d0_imm32 () -> bool
    {
        // - Read D0 and the immediate value.
        std::uint32_t d0 = read_register(register_type::d0);
        std::uint32_t imm = m_fetch_data;

        // - Perform the addition and update flags.
        std::uint32_t result = add32_with_flags(d0, imm, m_regs.flags);

        // - Write the result back to D0.
        write_register(register_type::d0, result);

        // - Consume the extra M-cycles for 32-bit operation.
        return consume_machine_cycles(3);
    }

    auto cpu::add_d0_dy () -> bool
    {
        // - Read D0 and DY.
        std::uint32_t d0 = read_register(register_type::d0);
        std::uint32_t dy = read_register(full_reg(m_opcode));

        // - Perform the addition and update flags.
        std::uint32_t result = add32_with_flags(d0, dy, m_regs.flags);

        // - Write the result back to D0.
        write_register(register_type::d0, result);

        // - Consume the extra M-cycles for 32-bit operation.
        return consume_machine_cycles(3);
    }

    auto cpu::sub_w0_imm16 () -> bool
    {
        // - Read W0 and the immediate value.
        std::uint16_t w0 = read_register(register_type::w0);
        std::uint16_t imm = static_cast<std::uint16_t>(m_fetch_data & 0xFFFF);

        // - Perform the subtraction and update flags.
        std::uint16_t result = sub16_with_flags(w0, imm, m_regs.flags);

        // - Write the result back to W0.
        write_register(register_type::w0, result);

        // - Consume the extra M-cycle for 16-bit operation.
        return consume_machine_cycles(1);
    }

    auto cpu::sub_w0_wy () -> bool
    {
        // - Read W0 and WY.
        std::uint16_t w0 = read_register(register_type::w0);
        std::uint16_t wy = read_register(word_reg(m_opcode));

        // - Perform the subtraction and update flags.
        std::uint16_t result = sub16_with_flags(w0, wy, m_regs.flags);

        // - Write the result back to W0.
        write_register(register_type::w0, result);

        // - Consume the extra M-cycle for 16-bit operation.
        return consume_machine_cycles(1);
    }

    auto cpu::sub_d0_imm32 () -> bool
    {
        // - Read D0 and the immediate value.
        std::uint32_t d0 = read_register(register_type::d0);
        std::uint32_t imm = m_fetch_data;

        // - Perform the subtraction and update flags.
        std::uint32_t result = sub32_with_flags(d0, imm, m_regs.flags);

        // - Write the result back to D0.
        write_register(register_type::d0, result);

        // - Consume the extra M-cycles for 32-bit operation.
        return consume_machine_cycles(3);
    }

    auto cpu::sub_d0_dy () -> bool
    {
        // - Read D0 and DY.
        std::uint32_t d0 = read_register(register_type::d0);
        std::uint32_t dy = read_register(full_reg(m_opcode));

        // - Perform the subtraction and update flags.
        std::uint32_t result = sub32_with_flags(d0, dy, m_regs.flags);

        // - Write the result back to D0.
        write_register(register_type::d0, result);

        // - Consume the extra M-cycles for 32-bit operation.
        return consume_machine_cycles(3);
    }

    auto cpu::inc_wx () -> bool
    {
        // - Read WX (register index is in upper nibble of lower byte).
        register_type wx_reg = word_reg(m_opcode >> 4);
        std::uint16_t wx = read_register(wx_reg);

        // - Perform the increment and update flags.
        std::uint16_t result = inc16_with_flags(wx, m_regs.flags);

        // - Write the result back to WX.
        write_register(wx_reg, result);

        // - Consume the extra M-cycle for 16-bit operation.
        return consume_machine_cycles(1);
    }

    auto cpu::inc_dx () -> bool
    {
        // - Read DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t dx = read_register(dx_reg);

        // - Perform the increment and update flags.
        std::uint32_t result = inc32_with_flags(dx, m_regs.flags);

        // - Write the result back to DX.
        write_register(dx_reg, result);

        // - Consume the extra M-cycles for 32-bit operation.
        return consume_machine_cycles(3);
    }

    auto cpu::dec_wx () -> bool
    {
        // - Read WX (register index is in upper nibble of lower byte).
        register_type wx_reg = word_reg(m_opcode >> 4);
        std::uint16_t wx = read_register(wx_reg);

        // - Perform the decrement and update flags.
        std::uint16_t result = dec16_with_flags(wx, m_regs.flags);

        // - Write the result back to WX.
        write_register(wx_reg, result);

        // - Consume the extra M-cycle for 16-bit operation.
        return consume_machine_cycles(1);
    }

    auto cpu::dec_dx () -> bool
    {
        // - Read DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t dx = read_register(dx_reg);

        // - Perform the decrement and update flags.
        std::uint32_t result = dec32_with_flags(dx, m_regs.flags);

        // - Write the result back to DX.
        write_register(dx_reg, result);

        // - Consume the extra M-cycles for 32-bit operation.
        return consume_machine_cycles(3);
    }
}

/* Private Methods - 8-Bit Bitwise and Logical Instructions *******************/

namespace g10
{
    auto cpu::and_l0_imm8 () -> bool
    {
        // - Read L0 and the immediate value.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);

        // - Perform the AND operation.
        std::uint8_t result = l0 & imm;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=1, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 1;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::and_l0_ly () -> bool
    {
        // - Read L0 and LY.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));

        // - Perform the AND operation.
        std::uint8_t result = l0 & ly;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=1, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 1;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::and_l0_pdy () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the AND operation.
        std::uint8_t result = l0 & value;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=1, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 1;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::or_l0_imm8 () -> bool
    {
        // - Read L0 and the immediate value.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);

        // - Perform the OR operation.
        std::uint8_t result = l0 | imm;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::or_l0_ly () -> bool
    {
        // - Read L0 and LY.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));

        // - Perform the OR operation.
        std::uint8_t result = l0 | ly;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::or_l0_pdy () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the OR operation.
        std::uint8_t result = l0 | value;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::xor_l0_imm8 () -> bool
    {
        // - Read L0 and the immediate value.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);

        // - Perform the XOR operation.
        std::uint8_t result = l0 ^ imm;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::xor_l0_ly () -> bool
    {
        // - Read L0 and LY.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));

        // - Perform the XOR operation.
        std::uint8_t result = l0 ^ ly;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::xor_l0_pdy () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the XOR operation.
        std::uint8_t result = l0 ^ value;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=0
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::not_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Perform the NOT operation.
        std::uint8_t result = ~lx;

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=unchanged, N=1, H=1, C=unchanged, V=0
        m_regs.flags.negative = 1;
        m_regs.flags.half_carry = 1;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::not_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the NOT operation.
        std::uint8_t result = ~value;

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=unchanged, N=1, H=1, C=unchanged, V=0
        m_regs.flags.negative = 1;
        m_regs.flags.half_carry = 1;
        m_regs.flags.overflow = 0;

        return true;
    }

    auto cpu::cmp_l0_imm8 () -> bool
    {
        // - Read L0 and the immediate value.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t imm = static_cast<std::uint8_t>(m_fetch_data & 0xFF);

        // - Perform the comparison (subtraction without storing result).
        std::int16_t result = static_cast<std::int16_t>(l0) - 
                              static_cast<std::int16_t>(imm);
        std::uint8_t result8 = static_cast<std::uint8_t>(result & 0xFF);

        // - Update flags: Z=?, N=1, H=?, C=?, V=?
        m_regs.flags.zero = (result8 == 0) ? 1 : 0;
        m_regs.flags.negative = 1;
        m_regs.flags.half_carry = ((l0 & 0x0F) < (imm & 0x0F)) ? 1 : 0;
        m_regs.flags.carry = (result < 0) ? 1 : 0;

        // `V`: Set if signed overflow occurred
        std::int8_t sl0 = static_cast<std::int8_t>(l0);
        std::int8_t simm = static_cast<std::int8_t>(imm);
        std::int8_t sr = static_cast<std::int8_t>(result8);
        m_regs.flags.overflow = 
            (((sl0 >= 0) != (simm >= 0)) && ((sl0 >= 0) != (sr >= 0))) ? 1 : 0;

        return true;
    }

    auto cpu::cmp_l0_ly () -> bool
    {
        // - Read L0 and LY.
        std::uint8_t l0 = read_register(register_type::l0);
        std::uint8_t ly = read_register(low_byte_reg(m_opcode));

        // - Perform the comparison (subtraction without storing result).
        std::int16_t result = static_cast<std::int16_t>(l0) - 
                              static_cast<std::int16_t>(ly);
        std::uint8_t result8 = static_cast<std::uint8_t>(result & 0xFF);

        // - Update flags: Z=?, N=1, H=?, C=?, V=?
        m_regs.flags.zero = (result8 == 0) ? 1 : 0;
        m_regs.flags.negative = 1;
        m_regs.flags.half_carry = ((l0 & 0x0F) < (ly & 0x0F)) ? 1 : 0;
        m_regs.flags.carry = (result < 0) ? 1 : 0;

        // V: Set if signed overflow occurred
        std::int8_t sl0 = static_cast<std::int8_t>(l0);
        std::int8_t sly = static_cast<std::int8_t>(ly);
        std::int8_t sr = static_cast<std::int8_t>(result8);
        m_regs.flags.overflow = 
            (((sl0 >= 0) != (sly >= 0)) && ((sl0 >= 0) != (sr >= 0))) ? 1 : 0;

        return true;
    }

    auto cpu::cmp_l0_pdy () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Read the address from DY and fetch the byte from memory.
        std::uint32_t address = read_register(full_reg(m_opcode));
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Perform the comparison (subtraction without storing result).
        std::int16_t result = static_cast<std::int16_t>(l0) - 
                              static_cast<std::int16_t>(value);
        std::uint8_t result8 = static_cast<std::uint8_t>(result & 0xFF);

        // - Update flags: Z=?, N=1, H=?, C=?, V=?
        m_regs.flags.zero = (result8 == 0) ? 1 : 0;
        m_regs.flags.negative = 1;
        m_regs.flags.half_carry = ((l0 & 0x0F) < (value & 0x0F)) ? 1 : 0;
        m_regs.flags.carry = (result < 0) ? 1 : 0;

        // V: Set if signed overflow occurred
        std::int8_t sl0 = static_cast<std::int8_t>(l0);
        std::int8_t sval = static_cast<std::int8_t>(value);
        std::int8_t sr = static_cast<std::int8_t>(result8);
        m_regs.flags.overflow = 
            (((sl0 >= 0) != (sval >= 0)) && ((sl0 >= 0) != (sr >= 0))) ? 1 : 0;

        return true;
    }
}

/* Private Methods - Bit Shift and Swap Instructions **************************/

namespace g10
{
    auto cpu::sla_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 7 for carry flag.
        std::uint8_t bit7 = (lx >> 7) & 0x01;

        // - Perform the shift left (bit 0 becomes 0).
        std::uint8_t result = lx << 1;

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::sla_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 7 for carry flag.
        std::uint8_t bit7 = (value >> 7) & 0x01;

        // - Perform the shift left (bit 0 becomes 0).
        std::uint8_t result = value << 1;

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::sra_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 0 for carry flag and bit 7 for sign preservation.
        std::uint8_t bit0 = lx & 0x01;
        std::uint8_t bit7 = lx & 0x80;

        // - Perform the arithmetic shift right (bit 7 is preserved).
        std::uint8_t result = (lx >> 1) | bit7;

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::sra_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 0 for carry flag and bit 7 for sign preservation.
        std::uint8_t bit0 = value & 0x01;
        std::uint8_t bit7 = value & 0x80;

        // - Perform the arithmetic shift right (bit 7 is preserved).
        std::uint8_t result = (value >> 1) | bit7;

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::srl_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 0 for carry flag.
        std::uint8_t bit0 = lx & 0x01;

        // - Perform the logical shift right (bit 7 becomes 0).
        std::uint8_t result = lx >> 1;

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::srl_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 0 for carry flag.
        std::uint8_t bit0 = value & 0x01;

        // - Perform the logical shift right (bit 7 becomes 0).
        std::uint8_t result = value >> 1;

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::swap_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Swap upper and lower nibbles.
        std::uint8_t result = ((lx & 0x0F) << 4) | ((lx & 0xF0) >> 4);

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;

        return true;
    }

    auto cpu::swap_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Swap upper and lower nibbles.
        std::uint8_t result = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;

        return true;
    }

    auto cpu::swap_wx () -> bool
    {
        // - Read WX (register index is in upper nibble of lower byte).
        register_type wx_reg = word_reg(m_opcode >> 4);
        std::uint16_t wx = read_register(wx_reg);

        // - Swap upper and lower bytes.
        std::uint16_t result = ((wx & 0x00FF) << 8) | ((wx & 0xFF00) >> 8);

        // - Write the result back to WX.
        write_register(wx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;

        return true;
    }

    auto cpu::swap_dx () -> bool
    {
        // - Read DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t dx = read_register(dx_reg);

        // - Swap upper and lower words.
        std::uint32_t result = ((dx & 0x0000FFFF) << 16) | ((dx & 0xFFFF0000) >> 16);

        // - Write the result back to DX.
        write_register(dx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = 0;

        return true;
    }
}

/* Private Methods - Bit Rotate Instructions **********************************/

namespace g10
{
    auto cpu::rla () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Save bit 7 for new carry and old carry for bit 0.
        std::uint8_t bit7 = (l0 >> 7) & 0x01;
        std::uint8_t old_carry = m_regs.flags.carry;

        // - Rotate left through carry (bit 0 = old carry, bit 7 -> new carry).
        std::uint8_t result = (l0 << 1) | old_carry;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=0, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::rl_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 7 for new carry and old carry for bit 0.
        std::uint8_t bit7 = (lx >> 7) & 0x01;
        std::uint8_t old_carry = m_regs.flags.carry;

        // - Rotate left through carry (bit 0 = old carry, bit 7 -> new carry).
        std::uint8_t result = (lx << 1) | old_carry;

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::rl_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 7 for new carry and old carry for bit 0.
        std::uint8_t bit7 = (value >> 7) & 0x01;
        std::uint8_t old_carry = m_regs.flags.carry;

        // - Rotate left through carry (bit 0 = old carry, bit 7 -> new carry).
        std::uint8_t result = (value << 1) | old_carry;

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::rlca () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Save bit 7 for new carry and for bit 0.
        std::uint8_t bit7 = (l0 >> 7) & 0x01;

        // - Rotate left circularly (bit 0 = old bit 7, bit 7 -> carry).
        std::uint8_t result = (l0 << 1) | bit7;

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=0, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::rlc_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 7 for new carry and for bit 0.
        std::uint8_t bit7 = (lx >> 7) & 0x01;

        // - Rotate left circularly (bit 0 = old bit 7, bit 7 -> carry).
        std::uint8_t result = (lx << 1) | bit7;

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::rlc_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 7 for new carry and for bit 0.
        std::uint8_t bit7 = (value >> 7) & 0x01;

        // - Rotate left circularly (bit 0 = old bit 7, bit 7 -> carry).
        std::uint8_t result = (value << 1) | bit7;

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit7, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit7;

        return true;
    }

    auto cpu::rra () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Save bit 0 for new carry and old carry for bit 7.
        std::uint8_t bit0 = l0 & 0x01;
        std::uint8_t old_carry = m_regs.flags.carry;

        // - Rotate right through carry (bit 7 = old carry, bit 0 -> new carry).
        std::uint8_t result = (l0 >> 1) | (old_carry << 7);

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=0, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::rr_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 0 for new carry and old carry for bit 7.
        std::uint8_t bit0 = lx & 0x01;
        std::uint8_t old_carry = m_regs.flags.carry;

        // - Rotate right through carry (bit 7 = old carry, bit 0 -> new carry).
        std::uint8_t result = (lx >> 1) | (old_carry << 7);

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::rr_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 0 for new carry and old carry for bit 7.
        std::uint8_t bit0 = value & 0x01;
        std::uint8_t old_carry = m_regs.flags.carry;

        // - Rotate right through carry (bit 7 = old carry, bit 0 -> new carry).
        std::uint8_t result = (value >> 1) | (old_carry << 7);

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::rrca () -> bool
    {
        // - Read L0.
        std::uint8_t l0 = read_register(register_type::l0);

        // - Save bit 0 for new carry and for bit 7.
        std::uint8_t bit0 = l0 & 0x01;

        // - Rotate right circularly (bit 7 = old bit 0, bit 0 -> carry).
        std::uint8_t result = (l0 >> 1) | (bit0 << 7);

        // - Write the result back to L0.
        write_register(register_type::l0, result);

        // - Update flags: Z=0, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::rrc_lx () -> bool
    {
        // - Read LX (register index is in upper nibble of lower byte).
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Save bit 0 for new carry and for bit 7.
        std::uint8_t bit0 = lx & 0x01;

        // - Rotate right circularly (bit 7 = old bit 0, bit 0 -> carry).
        std::uint8_t result = (lx >> 1) | (bit0 << 7);

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }

    auto cpu::rrc_pdx () -> bool
    {
        // - Read the address from DX (register index is in upper nibble of lower byte).
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Save bit 0 for new carry and for bit 7.
        std::uint8_t bit0 = value & 0x01;

        // - Rotate right circularly (bit 7 = old bit 0, bit 0 -> carry).
        std::uint8_t result = (value >> 1) | (bit0 << 7);

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - Update flags: Z=?, N=0, H=0, C=bit0, V=unchanged
        m_regs.flags.zero = (result == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 0;
        m_regs.flags.carry = bit0;

        return true;
    }
}

/* Private Methods - Bit Test and Manipulation Instructions *******************/

namespace g10
{
    auto cpu::bit_y_lx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read LX.
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Test the specified bit.
        std::uint8_t bit_value = (lx >> y) & 0x01;

        // - Update flags: Z=!bit, N=0, H=1, C=unchanged, V=unchanged
        m_regs.flags.zero = (bit_value == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 1;

        return true;
    }

    auto cpu::bit_y_pdx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read the address from DX.
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Test the specified bit.
        std::uint8_t bit_value = (value >> y) & 0x01;

        // - Update flags: Z=!bit, N=0, H=1, C=unchanged, V=unchanged
        m_regs.flags.zero = (bit_value == 0) ? 1 : 0;
        m_regs.flags.negative = 0;
        m_regs.flags.half_carry = 1;

        return true;
    }

    auto cpu::set_y_lx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read LX.
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Set the specified bit.
        std::uint8_t result = lx | (1 << y);

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - No flags affected.
        return true;
    }

    auto cpu::set_y_pdx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read the address from DX.
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Set the specified bit.
        std::uint8_t result = value | (1 << y);

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - No flags affected.
        return true;
    }

    auto cpu::res_y_lx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read LX.
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Reset (clear) the specified bit.
        std::uint8_t result = lx & ~(1 << y);

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - No flags affected.
        return true;
    }

    auto cpu::res_y_pdx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read the address from DX.
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Reset (clear) the specified bit.
        std::uint8_t result = value & ~(1 << y);

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - No flags affected.
        return true;
    }

    auto cpu::tog_y_lx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read LX.
        register_type lx_reg = low_byte_reg(m_opcode >> 4);
        std::uint8_t lx = read_register(lx_reg);

        // - Toggle the specified bit.
        std::uint8_t result = lx ^ (1 << y);

        // - Write the result back to LX.
        write_register(lx_reg, result);

        // - No flags affected.
        return true;
    }

    auto cpu::tog_y_pdx () -> bool
    {
        // - Extract Y (the bit index) from lower nibble.
        std::uint8_t y = m_opcode & 0x0F;

        // - Validate bit index (must be 0-7).
        if (y > 7)
            { return raise_exception(EC_INVALID_ARGUMENT); }

        // - Read the address from DX.
        register_type dx_reg = full_reg(m_opcode >> 4);
        std::uint32_t address = read_register(dx_reg);

        // - Fetch the byte from memory.
        std::uint8_t value = 0;
        if (read_byte(address, value) == false)
            { return false; }

        // - Toggle the specified bit.
        std::uint8_t result = value ^ (1 << y);

        // - Write the result back to memory.
        if (write_byte(address, result) == false)
            { return false; }

        // - No flags affected.
        return true;
    }
}
