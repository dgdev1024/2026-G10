/**
 * @file    g10-asm/codegen.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains implementations for the G10 CPU assembler's code generation
 *          component.
 */

/* Private Includes ***********************************************************/

#include <g10-asm/codegen.hpp>
#include <g10/cpu.hpp>
#include <cstring>

/* Public Methods *************************************************************/

namespace g10asm
{
    codegen::codegen (const program& prog, const std::string& source_name) :
        m_program { prog },
        m_current_section { 0 }
    {
        // Initialize object file with source filename
        if (!source_name.empty())
        {
            m_output.source_filename = source_name;
        }
    }

    auto codegen::generate () -> g10::result<g10obj::object_file>
    {
        // Initialize statement addresses tracking
        m_statement_addresses.resize(m_program.statements.size());

        // Pass 1: Resolve all label addresses and calculate sizes
        auto addr_result = resolve_addresses();
        if (addr_result.has_value() == false)
        {
            return g10::error(addr_result.error());
        }

        // Pass 2: Emit code and generate relocations
        auto emit_result = emit_code();
        if (emit_result.has_value() == false)
        {
            return g10::error(emit_result.error());
        }

        // Finalize the string table
        m_output.add_string(m_output.source_filename);

        return m_output;
    }
}

/* Private Methods - Pass 1: Address Resolution *****************************/

namespace g10asm
{
    auto codegen::resolve_addresses () -> g10::result<void>
    {
        std::uint32_t current_address = 0x2000;  // Default origin
        
        for (std::size_t i = 0; i < m_program.statements.size(); ++i)
        {
            const auto& stmt = m_program.statements[i];

            // Record this statement's address
            m_statement_addresses[i] = current_address;

            switch (stmt.type)
            {
                case statement_type::label_definition:
                {
                    // Labels don't consume space, but record their address
                    m_label_addresses[stmt.label_name] = current_address;
                    break;
                }

                case statement_type::directive_org:
                {
                    // Create new section at the new origin
                    current_address = static_cast<std::uint32_t>(stmt.org_address);
                    
                    // Check if we need to create a new section
                    bool need_new_section = true;
                    if (!m_output.sections.empty())
                    {
                        // Check if current section already has code
                        if (m_output.sections.back().size() > 0)
                        {
                            need_new_section = true;
                        }
                        else
                        {
                            // Reuse empty section
                            need_new_section = false;
                            m_output.sections.back().base_address = current_address;
                        }
                    }

                    if (need_new_section)
                    {
                        g10obj::code_section new_section;
                        new_section.base_address = current_address;
                        new_section.flags = g10obj::SECT_EXECUTABLE | 
                                          g10obj::SECT_INITIALIZED;
                        m_output.sections.push_back(new_section);
                        m_current_section = m_output.sections.size() - 1;
                    }

                    break;
                }

                case statement_type::instruction:
                {
                    // Ensure we have at least one section
                    if (m_output.sections.empty())
                    {
                        g10obj::code_section initial_section;
                        initial_section.base_address = current_address;
                        initial_section.flags = g10obj::SECT_EXECUTABLE | 
                                              g10obj::SECT_INITIALIZED;
                        m_output.sections.push_back(initial_section);
                        m_current_section = 0;
                    }

                    auto size = calculate_instruction_size(stmt);
                    current_address += size;
                    break;
                }

                case statement_type::directive_byte:
                case statement_type::directive_word:
                case statement_type::directive_dword:
                {
                    // Ensure we have at least one section
                    if (m_output.sections.empty())
                    {
                        g10obj::code_section initial_section;
                        initial_section.base_address = current_address;
                        initial_section.flags = g10obj::SECT_EXECUTABLE | 
                                              g10obj::SECT_INITIALIZED;
                        m_output.sections.push_back(initial_section);
                        m_current_section = 0;
                    }

                    auto size = calculate_directive_size(stmt);
                    current_address += size;
                    break;
                }

                case statement_type::directive_global:
                {
                    // Mark symbols as global
                    for (const auto& symbol_name : stmt.symbol_names)
                    {
                        // Will be resolved in Pass 2
                    }
                    break;
                }

                case statement_type::directive_extern:
                {
                    // Mark symbols as external
                    for (const auto& symbol_name : stmt.symbol_names)
                    {
                        m_external_symbols_seen[symbol_name] = true;
                    }
                    break;
                }

                default:
                    break;
            }
        }

        return {};
    }

    auto codegen::calculate_instruction_size (const statement& stmt) -> std::size_t
    {
        // All instructions start with a 2-byte opcode
        std::size_t size = 2;

        switch (stmt.inst)
        {
            // Zero-operand instructions: just the opcode
            case g10::instruction::nop:
            case g10::instruction::stop:
            case g10::instruction::halt:
            case g10::instruction::di:
            case g10::instruction::ei:
            case g10::instruction::eii:
            case g10::instruction::daa:
            case g10::instruction::scf:
            case g10::instruction::ccf:
            case g10::instruction::clv:
            case g10::instruction::sev:
            case g10::instruction::ret:
            case g10::instruction::reti:
            case g10::instruction::rla:
            case g10::instruction::rlca:
            case g10::instruction::rra:
            case g10::instruction::rrca:
            case g10::instruction::cpl:
                return 2;

            // One-operand instructions (register only): just the opcode
            case g10::instruction::push:
            case g10::instruction::pop:
            case g10::instruction::inc:
            case g10::instruction::dec:
            case g10::instruction::not_:
            case g10::instruction::sla:
            case g10::instruction::sra:
            case g10::instruction::srl:
            case g10::instruction::swap:
            case g10::instruction::rl:
            case g10::instruction::rlc:
            case g10::instruction::rr:
            case g10::instruction::rrc:
                return 2;

            // LSP, SSP, SPO, SPI
            case g10::instruction::lsp:
            case g10::instruction::ssp:
            case g10::instruction::spo:
            case g10::instruction::spi:
                return 2;

            // INT instruction: opcode + 1-byte vector
            case g10::instruction::int_:
                return 2 + 1;

            // LD instruction - variable size based on operands
            case g10::instruction::ld:
            {
                if (stmt.operands.size() >= 2)
                {
                    const auto& src = stmt.operands[1];
                    if (src.type == operand_type::immediate_value)
                    {
                        // Size based on destination register size
                        return 2 + stmt.operands[0].register_size;
                    }
                    else if (src.type == operand_type::memory_address ||
                             src.type == operand_type::label_reference)
                    {
                        // 32-bit address
                        return 2 + 4;
                    }
                    else if (src.type == operand_type::indirect_address)
                    {
                        // Register indirect - just opcode
                        return 2;
                    }
                }
                return 2;
            }

            // ST instruction - variable size
            case g10::instruction::st:
            {
                if (stmt.operands.size() >= 2)
                {
                    const auto& dst = stmt.operands[0];
                    if (dst.type == operand_type::memory_address ||
                        dst.type == operand_type::label_reference)
                    {
                        // 32-bit address
                        return 2 + 4;
                    }
                }
                return 2;
            }

            // LDQ, LDP, STQ, STP - quick load/store with 8/16-bit offset
            case g10::instruction::ldq:
            case g10::instruction::stq:
                return 2 + 2;  // 16-bit offset

            case g10::instruction::ldp:
            case g10::instruction::stp:
                return 2 + 1;  // 8-bit offset

            // MV, MWH, MWL - register operations
            case g10::instruction::mv:
            case g10::instruction::mwh:
            case g10::instruction::mwl:
                return 2;

            // Two-operand ALU instructions (register + immediate or register)
            case g10::instruction::add:
            case g10::instruction::adc:
            case g10::instruction::sub:
            case g10::instruction::sbc:
            case g10::instruction::and_:
            case g10::instruction::or_:
            case g10::instruction::xor_:
            case g10::instruction::cmp:
            case g10::instruction::cp:
            {
                if (stmt.operands.size() >= 2)
                {
                    const auto& src = stmt.operands[1];
                    if (src.type == operand_type::immediate_value)
                    {
                        // Size based on operand size
                        return 2 + src.register_size;
                    }
                }
                return 2;
            }

            // Bit operations (BIT, SET, RES, TOG)
            case g10::instruction::bit:
            case g10::instruction::set:
            case g10::instruction::res:
            case g10::instruction::tog:
                return 2 + 1;  // Bit number

            // Jump instructions - absolute jumps with 32-bit address
            case g10::instruction::jmp:
            case g10::instruction::jp:
            case g10::instruction::call:
                return 2 + 4;  // Opcode + 32-bit address

            // Relative jump instructions (JPB, JR) - 8-bit offset (or more)
            case g10::instruction::jpb:
            case g10::instruction::jr:
                return 2 + 1;  // Opcode + 8-bit offset (TODO: handle larger offsets)

            default:
                return 2;
        }
    }

    auto codegen::calculate_directive_size (const statement& stmt) -> std::size_t
    {
        switch (stmt.type)
        {
            case statement_type::directive_byte:
                return stmt.data_values.size();

            case statement_type::directive_word:
                return stmt.data_values.size() * 2;

            case statement_type::directive_dword:
                return stmt.data_values.size() * 4;

            default:
                return 0;
        }
    }
}

/* Private Methods - Pass 2: Code Emission ***********************************/

namespace g10asm
{
    auto codegen::emit_code () -> g10::result<void>
    {
        // Ensure we have at least one section
        if (m_output.sections.empty())
        {
            g10obj::code_section initial_section;
            initial_section.base_address = 0x2000;
            initial_section.flags = g10obj::SECT_EXECUTABLE | g10obj::SECT_INITIALIZED;
            m_output.sections.push_back(initial_section);
        }

        m_current_section = 0;

        for (std::size_t i = 0; i < m_program.statements.size(); ++i)
        {
            const auto& stmt = m_program.statements[i];

            switch (stmt.type)
            {
                case statement_type::label_definition:
                {
                    // Register label as symbol
                    bool is_global = false;
                    for (const auto& global_sym : m_program.global_symbols)
                    {
                        if (global_sym == stmt.label_name)
                        {
                            is_global = true;
                            break;
                        }
                    }
                    register_symbol(stmt.label_name, m_label_addresses[stmt.label_name], 
                                   is_global);
                    break;
                }

                case statement_type::directive_org:
                {
                    // Switch to appropriate section
                    std::uint32_t target_address = static_cast<std::uint32_t>(stmt.org_address);
                    for (std::size_t j = 0; j < m_output.sections.size(); ++j)
                    {
                        if (m_output.sections[j].base_address == target_address)
                        {
                            m_current_section = j;
                            break;
                        }
                    }
                    break;
                }

                case statement_type::instruction:
                {
                    auto emit_result = emit_instruction(stmt);
                    if (emit_result.has_value() == false)
                    {
                        return g10::error(emit_result.error());
                    }
                    break;
                }

                case statement_type::directive_byte:
                case statement_type::directive_word:
                case statement_type::directive_dword:
                {
                    auto emit_result = emit_directive(stmt);
                    if (emit_result.has_value() == false)
                    {
                        return g10::error(emit_result.error());
                    }
                    break;
                }

                default:
                    break;
            }
        }

        // Create symbol entries for external symbols
        for (const auto& [ext_name, _] : m_external_symbols_seen)
        {
            // Skip if already registered
            if (m_symbol_index_map.find(ext_name) != m_symbol_index_map.end())
            {
                continue;
            }

            g10obj::symbol ext_symbol;
            ext_symbol.name = ext_name;
            ext_symbol.value = 0;
            ext_symbol.section_index = 0xFFFF;
            ext_symbol.type = g10obj::symbol_type::undefined;
            ext_symbol.binding = g10obj::symbol_binding::extern_;
            ext_symbol.size = 0;

            m_symbol_index_map[ext_name] = m_output.symbols.size();
            m_output.symbols.push_back(ext_symbol);
        }

        return {};
    }

    auto codegen::emit_instruction (const statement& stmt) -> g10::result<void>
    {
        // Instruction opcode base values (from cpu.hpp documentation)
        std::uint16_t opcode = 0x0000;

        // Determine base opcode and generate code based on instruction type
        switch (stmt.inst)
        {
            // ================================================================
            // ZERO-OPERAND INSTRUCTIONS (Control)
            // ================================================================

            case g10::instruction::nop:
                opcode = 0x0000;
                emit_opcode(opcode);
                break;

            case g10::instruction::stop:
                opcode = 0x0100;
                emit_opcode(opcode);
                break;

            case g10::instruction::halt:
                opcode = 0x0200;
                emit_opcode(opcode);
                break;

            case g10::instruction::di:
                opcode = 0x0300;
                emit_opcode(opcode);
                break;

            case g10::instruction::ei:
                opcode = 0x0400;
                emit_opcode(opcode);
                break;

            case g10::instruction::eii:
                opcode = 0x0500;
                emit_opcode(opcode);
                break;

            case g10::instruction::daa:
                opcode = 0x0600;
                emit_opcode(opcode);
                break;

            case g10::instruction::scf:
                opcode = 0x0700;
                emit_opcode(opcode);
                break;

            case g10::instruction::ccf:
                opcode = 0x0800;
                emit_opcode(opcode);
                break;

            case g10::instruction::clv:
                opcode = 0x0900;
                emit_opcode(opcode);
                break;

            case g10::instruction::sev:
                opcode = 0x0A00;
                emit_opcode(opcode);
                break;

            // ================================================================
            // LOAD/STORE INSTRUCTIONS
            // ================================================================

            case g10::instruction::ld:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("LD requires 2 operands");

                const auto& dst = stmt.operands[0];
                const auto& src = stmt.operands[1];

                opcode = 0x1000 | (encode_register(dst) << 8);

                if (src.type == operand_type::register_name)
                {
                    // LD reg, reg
                    opcode |= encode_register(src);
                    emit_opcode(opcode);
                }
                else if (src.type == operand_type::immediate_value)
                {
                    // LD reg, immediate
                    emit_opcode(opcode);
                    if (dst.register_size == 4)
                        emit_dword(static_cast<std::uint32_t>(src.immediate_value));
                    else if (dst.register_size == 2)
                        emit_word(static_cast<std::uint16_t>(src.immediate_value));
                    else
                        emit_byte(static_cast<std::uint8_t>(src.immediate_value));
                }
                else if (src.type == operand_type::memory_address || 
                         src.type == operand_type::label_reference)
                {
                    // LD reg, [address]
                    opcode |= 0x80;  // Mark as memory load
                    emit_opcode(opcode);
                    
                    if (src.type == operand_type::label_reference)
                    {
                        add_relocation(std::string(stmt.operands[1].source_token.lexeme),
                                     g10obj::relocation_type::abs32);
                        emit_dword(0);  // Placeholder
                    }
                    else
                    {
                        emit_dword(static_cast<std::uint32_t>(src.immediate_value));
                    }
                }
                else if (src.type == operand_type::indirect_address)
                {
                    // LD reg, [reg]
                    opcode |= 0x40;
                    opcode |= encode_register(src);
                    emit_opcode(opcode);
                }
                break;
            }

            case g10::instruction::st:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("ST requires 2 operands");

                const auto& dst = stmt.operands[0];
                const auto& src = stmt.operands[1];

                opcode = 0x1100;

                if (dst.type == operand_type::register_name)
                {
                    // ST reg, reg
                    opcode |= (encode_register(dst) << 8);
                    opcode |= encode_register(src);
                    emit_opcode(opcode);
                }
                else if (dst.type == operand_type::memory_address ||
                         dst.type == operand_type::label_reference)
                {
                    // ST [address], reg
                    opcode |= 0x8000;
                    opcode |= (encode_register(src) << 8);
                    emit_opcode(opcode);
                    
                    if (dst.type == operand_type::label_reference)
                    {
                        add_relocation(std::string(stmt.operands[0].source_token.lexeme),
                                     g10obj::relocation_type::abs32);
                        emit_dword(0);  // Placeholder
                    }
                    else
                    {
                        emit_dword(static_cast<std::uint32_t>(dst.immediate_value));
                    }
                }
                else if (dst.type == operand_type::indirect_address)
                {
                    // ST [reg], reg
                    opcode |= 0x4000;
                    opcode |= (encode_register(dst) << 8);
                    opcode |= encode_register(src);
                    emit_opcode(opcode);
                }
                break;
            }

            // ================================================================
            // STACK OPERATIONS
            // ================================================================

            case g10::instruction::push:
            {
                if (stmt.operands.empty())
                    return g10::error("PUSH requires 1 operand");
                
                opcode = 0x1200 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::pop:
            {
                if (stmt.operands.empty())
                    return g10::error("POP requires 1 operand");
                
                opcode = 0x1300 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            // ================================================================
            // ARITHMETIC/LOGIC OPERATIONS
            // ================================================================

            case g10::instruction::add:
            case g10::instruction::adc:
            case g10::instruction::sub:
            case g10::instruction::sbc:
            case g10::instruction::and_:
            case g10::instruction::or_:
            case g10::instruction::xor_:
            case g10::instruction::cmp:
            case g10::instruction::cp:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("Arithmetic instruction requires 2 operands");

                // Map instruction to opcode
                std::uint16_t base_opcode;
                switch (stmt.inst)
                {
                    case g10::instruction::add:
                    case g10::instruction::cp:
                        base_opcode = 0x2000;
                        break;
                    case g10::instruction::adc:
                        base_opcode = 0x2100;
                        break;
                    case g10::instruction::sub:
                        base_opcode = 0x2200;
                        break;
                    case g10::instruction::sbc:
                        base_opcode = 0x2300;
                        break;
                    case g10::instruction::and_:
                        base_opcode = 0x2400;
                        break;
                    case g10::instruction::or_:
                        base_opcode = 0x2500;
                        break;
                    case g10::instruction::xor_:
                        base_opcode = 0x2600;
                        break;
                    case g10::instruction::cmp:
                        base_opcode = 0x2700;
                        break;
                    default:
                        base_opcode = 0x2000;
                }

                const auto& src = stmt.operands[1];
                if (src.type == operand_type::register_name)
                {
                    // reg op reg
                    opcode = base_opcode | encode_register(src);
                    emit_opcode(opcode);
                }
                else if (src.type == operand_type::immediate_value)
                {
                    // reg op immediate
                    opcode = base_opcode | 0x80;
                    emit_opcode(opcode);
                    if (src.register_size == 4)
                        emit_dword(static_cast<std::uint32_t>(src.immediate_value));
                    else if (src.register_size == 2)
                        emit_word(static_cast<std::uint16_t>(src.immediate_value));
                    else
                        emit_byte(static_cast<std::uint8_t>(src.immediate_value));
                }
                break;
            }

            // ================================================================
            // INCREMENT/DECREMENT
            // ================================================================

            case g10::instruction::inc:
            {
                if (stmt.operands.empty())
                    return g10::error("INC requires 1 operand");
                
                opcode = 0x2800 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::dec:
            {
                if (stmt.operands.empty())
                    return g10::error("DEC requires 1 operand");
                
                opcode = 0x2900 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            // ================================================================
            // BITWISE NOT
            // ================================================================

            case g10::instruction::not_:
            case g10::instruction::cpl:
            {
                if (stmt.operands.empty())
                {
                    // CPL without operand is an alias for NOT L0
                    opcode = 0x2A00 | 0x40;  // L0 register encoding
                    emit_opcode(opcode);
                }
                else
                {
                    opcode = 0x2A00 | encode_register(stmt.operands[0]);
                    emit_opcode(opcode);
                }
                break;
            }

            // ================================================================
            // SHIFT OPERATIONS
            // ================================================================

            case g10::instruction::sla:
            case g10::instruction::sra:
            case g10::instruction::srl:
            {
                if (stmt.operands.empty())
                    return g10::error("Shift instruction requires 1 operand");

                std::uint16_t base_opcode;
                switch (stmt.inst)
                {
                    case g10::instruction::sla:
                        base_opcode = 0x2B00;
                        break;
                    case g10::instruction::sra:
                        base_opcode = 0x2C00;
                        break;
                    case g10::instruction::srl:
                        base_opcode = 0x2D00;
                        break;
                    default:
                        base_opcode = 0x2B00;
                }

                opcode = base_opcode | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            // ================================================================
            // ROTATE OPERATIONS
            // ================================================================

            case g10::instruction::rl:
            case g10::instruction::rlc:
            case g10::instruction::rla:
            case g10::instruction::rlca:
            case g10::instruction::rr:
            case g10::instruction::rrc:
            case g10::instruction::rra:
            case g10::instruction::rrca:
            {
                std::uint16_t base_opcode;
                switch (stmt.inst)
                {
                    case g10::instruction::rl:
                    case g10::instruction::rla:
                        base_opcode = 0x2E00;
                        break;
                    case g10::instruction::rlc:
                    case g10::instruction::rlca:
                        base_opcode = 0x2E80;
                        break;
                    case g10::instruction::rr:
                    case g10::instruction::rra:
                        base_opcode = 0x2F00;
                        break;
                    case g10::instruction::rrc:
                    case g10::instruction::rrca:
                        base_opcode = 0x2F80;
                        break;
                    default:
                        base_opcode = 0x2E00;
                }

                if (stmt.operands.empty())
                {
                    // Accumulator version (RLA, RLCA, etc)
                    emit_opcode(base_opcode);
                }
                else
                {
                    // Register version (RL, RLC, etc)
                    opcode = base_opcode | encode_register(stmt.operands[0]);
                    emit_opcode(opcode);
                }
                break;
            }

            // ================================================================
            // BIT OPERATIONS
            // ================================================================

            case g10::instruction::bit:
            case g10::instruction::set:
            case g10::instruction::res:
            case g10::instruction::tog:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("Bit operation requires register and bit number");

                std::uint16_t base_opcode;
                switch (stmt.inst)
                {
                    case g10::instruction::bit:
                        base_opcode = 0x3000;
                        break;
                    case g10::instruction::set:
                        base_opcode = 0x3100;
                        break;
                    case g10::instruction::res:
                        base_opcode = 0x3200;
                        break;
                    case g10::instruction::tog:
                        base_opcode = 0x3300;
                        break;
                    default:
                        base_opcode = 0x3000;
                }

                opcode = base_opcode | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                
                // Emit bit number
                std::uint8_t bit_num = static_cast<std::uint8_t>(
                    stmt.operands[1].immediate_value & 0xFF
                );
                emit_byte(bit_num);
                break;
            }

            // ================================================================
            // JUMP INSTRUCTIONS
            // ================================================================

            case g10::instruction::jmp:
            case g10::instruction::jp:
            {
                if (stmt.operands.size() < 1)
                    return g10::error("JMP requires an address");

                // Condition code, address or just address
                std::uint8_t condition = 0;  // Default: NC (no condition)
                std::size_t addr_idx = 0;

                if (stmt.operands.size() >= 2)
                {
                    // Two operands: condition and address
                    condition = encode_condition(stmt.operands[0]);
                    addr_idx = 1;
                }
                else
                {
                    // One operand: just address
                    addr_idx = 0;
                }

                opcode = 0x2800 | condition;
                emit_opcode(opcode);

                // Emit 32-bit address
                if (stmt.operands[addr_idx].type == operand_type::label_reference)
                {
                    add_relocation(std::string(stmt.operands[addr_idx].source_token.lexeme),
                                 g10obj::relocation_type::abs32);
                    emit_dword(0);
                }
                else
                {
                    emit_dword(static_cast<std::uint32_t>(
                        stmt.operands[addr_idx].immediate_value
                    ));
                }
                break;
            }

            case g10::instruction::jpb:
            case g10::instruction::jr:
            {
                if (stmt.operands.size() < 1)
                    return g10::error("JPB requires an offset");

                // Condition code, offset or just offset
                std::uint8_t condition = 0;  // Default: NC
                std::size_t offset_idx = 0;

                if (stmt.operands.size() >= 2)
                {
                    // Two operands: condition and offset
                    condition = encode_condition(stmt.operands[0]);
                    offset_idx = 1;
                }
                else
                {
                    // One operand: just offset
                    offset_idx = 0;
                }

                opcode = 0x2900 | condition;
                emit_opcode(opcode);

                // Emit 8-bit offset (or calculate from label)
                if (stmt.operands[offset_idx].type == operand_type::label_reference)
                {
                    // For relative jumps, calculate offset from current position
                    std::uint32_t target_addr = resolve_label(
                        std::string(stmt.operands[offset_idx].source_token.lexeme)
                    );
                    std::int32_t offset = static_cast<std::int32_t>(target_addr) -
                                         static_cast<std::int32_t>(current_address() + 1);
                    emit_byte(static_cast<std::uint8_t>(offset & 0xFF));
                }
                else
                {
                    emit_byte(static_cast<std::uint8_t>(
                        stmt.operands[offset_idx].immediate_value & 0xFF
                    ));
                }
                break;
            }

            case g10::instruction::call:
            {
                if (stmt.operands.size() < 1)
                    return g10::error("CALL requires an address");

                opcode = 0x2A00;
                emit_opcode(opcode);

                if (stmt.operands[0].type == operand_type::label_reference)
                {
                    add_relocation(std::string(stmt.operands[0].source_token.lexeme),
                                 g10obj::relocation_type::abs32);
                    emit_dword(0);
                }
                else
                {
                    emit_dword(static_cast<std::uint32_t>(
                        stmt.operands[0].immediate_value
                    ));
                }
                break;
            }

            case g10::instruction::int_:
            {
                if (stmt.operands.empty())
                    return g10::error("INT requires a vector number");

                opcode = 0x2B00;
                emit_opcode(opcode);
                emit_byte(static_cast<std::uint8_t>(
                    stmt.operands[0].immediate_value & 0xFF
                ));
                break;
            }

            // ================================================================
            // RETURN INSTRUCTIONS
            // ================================================================

            case g10::instruction::ret:
            {
                std::uint8_t condition = 0;
                if (!stmt.operands.empty())
                {
                    condition = encode_condition(stmt.operands[0]);
                }
                opcode = 0x2C00 | condition;
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::reti:
            {
                opcode = 0x2D00;
                emit_opcode(opcode);
                break;
            }

            // ================================================================
            // MOVE OPERATIONS
            // ================================================================

            case g10::instruction::mv:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("MV requires 2 operands");

                opcode = 0x1400 | (encode_register(stmt.operands[0]) << 8);
                opcode |= encode_register(stmt.operands[1]);
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::swap:
            {
                if (stmt.operands.empty())
                    return g10::error("SWAP requires 1 operand");

                opcode = 0x1500 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            // ================================================================
            // QUICK LOAD/STORE
            // ================================================================

            case g10::instruction::ldq:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("LDQ requires 2 operands");

                opcode = 0x1600 | (encode_register(stmt.operands[0]) << 8);
                emit_opcode(opcode);
                emit_word(static_cast<std::uint16_t>(
                    stmt.operands[1].immediate_value & 0xFFFF
                ));
                break;
            }

            case g10::instruction::ldp:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("LDP requires 2 operands");

                opcode = 0x1700 | (encode_register(stmt.operands[0]) << 8);
                emit_opcode(opcode);
                emit_byte(static_cast<std::uint8_t>(
                    stmt.operands[1].immediate_value & 0xFF
                ));
                break;
            }

            case g10::instruction::stq:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("STQ requires 2 operands");

                opcode = 0x1800 | (encode_register(stmt.operands[0]) << 8);
                emit_opcode(opcode);
                emit_word(static_cast<std::uint16_t>(
                    stmt.operands[1].immediate_value & 0xFFFF
                ));
                break;
            }

            case g10::instruction::stp:
            {
                if (stmt.operands.size() < 2)
                    return g10::error("STP requires 2 operands");

                opcode = 0x1900 | (encode_register(stmt.operands[0]) << 8);
                emit_opcode(opcode);
                emit_byte(static_cast<std::uint8_t>(
                    stmt.operands[1].immediate_value & 0xFF
                ));
                break;
            }

            // ================================================================
            // STACK POINTER OPERATIONS
            // ================================================================

            case g10::instruction::lsp:
            {
                if (stmt.operands.empty())
                    return g10::error("LSP requires 1 operand");

                opcode = 0x1A00 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::ssp:
            {
                if (stmt.operands.empty())
                    return g10::error("SSP requires 1 operand");

                opcode = 0x1B00 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::spo:
            {
                if (stmt.operands.empty())
                    return g10::error("SPO requires 1 operand");

                opcode = 0x1C00 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            case g10::instruction::spi:
            {
                if (stmt.operands.empty())
                    return g10::error("SPI requires 1 operand");

                opcode = 0x1D00 | encode_register(stmt.operands[0]);
                emit_opcode(opcode);
                break;
            }

            // ================================================================
            // UNIMPLEMENTED INSTRUCTIONS
            // ================================================================

            default:
                return g10::error("Instruction not yet implemented: {}",
                    stmt.source_token.lexeme);
        }

        return {};
    }

    auto codegen::emit_directive (const statement& stmt) -> g10::result<void>
    {
        switch (stmt.type)
        {
            case statement_type::directive_byte:
            {
                for (const auto& value : stmt.data_values)
                {
                    emit_byte(static_cast<std::uint8_t>(value & 0xFF));
                }
                break;
            }

            case statement_type::directive_word:
            {
                for (const auto& value : stmt.data_values)
                {
                    emit_word(static_cast<std::uint16_t>(value & 0xFFFF));
                }
                break;
            }

            case statement_type::directive_dword:
            {
                for (const auto& value : stmt.data_values)
                {
                    emit_dword(static_cast<std::uint32_t>(value & 0xFFFFFFFF));
                }
                break;
            }

            default:
                break;
        }

        return {};
    }
}

/* Private Methods - Output Helpers ******************************************/

namespace g10asm
{
    auto codegen::emit_opcode (std::uint16_t opcode) -> void
    {
        emit_word(opcode);
    }

    auto codegen::emit_byte (std::uint8_t value) -> void
    {
        m_output.sections[m_current_section].code.push_back(value);
    }

    auto codegen::emit_word (std::uint16_t value) -> void
    {
        // Little-endian
        emit_byte(static_cast<std::uint8_t>(value & 0xFF));
        emit_byte(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    }

    auto codegen::emit_dword (std::uint32_t value) -> void
    {
        // Little-endian
        emit_byte(static_cast<std::uint8_t>(value & 0xFF));
        emit_byte(static_cast<std::uint8_t>((value >> 8) & 0xFF));
        emit_byte(static_cast<std::uint8_t>((value >> 16) & 0xFF));
        emit_byte(static_cast<std::uint8_t>((value >> 24) & 0xFF));
    }
}

/* Private Methods - Operand Encoding ****************************************/

namespace g10asm
{
    auto codegen::encode_register (const operand& op) -> std::uint8_t
    {
        // Encode as: SSSS RRRR
        // SSSS = size field (0-7)
        // RRRR = register index (0-15)

        std::uint8_t size_field = 0;
        switch (op.register_size)
        {
            case 4: size_field = 0; break;  // 32-bit (D)
            case 2: size_field = 1; break;  // 16-bit (W)
            case 1:
                // Could be H or L, distinguish by looking at the actual register type
                // For now, assume 1-byte registers use size_field 2
                size_field = 2; 
                break;
            default: size_field = 0; break;
        }

        return static_cast<std::uint8_t>(
            ((size_field & 0x0F) << 4) | (op.register_index & 0x0F)
        );
    }

    auto codegen::encode_condition (const operand& op) -> std::uint8_t
    {
        // Condition codes are stored as immediate values
        // NC=0, ZS=1, ZC=2, CS=3, CC=4, VS=5, VC=6
        
        // If this is a condition code keyword, extract the condition code value
        // from its source token
        const auto& token = op.source_token;
        
        if (token.type == token_type::keyword && 
            token.kw_type == keyword_type::condition_code)
        {
            // Look up the condition code in the keyword table
            auto kw_result = keyword_table::lookup(token.lexeme);
            if (kw_result.has_value())
            {
                return static_cast<std::uint8_t>(kw_result->get().param1);
            }
        }

        // If it's an immediate value, use it directly
        return static_cast<std::uint8_t>(op.immediate_value & 0xFF);
    }
}

/* Private Methods - Symbol Resolution ****************************************/

namespace g10asm
{
    auto codegen::resolve_label (const std::string& label_name) -> std::uint32_t
    {
        // Check local symbol table first
        auto it = m_label_addresses.find(label_name);
        if (it != m_label_addresses.end())
        {
            return it->second;
        }

        // It's an external symbol - create relocation
        add_relocation(label_name, g10obj::relocation_type::abs32);
        return 0;  // Placeholder - will be resolved by linker
    }

    auto codegen::register_symbol (const std::string& label_name, 
        std::uint32_t address, bool is_global) -> void
    {
        // Skip if already registered
        if (m_symbol_index_map.find(label_name) != m_symbol_index_map.end())
        {
            return;
        }

        g10obj::symbol sym;
        sym.name = label_name;
        sym.value = address;
        sym.section_index = m_current_section;
        sym.type = g10obj::symbol_type::label;
        sym.binding = is_global ? g10obj::symbol_binding::global : 
                                 g10obj::symbol_binding::local;
        sym.size = 0;

        m_symbol_index_map[label_name] = m_output.symbols.size();
        m_output.symbols.push_back(sym);
    }
}

/* Private Methods - Output Helpers ******************************************/

namespace g10asm
{
    auto codegen::add_relocation (const std::string& symbol_name,
        g10obj::relocation_type reloc_type, std::int32_t addend) -> void
    {
        g10obj::relocation reloc;
        reloc.offset = current_offset();
        reloc.section_index = m_current_section;
        reloc.symbol_name = symbol_name;
        reloc.type = reloc_type;
        reloc.addend = addend;

        m_output.relocations.push_back(reloc);

        // Track that we've seen this external symbol
        m_external_symbols_seen[symbol_name] = true;
    }
}
