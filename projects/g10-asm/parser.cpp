/**
 * @file    g10-asm/parser.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-19
 * 
 * @brief   Contains definitions for the G10 CPU assembler's syntax analyzer
 *          component.
 */

/* Private Includes ***********************************************************/

#include <g10-asm/parser.hpp>

/* Public Methods *************************************************************/

namespace g10asm
{
    parser::parser (lexer& lex) :
        m_lexer { lex }
    {
    }

    auto parser::parse_program () -> g10::result_ref<program>
    {
        // - Create a new program AST to hold the parsed statements.
        m_program = program {};

        // - Parse statements until we reach the end of the token stream.
        while (is_at_end() == false)
        {
            // - Skip over any newline tokens.
            while (match_token(token_type::end_of_line) == true) {}
            if (is_at_end() == true)
                { break; }

            // - Peek the current token. Are we at the end of the file?
            auto token_result = current_token();
            if (token_result.has_value() == false)
            {
                return g10::error(
                    "Unexpected end of input while parsing program."
                );
            }
            if (token_result->get().type == token_type::end_of_file)
                { break; }

            // - Parse the next statement.
            auto statement_result = parse_statement();
            if (statement_result.has_value() == false)
            {
                // - An error occurred while parsing the statement. Synchronize
                //   to the next statement and continue parsing.
                std::println(stderr, "{}", statement_result.error());
                synchronize();
                continue;
            }

            // - Move the parsed statement into the program's statement list.
            m_program.statements.emplace_back(
                std::move(statement_result.value())
            );

            // - Expect either a newline or end-of-file after the statement.
            if (check_token(token_type::end_of_file) == false)
            {
                auto eol = consume_token(token_type::end_of_line,
                    "Expected end of line after statement.");
                if (eol.has_value() == false)
                {
                    std::println(stderr, "{}", eol.error());
                    synchronize();
                }
            }
        }

        // - Mark the parser as successful and return the parsed program.
        m_good = true;
        return m_program;
    }
}

/* Private Methods - Token Consumption ****************************************/

namespace g10asm
{
    auto parser::current_token () -> g10::result_ref<token>
    {
        return m_lexer.peek_token(0);
    }

    auto parser::peek_token (std::int64_t offset) -> g10::result_ref<token>
    {
        return m_lexer.peek_token(offset);
    }

    auto parser::advance_token () -> g10::result_ref<token>
    {
        return m_lexer.next_token();
    }

    auto parser::check_token (token_type expected) -> bool
    {
        auto token_result = current_token();
        if (token_result.has_value() == false)
            { return false; }

        return token_result->get().type == expected;
    }

    auto parser::match_token (token_type expected) -> bool
    {
        if (check_token(expected) == true)
        {
            advance_token();
            return true;
        }

        return false;
    }

    auto parser::is_at_end () -> bool
    {
        return m_lexer.is_at_end();
    }
}

/* Private Methods - Statement Parsing ****************************************/

namespace g10asm
{
    auto parser::parse_statement () -> g10::result<statement>
    {
        // - Check for unexpected end of token stream.
        auto token_result = current_token();
        if (token_result.has_value() == false)
        {
            return error("Unexpected end of input while parsing statement.");
        }

        // - Check for a label definition (identifier followed by a colon).
        const auto& token = token_result->get();
        if (token.type == token_type::identifier)
        {
            auto next_token_result = peek_token(1);
            if (
                next_token_result.has_value() == true &&
                next_token_result->get().type == token_type::colon
            )
            {
                return parse_label_definition();
            }
        }

        // - Check for a directive (keyword token of directive type).
        if (
            token.type == token_type::keyword &&
            token.kw_type == keyword_type::directive
        )
        {
            return parse_directive();
        }

        // - Check for an instruction (keyword token of instruction type).
        if (
            token.type == token_type::keyword &&
            token.kw_type == keyword_type::instruction
        )
        {            
            return parse_instruction();
        }

        // - If we reach here, the statement is invalid.
        return error(panic(token, "Unexpected token while parsing statement."));
    }

    auto parser::parse_label_definition () -> g10::result<statement>
    {
        // - Get the label's identifier token.
        auto id_token_result = advance_token();
        if (id_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing label definition."
            );
        }

        // - Consume the colon token.
        auto colon_token_result = consume_token(token_type::colon,
            "Expected ':' after label name in label definition.");
        if (colon_token_result.has_value() == false)
        {
            return g10::error(colon_token_result.error());
        }

        // - Create the label definition statement.
        statement label_stmt {};
        label_stmt.type = statement_type::label_definition;
        label_stmt.source_token = id_token_result->get();
        label_stmt.line_number = id_token_result->get().line_number;
        label_stmt.source_file = id_token_result->get().source_file;
        label_stmt.label_name = std::string { id_token_result->get().lexeme };

        // - Register the label in the program's symbol table.
        m_program.label_table[label_stmt.label_name] =
            m_program.statements.size();

        return label_stmt;
    }

    auto parser::parse_instruction () -> g10::result<statement>
    {
        // - Get the instruction mnemonic token.
        auto inst_token_result = advance_token();
        if (inst_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing instruction."
            );
        }

        // - Create the instruction statement.
        const auto& inst_token = inst_token_result->get();
        statement inst_stmt {};
        inst_stmt.type = statement_type::instruction;
        inst_stmt.source_token = inst_token;
        inst_stmt.line_number = inst_token.line_number;
        inst_stmt.source_file = inst_token.source_file;

        // - Look up the instruction in the keyword table. We already know it's
        //   a keyword token of instruction type.
        auto kw_result = keyword_table::lookup(inst_token.lexeme);
        inst_stmt.inst = static_cast<g10::instruction>(kw_result->get().param1);

        // - Parse the instruction's operands, if any.
        if (
            check_token(token_type::end_of_line) == false &&
            check_token(token_type::end_of_file) == false
        )
        {
            // - Parse the first operand.
            auto operand1_result = parse_operand();
            if (operand1_result.has_value() == false)
            {
                return g10::error(operand1_result.error());
            }

            inst_stmt.operands.emplace_back(
                std::move(operand1_result.value())
            );

            // - Check for a second operand.
            while (match_token(token_type::comma) == true)
            {
                // - Parse the second operand.
                auto operand2_result = parse_operand();
                if (operand2_result.has_value() == false)
                {
                    return g10::error(operand2_result.error());
                }

                inst_stmt.operands.emplace_back(
                    std::move(operand2_result.value())
                );
            }
        }

        // - Validate the instruction's operands and operand count.
        auto validation_result = validate_instruction(inst_stmt);
        if (validation_result.has_value() == false)
        {
            return g10::error(validation_result.error());
        }

        return inst_stmt;
    }

    auto parser::parse_directive () -> g10::result<statement>
    {
        // - Get the directive token.
        auto dir_token_result = current_token();
        if (dir_token_result.has_value() == false)
        {
            return error(
                "Unexpected end of input while parsing directive."
            );
        }

        // - Look up the directive in the keyword table.
        const auto& dir_token = dir_token_result->get();
        auto kw_result = keyword_table::lookup(dir_token.lexeme);
        if (kw_result.has_value() == false)
        {
            return error(panic(dir_token, "Unknown directive."));
        }

        // - Dispatch to the appropriate directive parsing function.
        auto dir_type = static_cast<directive_type>(kw_result->get().param1);
        switch (dir_type)
        {
            case directive_type::org:       return parse_directive_org();
            case directive_type::byte:      return parse_directive_byte();
            case directive_type::word:      return parse_directive_word();
            case directive_type::dword:     return parse_directive_dword();
            case directive_type::global_:   return parse_directive_global();
            case directive_type::extern_:   return parse_directive_extern();
            default:
                return error(panic(dir_token, "Unsupported directive."));
        }
    }
}

/* Private Methods - Directive Parsing ****************************************/

namespace g10asm
{
    auto parser::parse_directive_org () -> g10::result<statement>
    {
        // - Consume the .ORG directive token.
        auto dir_token_result = advance_token();
        if (dir_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing .ORG directive."
            );
        }

        // - Create the .ORG statement.
        const auto& dir_token = dir_token_result->get();
        statement org_stmt {};
        org_stmt.type = statement_type::directive_org;
        org_stmt.source_token = dir_token;
        org_stmt.line_number = dir_token.line_number;
        org_stmt.source_file = dir_token.source_file;

        // - Expect an address value (integer literal).
        auto addr_token_result = current_token();
        if (addr_token_result.has_value() == false ||
            addr_token_result->get().type != token_type::integer_literal)
        {
            return error(panic(
                dir_token, "The '.ORG' directive requires an address value."
            ));
        }

        advance_token();  // Consume address
        org_stmt.org_address = addr_token_result->get().int_value.value_or(0);

        return org_stmt;
    }

    auto parser::parse_directive_byte () -> g10::result<statement>
    {
        // - Consume the .BYTE directive token.
        auto dir_token_result = advance_token();
        if (dir_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing .BYTE directive."
            );
        }

        // - Create the .BYTE statement.
        const auto& dir_token = dir_token_result->get();
        statement byte_stmt {};
        byte_stmt.type = statement_type::directive_byte;
        byte_stmt.source_token = dir_token;
        byte_stmt.line_number = dir_token.line_number;
        byte_stmt.source_file = dir_token.source_file;

        // - Parse data values (integers, characters, or strings).
        do
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false)
            {
                return error(panic(
                    dir_token, "The '.BYTE' directive requires at least one data value."
                ));
            }

            const auto& tok = tok_result->get();

            if (
                tok.type == token_type::integer_literal ||
                tok.type == token_type::character_literal
            )
            {
                advance_token();
                byte_stmt.data_values.push_back(tok.int_value.value_or(0));
            }
            else if (tok.type == token_type::string_literal)
            {
                advance_token();

                // Convert string to individual bytes
                for (char c : tok.lexeme)
                {
                    // Skip the surrounding quotes
                    if (c != '"')
                    {
                        byte_stmt.data_values.push_back(
                            static_cast<std::int64_t>(static_cast<unsigned char>(c)));
                    }
                }
            }
            else
            {
                break;  // Not a data value, stop parsing
            }
        } while (match_token(token_type::comma));

        if (byte_stmt.data_values.empty())
        {
            return error(panic(
                dir_token, "The '.BYTE' directive requires at least one data value."
            ));
        }

        return byte_stmt;
    }

    auto parser::parse_directive_word () -> g10::result<statement>
    {
        // - Consume the .WORD directive token.
        auto dir_token_result = advance_token();
        if (dir_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing .WORD directive."
            );
        }

        // - Create the .WORD statement.
        const auto& dir_token = dir_token_result->get();
        statement word_stmt {};
        word_stmt.type = statement_type::directive_word;
        word_stmt.source_token = dir_token;
        word_stmt.line_number = dir_token.line_number;
        word_stmt.source_file = dir_token.source_file;

        // - Parse data values (16-bit integers).
        do
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false ||
                tok_result->get().type != token_type::integer_literal)
            {
                if (word_stmt.data_values.empty())
                {
                    return error(panic(
                        dir_token, "The '.WORD' directive requires at least one data value."
                    ));
                }
                break;
            }

            advance_token();
            word_stmt.data_values.push_back(tok_result->get().int_value.value_or(0));
        } while (match_token(token_type::comma));

        if (word_stmt.data_values.empty())
        {
            return error(panic(
                dir_token, "The '.WORD' directive requires at least one data value."
            ));
        }

        return word_stmt;
    }

    auto parser::parse_directive_dword () -> g10::result<statement>
    {
        // - Consume the .DWORD directive token.
        auto dir_token_result = advance_token();
        if (dir_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing .DWORD directive."
            );
        }

        // - Create the .DWORD statement.
        const auto& dir_token = dir_token_result->get();
        statement dword_stmt {};
        dword_stmt.type = statement_type::directive_dword;
        dword_stmt.source_token = dir_token;
        dword_stmt.line_number = dir_token.line_number;
        dword_stmt.source_file = dir_token.source_file;

        // - Parse data values (32-bit integers).
        do
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false ||
                tok_result->get().type != token_type::integer_literal)
            {
                if (dword_stmt.data_values.empty())
                {
                    return error(panic(
                        dir_token, "The '.DWORD' directive requires at least one data value."
                    ));
                }
                break;
            }

            advance_token();
            dword_stmt.data_values.push_back(tok_result->get().int_value.value_or(0));
        } while (match_token(token_type::comma));

        if (dword_stmt.data_values.empty())
        {
            return error(panic(
                dir_token, "The '.DWORD' directive requires at least one data value."
            ));
        }

        return dword_stmt;
    }

    auto parser::parse_directive_global () -> g10::result<statement>
    {
        // - Consume the .GLOBAL directive token.
        auto dir_token_result = advance_token();
        if (dir_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing .GLOBAL directive."
            );
        }

        // - Create the .GLOBAL statement.
        const auto& dir_token = dir_token_result->get();
        statement global_stmt {};
        global_stmt.type = statement_type::directive_global;
        global_stmt.source_token = dir_token;
        global_stmt.line_number = dir_token.line_number;
        global_stmt.source_file = dir_token.source_file;

        // - Parse symbol names (identifiers).
        do
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false ||
                tok_result->get().type != token_type::identifier)
            {
                if (global_stmt.symbol_names.empty())
                {
                    return error(panic(
                        dir_token, "The '.GLOBAL' directive requires at least one symbol name."
                    ));
                }
                break;
            }

            advance_token();
            global_stmt.symbol_names.emplace_back(tok_result->get().lexeme);
            
            // Track global symbol in program
            m_program.global_symbols.emplace_back(tok_result->get().lexeme);
        } while (match_token(token_type::comma));

        if (global_stmt.symbol_names.empty())
        {
            return error(panic(
                dir_token, "The '.GLOBAL' directive requires at least one symbol name."
            ));
        }

        return global_stmt;
    }

    auto parser::parse_directive_extern () -> g10::result<statement>
    {
        // - Consume the .EXTERN directive token.
        auto dir_token_result = advance_token();
        if (dir_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing .EXTERN directive."
            );
        }

        // - Create the .EXTERN statement.
        const auto& dir_token = dir_token_result->get();
        statement extern_stmt {};
        extern_stmt.type = statement_type::directive_extern;
        extern_stmt.source_token = dir_token;
        extern_stmt.line_number = dir_token.line_number;
        extern_stmt.source_file = dir_token.source_file;

        // - Parse symbol names (identifiers).
        do
        {
            auto tok_result = current_token();
            if (tok_result.has_value() == false ||
                tok_result->get().type != token_type::identifier)
            {
                if (extern_stmt.symbol_names.empty())
                {
                    return error(panic(
                        dir_token, "The '.EXTERN' directive requires at least one symbol name."
                    ));
                }
                break;
            }

            advance_token();
            extern_stmt.symbol_names.emplace_back(tok_result->get().lexeme);
            
            // Track extern symbol in program
            m_program.extern_symbols.emplace_back(tok_result->get().lexeme);
        } while (match_token(token_type::comma));

        if (extern_stmt.symbol_names.empty())
        {
            return error(panic(
                dir_token, "The '.EXTERN' directive requires at least one symbol name."
            ));
        }

        return extern_stmt;
    }
}

/* Private Methods - Operand Parsing ******************************************/

namespace g10asm
{
    auto parser::parse_operand () -> g10::result<operand>
    {
        // - Check the current token to determine the operand type.
        auto token_result = current_token();
        if (token_result.has_value() == false)
        {
            return error("Unexpected end of input while parsing operand.");
        }
        const auto& token = token_result->get();

        // - Check for either a direct or indirect memory address operand.
        if (token.type == token_type::left_bracket)
        {
            return parse_address_operand();
        }

        // - Check for a register operand.
        if (
            token.type == token_type::keyword &&
            token.kw_type == keyword_type::register_name
        )
        {
            return parse_register_operand();
        }

        // - Check for a condition code operand.
        if (
            token.type == token_type::keyword &&
            token.kw_type == keyword_type::condition_code
        )
        {
            // Condition codes are treated as immediate values for now
            // They will be resolved to actual condition values during code generation
            advance_token();
            operand op;
            op.type = operand_type::immediate_value;
            op.source_token = token;
            op.immediate_value = 0;  // Placeholder - actual value determined during code generation
            return op;
        }

        // - Check for an immediate value operand (an integer or character literal).
        if (
            token.type == token_type::integer_literal ||
            token.type == token_type::character_literal
        )
        {
            return parse_immediate_operand();
        }

        // - Check for a label reference operand (an identifier).
        if (token.type == token_type::identifier)
        {
            return parse_label_operand();
        }

        // - If we reach here, the operand is invalid.
        return error(panic(token, "Unexpected token while parsing operand."));
    }

    auto parser::parse_register_operand () -> g10::result<operand>
    {
        // - Get the register token and its keyword information.
        auto reg_token_result = advance_token();
        if (reg_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing register operand."
            );
        }

        const auto& reg_token = reg_token_result->get();
        auto kw_result = keyword_table::lookup(reg_token.lexeme);
        auto reg_type = static_cast<g10::register_type>(kw_result->get().param1);

        // - Create the register operand.
        operand reg_op {};
        reg_op.type = operand_type::register_name;
        reg_op.source_token = reg_token;
        reg_op.register_index = static_cast<std::uint8_t>(
            static_cast<std::uint8_t>(reg_type) & 0x0F
        );
        switch ((static_cast<std::uint8_t>(reg_type) >> 4) & 0x7)
        {
            case 0: reg_op.register_size = 4; break; // Double-word register
            case 1: reg_op.register_size = 2; break; // Word register
            case 2: reg_op.register_size = 1; break; // High byte register
            case 4: reg_op.register_size = 1; break; // Low byte register
            default:
                return g10::error(
                    "Invalid register size while parsing register operand."
                );
        }

        return reg_op;
    }

    auto parser::parse_immediate_operand () -> g10::result<operand>
    {
        // - Get the immediate value token.
        auto imm_token_result = advance_token();
        if (imm_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing immediate operand."
            );
        }

        const auto& imm_token = imm_token_result->get();

        // - Create the immediate value operand.
        operand imm_op {};
        imm_op.type = operand_type::immediate_value;
        imm_op.source_token = imm_token;

        // - The immediate must have an integer value.
        if (imm_token.int_value.has_value() == false)
        {
            return g10::error(
                "Invalid immediate value while parsing immediate operand."
            );
        }

        imm_op.immediate_value = imm_token.int_value.value();
        return imm_op;
    }

    auto parser::parse_indirect_operand () -> g10::result<operand>
    {
        // This function is for parsing indirect/address operands wrapped in brackets: [addr] or [reg]
        // It's called parse_address_operand in usage above
        return parse_address_operand();
    }

    auto parser::parse_address_operand () -> g10::result<operand>
    {
        // - Consume the opening bracket.
        advance_token();

        // - Check what's inside the brackets.
        auto inner_token_result = current_token();
        if (inner_token_result.has_value() == false)
        {
            return error(panic(
                "Expected register or address inside brackets."
            ));
        }

        const auto& inner_token = inner_token_result->get();
        operand op;
        op.is_indirect = true;

        // - Check for indirect register: [D0], [W1]
        if (inner_token.type == token_type::keyword &&
            inner_token.kw_type == keyword_type::register_name)
        {
            auto reg_op_result = parse_register_operand();
            if (reg_op_result.has_value() == false)
            {
                return g10::error(reg_op_result.error());
            }

            op = std::move(reg_op_result.value());
            op.type = operand_type::indirect_address;
            op.is_indirect = true;
        }
        // - Check for direct memory address: [0x8000]
        else if (inner_token.type == token_type::integer_literal)
        {
            auto imm_op_result = parse_immediate_operand();
            if (imm_op_result.has_value() == false)
            {
                return g10::error(imm_op_result.error());
            }

            op = std::move(imm_op_result.value());
            op.type = operand_type::memory_address;
            op.is_address = true;
            op.is_indirect = true;
        }
        // - Check for label reference: [label]
        else if (inner_token.type == token_type::identifier)
        {
            auto label_op_result = parse_label_operand();
            if (label_op_result.has_value() == false)
            {
                return g10::error(label_op_result.error());
            }

            op = std::move(label_op_result.value());
            op.type = operand_type::memory_address;
            op.is_address = true;
            op.is_indirect = true;
        }
        else
        {
            return error(panic(inner_token,
                "Expected register or address inside brackets."
            ));
        }

        // - Consume the closing bracket.
        auto close_result = consume_token(token_type::right_bracket,
            "Expected ']' after indirect operand.");
        if (close_result.has_value() == false)
        {
            return g10::error(close_result.error());
        }

        return op;
    }

    auto parser::parse_label_operand () -> g10::result<operand>
    {
        // - Consume the identifier token.
        auto id_token_result = advance_token();
        if (id_token_result.has_value() == false)
        {
            return g10::error(
                "Unexpected end of input while parsing label operand."
            );
        }

        const auto& id_token = id_token_result->get();

        operand op;
        op.type = operand_type::label_reference;
        op.source_token = id_token;
        // Store the label name - we'll need to look up the actual address during code generation
        // For now, store it as the immediate value (will be resolved later)
        op.immediate_value = 0;  // Placeholder - will be resolved during linking

        return op;
    }
}

/* Private Methods - Validation ***********************************************/

namespace g10asm
{
    auto parser::validate_instruction (const statement& inst_stmt)
        -> g10::result<void>
    {
        // Instruction-specific operand validation
        switch (inst_stmt.inst)
        {
            // Zero-operand instructions
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
            case g10::instruction::reti:
            case g10::instruction::rla:
            case g10::instruction::rlca:
            case g10::instruction::rra:
            case g10::instruction::rrca:
            case g10::instruction::cpl:
                return validate_operand_count(inst_stmt, 0);

            // RET instruction - can have 0 or 1 operands
            // - 0 operands: return unconditionally (condition assumed to be NC)
            // - 1 operand: condition code
            case g10::instruction::ret:
                return validate_ret(inst_stmt);

            // One-operand instructions
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
            case g10::instruction::int_:
            case g10::instruction::lsp:
            case g10::instruction::ssp:
            case g10::instruction::spo:
            case g10::instruction::spi:
                return validate_operand_count(inst_stmt, 1);

            // Jump and call instructions - can have 1 or 2 operands
            // - 1 operand: address/label (condition assumed to be NC)
            // - 2 operands: condition, address/label
            case g10::instruction::jmp:
            case g10::instruction::jp:
            case g10::instruction::jpb:
            case g10::instruction::jr:
            case g10::instruction::call:
                return validate_jump_or_call(inst_stmt);

            // Two-operand instructions
            case g10::instruction::ld:
            case g10::instruction::ldq:
            case g10::instruction::ldp:
            case g10::instruction::st:
            case g10::instruction::stq:
            case g10::instruction::stp:
            case g10::instruction::mv:
            case g10::instruction::mwh:
            case g10::instruction::mwl:
            case g10::instruction::add:
            case g10::instruction::adc:
            case g10::instruction::sub:
            case g10::instruction::sbc:
            case g10::instruction::and_:
            case g10::instruction::or_:
            case g10::instruction::xor_:
            case g10::instruction::cmp:
            case g10::instruction::cp:
                return validate_operand_count(inst_stmt, 2);

            // Three-operand instructions (bit operations)
            case g10::instruction::bit:
            case g10::instruction::set:
            case g10::instruction::res:
            case g10::instruction::tog:
                return validate_operand_count(inst_stmt, 2);  // BIT n, reg

            default:
                return g10::error("Unknown instruction for validation.");
        }
    }

    auto parser::validate_operand_count (const statement& inst_stmt,
        std::size_t expected_count) -> g10::result<void>
    {
        if (inst_stmt.operands.size() != expected_count)
        {
            return g10::error(panic(
                inst_stmt.source_token,
                "Instruction '{}' expects {} operand(s), got {}.",
                inst_stmt.source_token.lexeme,
                expected_count,
                inst_stmt.operands.size()
            ));
        }

        // Check for the following instructions:
        // - `ADD`, `ADC`, `SUB`, `SBC`, `AND`, `OR`, `XOR`, `CMP`, `CP`
        // - The first operand must be one of the accumuator registers: `D0`, `W0`, or `L0`.
        switch (inst_stmt.inst)
        {
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
                    const auto& first_op = inst_stmt.operands[0];
                    if (
                        first_op.type != operand_type::register_name ||
                        !(
                            (first_op.register_size == 4 && first_op.register_index == 0) || // D0
                            (first_op.register_size == 2 && first_op.register_index == 0) || // W0
                            (first_op.register_size == 1 && first_op.register_index == 0)    // L0
                        )
                    )
                    {
                        return g10::error(panic(
                            inst_stmt.source_token,
                            "Instruction '{}' requires the first operand to be an accumulator register (D0, W0, or L0).",
                            inst_stmt.source_token.lexeme
                        ));
                    }
                }
                break;
            default:
                break;
        }

        return {};
    }

    auto parser::validate_operand_type (const operand& op,
        operand_type expected_type) -> g10::result<void>
    {
        if (op.type != expected_type)
        {
            return g10::error(panic(
                "Expected operand of type {}, got type {}.",
                static_cast<int>(expected_type),
                static_cast<int>(op.type)
            ));
        }

        return {};
    }

    auto parser::validate_jump_or_call (const statement& inst_stmt)
        -> g10::result<void>
    {
        // Jump and call instructions can have 1 or 2 operands
        const auto operand_count = inst_stmt.operands.size();

        if (operand_count == 0 || operand_count > 2)
        {
            return g10::error(panic(
                inst_stmt.source_token,
                "Instruction '{}' expects 1 or 2 operand(s), got {}.",
                inst_stmt.source_token.lexeme,
                operand_count
            ));
        }

        // If 2 operands: first should be condition code (immediate), second is address/label
        if (operand_count == 2)
        {
            const auto& first_op = inst_stmt.operands[0];
            const auto& second_op = inst_stmt.operands[1];

            // First operand should be a condition code (stored as immediate_value)
            if (first_op.type != operand_type::immediate_value)
            {
                return g10::error(panic(
                    inst_stmt.source_token,
                    "Instruction '{}' expects first operand to be a condition code when 2 operands are provided.",
                    inst_stmt.source_token.lexeme
                ));
            }

            // Second operand should be an address, label, or immediate
            // For `JMP`, the second operand can also be a double-word register (for indirect jumps)
            if (second_op.type != operand_type::label_reference &&
                second_op.type != operand_type::immediate_value &&
                second_op.type != operand_type::memory_address &&
                !(
                    inst_stmt.inst == g10::instruction::jmp &&
                    second_op.type == operand_type::register_name &&
                    second_op.register_size == 4
                ))
            {
                return g10::error(panic(
                    inst_stmt.source_token,
                    (inst_stmt.inst == g10::instruction::jmp)
                        ? "Instruction '{}' expects second operand to be an address, label, or double-word register."
                        : "Instruction '{}' expects second operand to be an address or label.",
                    inst_stmt.source_token.lexeme
                ));
            }
        }
        // If 1 operand: should be address/label (condition is implicitly NC)
        else if (operand_count == 1)
        {
            const auto& op = inst_stmt.operands[0];

            // Operand should be an address, label, or immediate
            // For `JMP`, the operand can also be a double-word register (for indirect jumps)
            if (op.type != operand_type::label_reference &&
                op.type != operand_type::immediate_value &&
                op.type != operand_type::memory_address &&
                !(
                    inst_stmt.inst == g10::instruction::jmp &&
                    op.type == operand_type::register_name &&
                    op.register_size == 4
                ))
            {
                return g10::error(panic(
                    inst_stmt.source_token,
                    (inst_stmt.inst == g10::instruction::jmp)
                        ? "Instruction '{}' expects operand to be an address, label, or double-word register."
                        : "Instruction '{}' expects operand to be an address or label.",
                    inst_stmt.source_token.lexeme
                ));
            }
        }

        return {};
    }

    auto parser::validate_ret (const statement& inst_stmt)
        -> g10::result<void>
    {
        // RET instruction can have 0 or 1 operands
        const auto operand_count = inst_stmt.operands.size();

        if (operand_count > 1)
        {
            return g10::error(panic(
                inst_stmt.source_token,
                "Instruction 'RET' expects 0 or 1 operand(s), got {}.",
                operand_count
            ));
        }

        // If 1 operand: should be a condition code
        if (operand_count == 1)
        {
            const auto& op = inst_stmt.operands[0];

            // Operand should be a condition code (stored as immediate_value)
            if (op.type != operand_type::immediate_value)
            {
                return g10::error(panic(
                    inst_stmt.source_token,
                    "Instruction 'RET' expects operand to be a condition code."
                ));
            }
        }
        // If 0 operands: condition is implicitly NC (unconditional return)

        return {};
    }
}

/* Private Methods - Error Handling and Recovery ******************************/

namespace g10asm
{
    auto parser::synchronize () -> void
    {
        // - Exit panic mode if we are not currently in it.
        m_panic = false;

        // - Advance tokens until we reach a known synchronization point.
        while (is_at_end() == false)
        {
            // - A newline token is a natural synchronization point.
            if (match_token(token_type::end_of_line) == true)
                { return; }

            // - The end of the token stream is also a synchronization point.
            auto token_result = current_token();
            if (token_result.has_value() == false)
                { return; }

            // The following tokens start a new statement:
            // - Instruction keywords
            // - Directive keywords
            const auto& token = token_result->get();
            if (token.type == token_type::keyword)
            {
                switch (token.kw_type)
                {
                    case keyword_type::instruction:
                    case keyword_type::directive:
                        return;
                    default: break;
                }
            }

            // - Otherwise, keep advancing tokens.
            advance_token();
        }
    }
}