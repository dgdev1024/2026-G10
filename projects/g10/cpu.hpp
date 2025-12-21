/**
 * @file    g10/cpu.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-14
 * 
 * @brief   Contains declarations for the G10 CPU context class.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10
{
    /**
     * @brief   Strongly enumerates the types of registers accessible within the
     *          G10 CPU's register file.
     */
    enum class register_type : std::uint8_t
    {
        // General-Purpose Full Registers (`Dn`)
        d0      = 0b00000000,
        d1      = 0b00000001,
        d2      = 0b00000010,
        d3      = 0b00000011,
        d4      = 0b00000100,
        d5      = 0b00000101,
        d6      = 0b00000110,
        d7      = 0b00000111,
        d8      = 0b00001000,
        d9      = 0b00001001,
        d10     = 0b00001010,
        d11     = 0b00001011,
        d12     = 0b00001100,
        d13     = 0b00001101,
        d14     = 0b00001110,
        d15     = 0b00001111,

        // General-Purpose Word Registers (`Wn`, lower 16 bits of `Dn`)
        w0      = 0b00010000,
        w1      = 0b00010001,
        w2      = 0b00010010,
        w3      = 0b00010011,
        w4      = 0b00010100,
        w5      = 0b00010101,
        w6      = 0b00010110,
        w7      = 0b00010111,
        w8      = 0b00011000,
        w9      = 0b00011001,
        w10     = 0b00011010,
        w11     = 0b00011011,
        w12     = 0b00011100,
        w13     = 0b00011101,
        w14     = 0b00011110,
        w15     = 0b00011111,

        // General-Purpose High Byte Registers (`Hn`, upper 8 bits of `Wn`)
        h0      = 0b00100000,
        h1      = 0b00100001,
        h2      = 0b00100010,
        h3      = 0b00100011,
        h4      = 0b00100100,
        h5      = 0b00100101,
        h6      = 0b00100110,
        h7      = 0b00100111,
        h8      = 0b00101000,
        h9      = 0b00101001,
        h10     = 0b00101010,
        h11     = 0b00101011,
        h12     = 0b00101100,
        h13     = 0b00101101,
        h14     = 0b00101110,
        h15     = 0b00101111,

        // General-Purpose Low Byte Registers (`Ln`, lower 8 bits of `Wn`)
        l0      = 0b01000000,
        l1      = 0b01000001,
        l2      = 0b01000010,
        l3      = 0b01000011,
        l4      = 0b01000100,
        l5      = 0b01000101,
        l6      = 0b01000110,
        l7      = 0b01000111,
        l8      = 0b01001000,
        l9      = 0b01001001,
        l10     = 0b01001010,
        l11     = 0b01001011,
        l12     = 0b01001100,
        l13     = 0b01001101,
        l14     = 0b01001110,
        l15     = 0b01001111,

        // Special-Purpose Registers
        pc      = 0b10000000, // Program Counter
        sp      = 0b10000001, // Stack Pointer
        flags   = 0b10000010, // Flags Register
        ec      = 0b10000011  // Exception Code Register
    };

    /**
     * @brief   Strongly enumerates the instruction mnemonics supported by the
     *          G10 CPU.
     */
    enum class instruction : std::uint8_t
    {
        nop,                        /** @brief `NOP` - No Operation */
        stop,                       /** @brief `STOP` - Enter `STOP` State */
        halt,                       /** @brief `HALT` - Enter `HALT` State */
        di,                         /** @brief `DI` - Disable Interrupts */
        ei,                         /** @brief `EI` - Enable Interrupts */
        eii,                        /** @brief `EII` - Enable Interrupts Immediately */
        daa,                        /** @brief `DAA` - Decimal Adjust Accumulator */
        scf,                        /** @brief `SCF` - Set Carry Flag */
        ccf,                        /** @brief `CCF` - Complement Carry Flag */
        clv,                        /** @brief `CLV` - Clear Overflow Flag */
        sev,                        /** @brief `SEV` - Set Overflow Flag */
        ld,                         /** @brief `LD` - Load */
        ldq,                        /** @brief `LDQ` - Load Quick (relative to `$FFFF0000`) */
        ldp,                        /** @brief `LDP` - Load Port (relative to `$FFFFFF00`) */
        st,                         /** @brief `ST` - Store */
        stq,                        /** @brief `STQ` - Store Quick (relative to `$FFFF0000`) */
        stp,                        /** @brief `STP` - Store Port (relative to `$FFFFFF00`) */
        mv,                         /** @brief `MV` - Move */
        mwh,                        /** @brief `MWH` - Move Word High */
        mwl,                        /** @brief `MWL` - Move Word Low */
        lsp,                        /** @brief `LSP` - Load Stack Pointer */
        pop,                        /** @brief `POP` - Pop from Stack */
        ssp,                        /** @brief `SSP` - Store Stack Pointer */
        push,                       /** @brief `PUSH` - Push to Stack */
        spo,                        /** @brief `SPO` - Stack Pointer Out */
        spi,                        /** @brief `SPI` - Stack Pointer In */
        jmp,                        /** @brief `JMP` - Jump */
        jpb,                        /** @brief `JPB` - Jump By */
        call,                       /** @brief `CALL` - Call Subroutine */
        int_,                       /** @brief `INT` - Software Interrupt */
        ret,                        /** @brief `RET` - Return from Subroutine */
        reti,                       /** @brief `RETI` - Return from Interrupt */
        add,                        /** @brief `ADD` - Add */
        adc,                        /** @brief `ADC` - Add with Carry */
        sub,                        /** @brief `SUB` - Subtract */
        sbc,                        /** @brief `SBC` - Subtract with Carry */
        inc,                        /** @brief `INC` - Increment */
        dec,                        /** @brief `DEC` - Decrement */
        and_,                       /** @brief `AND` - Bitwise AND */
        or_,                        /** @brief `OR` - Bitwise OR */
        xor_,                       /** @brief `XOR` - Bitwise XOR */
        not_,                       /** @brief `NOT` - Bitwise NOT */
        cmp,                        /** @brief `CMP` - Compare */
        sla,                        /** @brief `SLA` - Shift Left Arithmetic */
        sra,                        /** @brief `SRA` - Shift Right Arithmetic */
        srl,                        /** @brief `SRL` - Shift Right Logical */
        swap,                       /** @brief `SWAP` - Swap Halves */
        rla,                        /** @brief `RLA` - Rotate Accumulator Left Through Carry */
        rl,                         /** @brief `RL` - Rotate Left Through Carry */
        rlca,                       /** @brief `RLCA` - Rotate Accumulator Left Circular */
        rlc,                        /** @brief `RLC` - Rotate Left Circular */
        rra,                        /** @brief `RRA` - Rotate Accumulator Right Through Carry */
        rr,                         /** @brief `RR` - Rotate Right Through Carry */
        rrca,                       /** @brief `RRCA` - Rotate Accumulator Right Circular */
        rrc,                        /** @brief `RRC` - Rotate Right Circular */
        bit,                        /** @brief `BIT` - Test Bit */
        set,                        /** @brief `SET` - Set Bit */
        res,                        /** @brief `RES` - Reset Bit */
        tog,                        /** @brief `TOG` - Toggle Bit */

        // Aliases
        tcf,                        /** @brief `TCF` - Alias for the `CCF` instruction */
        jp,                         /** @brief `JP` - Alias for all `JMP` instructions */
        jr,                         /** @brief `JR` - Alias for all `JPB` instructions */
        cpl,                        /** @brief `CPL` - Alias for the `NOT L0` instruction */
        cp,                         /** @brief `CP` - Alias for all `CMP` instructions */
    };

    /**
     * @brief   Weakly enumerates the condition codes used by the G10 CPU for
     *          conditional branch instructions.
     */
    enum condition_code : std::uint8_t
    {
        CC_NO_CONDITION             = 0x0,  /** @brief `NC` - No condition (always branch) */
        CC_ZERO_SET                 = 0x1,  /** @brief `ZS` - Zero flag is set */
        CC_ZERO_CLEAR               = 0x2,  /** @brief `ZC` - Zero flag is clear */
        CC_CARRY_SET                = 0x3,  /** @brief `CS` - Carry flag is set */
        CC_CARRY_CLEAR              = 0x4,  /** @brief `CC` - Carry flag is clear */
        CC_OVERFLOW_SET             = 0x5,  /** @brief `VS` - Overflow flag is set */
        CC_OVERFLOW_CLEAR           = 0x6,  /** @brief `VC` - Overflow flag is clear */
    
        CC_COUNT                            /** @brief Number of condition codes defined */
    };

    /**
     * @brief   Weakly enumerates the exception codes used by the G10 CPU.
     */
    enum exception_code : std::uint8_t
    {
        EC_OK                       = 0x00, /** @brief No exception occurred */
        EC_INVALID_INSTRUCTION      = 0x01, /** @brief An invalid instruction was encountered */
        EC_INVALID_ARGUMENT         = 0x02, /** @brief An instruction contained an invalid argument */
        EC_INVALID_READ_ACCESS      = 0x03, /** @brief An invalid read access occurred on the system bus */
        EC_INVALID_WRITE_ACCESS     = 0x04, /** @brief An invalid write access occurred on the system bus */
        EC_INVALID_EXECUTE_ACCESS   = 0x05, /** @brief An invalid execute access occurred on the system bus */
        EC_DIVIDE_BY_ZERO           = 0x06, /** @brief A division by zero was attempted */
        EC_STACK_OVERFLOW           = 0x07, /** @brief A stack overflow occurred */
        EC_STACK_UNDERFLOW          = 0x08, /** @brief A stack underflow occurred */
        EC_HARDWARE_ERROR           = 0x09, /** @brief An error occured during a virtual hardware component's ticking operation */
        EC_DOUBLE_FAULT             = 0x0A  /** @brief A Double Fault condition is being forced */
    };
}

/* Helper Functions - Opcode Field Decoding **********************************/

namespace g10
{
    /**
     * @brief   Convert a nibble value into a full register identifier.
     */
    inline constexpr auto full_reg (std::uint16_t i) noexcept -> register_type
    {
        return static_cast<register_type>(i & 0xF);
    }

    /**
     * @brief   Convert a nibble value into a word register identifier.
     */
    inline constexpr auto word_reg (std::uint16_t i) noexcept -> register_type
    {
        return static_cast<register_type>(0b00010000 | (i & 0xF));
    }

    /**
     * @brief   Convert a nibble value into a high-byte register id.
     */
    inline constexpr auto high_byte_reg (std::uint16_t i) noexcept
        -> register_type
    {
        return static_cast<register_type>(0b00100000 | (i & 0xF));
    }

    /**
     * @brief   Convert a nibble value into a low-byte register id.
     */
    inline constexpr auto low_byte_reg (std::uint16_t i) noexcept
        -> register_type
    {
        return static_cast<register_type>(0b01000000 | (i & 0xF));
    }

    /**
     * @brief   Extract the 3-bit condition code from the opcode.
     */
    inline constexpr auto cond (std::uint16_t i) noexcept -> condition_code
    {
        return static_cast<condition_code>(((i >> 8) & 0x7));
    }

}

/* Public Unions and Structures ***********************************************/

namespace g10
{
    /**
     * @brief   Defines a union representing the G10 CPU's flags register.
     * 
     * The flags register contains individual status flags that reflect the
     * results of arithmetic and logical operations performed by the CPU, as well
     * as other conditions affecting program flow.
     */
    union flags_register final
    {
        struct
        {
            std::uint8_t            : 3;
            std::uint8_t overflow   : 1;    /** @brief Bit 3: `V` - Overflow Flag */
            std::uint8_t carry      : 1;    /** @brief Bit 4: `C` - Carry Flag */
            std::uint8_t half_carry : 1;    /** @brief Bit 5: `H` - Half-Carry Flag */
            std::uint8_t negative   : 1;    /** @brief Bit 6: `N` - Negative/Subtract Flag */
            std::uint8_t zero       : 1;    /** @brief Bit 7: `Z` - Zero Flag */
        };

        std::uint8_t raw;                   /** @brief The raw 8-bit value of the flags register */
    };

    /**
     * @brief   Defines a union representing the G10 CPU's speed switch register.
     * 
     * The speed switch register controls the CPU's operating speed mode,
     * allowing it to switch between normal- and double-speed operation. This
     * register is also used to indicate the current speed mode of the CPU and
     * arm the speed switch mechanism.
     */
    union speed_switch_register final
    {
        struct
        {
            std::uint8_t armed          : 1;    /** @brief Bit 0: Speed Switch Armed Flag */
            std::uint8_t                : 6;
            std::uint8_t double_speed   : 1;    /** @brief Bit 7: Double Speed Mode Flag */
        };

        std::uint8_t raw;
    };

    /**
     * @brief   Defines a structure representing the G10 CPU's register file.
     * 
     * The register file contains all general-purpose and special-purpose
     * registers used by the G10 CPU during instruction execution.
     */
    struct register_file final
    {
        std::uint32_t       gp[16];         /** @brief General-purpose registers `D0` to `D15` */
        std::uint32_t       pc;             /** @brief Program Counter (`PC`) register */
        std::uint32_t       sp;             /** @brief Stack Pointer (`SP`) register */
        std::uint32_t       ie;             /** @brief Interrupt Enable (`IE`) register */
        std::uint32_t       irq;            /** @brief Interrupt Request (`IRQ`) register */
        flags_register      flags;          /** @brief Flags register */
        std::uint8_t        ec;             /** @brief Exception Code (`EC`) register */
    };
}

/* Public Classes *************************************************************/

namespace g10
{
    /**
     * @brief   Forward-declaration of the G10 CPU's system bus interface.
     */
    class bus;

    /**
     * @brief   Defines a class representing the G10 CPU context.
     * 
     * The G10 CPU context class encapsulates the state and behavior of the G10
     * CPU, including its registers, flags, instruction decoding and execution,
     * and interaction with the connected system bus.
     */
    class g10api cpu final
    {
    public:

        /**
         * @brief   Constructs a new G10 CPU context, connected to the specified
         *          system bus.
         * 
         * @param   bus     A reference to the system bus to which the CPU
         *                  will be connected.
         */
        explicit cpu (bus& bus);

        /**
         * @brief   The CPU context's destructor.
         */
        ~cpu () = default;

        /**
         * @brief   Resets the CPU context, setting all registers, flags, and
         *          internal states to their default, power-on values.
         * 
         * This method also resets the connected system bus.
         */
        auto reset () -> void;

        /**
         * @brief   Awakens the CPU from the `STOP` state, allowing it to resume
         *          normal operation.
         * 
         * This method is called in response to an external event, such as a
         * button press or a hardware reset. If the CPU is not currently in the
         * `STOP` state, or if it was placed into the `STOP` state due to a Double
         * Fault condition, this method has no effect.
         */
        auto wake () -> void;

        /**
         * @brief   Ticks the CPU context, stepping it through one frame of
         *          execution, causing it to possibly fetch, decode, and execute
         *          the next instruction, and/or service any pending interrupts.
         * 
         * @return  If the CPU ticked without errors, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto tick () -> bool;

        /**
         * @brief   Raises the specified exception, setting the appropriate
         *          exception code in the `EC` register and initiating the
         *          exception handling process.
         * 
         * Exceptions can be raised internally by the CPU, or externally by
         * virtual hardware components connected to the system bus.
         * 
         * @param   code        The exception code to raise.
         * 
         * @return  If `code` is `EC_OK`, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto raise_exception (exception_code code) -> bool;

        /**
         * @brief   Consumes the specified number of CPU clock cycles ("T-cycles"),
         *          advancing the CPU's clock and ticking the connected system bus's
         *          virtual hardware components accordingly.
         * 
         * @param   t_cycles    The number of T-cycles to consume.
         * 
         * @return  If all T-cycles were consumed without errors, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto consume_tick_cycles (std::uint32_t t_cycles) -> bool;

        /**
         * @brief   Consumes the specified number of virtual machine clock
         *          cycles ("M-cycles"), advancing the CPU's clock and ticking the
         *          connected system bus's virtual hardware components accordingly.
         * 
         * One M-cycle consists of four T-cycles.
         * 
         * @param   m_cycles    The number of M-cycles to consume.
         * 
         * @return  If all M-cycles were consumed without errors, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto consume_machine_cycles (std::uint32_t m_cycles) -> bool;

        /**
         * @brief   Retrieves a constant reference to the CPU's register file,
         *          containing all general-purpose and special-purpose registers.
         * 
         * @return  A constant reference to the CPU's register file.
         */
        inline auto get_register_file () const -> const register_file&
            { return m_regs; }

        /**
         * @brief   Checks whether the CPU is currently in the process of
         *          switching speed modes.
         *
         * @return  `true` if the CPU is switching speed modes; 
         *          Otherwise, `false`.
         */
        inline auto is_switching_speed () const -> bool
            { return m_speed_switching; }

        /**
         * @brief   Checks whether a speed switch is currently armed.
         * 
         * If a `STOP` instruction is executed while a speed switch is armed,
         * the CPU will initate the speed switch process instead of entering the
         * `STOP` state.
         *
         * @return  `true` if a speed switch is armed; 
         *          Otherwise, `false`.
         */
        inline auto is_speed_switch_armed () const -> bool
            { return m_speed_switch_reg.armed == 1; }

        /**
         * @brief   Checks whether the CPU is currently operating in double
         *          speed mode.
         * 
         * In double speed mode, the CPU operates at twice the normal clock
         * speed of ~4.19 MHz, effectively running at ~8.38 MHz.
         * 
         * @return  `true` if the CPU is in double speed mode; 
         *          Otherwise, `false`.
         */
        inline auto is_double_speed () const -> bool
            { return m_speed_switch_reg.double_speed == 1; }

        /**
         * @brief   Checks whether the CPU is currently stopped.
         *
         * The CPU may be stopped due to a normal stop condition or due to a
         * Double Fault condition.
         *
         * @return  `true` if the CPU is stopped; 
         *          Otherwise, `false`.
         */
        inline auto is_stopped () const -> bool
            { return m_stopped || m_double_fault; }

        /**
         * @brief   Checks whether the CPU is currently stopped due to a Double
         *          Fault condition.
         * 
         * @return  `true` if the CPU is stopped due to a Double Fault; 
         *          Otherwise, `false`.
         */
        inline auto is_double_faulted () const -> bool
            { return m_double_fault; }

        /**
         * @brief   Checks whether the CPU is currently halted.
         *
         * @return  `true` if the CPU is halted; 
         *          Otherwise, `false`.
         */
        inline auto is_halted () const -> bool
            { return m_halted; }

    public: /* Public Methods - Hardware Registers ****************************/

        // The G10 CPU offers a few hardware registers that can be accessed
        // from the system bus to which it is connected. These registers can be
        // memory-mapped to specific addresses within the virtual hardware's
        // address space, and are listed as follows:
        //
        // - `IRQ0` - Interrupt Request - Byte 0 (Read/Write)
        // - `IRQ1` - Interrupt Request - Byte 1 (Read/Write)
        // - `IRQ2` - Interrupt Request - Byte 2 (Read/Write)
        // - `IRQ3` - Interrupt Request - Byte 3 (Read/Write)
        // - `IE0`  - Interrupt Enable  - Byte 0 (Read/Write)
        // - `IE1`  - Interrupt Enable  - Byte 1 (Read/Write)
        // - `IE2`  - Interrupt Enable  - Byte 2 (Read/Write)
        // - `IE3`  - Interrupt Enable  - Byte 3 (Read/Write)
        // - `SPD`  - Speed Switch Register (Mixed)

        /**
         * @brief   Reads the value of the `IRQ0` hardware register, which
         *          contains the low byte of the CPU's 32-bit `IRQ` register.
         * 
         * @return  The value of the `IRQ0` hardware register.
         */
        auto read_irq0 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `IRQ1` hardware register, which
         *          contains bits 8-15 of the CPU's 32-bit `IRQ` register.
         * 
         * @return  The value of the `IRQ1` hardware register.
         */
        auto read_irq1 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `IRQ2` hardware register, which
         *          contains bits 16-23 of the CPU's 32-bit `IRQ` register.
         * 
         * @return  The value of the `IRQ2` hardware register.
         */
        auto read_irq2 () const -> std::uint8_t;
        
        /**
         * @brief   Reads the value of the `IRQ3` hardware register, which
         *          contains bits 24-31 of the CPU's 32-bit `IRQ` register.
         * 
         * @return  The value of the `IRQ3` hardware register.
         */
        auto read_irq3 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `IE0` hardware register, which
         *          contains the low byte of the CPU's 32-bit `IE` register.
         * 
         * @return  The value of the `IE0` hardware register.
         */
        auto read_ie0 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `IE1` hardware register, which
         *          contains bits 8-15 of the CPU's 32-bit `IE` register.
         * 
         * @return  The value of the `IE1` hardware register.
         */
        auto read_ie1 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `IE2` hardware register, which
         *          contains bits 16-23 of the CPU's 32-bit `IE` register.
         * 
         * @return  The value of the `IE2` hardware register.
         */
        auto read_ie2 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `IE3` hardware register, which
         *          contains bits 24-31 of the CPU's 32-bit `IE` register.
         * 
         * @return  The value of the `IE3` hardware register.
         */
        auto read_ie3 () const -> std::uint8_t;

        /**
         * @brief   Reads the value of the `SPD` hardware register, which
         *          contains the CPU's speed switch register.
         * 
         * @return  The value of the `SPD` hardware register.
         */
        auto read_spd () const -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IRQ0` hardware register,
         *          which contains the low byte of the CPU's 32-bit `IRQ` register.
         * 
         * @param   value   The value to write to the `IRQ0` hardware register.
         * 
         * @return  The value written to the `IRQ0` hardware register.
         */
        auto write_irq0 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IRQ1` hardware register,
         *          which contains bits 8-15 of the CPU's 32-bit `IRQ` register.
         * 
         * @param   value   The value to write to the `IRQ1` hardware register.
         * 
         * @return  The value written to the `IRQ1` hardware register.
         */
        auto write_irq1 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IRQ2` hardware register,
         *          which contains bits 16-23 of the CPU's 32-bit `IRQ` register.
         * 
         * @param   value   The value to write to the `IRQ2` hardware register.
         * 
         * @return  The value written to the `IRQ2` hardware register.
         */
        auto write_irq2 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IRQ3` hardware register,
         *          which contains bits 24-31 of the CPU's 32-bit `IRQ` register.
         * 
         * @param   value   The value to write to the `IRQ3` hardware register.
         * 
         * @return  The value written to the `IRQ3` hardware register.
         */
        auto write_irq3 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IE0` hardware register,
         *          which contains the low byte of the CPU's 32-bit `IE` register.
         * 
         * @param   value   The value to write to the `IE0` hardware register.
         * 
         * @return  The value written to the `IE0` hardware register.
         */
        auto write_ie0 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IE1` hardware register,
         *          which contains bits 8-15 of the CPU's 32-bit `IE` register.
         * 
         * @param   value   The value to write to the `IE1` hardware register.
         * 
         * @return  The value written to the `IE1` hardware register.
         */
        auto write_ie1 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IE2` hardware register,
         *          which contains bits 16-23 of the CPU's 32-bit `IE` register.
         * 
         * @param   value   The value to write to the `IE2` hardware register.
         * 
         * @return  The value written to the `IE2` hardware register.
         */
        auto write_ie2 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `IE3` hardware register,
         *          which contains bits 24-31 of the CPU's 32-bit `IE` register.
         * 
         * @param   value   The value to write to the `IE3` hardware register.
         * 
         * @return  The value written to the `IE3` hardware register.
         */
        auto write_ie3 (std::uint8_t value) -> std::uint8_t;

        /**
         * @brief   Writes the specified value to the `SPD` hardware register,
         *          which contains the CPU's speed switch register.
         * 
         * @param   value   The value to write to the `SPD` hardware register.
         * 
         * @return  The value written to the `SPD` hardware register.
         */
        auto write_spd (std::uint8_t value) -> std::uint8_t;

    private: /* Private Methods - Register and Flag Access ********************/

        /**
         * @brief   Reads the value of the specified register from the CPU's
         *          register file.
         * 
         * @param   reg     The register to read.
         * 
         * @return  The value of the specified register.
         */
        auto read_register (register_type reg) const -> std::uint32_t;

        /**
         * @brief   Writes the specified value to the specified register in
         *          the CPU's register file.
         * 
         * @param   reg     The register to write to.
         * @param   value   The value to write to the register.
         */
        auto write_register (register_type reg, std::uint32_t value) -> void;

    private: /* Private Methods - Internal Bus Access *************************/

        /**
         * @brief   Fetches the next opcode (two bytes) from the instruction
         *          stream, consuming one M-cycle for each read operation.
         * 
         * @return  If the memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto fetch_opcode () -> bool;

        /**
         * @brief   Reads one immediate byte from the instruction stream,
         *          consuming one M-cycle.
         * 
         * @return  If the memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto fetch_imm8 () -> bool;

        /**
         * @brief   Reads one immediate word (two bytes) from the instruction
         *          stream, consuming one M-cycle for each read operation.
         * 
         * @return  If each memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto fetch_imm16 () -> bool;

        /**
         * @brief   Reads one immediate double word (four bytes) from the
         *          instruction stream, consuming one M-cycle for each read
         *          operation.
         * 
         * @return  If each memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto fetch_imm32 () -> bool;
        
        /**
         * @brief   Reads one byte from the specified memory address on the
         *          system bus, consuming one M-cycle.
         * 
         * @param   address     The memory address from which to read the byte.
         * @param   out_value   A reference to a variable in which to store
         *                      the read byte.
         * 
         * @return  If the memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto read_byte (std::uint32_t address, std::uint8_t& out_value) -> bool;

        /**
         * @brief   Reads one word (two bytes) from the specified memory
         *          address on the system bus, consuming one M-cycle for each
         *          read operation.
         * 
         * @param   address     The memory address from which to read the word.
         * @param   out_value   A reference to a variable in which to store
         *                      the read word.
         * 
         * @return  If each memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto read_word (std::uint32_t address, std::uint16_t& out_value) -> bool;

        /**
         * @brief   Reads one double word (four bytes) from the specified
         *          memory address on the system bus, consuming one M-cycle for
         *          each read operation.
         * 
         * @param   address     The memory address from which to read the
         *                      double word.
         * @param   out_value   A reference to a variable in which to store
         *                      the read double word.
         * 
         * @return  If each memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto read_dword (std::uint32_t address, std::uint32_t& out_value) -> bool;

        /**
         * @brief   Writes one byte to the specified memory address on the
         *          system bus, consuming one M-cycle.
         * 
         * @param   address     The memory address to which to write the byte.
         * @param   value       The byte value to write.
         * 
         * @return  If the memory write and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto write_byte (std::uint32_t address, std::uint8_t value) -> bool;

        /**
         * @brief   Writes one word (two bytes) to the specified memory
         *          address on the system bus, consuming one M-cycle for each
         *          write operation.
         * 
         * @param   address     The memory address to which to write the word.
         * @param   value       The word value to write.
         * 
         * @return  If each memory write and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto write_word (std::uint32_t address, std::uint16_t value) -> bool;

        /**
         * @brief   Writes one double word (four bytes) to the specified
         *          memory address on the system bus, consuming one M-cycle for
         *          each write operation.
         * 
         * @param   address     The memory address to which to write the
         *                      double word.
         * @param   value       The double word value to write.
         * 
         * @return  If each memory write and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto write_dword (std::uint32_t address, std::uint32_t value) -> bool;

        /**
         * @brief   Pops one double word (four bytes) from the CPU's stack,
         *          consuming one M-cycle for each read operation.
         * 
         * @param   value       A reference to a variable in which to store
         *                      the popped double word.
         * 
         * @return  If each memory read and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto pop_dword (std::uint32_t& value) -> bool;

        /**
         * @brief   Pushes one double word (four bytes) onto the CPU's stack,
         *          consuming one M-cycle for each write operation.
         * 
         * @param   value       The double word to push onto the stack.
         * 
         * @return  If each memory write and cycle consumption succeeded, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        auto push_dword (std::uint32_t value) -> bool;
        
    private: /* Private Methods - Interrupts **********************************/

        /**
         * @brief   Checks whether any enabled interrupts are currently pending.
         * 
         * @return  `true` if any enabled interrupts are pending;
         *          Otherwise, `false`.
         */
        auto is_any_interrupt_pending () const -> bool;

        /**
         * @brief   Disables all CPU interrupts by immediately clearing the
         *          `IME` flag.
         * 
         * This method is called in response to the `DI` instruction. It also
         * clears the `IMP` flag to prevent interrupts from being re-enabled
         * after the next instruction.
         */
        auto disable_interrupts () -> void;

        /**
         * @brief   Enables CPU interrupts, either immediately or after the
         *          next instruction, by setting the `IME` and/or `IMP` flags
         *          accordingly.
         * 
         * This method is called in response to the `EI`, `EII` and `RETI`
         * instructions. `EI` sets the `IMP` flag which causes interrupts to be
         * enabled after the next instruction, while `EII` and `RETI` set the
         * `IME` flag immediately.
         * 
         * @param   immediately     If `true`, sets the `IME` flag immediately,
         *                          enabling interrupts right away;
         *                          If `false`, sets the `IMP` flag, causing
         *                          interrupts to be enabled after the next
         *                          instruction is executed.
         */
        auto enable_interrupts (bool immediately) -> void;

        /**
         * @brief   Calls the specified interrupt vector, pushing the current
         *          `PC` and `FLAGS` onto the stack and jumping to the interrupt
         *          handler address.
         * 
         * @param   vector      The interrupt vector to call.
         * 
         * @return  If the interrupt was successfully called, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto call_interrupt (std::uint8_t vector) -> bool;

        /**
         * @brief   Services the highest-priority pending interrupt, if any.
         * 
         * @return  If an interrupt was successfully serviced, no interrupts
         *          were pending, if `IME` was disabled, or if `IMP` was set,
         *          returns `true`;
         *          Otherwise, returns `false`.
         */
        auto service_interrupt () -> bool;

    private: /* Private Methods - Halt and Stop *******************************/

        /**
         * @brief   Places the CPU into a low-power `HALT` state, in which it
         *          ceases instruction execution until an enabled interrupt is
         *          pending.
         * 
         * This method is called in response to the `HALT` instruction.
         */
        auto enter_halt_state () -> void;

        /**
         * @brief   Places the CPU into an ultra-low-power `STOP` state, during
         *          which its internal clock is disabled entirely, and no further
         *          activity occurs until it is awakened by an external event,
         *          such as a button press or a hardware reset.
         * 
         * This method is called in response to the `STOP` instruction. If a 
         * `STOP` state is attempted while a speed switch is armed, the speed
         * switch process will be initiated instead.
         */
        auto enter_stop_state () -> void;

        /**
         * @brief   Exits the CPU from the `HALT` state, resuming normal
         *          instruction execution.
         */
        auto exit_halt_state () -> void;

    private: /* Private Methods - CPU Control Instructions ********************/

        /**
         * @brief   Executes a `NOP` instruction, which performs no operation
         *          and simply consumes the appropriate number of M-cycles.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0000 NOP`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     1 M-cycle
         * @note    Flags:      None
         */
        auto nop () -> bool;

        /**
         * @brief   Executes a `STOP` instruction, placing the CPU into the
         *          `STOP` state.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0100 STOP`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles (Minimum; actual timing will vary)
         * @note    Flags:      None
         */
        auto stop () -> bool;

        /**
         * @brief   Executes a `HALT` instruction, placing the CPU into the
         *          `HALT` state.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0200 HALT`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles (Minimum; actual timing will vary)
         * @note    Flags:      None
         */
        auto halt () -> bool;

        /**
         * @brief   Executes a `DI` instruction, disabling all CPU interrupts
         *          immediately.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0300 DI`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto di () -> bool;

        /**
         * @brief   Executes an `EI` instruction, enabling CPU interrupts
         *          after the next instruction is executed.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0400 EI`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto ei () -> bool;

        /**
         * @brief   Executes an `EII` instruction, enabling CPU interrupts
         *          immediately.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0500 EII`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto eii () -> bool; 

        /**
         * @brief   Executes a `DAA` instruction, which decimal-adjusts the
         *          byte accumulator register `L0` for binary-coded decimal (BCD)
         *          representation.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0600 DAA`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `H` - Clear;
         *                      `C` - Set or cleared according to operation
         */
        auto daa () -> bool;

        /**
         * @brief   Executes an `SCF` instruction, which sets the CPU's carry
         *          flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0700 SCF`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set
         */
        auto scf () -> bool;

        /**
         * @brief   Executes a `CCF` instruction, which complements (toggles)
         *          the CPU's carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0800 CCF`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set if cleared; Cleared if set
         */
        auto ccf () -> bool;

        /**
         * @brief   Executes a `CLV` instruction, which clears the CPU's
         *          overflow flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0900 CLV`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `V` - Cleared
         */
        auto clv () -> bool;

        /**
         * @brief   Executes a `SEV` instruction, which sets the CPU's
         *          overflow flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x0A00 SEV`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `V` - Set
         */
        auto sev () -> bool;

    private: /* Private Methods - 8-Bit Load/Store/Move Instructions **********/

        /**
         * @brief   Executes an `LD LX, IMM8` instruction, which loads an
         *          immediate 8-bit value into the specified low byte register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x10X0 LD LX, IMM8`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto ld_lx_imm8 () -> bool;

        /**
         * @brief   Executes an `LD LX, [ADDR32]` instruction, which loads an
         *          8-bit value from the specified 32-bit memory address into
         *          the specified low byte register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x11X0 LD LX, [ADDR32]`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     7 M-cycles
         * @note    Flags:      None
         */
        auto ld_lx_addr32 () -> bool;

        /**
         * @brief   Executes an `LD LX, [DY]` instruction, which loads an
         *          8-bit value from the memory address pointed to by the
         *          `DY` register into the specified low byte register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x12XY LD LX, [DY]`
         * @note    Parameters: `X` - Destination low byte register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto ld_lx_pdy () -> bool;

        /**
         * @brief   Executes an `LDQ LX, [ADDR16]` instruction, which loads an
         *          8-bit value from the specified 16-bit relative memory
         *          address into the specified low byte register. The address
         *          read from is relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x13X0 LD LX, [ADDR16]`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + 16-bit Address)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      None
         */
        auto ldq_lx_addr16 () -> bool;

        /**
         * @brief   Executes an `LDQ LX, [WY]` instruction, which loads an
         *          8-bit value from the relative memory address pointed to by
         *          the `WY` word register into the specified low byte
         *          register. The address read from is relative to absolute
         *          address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x14XY LD LX, [WY]`
         * @note    Parameters: `X` - Destination low byte register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto ldq_lx_pwy () -> bool;

        /**
         * @brief   Executes an `LDP LX, [ADDR8]` instruction, which loads an
         *          8-bit value from the specified 8-bit relative memory
         *          address into the specified low byte register. The address
         *          read from is relative to absolute address `$FFFFFF00`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x15X0 LD LX, [ADDR8]`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     3 Bytes (Opcode + 8-bit Address)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto ldp_lx_addr8 () -> bool;

        /**
         * @brief   Executes an `LDP LX, [LY]` instruction, which loads an
         *          8-bit value from the relative memory address pointed to by
         *          the `LY` low byte register into the specified low byte
         *          register. The address read from is relative to absolute
         *          address `$FFFFFF00`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x16XY LD LX, [LY]`
         * @note    Parameters: `X` - Destination low byte register index (0 - 15)
         *                      `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto ldp_lx_ply () -> bool;

        /**
         * @brief   Executes an `ST [ADDR32], LY` instruction, which stores
         *          the value of the low byte register `LY` into the specified
         *          32-bit memory address.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x170Y ST [ADDR32], LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     7 M-cycles
         * @note    Flags:      None
         */
        auto st_addr32_ly () -> bool;

        /**
         * @brief   Executes an `ST [DX], LY` instruction, which stores the
         *          value of the low byte register `LY` into the memory address
         *          pointed to by the `DX` register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x18XY ST [DX], LY`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto st_pdx_ly () -> bool;

        /**
         * @brief   Executes an `STQ [ADDR16], LY` instruction, which stores
         *          the value of the low byte register `LY` into the specified
         *          16-bit relative memory address. The address written to is
         *          relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x190Y ST [ADDR16], LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + 16-bit Address)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      None
         */
        auto stq_addr16_ly () -> bool;

        /**
         * @brief   Executes an `STQ [WX], LY` instruction, which stores the
         *          value of the low byte register `LY` into the relative
         *          memory address pointed to by the `WX` word register. The
         *          address written to is relative to absolute address
         *          `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x1AXY ST [WX], LY`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto stq_pwx_ly () -> bool;

        /**
         * @brief   Executes an `STP [ADDR8], LY` instruction, which stores
         *          the value of the low byte register `LY` into the specified
         *          8-bit relative memory address. The address written to is
         *          relative to absolute address `$FFFFFF00`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x1B0Y STP [ADDR8], LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     3 Bytes (Opcode + 8-bit Address)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto stp_addr8_ly () -> bool;

        /**
         * @brief   Executes an `STP [LX], LY` instruction, which stores the
         *          value of the low byte register `LY` into the relative
         *          memory address pointed to by the `LX` low byte register. The
         *          address written to is relative to absolute address
         *          `$FFFFFF00`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x1CXY STP [LX], LY`
         * @note    Parameters: `X` - Destination low byte register index (0 - 15)
         *                      `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto stp_plx_ly () -> bool;

        /**
         * @brief   Executes an `MV LX, LY` instruction, which moves the value
         *          from the source low byte register `LY` to the destination
         *          low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x1DXY MV LX, LY`
         * @note    Parameters: `X` - Destination low byte register index (0 - 15)
         *                      `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mv_lx_ly () -> bool;

        /**
         * @brief   Executes an `MV HX, LY` instruction, which moves the value
         *          from the source low byte register `LY` to the destination
         *          high byte register `HX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x1EXY MV HX, LY`
         * @note    Parameters: `X` - Destination high byte register index (0 - 15)
         *                      `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mv_hx_ly () -> bool;

        /**
         * @brief   Executes an `MV LX, HY` instruction, which moves the value
         *          from the source high byte register `HY` to the destination
         *          low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x1FXY MV LX, HY`
         * @note    Parameters: `X` - Destination low byte register index (0 - 15)
         *                      `Y` - Source high byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mv_lx_hy () -> bool;

    private: /* Private Methods - 16-Bit Load/Store/Move Instructions *********/

        /**
         * @brief   Executes an `LD WX, IMM16` instruction, which loads an
         *          immediate 16-bit value into the specified word register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x20X0 LD WX, IMM16`
         * @note    Parameters: `X` - Word register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + Immediate Word)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto ld_wx_imm16 () -> bool;

        /**
         * @brief   Executes an `LD WX, [ADDR32]` instruction, which loads a
         *          16-bit value from the specified 32-bit memory address into
         *          the specified word register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x21X0 LD WX, [ADDR32]`
         * @note    Parameters: `X` - Word register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     8 M-cycles
         * @note    Flags:      None
         */
        auto ld_wx_addr32 () -> bool;

        /**
         * @brief   Executes an `LD WX, [DY]` instruction, which loads a
         *          16-bit value from the memory address pointed to by the
         *          `DY` register into the specified word register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x22XY LD WX, [DY]`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto ld_wx_pdy () -> bool;

        /**
         * @brief   Executes an `LDQ WX, [ADDR16]` instruction, which loads a
         *          16-bit value from the specified 16-bit relative memory
         *          address into the specified word register. The address read
         *          from is relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x23X0 LD WX, [ADDR16]`
         * @note    Parameters: `X` - Word register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + 16-bit Address)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto ldq_wx_addr16 () -> bool;

        /**
         * @brief   Executes an `LDQ WX, [WY]` instruction, which loads a
         *          16-bit value from the relative memory address pointed to by
         *          the `WY` word register into the specified word register.
         *          The address read from is relative to absolute address
         *          `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x24XY LD WX, [WY]`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto ldq_wx_pwy () -> bool;

        /**
         * @brief   Executes an `ST [ADDR32], WY` instruction, which stores
         *          the value of the word register `WY` into the specified
         *          32-bit memory address.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x270Y ST [ADDR32], WY`
         * @note    Parameters: `Y` - Source word register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     8 M-cycles
         * @note    Flags:      None
         */
        auto st_addr32_wy () -> bool;

        /**
         * @brief   Executes an `ST [DX], WY` instruction, which stores the
         *          value of the word register `WY` into the memory address
         *          pointed to by the `DX` register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x28XY ST [DX], WY`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto st_pdx_wy () -> bool;

        /**
         * @brief   Executes an `STQ [ADDR16], WY` instruction, which stores
         *          the value of the word register `WY` into the specified
         *          16-bit relative memory address. The address written to is
         *          relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x290Y ST [ADDR16], WY`
         * @note    Parameters: `Y` - Source word register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + 16-bit Address)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto stq_addr16_wy () -> bool;

        /**
         * @brief   Executes an `STQ [WX], WY` instruction, which stores the
         *          value of the word register `WY` into the relative memory
         *          address pointed to by the `WX` word register. The address
         *          written to is relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x2AXY ST [WX], WY`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None
         */
        auto stq_pwx_wy () -> bool;

        /**
         * @brief   Executes an `MV WX, WY` instruction, which moves the value
         *          from the source word register `WY` to the destination word
         *          register `WX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x2DXY MV WX, WY`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mv_wx_wy () -> bool;

        /**
         * @brief   Executes an `MWH DX, WY` instruction, which moves the value
         *          from the source word register `WY` into the upper 16 bits
         *          of the full register `DX`, leaving the lower 16 bits of `DX`
         *          unchanged. `MWH` stands for "Move Word High".
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x2EXY MWH DX, WY`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mwh_dx_wy () -> bool;

        /**
         * @brief   Executes an `MWL WX, DY` instruction, which moves the upper
         *          16 bits of the full register `DY` into the word register
         *          `WX`, leaving the lower 16 bits of `DY` unchanged. `MWL`
         *          stands for "Move Word Low".
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x2FXY MWL WX, DY`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mwl_wx_dy () -> bool;

    private: /* Private Methods - 32-Bit Load/Store/Move Instructions *********/

        /**
         * @brief   Executes an `LD DX, IMM32` instruction, which loads an
         *          immediate 32-bit value into the specified full register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x30X0 LD DX, IMM32`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + Immediate Double Word)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto ld_dx_imm32 () -> bool;

        /**
         * @brief   Executes an `LD DX, [ADDR32]` instruction, which loads a
         *          32-bit value from the specified 32-bit memory address into
         *          the specified full register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x31X0 LD DX, [ADDR32]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     10 M-cycles
         * @note    Flags:      None
         */
        auto ld_dx_addr32 () -> bool;

        /**
         * @brief   Executes an `LD DX, [DY]` instruction, which loads a
         *          32-bit value from the memory address pointed to by the
         *          `DY` register into the specified full register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x32XY LD DX, [DY]`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto ld_dx_pdy () -> bool;

        /**
         * @brief   Executes an `LDQ DX, [ADDR16]` instruction, which loads a
         *          32-bit value from the specified 16-bit relative memory
         *          address into the specified full register. The address read
         *          from is relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x33X0 LD DX, [ADDR16]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + 16-bit Address)
         * @note    Timing:     8 M-cycles
         * @note    Flags:      None
         */
        auto ldq_dx_addr16 () -> bool;

        /**
         * @brief   Executes an `LDQ DX, [WY]` instruction, which loads a
         *          32-bit value from the relative memory address pointed to by
         *          the `WY` word register into the specified full register.
         *          The address read from is relative to absolute address
         *          `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x34XY LD DX, [WY]`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto ldq_dx_pwy () -> bool;

        /**
         * @brief   Executes an `LSP DX, IMM32` instruction, which loads a
         *          32-bit immediate value into the CPU's stack pointer
         *          register `SP`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         *
         * @note    Opcodes:    `0x3500 LSP IMM32`
         * @note    Length:     6 Bytes (Opcode + Immediate Double Word)
         * @note    Timing:     7 M-cycles
         * @note    Flags:      None
         */
        auto lsp_imm32 () -> bool;

        /**
         * @brief   Executes a `POP DX` instruction, which pops a 32-bit value
         *          from the stack and loads it into the specified full register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x36X0 POP DX`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     7 M-cycles
         * @note    Flags:      None
         */
        auto pop_dx () -> bool;

        /**
         * @brief   Executes an `ST [ADDR32], DY` instruction, which stores
         *          the value of the full register `DY` into the specified
         *          32-bit memory address.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x370Y ST [ADDR32], DY`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     10 M-cycles
         * @note    Flags:      None
         */
        auto st_addr32_dy () -> bool;

        /**
         * @brief   Executes an `ST [DX], DY` instruction, which stores the
         *          value of the full register `DY` into the memory address
         *          pointed to by the `DX` register.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x38XY ST [DX], DY`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto st_pdx_dy () -> bool;

        /**
         * @brief   Executes an `STQ [ADDR16], DY` instruction, which stores
         *          the value of the full register `DY` into the specified
         *          16-bit relative memory address. The address written to is
         *          relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x390Y ST [ADDR16], DY`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     4 Bytes (Opcode + 16-bit Address)
         * @note    Timing:     8 M-cycles
         * @note    Flags:      None
         */
        auto stq_addr16_dy () -> bool;

        /**
         * @brief   Executes an `STQ [WX], DY` instruction, which stores the
         *          value of the full register `DY` into the relative memory
         *          address pointed to by the `WX` word register. The address
         *          written to is relative to absolute address `$FFFF0000`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x3AXY ST [WX], DY`
         * @note    Parameters: `X` - Destination word register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     6 M-cycles
         * @note    Flags:      None
         */
        auto stq_pwx_dy () -> bool;

        /**
         * @brief   Executes an `SSP [ADDR32]` instruction, which stores the
         *          value of the CPU's stack pointer register `SP` into the
         *          specified 32-bit memory address.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x3B00 SSP [ADDR32]`
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     7 M-cycles
         * @note    Flags:      None
         */
        auto ssp_addr32 () -> bool;

        /**
         * @brief   Executes a `PUSH DY` instruction, which pushes the value
         *          of the specified full register onto the stack.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x3CX0 PUSH DY`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     7 M-cycles
         * @note    Flags:      None
         */
        auto push_dy () -> bool;

        /**
         * @brief   Executes an `MV DX, DY` instruction, which moves the value
         *          from the source full register `DY` to the destination full
         *          register `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x3DXY MV DX, DY`
         * @note    Parameters: `X` - Destination full register index (0 - 15)
         *                      `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto mv_dx_dy () -> bool;

        /**
         * @brief   Executes an `SPO DX` instruction, which moves the value of
         *          the CPU's stack pointer register `SP` into the specified
         *          full register `DX`. `SPO` stands for "Stack Pointer Out".
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x3EX0 SPO DX`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None
         */
        auto spo_dx () -> bool;

        /**
         * @brief   Executes an `SPI DY` instruction, which moves the value of
         *          the specified full register `DY` into the CPU's stack
         *          pointer register `SP`. `SPI` stands for "Stack Pointer In".
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x3F0Y SPI DY`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      None
         */
        auto spi_dy () -> bool;

    private: /* Private Methods - Branching Instructions **********************/

        /**
         * @brief   Executes a `JMP X, IMM32` instruction, which moves the
         *          program counter register to the specified 32-bit immediate
         *          value if the condition `X` is met. `X` is one of the enumerated
         *          values in @a `g10::condition_code`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x40X0 JMP X, IMM32`
         * @note    Parameters: `X` - Condition code (0 - 6)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     6 M-Cycles if jump not taken;
         *                      7 M-Cycles if jump taken
         * @note    Flags:      None
         */
        auto jmp_x_imm32 () -> bool;

        /**
         * @brief   Executes a `JMP X, DY` instruction, which moves the
         *          program counter register to the address contained in the
         *          full register `DY` if the condition `X` is met. `X` is one
         *          of the enumerated values in @a `g10::condition_code`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x41XY JMP X, DY`
         * @note    Parameters: `X` - Condition code (0 - 6)
         *                      `Y` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-Cycles if jump not taken;
         *                      3 M-Cycles if jump taken
         * @note    Flags:      None
         */
        auto jmp_x_dy () -> bool;

        /**
         * @brief   Executes a `JPB X, SIMM16` instruction, which moves the
         *          program counter register by the signed immediate 16-bit
         *          offset if the condition `X` is met. `X` is one of the
         *          enumerated values in @a `g10::condition_code`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x42X0 JPB X, SIMM16`
         * @note    Parameters: `X` - Condition code (0 - 6)
         * @note    Length:     4 Bytes (Opcode + Signed Immediate Word)
         * @note    Timing:     4 M-Cycles if jump not taken;
         *                      5 M-Cycles if jump taken
         * @note    Flags:      None
         */
        auto jpb_x_simm16 () -> bool;

        /**
         * @brief   Executes a `CALL X, IMM32` instruction, which pushes the
         *          current program counter onto the stack and then moves the
         *          program counter register to the specified 32-bit immediate
         *          value if the condition `X` is met. `X` is one of the
         *          enumerated values in @a `g10::condition_code`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x43X0 CALL X, IMM32`
         * @note    Parameters: `X` - Condition code (0 - 6)
         * @note    Length:     6 Bytes (Opcode + 32-bit Address)
         * @note    Timing:     6 M-Cycles if call not taken;
         *                      12 M-Cycles if call taken
         * @note    Flags:      None
         */
        auto call_x_imm32 () -> bool;

        /**
         * @brief   Executes an `INT XX` instruction, which explicitly and
         *          immediately calls the interrupt vector specified by the
         *          8-bit immediate value `XX`.
         * 
         * This instruction can be used to call one of the G10's 32 interrupt
         * vector subroutines directly, as if it were a regular subroutine call.
         * When an interrupt vector is called via this instruction, the CPU's
         * interrupt-related registers and state (`IME`, `IMP`, `IE`, `IRQ`, etc.)
         * are not affected; the interrupt vector is simply called like any
         * other subroutine.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x44XX INT XX`
         * @note    Parameters: `XX` - Interrupt vector index (0 - 31)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     8 M-cycles
         * @note    Flags:      None
         */
        auto int_xx () -> bool;

        /**
         * @brief   Executes a `RET X` instruction, which pops the top value
         *          from the stack and loads it into the program counter
         *          register if the condition `X` is met. `X` is one of the
         *          enumerated values in @a `g10::condition_code`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x45X0 RET X`
         * @note    Parameters: `X` - Condition code (0 - 6)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles if return not taken;
         *                      9 M-cycles if conditional return taken;
         *                      8 M-cycles if unconditional return taken
         * @note    Flags:      None
         */
        auto ret_x () -> bool;

        /**
         * @brief   Executes a `RETI` instruction, which pops the top value
         *          from the stack and loads it into the program counter
         *          register, then immediately enables interrupts by setting
         *          the CPU's `IME` flag to `true`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x4600 RETI`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     8 M-cycles
         * @note    Flags:      None
         */
        auto reti () -> bool;

    private: /* Private Methods - 8-Bit Arithmetic Instructions **************/

        /**
         * @brief   Executes an `ADD L0, IMM8` instruction, which adds an
         *          immediate 8-bit value to the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5000 ADD L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Set if carry from bit 7;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_l0_imm8 () -> bool;

        /**
         * @brief   Executes an `ADD L0, LY` instruction, which adds the value
         *          of the low byte register `LY` to the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x510Y ADD L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Set if carry from bit 7;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_l0_ly () -> bool;

        /**
         * @brief   Executes an `ADD L0, [DY]` instruction, which adds the value
         *          at the memory address pointed to by `DY` to the accumulator
         *          register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x520Y ADD L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Set if carry from bit 7;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_l0_pdy () -> bool;

        /**
         * @brief   Executes an `ADC L0, IMM8` instruction, which adds an
         *          immediate 8-bit value and the carry flag to the accumulator
         *          register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5300 ADC L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Set if carry from bit 7;
         *                      `V` - Set if signed overflow occurred
         */
        auto adc_l0_imm8 () -> bool;

        /**
         * @brief   Executes an `ADC L0, LY` instruction, which adds the value
         *          of the low byte register `LY` and the carry flag to the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x540Y ADC L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Set if carry from bit 7;
         *                      `V` - Set if signed overflow occurred
         */
        auto adc_l0_ly () -> bool;

        /**
         * @brief   Executes an `ADC L0, [DY]` instruction, which adds the value
         *          at the memory address pointed to by `DY` and the carry flag
         *          to the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x550Y ADC L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Set if carry from bit 7;
         *                      `V` - Set if signed overflow occurred
         */
        auto adc_l0_pdy () -> bool;

        /**
         * @brief   Executes a `SUB L0, IMM8` instruction, which subtracts an
         *          immediate 8-bit value from the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5600 SUB L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if borrow from bit 8;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_l0_imm8 () -> bool;

        /**
         * @brief   Executes a `SUB L0, LY` instruction, which subtracts the value
         *          of the low byte register `LY` from the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x570Y SUB L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if borrow from bit 8;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_l0_ly () -> bool;

        /**
         * @brief   Executes a `SUB L0, [DY]` instruction, which subtracts the
         *          value at the memory address pointed to by `DY` from the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x580Y SUB L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if borrow from bit 8;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_l0_pdy () -> bool;

        /**
         * @brief   Executes a `SBC L0, IMM8` instruction, which subtracts an
         *          immediate 8-bit value and the carry flag from the accumulator
         *          register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5900 SBC L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if borrow from bit 8;
         *                      `V` - Set if signed overflow occurred
         */
        auto sbc_l0_imm8 () -> bool;

        /**
         * @brief   Executes a `SBC L0, LY` instruction, which subtracts the value
         *          of the low byte register `LY` and the carry flag from the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5A0Y SBC L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if borrow from bit 8;
         *                      `V` - Set if signed overflow occurred
         */
        auto sbc_l0_ly () -> bool;

        /**
         * @brief   Executes a `SBC L0, [DY]` instruction, which subtracts the
         *          value at the memory address pointed to by `DY` and the carry
         *          flag from the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5B0Y SBC L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if borrow from bit 8;
         *                      `V` - Set if signed overflow occurred
         */
        auto sbc_l0_pdy () -> bool;

        /**
         * @brief   Executes an `INC LX` instruction, which increments the value
         *          of the low byte register `LX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5CX0 INC LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Unchanged;
         *                      `V` - Set if signed overflow occurred
         */
        auto inc_lx () -> bool;

        /**
         * @brief   Executes an `INC [DX]` instruction, which increments the value
         *          at the memory address pointed to by `DX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5DX0 INC [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 3;
         *                      `C` - Unchanged;
         *                      `V` - Set if signed overflow occurred
         */
        auto inc_pdx () -> bool;

        /**
         * @brief   Executes a `DEC LX` instruction, which decrements the value
         *          of the low byte register `LX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5EX0 DEC LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Unchanged;
         *                      `V` - Set if signed overflow occurred
         */
        auto dec_lx () -> bool;

        /**
         * @brief   Executes a `DEC [DX]` instruction, which decrements the value
         *          at the memory address pointed to by `DX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x5FX0 DEC [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Unchanged;
         *                      `V` - Set if signed overflow occurred
         */
        auto dec_pdx () -> bool;

    private: /* Private Methods - 16-Bit and 32-Bit Arithmetic Instructions **/

        /**
         * @brief   Executes an `ADD W0, IMM16` instruction, which adds an
         *          immediate 16-bit value to register `W0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6000 ADD W0, IMM16`
         * @note    Length:     4 Bytes (Opcode + Immediate Word)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 11;
         *                      `C` - Set if carry from bit 15;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_w0_imm16 () -> bool;

        /**
         * @brief   Executes an `ADD W0, WY` instruction, which adds the value
         *          of the word register `WY` to register `W0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x610Y ADD W0, WY`
         * @note    Parameters: `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 11;
         *                      `C` - Set if carry from bit 15;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_w0_wy () -> bool;

        /**
         * @brief   Executes an `ADD D0, IMM32` instruction, which adds an
         *          immediate 32-bit value to register `D0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6200 ADD D0, IMM32`
         * @note    Length:     6 Bytes (Opcode + Immediate Double Word)
         * @note    Timing:     9 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 27;
         *                      `C` - Set if carry from bit 31;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_d0_imm32 () -> bool;

        /**
         * @brief   Executes an `ADD D0, DY` instruction, which adds the value
         *          of the full register `DY` to register `D0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x630Y ADD D0, DY`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 27;
         *                      `C` - Set if carry from bit 31;
         *                      `V` - Set if signed overflow occurred
         */
        auto add_d0_dy () -> bool;

        /**
         * @brief   Executes a `SUB W0, IMM16` instruction, which subtracts an
         *          immediate 16-bit value from register `W0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6400 SUB W0, IMM16`
         * @note    Length:     4 Bytes (Opcode + Immediate Word)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 12;
         *                      `C` - Set if borrow from bit 16;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_w0_imm16 () -> bool;

        /**
         * @brief   Executes a `SUB W0, WY` instruction, which subtracts the value
         *          of the word register `WY` from register `W0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x650Y SUB W0, WY`
         * @note    Parameters: `Y` - Source word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 12;
         *                      `C` - Set if borrow from bit 16;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_w0_wy () -> bool;

        /**
         * @brief   Executes a `SUB D0, IMM32` instruction, which subtracts an
         *          immediate 32-bit value from register `D0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6600 SUB D0, IMM32`
         * @note    Length:     6 Bytes (Opcode + Immediate Double Word)
         * @note    Timing:     9 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 28;
         *                      `C` - Set if borrow from bit 32;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_d0_imm32 () -> bool;

        /**
         * @brief   Executes a `SUB D0, DY` instruction, which subtracts the value
         *          of the full register `DY` from register `D0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x670Y SUB D0, DY`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 28;
         *                      `C` - Set if borrow from bit 32;
         *                      `V` - Set if signed overflow occurred
         */
        auto sub_d0_dy () -> bool;

        /**
         * @brief   Executes an `INC WX` instruction, which increments the value
         *          of the word register `WX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6CX0 INC WX`
         * @note    Parameters: `X` - Word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 11;
         *                      `C` - Unchanged;
         *                      `V` - Unchanged
         */
        auto inc_wx () -> bool;

        /**
         * @brief   Executes an `INC DX` instruction, which increments the value
         *          of the full register `DX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6DX0 INC DX`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set if carry from bit 27;
         *                      `C` - Unchanged;
         *                      `V` - Unchanged
         */
        auto inc_dx () -> bool;

        /**
         * @brief   Executes a `DEC WX` instruction, which decrements the value
         *          of the word register `WX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6EX0 DEC WX`
         * @note    Parameters: `X` - Word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 12;
         *                      `C` - Unchanged;
         *                      `V` - Unchanged
         */
        auto dec_wx () -> bool;

        /**
         * @brief   Executes a `DEC DX` instruction, which decrements the value
         *          of the full register `DX` by 1.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x6FX0 DEC DX`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     5 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 28;
         *                      `C` - Unchanged;
         *                      `V` - Unchanged
         */
        auto dec_dx () -> bool;

    private: /* Private Methods - 8-Bit Bitwise and Logical Instructions *****/

        /**
         * @brief   Executes an `AND L0, IMM8` instruction, which performs a
         *          bitwise AND between an immediate 8-bit value and the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7000 AND L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto and_l0_imm8 () -> bool;

        /**
         * @brief   Executes an `AND L0, LY` instruction, which performs a
         *          bitwise AND between the low byte register `LY` and the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x710Y AND L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto and_l0_ly () -> bool;

        /**
         * @brief   Executes an `AND L0, [DY]` instruction, which performs a
         *          bitwise AND between the value at the memory address pointed
         *          to by `DY` and the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x720Y AND L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Set;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto and_l0_pdy () -> bool;

        /**
         * @brief   Executes an `OR L0, IMM8` instruction, which performs a
         *          bitwise OR between an immediate 8-bit value and the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7300 OR L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto or_l0_imm8 () -> bool;

        /**
         * @brief   Executes an `OR L0, LY` instruction, which performs a
         *          bitwise OR between the low byte register `LY` and the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x740Y OR L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto or_l0_ly () -> bool;

        /**
         * @brief   Executes an `OR L0, [DY]` instruction, which performs a
         *          bitwise OR between the value at the memory address pointed
         *          to by `DY` and the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x750Y OR L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto or_l0_pdy () -> bool;

        /**
         * @brief   Executes an `XOR L0, IMM8` instruction, which performs a
         *          bitwise XOR between an immediate 8-bit value and the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7600 XOR L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto xor_l0_imm8 () -> bool;

        /**
         * @brief   Executes an `XOR L0, LY` instruction, which performs a
         *          bitwise XOR between the low byte register `LY` and the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x770Y XOR L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto xor_l0_ly () -> bool;

        /**
         * @brief   Executes an `XOR L0, [DY]` instruction, which performs a
         *          bitwise XOR between the value at the memory address pointed
         *          to by `DY` and the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x780Y XOR L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Cleared
         */
        auto xor_l0_pdy () -> bool;

        /**
         * @brief   Executes a `NOT LX` instruction, which performs a bitwise
         *          NOT (complement) on the low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x79X0 NOT LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Unchanged;
         *                      `N` - Set;
         *                      `H` - Set;
         *                      `C` - Unchanged;
         *                      `V` - Cleared
         */
        auto not_lx () -> bool;

        /**
         * @brief   Executes a `NOT [DX]` instruction, which performs a bitwise
         *          NOT (complement) on the value at the memory address pointed
         *          to by `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7AX0 NOT [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Unchanged;
         *                      `N` - Set;
         *                      `H` - Set;
         *                      `C` - Unchanged;
         *                      `V` - Cleared
         */
        auto not_pdx () -> bool;

        /**
         * @brief   Executes a `CMP L0, IMM8` instruction, which compares an
         *          immediate 8-bit value with the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7D00 CMP L0, IMM8`
         * @note    Length:     3 Bytes (Opcode + Immediate Byte)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if operands are equal;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if L0 < IMM8;
         *                      `V` - Set if signed overflow occurred
         */
        auto cmp_l0_imm8 () -> bool;

        /**
         * @brief   Executes a `CMP L0, LY` instruction, which compares the low
         *          byte register `LY` with the accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7E0Y CMP L0, LY`
         * @note    Parameters: `Y` - Source low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if operands are equal;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if L0 < LY;
         *                      `V` - Set if signed overflow occurred
         */
        auto cmp_l0_ly () -> bool;

        /**
         * @brief   Executes a `CMP L0, [DY]` instruction, which compares the
         *          value at the memory address pointed to by `DY` with the
         *          accumulator register `L0`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x7F0Y CMP L0, [DY]`
         * @note    Parameters: `Y` - Source full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if operands are equal;
         *                      `N` - Set;
         *                      `H` - Set if borrow from bit 4;
         *                      `C` - Set if L0 < [DY];
         *                      `V` - Set if signed overflow occurred
         */
        auto cmp_l0_pdy () -> bool;

    private: /* Private Methods - Bit Shift and Swap Instructions ************/

        /**
         * @brief   Executes a `SLA LX` instruction, which shifts the bits in
         *          the low byte register `LX` left arithmetically.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x80X0 SLA LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before shift;
         *                      `V` - Unchanged
         */
        auto sla_lx () -> bool;

        /**
         * @brief   Executes a `SLA [DX]` instruction, which shifts the bits at
         *          the memory address pointed to by `DX` left arithmetically.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x81X0 SLA [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before shift;
         *                      `V` - Unchanged
         */
        auto sla_pdx () -> bool;

        /**
         * @brief   Executes a `SRA LX` instruction, which shifts the bits in
         *          the low byte register `LX` right arithmetically.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x82X0 SRA LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before shift;
         *                      `V` - Unchanged
         */
        auto sra_lx () -> bool;

        /**
         * @brief   Executes a `SRA [DX]` instruction, which shifts the bits at
         *          the memory address pointed to by `DX` right arithmetically.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x83X0 SRA [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before shift;
         *                      `V` - Unchanged
         */
        auto sra_pdx () -> bool;

        /**
         * @brief   Executes a `SRL LX` instruction, which shifts the bits in
         *          the low byte register `LX` right logically.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x84X0 SRL LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before shift;
         *                      `V` - Unchanged
         */
        auto srl_lx () -> bool;

        /**
         * @brief   Executes a `SRL [DX]` instruction, which shifts the bits at
         *          the memory address pointed to by `DX` right logically.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x85X0 SRL [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before shift;
         *                      `V` - Unchanged
         */
        auto srl_pdx () -> bool;

        /**
         * @brief   Executes a `SWAP LX` instruction, which swaps the upper and
         *          lower nibbles in the low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x86X0 SWAP LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Unchanged
         */
        auto swap_lx () -> bool;

        /**
         * @brief   Executes a `SWAP [DX]` instruction, which swaps the upper
         *          and lower nibbles at the memory address pointed to by `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x87X0 SWAP [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Unchanged
         */
        auto swap_pdx () -> bool;

        /**
         * @brief   Executes a `SWAP WX` instruction, which swaps the upper and
         *          lower bytes in the word register `WX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x88X0 SWAP WX`
         * @note    Parameters: `X` - Word register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Unchanged
         */
        auto swap_wx () -> bool;

        /**
         * @brief   Executes a `SWAP DX` instruction, which swaps the upper and
         *          lower words in the full register `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x89X0 SWAP DX`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Cleared;
         *                      `V` - Unchanged
         */
        auto swap_dx () -> bool;

    private: /* Private Methods - Bit Rotate Instructions ********************/

        /**
         * @brief   Executes an `RLA` instruction, which rotates the bits in
         *          the accumulator `L0` left through the Carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x9000 RLA`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Cleared;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before rotate;
         *                      `V` - Unchanged
         */
        auto rla () -> bool;

        /**
         * @brief   Executes an `RL LX` instruction, which rotates the bits in
         *          the low byte register `LX` left through the Carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x91X0 RL LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before rotate;
         *                      `V` - Unchanged
         */
        auto rl_lx () -> bool;

        /**
         * @brief   Executes an `RL [DX]` instruction, which rotates the bits at
         *          the memory address pointed to by `DX` left through the Carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x92X0 RL [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before rotate;
         *                      `V` - Unchanged
         */
        auto rl_pdx () -> bool;

        /**
         * @brief   Executes an `RLCA` instruction, which rotates the bits in
         *          the accumulator `L0` left circularly.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x9300 RLCA`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Cleared;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before rotate;
         *                      `V` - Unchanged
         */
        auto rlca () -> bool;

        /**
         * @brief   Executes an `RLC LX` instruction, which rotates the bits in
         *          the low byte register `LX` left circularly.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x94X0 RLC LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before rotate;
         *                      `V` - Unchanged
         */
        auto rlc_lx () -> bool;

        /**
         * @brief   Executes an `RLC [DX]` instruction, which rotates the bits at
         *          the memory address pointed to by `DX` left circularly.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x95X0 RLC [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 7 before rotate;
         *                      `V` - Unchanged
         */
        auto rlc_pdx () -> bool;

        /**
         * @brief   Executes an `RRA` instruction, which rotates the bits in
         *          the accumulator `L0` right through the Carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x9600 RRA`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Cleared;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before rotate;
         *                      `V` - Unchanged
         */
        auto rra () -> bool;

        /**
         * @brief   Executes an `RR LX` instruction, which rotates the bits in
         *          the low byte register `LX` right through the Carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x97X0 RR LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before rotate;
         *                      `V` - Unchanged
         */
        auto rr_lx () -> bool;

        /**
         * @brief   Executes an `RR [DX]` instruction, which rotates the bits at
         *          the memory address pointed to by `DX` right through the Carry flag.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x98X0 RR [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before rotate;
         *                      `V` - Unchanged
         */
        auto rr_pdx () -> bool;

        /**
         * @brief   Executes an `RRCA` instruction, which rotates the bits in
         *          the accumulator `L0` right circularly.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x9900 RRCA`
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Cleared;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before rotate;
         *                      `V` - Unchanged
         */
        auto rrca () -> bool;

        /**
         * @brief   Executes an `RRC LX` instruction, which rotates the bits in
         *          the low byte register `LX` right circularly.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x9AX0 RRC LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before rotate;
         *                      `V` - Unchanged
         */
        auto rrc_lx () -> bool;

        /**
         * @brief   Executes an `RRC [DX]` instruction, which rotates the bits at
         *          the memory address pointed to by `DX` right circularly.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0x9BX0 RRC [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      `Z` - Set if result is zero;
         *                      `N` - Cleared;
         *                      `H` - Cleared;
         *                      `C` - Set to bit 0 before rotate;
         *                      `V` - Unchanged
         */
        auto rrc_pdx () -> bool;

    private: /* Private Methods - Bit Test and Manipulation Instructions *****/

        /**
         * @brief   Executes a `BIT Y, LX` instruction, which tests bit `Y` in
         *          the low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA0XY BIT Y, LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      `Z` - Set if tested bit is 0;
         *                      `N` - Cleared;
         *                      `H` - Set;
         *                      `C` - Unchanged;
         *                      `V` - Unchanged
         */
        auto bit_y_lx () -> bool;

        /**
         * @brief   Executes a `BIT Y, [DX]` instruction, which tests bit `Y` at
         *          the memory address pointed to by `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA1XY BIT Y, [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     3 M-cycles
         * @note    Flags:      `Z` - Set if tested bit is 0;
         *                      `N` - Cleared;
         *                      `H` - Set;
         *                      `C` - Unchanged;
         *                      `V` - Unchanged
         */
        auto bit_y_pdx () -> bool;

        /**
         * @brief   Executes a `SET Y, LX` instruction, which sets bit `Y` in
         *          the low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA2XY SET Y, LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None affected
         */
        auto set_y_lx () -> bool;

        /**
         * @brief   Executes a `SET Y, [DX]` instruction, which sets bit `Y` at
         *          the memory address pointed to by `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA3XY SET Y, [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None affected
         */
        auto set_y_pdx () -> bool;

        /**
         * @brief   Executes a `RES Y, LX` instruction, which resets (clears)
         *          bit `Y` in the low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA4XY RES Y, LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None affected
         */
        auto res_y_lx () -> bool;

        /**
         * @brief   Executes a `RES Y, [DX]` instruction, which resets (clears)
         *          bit `Y` at the memory address pointed to by `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA5XY RES Y, [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None affected
         */
        auto res_y_pdx () -> bool;

        /**
         * @brief   Executes a `TOG Y, LX` instruction, which toggles bit `Y` in
         *          the low byte register `LX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA6XY TOG Y, LX`
         * @note    Parameters: `X` - Low byte register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     2 M-cycles
         * @note    Flags:      None affected
         */
        auto tog_y_lx () -> bool;

        /**
         * @brief   Executes a `TOG Y, [DX]` instruction, which toggles bit `Y`
         *          at the memory address pointed to by `DX`.
         * 
         * @return  If the instruction executed successfully, returns `true`;
         *          Otherwise, returns `false`.
         * 
         * @note    Opcodes:    `0xA7XY TOG Y, [DX]`
         * @note    Parameters: `X` - Full register index (0 - 15)
         *                      `Y` - Bit index (0 - 7)
         * @note    Length:     2 Bytes (Opcode)
         * @note    Timing:     4 M-cycles
         * @note    Flags:      None affected
         */
        auto tog_y_pdx () -> bool;

    private: /* Private Members ***********************************************/

        /**
         * @brief   A reference to the CPU's connected system bus.
         */
        bus& m_bus;

        /**
         * @brief   The CPU's register file, containing all general-purpose
         *          and special-purpose registers.
         */
        register_file m_regs {};

        /**
         * @brief   The CPU's speed switch register, which manages transitions
         *          between normal and high-speed modes.
         */
        speed_switch_register m_speed_switch_reg {};

        /**
         * @brief   Indicates whether a speed switch is currently in progress.
         */
        bool m_speed_switching { false };

        /**
         * @brief   Stores the last data that was fetched.
         */
        std::uint32_t m_fetch_data { 0 };

        /**
         * @brief   Stores the address from which the last data was fetched.
         */
        std::uint32_t m_fetch_address { 0 };

        /**
         * @brief   Stores the currently executing opcode.
         */
        std::uint16_t m_opcode { 0 };

        /**
         * @brief   Stores the address of the currently executing opcode.
         */
        std::uint32_t m_opcode_address { 0 };

        /**
         * @brief   Indicates whether the CPU is currently stopped.
         */
        bool m_stopped { false };

        /**
         * @brief   Indicates whether the CPU is stopped due to a Double Fault
         *          condition.
         */
        bool m_double_fault { false };

        /**
         * @brief   Indicates whether the CPU is currently halted.
         */
        bool m_halted { false };

        /**
         * @brief   The CPU's Interrupt Master Enable (`IME`) flag indicates
         *          whether interrupts are globally enabled or disabled.
         */
        bool m_ime { false };

        /**
         * @brief   The CPU's Interrupt Master Pending (`IMP`) flag indicates
         *          whether the `IME` flag is to be set after the next instruction
         *          is executed.
         */
        bool m_imp { false };

        /**
         * @brief   Indicates whether the CPU is currently servicing interrupt
         *          vector #0 in order to handle an exception.
         */
        bool m_handling_exception { false };

    };
}
