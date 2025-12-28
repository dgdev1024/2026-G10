/**
 * @file    g10asm/parser.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-22
 * 
 * @brief   Provides definitions for the G10 Assembler Tool's syntax analysis
 *          component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/parser.hpp>

/* Private Static Members *****************************************************/

namespace g10asm
{
    bool parser::s_good = false;
}

/* Public Methods *************************************************************/

namespace g10asm
{
    auto parser::parse (lexer& lex) -> g10::result<ast_module>
    {
        // - Reset the parser state and lexer position.
        s_good = false;
        lex.reset_position();

        // - Get the root token for, then create the AST module node.
        auto root_tk_result = lex.peek_token(0);
        ast_module module_node { root_tk_result };
        if (module_node.valid == false)
        {
            return g10::error(
                "Failed to create AST module node: '{}'",
                root_tk_result.error()
            );
        }

        // - Begin parsing the AST module.
        while (lex.is_at_end() == false)
        {
            // - Skip any newline tokens before parsing statements.
            lex.skip_tokens(token_type::new_line);

            // - Check if we've reached the end-of-file token.
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == true)
            {
                const token& peek_tk = peek_result.value();
                if (peek_tk.type == token_type::end_of_file)
                {
                    break;
                }
            }

            // - Otherwise, check again if we've somehow gone past the end.
            if (lex.is_at_end() == true)
            {
                break;
            }

            auto stmt_result = parse_statement(lex);
            if (stmt_result.has_value() == false)
            {
                std::println(
                    "Parsing error:\n"
                    "{}",
                    stmt_result.error()
                );
                return g10::error("Failed to parse statement.");
            }

            module_node.children.push_back(
                std::move(stmt_result.value())
            );
        }

        // - If we reach this point, parsing was successful.
        s_good = true;
        return module_node;
    }
}

/* Private Methods - Statements ***********************************************/

namespace g10asm
{
    auto parser::parse_statement (lexer& lex) -> g10::result_uptr<ast_node>
    {
        // - Peek at the current token to determine the statement type.
        auto current_tk_result = lex.peek_token(0);
        if (current_tk_result.has_value() == false)
        {
            return g10::error(current_tk_result.error());
        }

        const token& current_tk = current_tk_result.value();

        // - Check if the current token contains a keyword.
        if (current_tk.type == token_type::keyword)
        {
            // - Determine if the keyword is a directive or instruction.
            const keyword& kw = current_tk.keyword_value.value().get();
            if (kw.type == keyword_type::assembler_directive)
            {
                return parse_directive(lex);
            }
            else if (kw.type == keyword_type::instruction_mnemonic)
            {
                return parse_instruction(lex);
            }
        }

        // - Check if the current token is an identifier followed by a colon
        //   (label definition).
        if (current_tk.type == token_type::identifier)
        {
            auto next_tk_result = lex.peek_token(1);
            if (next_tk_result.has_value() == true)
            {
                const token& next_tk = next_tk_result.value();
                if (next_tk.type == token_type::colon)
                {
                    return parse_label_definition(lex);
                }
            }
        }

        // - Check if the current token is a variable token (starts with '$').
        //   This indicates a variable assignment statement.
        if (current_tk.type == token_type::variable)
        {
            return parse_var_assignment(lex);
        }

        // - If we reach this point, the statement type is not yet supported.
        return g10::error(
            " - Unsupported statement type starting with token '{}'.\n"
            " - In file '{}:{}:{}'",
            current_tk.lexeme,
            current_tk.source_file,
            current_tk.source_line,
            current_tk.source_column
        );
    }

    auto parser::parse_label_definition (lexer& lex) 
        -> g10::result_uptr<ast_node>
    {
        // - Peek at the identifier token for error reporting.
        auto peek_tk_result = lex.peek_token(0);
        if (peek_tk_result.has_value() == false)
        {
            return g10::error(peek_tk_result.error());
        }

        const token& peek_tk = peek_tk_result.value();

        // - Consume the identifier token (the label name).
        auto label_tk_result = lex.consume_token(
            token_type::identifier,
            " - Expected identifier for label definition.\n"
            " - In file '{}:{}:{}'",
            peek_tk.source_file,
            peek_tk.source_line,
            peek_tk.source_column
        );

        if (label_tk_result.has_value() == false)
        {
            return g10::error(label_tk_result.error());
        }

        const token& label_tk = label_tk_result.value();

        // - Consume the colon token.
        auto colon_tk_result = lex.consume_token(
            token_type::colon,
            " - Expected ':' after label identifier '{}'.\n"
            " - In file '{}:{}:{}'",
            label_tk.lexeme,
            label_tk.source_file,
            label_tk.source_line,
            label_tk.source_column
        );

        if (colon_tk_result.has_value() == false)
        {
            return g10::error(colon_tk_result.error());
        }

        // - Create the AST node for the label definition.
        auto label_def_node = std::make_unique<ast_label_definition>(label_tk);
        if (label_def_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for label definition '{}'.\n"
                " - In file '{}:{}:{}'",
                label_tk.lexeme,
                label_tk.source_file,
                label_tk.source_line,
                label_tk.source_column
            );
        }

        label_def_node->label_name = label_tk.lexeme;

        // - Return the label definition node.
        return label_def_node;
    }

    auto parser::parse_instruction (lexer& lex) 
        -> g10::result_uptr<ast_node>
    {
        // - Peek at the keyword token for error reporting.
        auto peek_tk_result = lex.peek_token(0);
        if (peek_tk_result.has_value() == false)
        {
            return g10::error(peek_tk_result.error());
        }
        const token& peek_tk = peek_tk_result.value();

        // - Consume the instruction mnemonic keyword token.
        auto instr_tk_result = lex.consume_token(
            keyword_type::instruction_mnemonic,
            " - Expected instruction mnemonic keyword.\n"
            " - In file '{}:{}:{}'",
            peek_tk.source_file,
            peek_tk.source_line,
            peek_tk.source_column
        );
        if (instr_tk_result.has_value() == false)
        {
            return g10::error(instr_tk_result.error());
        }

        const token& instr_tk = instr_tk_result.value();
        const keyword& instr_kw = instr_tk.keyword_value.value().get();

        // - Create the AST node for the instruction.
        auto instr_node = std::make_unique<ast_instruction>(instr_tk);
        if (instr_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for instruction '{}'.\n"
                " - In file '{}:{}:{}'",
                instr_tk.lexeme,
                instr_tk.source_file,
                instr_tk.source_line,
                instr_tk.source_column
            );
        }

        // - Store the instruction opcode (mnemonic enum value).
        instr_node->instruction = 
            static_cast<g10::instruction>(instr_kw.param1);

        // - Instructions can accept anywhere between zero and two operands.
        //   Parse operands until we encounter a newline or end-of-file token.
        while (true)
        {
            // - Peek at the next token to see if it's the end of the instruction.
            auto next_tk_result = lex.peek_token(0);
            if (next_tk_result.has_value() == false)
            {
                return g10::error(next_tk_result.error());
            }

            const token& next_tk = next_tk_result.value();

            // - If the next token is a newline or end-of-file, we're done.
            if (
                next_tk.type == token_type::new_line ||
                next_tk.type == token_type::end_of_file
            )
            {
                break;
            }

            // - Parse the operand.
            auto operand_result = parse_operand(lex);
            if (operand_result.has_value() == false)
            {
                return g10::error(
                    " - Failed to parse operand for instruction '{}': '{}'\n"
                    " - In file '{}:{}:{}'",
                    instr_tk.lexeme,
                    operand_result.error(),
                    instr_tk.source_file,
                    instr_tk.source_line,
                    instr_tk.source_column
                );
            }

            instr_node->operands.push_back(std::move(operand_result.value()));

            // - Peek at the next token to see if it's a comma.
            auto comma_peek_result = lex.peek_token(0);
            if (comma_peek_result.has_value() == false)
            {
                return g10::error(comma_peek_result.error());
            }

            const token& comma_peek_tk = comma_peek_result.value();

            // - If it's a comma, consume it and continue parsing operands.
            if (comma_peek_tk.type == token_type::comma)
            {
                lex.consume_token();
                continue;
            }

            // - Otherwise, we're done parsing operands.
            break;
        }

        // - Validate operand count (instructions can have 0-2 operands).
        if (instr_node->operands.size() > 2)
        {
            return g10::error(
                " - Instruction '{}' has too many operands ({}).\n"
                " - In file '{}:{}:{}'",
                instr_tk.lexeme,
                instr_node->operands.size(),
                instr_tk.source_file,
                instr_tk.source_line,
                instr_tk.source_column
            );
        }

        return instr_node;
    }
}

/* Private Methods - Directives ***********************************************/

namespace g10asm
{
    auto parser::parse_directive (lexer& lex) -> g10::result_uptr<ast_node>
    {
        // - Peek at the directive keyword token for error reporting.
        auto peek_tk_result = lex.peek_token(0);
        if (peek_tk_result.has_value() == false)
        {
            return g10::error(peek_tk_result.error());
        }

        const token& peek_tk = peek_tk_result.value();

        // - Consume the directive keyword token.
        auto dir_tk_result = lex.consume_token(
            keyword_type::assembler_directive,
            " - Expected assembler directive keyword.\n"
            " - In file '{}:{}:{}'",
            peek_tk.source_file,
            peek_tk.source_line,
            peek_tk.source_column
        );

        if (dir_tk_result.has_value() == false)
        {
            return g10::error(dir_tk_result.error());
        }

        const token& dir_tk = dir_tk_result.value();
        const keyword& dir_kw = dir_tk.keyword_value.value().get();

        // - Determine the directive type and delegate to the appropriate
        //   parsing function.
        const auto dir_type = static_cast<directive_type>(dir_kw.param1);
        switch (dir_type)
        {
            case directive_type::org:
                return parse_dir_org(lex, dir_tk);

            case directive_type::rom:
                return parse_dir_rom(lex, dir_tk);

            case directive_type::ram:
                return parse_dir_ram(lex, dir_tk);

            case directive_type::int_:
                return parse_dir_int(lex, dir_tk);

            case directive_type::byte:
                return parse_dir_byte(lex, dir_tk);

            case directive_type::word:
                return parse_dir_word(lex, dir_tk);

            case directive_type::dword:
                return parse_dir_dword(lex, dir_tk);

            case directive_type::global:
                return parse_dir_global(lex, dir_tk);

            case directive_type::extern_:
                return parse_dir_extern(lex, dir_tk);

            case directive_type::let:
                return parse_dir_let(lex, dir_tk);

            case directive_type::const_:
                return parse_dir_const(lex, dir_tk);

            default:
                return g10::error(
                    " - Unsupported directive type '{}' (type={}).\n"
                    " - In file '{}:{}:{}'",
                    dir_tk.lexeme,
                    static_cast<int>(dir_type),
                    dir_tk.source_file,
                    dir_tk.source_line,
                    dir_tk.source_column
                );
        }
    }

    auto parser::parse_dir_org (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.org` directive.
        auto org_node = std::make_unique<ast_dir_org>(dir_tk);
        if (org_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.org` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the address expression for the `.org` directive.
        auto address_expr_result = parse_expression(lex);
        if (address_expr_result.has_value() == false)
        {
            return g10::error(
                " - Failed to parse address expression for `.org` directive: '{}'\n"
                " - In file '{}:{}:{}'",
                address_expr_result.error(),
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        org_node->address_expression = std::move(address_expr_result.value());

        return org_node;
    }

    auto parser::parse_dir_rom (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.rom` directive.
        auto rom_node = std::make_unique<ast_dir_rom>(dir_tk);
        if (rom_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.rom` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return rom_node;
    }

    auto parser::parse_dir_ram (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.ram` directive.
        auto ram_node = std::make_unique<ast_dir_ram>(dir_tk);
        if (ram_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.ram` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return ram_node;
    }

    auto parser::parse_dir_int (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.int` directive.
        auto int_node = std::make_unique<ast_dir_int>(dir_tk);
        if (int_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.int` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the interrupt vector number expression.
        auto vector_result = parse_expression(lex);
        if (!vector_result.has_value())
        {
            return g10::error(
                " - Failed to parse interrupt vector number for `.int` directive: {}\n"
                " - In file '{}:{}:{}'",
                vector_result.error(),
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        int_node->vector_expression = std::move(vector_result.value());
        return int_node;
    }

    auto parser::parse_dir_byte (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.byte` directive.
        auto byte_node = std::make_unique<ast_dir_byte>(dir_tk);
        if (byte_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.byte` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the comma-separated list of byte values.
        while (true)
        {
            // - Peek at the next token to see if it's a value.
            auto value_peek_result = lex.peek_token(0);
            if (value_peek_result.has_value() == false)
            {
                return g10::error(value_peek_result.error());
            }

            const token& value_peek_tk = value_peek_result.value();

            // - If the next token is a newline or end-of-file, we're done.
            if (
                value_peek_tk.type == token_type::new_line ||
                value_peek_tk.type == token_type::end_of_file
            )
            {
                break;
            }

            // - Parse the value expression (can be string literals or numeric).
            auto value_expr_result = parse_expression(lex);
            if (value_expr_result.has_value() == false)
            {
                return g10::error(
                    " - Failed to parse value expression for `.byte` directive: '{}'\n"
                    " - In file '{}:{}:{}'",
                    value_expr_result.error(),
                    dir_tk.source_file,
                    dir_tk.source_line,
                    dir_tk.source_column
                );
            }

            byte_node->values.push_back(std::move(value_expr_result.value()));

            // - Peek at the next token to see if it's a comma.
            auto comma_peek_result = lex.peek_token(0);
            if (comma_peek_result.has_value() == false)
            {
                return g10::error(comma_peek_result.error());
            }

            const token& comma_peek_tk = comma_peek_result.value();

            // - If it's a comma, consume it and continue parsing values.
            if (comma_peek_tk.type == token_type::comma)
            {
                lex.consume_token();
                continue;
            }

            // - Otherwise, we're done parsing values.
            break;
        }

        // - Ensure at least one value was specified.
        if (byte_node->values.empty() == true)
        {
            return g10::error(
                " - `.byte` directive requires at least one value.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return byte_node;
    }

    auto parser::parse_dir_word (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.word` directive.
        auto word_node = std::make_unique<ast_dir_word>(dir_tk);
        if (word_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.word` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the comma-separated list of word values.
        while (true)
        {
            // - Peek at the next token to see if it's a value.
            auto value_peek_result = lex.peek_token(0);
            if (value_peek_result.has_value() == false)
            {
                return g10::error(value_peek_result.error());
            }

            const token& value_peek_tk = value_peek_result.value();

            // - If the next token is a newline or end-of-file, we're done.
            if (
                value_peek_tk.type == token_type::new_line ||
                value_peek_tk.type == token_type::end_of_file
            )
            {
                break;
            }

            // - Parse the value expression.
            auto value_expr_result = parse_expression(lex);
            if (value_expr_result.has_value() == false)
            {
                return g10::error(
                    " - Failed to parse value expression for `.word` directive: '{}'\n"
                    " - In file '{}:{}:{}'",
                    value_expr_result.error(),
                    dir_tk.source_file,
                    dir_tk.source_line,
                    dir_tk.source_column
                );
            }

            word_node->values.push_back(std::move(value_expr_result.value()));

            // - Peek at the next token to see if it's a comma.
            auto comma_peek_result = lex.peek_token(0);
            if (comma_peek_result.has_value() == false)
            {
                return g10::error(comma_peek_result.error());
            }

            const token& comma_peek_tk = comma_peek_result.value();

            // - If it's a comma, consume it and continue parsing values.
            if (comma_peek_tk.type == token_type::comma)
            {
                lex.consume_token();
                continue;
            }

            // - Otherwise, we're done parsing values.
            break;
        }

        // - Ensure at least one value was specified.
        if (word_node->values.empty() == true)
        {
            return g10::error(
                " - `.word` directive requires at least one value.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return word_node;
    }

    auto parser::parse_dir_dword (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.dword` directive.
        auto dword_node = std::make_unique<ast_dir_dword>(dir_tk);
        if (dword_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.dword` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the comma-separated list of dword values.
        while (true)
        {
            // - Peek at the next token to see if it's a value.
            auto value_peek_result = lex.peek_token(0);
            if (value_peek_result.has_value() == false)
            {
                return g10::error(value_peek_result.error());
            }

            const token& value_peek_tk = value_peek_result.value();

            // - If the next token is a newline or end-of-file, we're done.
            if (
                value_peek_tk.type == token_type::new_line ||
                value_peek_tk.type == token_type::end_of_file
            )
            {
                break;
            }

            // - Parse the value expression.
            auto value_expr_result = parse_expression(lex);
            if (value_expr_result.has_value() == false)
            {
                return g10::error(
                    " - Failed to parse value expression for `.dword` directive: '{}'\n"
                    " - In file '{}:{}:{}'",
                    value_expr_result.error(),
                    dir_tk.source_file,
                    dir_tk.source_line,
                    dir_tk.source_column
                );
            }

            dword_node->values.push_back(std::move(value_expr_result.value()));

            // - Peek at the next token to see if it's a comma.
            auto comma_peek_result = lex.peek_token(0);
            if (comma_peek_result.has_value() == false)
            {
                return g10::error(comma_peek_result.error());
            }

            const token& comma_peek_tk = comma_peek_result.value();

            // - If it's a comma, consume it and continue parsing values.
            if (comma_peek_tk.type == token_type::comma)
            {
                lex.consume_token();
                continue;
            }

            // - Otherwise, we're done parsing values.
            break;
        }

        // - Ensure at least one value was specified.
        if (dword_node->values.empty() == true)
        {
            return g10::error(
                " - `.dword` directive requires at least one value.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return dword_node;
    }

    auto parser::parse_dir_global (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.global` directive.
        auto global_node = std::make_unique<ast_dir_global>(dir_tk);
        if (global_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.global` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the comma-separated list of symbol identifiers.
        while (true)
        {
            // - Peek at the next token to see if it's an identifier.
            auto symbol_peek_result = lex.peek_token(0);
            if (symbol_peek_result.has_value() == false)
            {
                return g10::error(symbol_peek_result.error());
            }

            const token& symbol_peek_tk = symbol_peek_result.value();

            // - If the next token is a newline or end-of-file, we're done.
            if (
                symbol_peek_tk.type == token_type::new_line ||
                symbol_peek_tk.type == token_type::end_of_file
            )
            {
                break;
            }

            // - Consume the identifier token.
            auto symbol_tk_result = lex.consume_token(
                token_type::identifier,
                " - Expected identifier for symbol in `.global` directive.\n"
                " - In file '{}:{}:{}'",
                symbol_peek_tk.source_file,
                symbol_peek_tk.source_line,
                symbol_peek_tk.source_column
            );

            if (symbol_tk_result.has_value() == false)
            {
                return g10::error(symbol_tk_result.error());
            }

            const token& symbol_tk = symbol_tk_result.value();
            global_node->symbols.push_back(symbol_tk.lexeme);

            // - Peek at the next token to see if it's a comma.
            auto comma_peek_result = lex.peek_token(0);
            if (comma_peek_result.has_value() == false)
            {
                return g10::error(comma_peek_result.error());
            }

            const token& comma_peek_tk = comma_peek_result.value();

            // - If it's a comma, consume it and continue parsing symbols.
            if (comma_peek_tk.type == token_type::comma)
            {
                lex.consume_token();
                continue;
            }

            // - Otherwise, we're done parsing symbols.
            break;
        }

        // - Ensure at least one symbol was specified.
        if (global_node->symbols.empty() == true)
        {
            return g10::error(
                " - `.global` directive requires at least one symbol.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return global_node;
    }

    auto parser::parse_dir_extern (lexer& lex, const token& dir_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.extern` directive.
        auto extern_node = std::make_unique<ast_dir_extern>(dir_tk);
        if (extern_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.extern` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Parse the comma-separated list of symbol identifiers.
        while (true)
        {
            // - Peek at the next token to see if it's an identifier.
            auto symbol_peek_result = lex.peek_token(0);
            if (symbol_peek_result.has_value() == false)
            {
                return g10::error(symbol_peek_result.error());
            }

            const token& symbol_peek_tk = symbol_peek_result.value();

            // - If the next token is a newline or end-of-file, we're done.
            if (
                symbol_peek_tk.type == token_type::new_line ||
                symbol_peek_tk.type == token_type::end_of_file
            )
            {
                break;
            }

            // - Consume the identifier token.
            auto symbol_tk_result = lex.consume_token(
                token_type::identifier,
                " - Expected identifier for symbol in `.extern` directive.\n"
                " - In file '{}:{}:{}'",
                symbol_peek_tk.source_file,
                symbol_peek_tk.source_line,
                symbol_peek_tk.source_column
            );

            if (symbol_tk_result.has_value() == false)
            {
                return g10::error(symbol_tk_result.error());
            }

            const token& symbol_tk = symbol_tk_result.value();
            extern_node->symbols.push_back(symbol_tk.lexeme);

            // - Peek at the next token to see if it's a comma.
            auto comma_peek_result = lex.peek_token(0);
            if (comma_peek_result.has_value() == false)
            {
                return g10::error(comma_peek_result.error());
            }

            const token& comma_peek_tk = comma_peek_result.value();

            // - If it's a comma, consume it and continue parsing symbols.
            if (comma_peek_tk.type == token_type::comma)
            {
                lex.consume_token();
                continue;
            }

            // - Otherwise, we're done parsing symbols.
            break;
        }

        // - Ensure at least one symbol was specified.
        if (extern_node->symbols.empty() == true)
        {
            return g10::error(
                " - `.extern` directive requires at least one symbol.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        return extern_node;  
    }

    auto parser::parse_dir_let (lexer& lex, const token& dir_tk)
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.let` directive.
        auto let_node = std::make_unique<ast_dir_let>(dir_tk);
        if (let_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.let` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Peek at the next token, which should be a variable token.
        auto var_peek_result = lex.peek_token(0);
        if (var_peek_result.has_value() == false)
        {
            return g10::error(var_peek_result.error());
        }

        const token& var_peek_tk = var_peek_result.value();

        // - Consume the variable token (must start with '$').
        auto var_tk_result = lex.consume_token(
            token_type::variable,
            " - Expected variable name (starting with '$') after `.let`.\n"
            " - In file '{}:{}:{}'",
            var_peek_tk.source_file,
            var_peek_tk.source_line,
            var_peek_tk.source_column
        );

        if (var_tk_result.has_value() == false)
        {
            return g10::error(var_tk_result.error());
        }

        const token& var_tk = var_tk_result.value();

        // - Store the variable name (without the '$' prefix).
        let_node->variable_name = var_tk.lexeme.substr(1);

        // - Consume the '=' assignment operator.
        auto assign_tk_result = lex.consume_token(
            token_type::assign_equal,
            " - Expected '=' after variable name in `.let` directive.\n"
            " - In file '{}:{}:{}'",
            var_tk.source_file,
            var_tk.source_line,
            var_tk.source_column
        );

        if (assign_tk_result.has_value() == false)
        {
            return g10::error(assign_tk_result.error());
        }

        // - Parse the initialization expression.
        auto init_expr_result = parse_expression(lex);
        if (init_expr_result.has_value() == false)
        {
            return g10::error(
                " - Failed to parse initialization expression for `.let` directive: '{}'\n"
                " - In file '{}:{}:{}'",
                init_expr_result.error(),
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        let_node->init_expression = std::move(init_expr_result.value());

        return let_node;
    }

    auto parser::parse_dir_const (lexer& lex, const token& dir_tk)
        -> g10::result_uptr<ast_node>
    {
        // - Create the AST node for the `.const` directive.
        auto const_node = std::make_unique<ast_dir_const>(dir_tk);
        if (const_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for `.const` directive.\n"
                " - In file '{}:{}:{}'",
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        // - Peek at the next token, which should be a variable token.
        auto var_peek_result = lex.peek_token(0);
        if (var_peek_result.has_value() == false)
        {
            return g10::error(var_peek_result.error());
        }

        const token& var_peek_tk = var_peek_result.value();

        // - Consume the variable token (must start with '$').
        auto var_tk_result = lex.consume_token(
            token_type::variable,
            " - Expected constant name (starting with '$') after `.const`.\n"
            " - In file '{}:{}:{}'",
            var_peek_tk.source_file,
            var_peek_tk.source_line,
            var_peek_tk.source_column
        );

        if (var_tk_result.has_value() == false)
        {
            return g10::error(var_tk_result.error());
        }

        const token& var_tk = var_tk_result.value();

        // - Store the constant name (without the '$' prefix).
        const_node->constant_name = var_tk.lexeme.substr(1);

        // - Consume the '=' assignment operator.
        auto assign_tk_result = lex.consume_token(
            token_type::assign_equal,
            " - Expected '=' after constant name in `.const` directive.\n"
            " - In file '{}:{}:{}'",
            var_tk.source_file,
            var_tk.source_line,
            var_tk.source_column
        );

        if (assign_tk_result.has_value() == false)
        {
            return g10::error(assign_tk_result.error());
        }

        // - Parse the value expression.
        auto value_expr_result = parse_expression(lex);
        if (value_expr_result.has_value() == false)
        {
            return g10::error(
                " - Failed to parse value expression for `.const` directive: '{}'\n"
                " - In file '{}:{}:{}'",
                value_expr_result.error(),
                dir_tk.source_file,
                dir_tk.source_line,
                dir_tk.source_column
            );
        }

        const_node->value_expression = std::move(value_expr_result.value());

        return const_node;
    }

    auto parser::parse_var_assignment (lexer& lex)
        -> g10::result_uptr<ast_node>
    {
        // - Peek at the variable token for error reporting.
        auto var_peek_result = lex.peek_token(0);
        if (var_peek_result.has_value() == false)
        {
            return g10::error(var_peek_result.error());
        }

        const token& var_peek_tk = var_peek_result.value();

        // - Consume the variable token.
        auto var_tk_result = lex.consume_token(
            token_type::variable,
            " - Expected variable name (starting with '$') for assignment.\n"
            " - In file '{}:{}:{}'",
            var_peek_tk.source_file,
            var_peek_tk.source_line,
            var_peek_tk.source_column
        );

        if (var_tk_result.has_value() == false)
        {
            return g10::error(var_tk_result.error());
        }

        const token& var_tk = var_tk_result.value();

        // - Create the AST node for the variable assignment statement.
        auto assign_node = std::make_unique<ast_stmt_var_assignment>(var_tk);
        if (assign_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for variable assignment.\n"
                " - In file '{}:{}:{}'",
                var_tk.source_file,
                var_tk.source_line,
                var_tk.source_column
            );
        }

        // - Store the variable name (without the '$' prefix).
        assign_node->variable_name = var_tk.lexeme.substr(1);

        // - Peek at the next token, which should be an assignment operator.
        auto op_peek_result = lex.peek_token(0);
        if (op_peek_result.has_value() == false)
        {
            return g10::error(op_peek_result.error());
        }

        const token& op_peek_tk = op_peek_result.value();

        // - Check that the next token is an assignment operator.
        bool is_assignment_op = false;
        switch (op_peek_tk.type)
        {
            case token_type::assign_equal:
            case token_type::assign_plus:
            case token_type::assign_minus:
            case token_type::assign_times:
            case token_type::assign_exponent:
            case token_type::assign_divide:
            case token_type::assign_modulo:
            case token_type::assign_and:
            case token_type::assign_or:
            case token_type::assign_xor:
            case token_type::assign_shift_left:
            case token_type::assign_shift_right:
                is_assignment_op = true;
                break;
            default:
                break;
        }

        if (is_assignment_op == false)
        {
            return g10::error(
                " - Expected assignment operator (=, +=, -=, *=, etc.) after variable '${}'. Found '{}'.\n"
                " - In file '{}:{}:{}'",
                assign_node->variable_name,
                op_peek_tk.lexeme,
                op_peek_tk.source_file,
                op_peek_tk.source_line,
                op_peek_tk.source_column
            );
        }

        // - Consume the assignment operator.
        auto op_tk_result = lex.consume_token();
        if (op_tk_result.has_value() == false)
        {
            return g10::error(op_tk_result.error());
        }

        const token& op_tk = op_tk_result.value();
        assign_node->assignment_operator = op_tk.type;

        // - Parse the value expression.
        auto value_expr_result = parse_expression(lex);
        if (value_expr_result.has_value() == false)
        {
            return g10::error(
                " - Failed to parse value expression for variable assignment: '{}'\n"
                " - In file '{}:{}:{}'",
                value_expr_result.error(),
                var_tk.source_file,
                var_tk.source_line,
                var_tk.source_column
            );
        }

        assign_node->value_expression = std::move(value_expr_result.value());

        return assign_node;
    }
}

/* Private Methods - Operands *************************************************/

namespace g10asm
{
    auto parser::parse_operand (lexer& lex) -> g10::result_uptr<ast_node>
    {
        // - Peek the next token to determine the operand type.
        auto operand_tk_result = lex.peek_token(0);
        if (operand_tk_result.has_value() == false)
        {
            return g10::error(operand_tk_result.error());
        }
        const token& operand_tk = operand_tk_result.value();
        const auto& operand_kw = operand_tk.keyword_value;

        // - Determine the operand type based on the token type.
        if (operand_kw.has_value() == true)
        {
            // - Consume the operand token.
            lex.consume_token();

            // - The operand is a keyword-based operand (register, etc.).
            //   Determine the specific keyword type.
            switch (operand_kw.value().get().type)
            {
                case keyword_type::register_name:
                {
                    auto register_node = 
                        std::make_unique<ast_opr_register>(operand_tk);
                    register_node->reg = 
                        static_cast<g10::register_type>(
                            operand_kw.value().get().param1
                        );
                    return register_node;
                }

                case keyword_type::branching_condition:
                {
                    auto condition_node = 
                        std::make_unique<ast_opr_condition>(operand_tk);
                    condition_node->condition = 
                        static_cast<g10::condition_code>(
                            operand_kw.value().get().param1
                        );
                    return condition_node;
                }

                default:
                    return g10::error(
                        " - Unsupported keyword type '{}' ('{}') for operand.\n"
                        " - In file '{}:{}:{}'",
                        operand_tk.type_to_string(),
                        operand_tk.lexeme,
                        operand_tk.source_file,
                        operand_tk.source_line,
                        operand_tk.source_column
                    );
            }
        }
        else
        {
            // - The operand is not a keyword-based operand. Check if it's
            //   a direct memory operand or an immediate operand.
            if (operand_tk.type == token_type::left_bracket)
            {
                return parse_opr_direct(lex);
            }
            else
            {
                return parse_opr_immediate(lex);
            }
        }
    }

    auto parser::parse_opr_immediate (lexer& lex) 
        -> g10::result_uptr<ast_node>
    {
        // - Peek at the first token for error reporting.
        auto peek_tk_result = lex.peek_token(0);
        if (peek_tk_result.has_value() == false)
        {
            return g10::error(peek_tk_result.error());
        }

        const token& peek_tk = peek_tk_result.value();

        // - Parse the expression representing the immediate value.
        auto expr_result = parse_expression(lex);
        if (expr_result.has_value() == false)
        {
            return g10::error(
                " - Failed to parse immediate operand expression: '{}'\n"
                " - In file '{}:{}:{}'",
                expr_result.error(),
                peek_tk.source_file,
                peek_tk.source_line,
                peek_tk.source_column
            );
        }

        // - Create the AST node for the immediate operand.
        auto immediate_node = std::make_unique<ast_opr_immediate>(peek_tk);
        if (immediate_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for immediate operand.\n"
                " - In file '{}:{}:{}'",
                peek_tk.source_file,
                peek_tk.source_line,
                peek_tk.source_column
            );
        }

        immediate_node->value = std::move(expr_result.value());
        return immediate_node;
    }

    auto parser::parse_opr_direct (lexer& lex) 
        -> g10::result_uptr<ast_node>
    {
        // - Peek at the opening bracket for error reporting.
        auto bracket_peek_result = lex.peek_token(0);
        if (bracket_peek_result.has_value() == false)
        {
            return g10::error(bracket_peek_result.error());
        }

        const token& bracket_peek_tk = bracket_peek_result.value();

        // - Consume the opening left bracket.
        auto bracket_tk_result = lex.consume_token(
            token_type::left_bracket,
            " - Expected '[' for memory operand.\n"
            " - In file '{}:{}:{}'",
            bracket_peek_tk.source_file,
            bracket_peek_tk.source_line,
            bracket_peek_tk.source_column
        );

        if (bracket_tk_result.has_value() == false)
        {
            return g10::error(bracket_tk_result.error());
        }

        const token& bracket_tk = bracket_tk_result.value();

        // - Peek at the next token to determine if this is a direct or
        //   indirect memory operand.
        auto next_tk_result = lex.peek_token(0);
        if (next_tk_result.has_value() == false)
        {
            return g10::error(next_tk_result.error());
        }

        const token& next_tk = next_tk_result.value();

        // - If the next token is a register keyword, this is an indirect
        //   memory operand. Delegate to parse_opr_indirect.
        if (next_tk.type == token_type::keyword &&
            next_tk.keyword_value.has_value() == true &&
            next_tk.keyword_value.value().get().type == keyword_type::register_name)
        {
            return parse_opr_indirect(lex, bracket_tk);
        }

        // - Otherwise, parse the expression representing the memory address.
        auto expr_result = parse_expression(lex);
        if (expr_result.has_value() == false)
        {
            return g10::error(
                " - Failed to parse direct memory operand expression: '{}'\n"
                " - In file '{}:{}:{}'",
                expr_result.error(),
                bracket_tk.source_file,
                bracket_tk.source_line,
                bracket_tk.source_column
            );
        }

        // - Consume the closing right bracket.
        auto close_bracket_result = lex.consume_token(
            token_type::right_bracket,
            " - Expected ']' after direct memory operand expression.\n"
            " - In file '{}:{}:{}'",
            bracket_tk.source_file,
            bracket_tk.source_line,
            bracket_tk.source_column
        );

        if (close_bracket_result.has_value() == false)
        {
            return g10::error(close_bracket_result.error());
        }

        // - Create the AST node for the direct memory operand.
        auto direct_node = std::make_unique<ast_opr_direct>(bracket_tk);
        if (direct_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for direct memory operand.\n"
                " - In file '{}:{}:{}'",
                bracket_tk.source_file,
                bracket_tk.source_line,
                bracket_tk.source_column
            );
        }

        direct_node->address = std::move(expr_result.value());
        return direct_node;
    }

    auto parser::parse_opr_indirect (lexer& lex, const token& bracket_tk) 
        -> g10::result_uptr<ast_node>
    {
        // - Consume the register token.
        auto reg_tk_result = lex.consume_token(
            keyword_type::register_name,
            " - Expected register for indirect memory operand.\n"
            " - In file '{}:{}:{}'",
            bracket_tk.source_file,
            bracket_tk.source_line,
            bracket_tk.source_column
        );

        if (reg_tk_result.has_value() == false)
        {
            return g10::error(reg_tk_result.error());
        }

        const token& reg_tk = reg_tk_result.value();

        // - Consume the closing right bracket.
        auto close_bracket_result = lex.consume_token(
            token_type::right_bracket,
            " - Expected ']' after indirect memory operand register.\n"
            " - In file '{}:{}:{}'",
            bracket_tk.source_file,
            bracket_tk.source_line,
            bracket_tk.source_column
        );

        if (close_bracket_result.has_value() == false)
        {
            return g10::error(close_bracket_result.error());
        }

        // - Create the AST node for the indirect memory operand using
        //   the register token (not the bracket token).
        auto indirect_node = std::make_unique<ast_opr_indirect>(reg_tk);
        if (indirect_node->valid == false)
        {
            return g10::error(
                " - Failed to create AST node for indirect memory operand.\n"
                " - In file '{}:{}:{}'",
                reg_tk.source_file,
                reg_tk.source_line,
                reg_tk.source_column
            );
        }

        // - Store the base register.
        indirect_node->base_register = 
            static_cast<g10::register_type>(
                reg_tk.keyword_value.value().get().param1
            );

        return indirect_node;
    }
}

/* Private Methods - Expressions **********************************************/

namespace g10asm
{
    /**
     * @brief   Entry point for expression parsing.
     * 
     * This method delegates to the lowest-precedence binary operator parser,
     * which chains upward through the precedence hierarchy. The precedence
     * order (lowest to highest) is:
     * 
     *   1. Bitwise OR (`|`)
     *   2. Bitwise XOR (`^`)
     *   3. Bitwise AND (`&`)
     *   4. Shift (`<<`, `>>`)
     *   5. Additive (`+`, `-`)
     *   6. Multiplicative (`*`, `/`, `%`)
     *   7. Exponentiation (`**`) - right-associative
     *   8. Unary (`-`, `~`, `!`)
     *   9. Primary (literals, identifiers, grouped expressions)
     */
    auto parser::parse_expression (lexer& lex) 
        -> g10::result_uptr<ast_expression>
    {
        return parse_bitwise_or_expression(lex);
    }

    /**
     * @brief   Parses bitwise OR expressions (`|`).
     * 
     * This is the lowest-precedence binary operator. The grammar is:
     *   bitwise_or_expr := bitwise_xor_expr ( '|' bitwise_xor_expr )*
     * 
     * Left-associative: `a | b | c` parses as `(a | b) | c`.
     */
    auto parser::parse_bitwise_or_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the left operand (higher precedence expression).
        auto left_result = parse_bitwise_xor_expression(lex);
        if (left_result.has_value() == false)
        {
            return g10::error(left_result.error());
        }

        auto left = std::move(left_result.value());

        // - While we see a bitwise OR operator, consume it and parse the
        //   right operand, building a left-associative binary expression tree.
        while (true)
        {
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == false)
            {
                break;
            }

            const token& op_tk = peek_result.value();
            if (op_tk.type != token_type::bitwise_or)
            {
                break;
            }

            // - Consume the operator token.
            lex.skip_tokens(1);

            // - Parse the right operand.
            auto right_result = parse_bitwise_xor_expression(lex);
            if (right_result.has_value() == false)
            {
                return g10::error(right_result.error());
            }

            // - Create the binary expression node.
            auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
            binary_node->operator_type = op_tk.type;
            binary_node->left_operand = std::move(left);
            binary_node->right_operand = std::move(right_result.value());

            // - The new binary node becomes the left operand for the next
            //   iteration (left-associativity).
            left = std::move(binary_node);
        }

        return left;
    }

    /**
     * @brief   Parses bitwise XOR expressions (`^`).
     * 
     * Higher precedence than bitwise OR. The grammar is:
     *   bitwise_xor_expr := bitwise_and_expr ( '^' bitwise_and_expr )*
     * 
     * Left-associative: `a ^ b ^ c` parses as `(a ^ b) ^ c`.
     */
    auto parser::parse_bitwise_xor_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the left operand (higher precedence expression).
        auto left_result = parse_bitwise_and_expression(lex);
        if (left_result.has_value() == false)
        {
            return g10::error(left_result.error());
        }

        auto left = std::move(left_result.value());

        // - While we see a bitwise XOR operator, consume it and parse the
        //   right operand.
        while (true)
        {
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == false)
            {
                break;
            }

            const token& op_tk = peek_result.value();
            if (op_tk.type != token_type::bitwise_xor)
            {
                break;
            }

            // - Consume the operator token.
            lex.skip_tokens(1);

            // - Parse the right operand.
            auto right_result = parse_bitwise_and_expression(lex);
            if (right_result.has_value() == false)
            {
                return g10::error(right_result.error());
            }

            // - Create the binary expression node.
            auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
            binary_node->operator_type = op_tk.type;
            binary_node->left_operand = std::move(left);
            binary_node->right_operand = std::move(right_result.value());

            left = std::move(binary_node);
        }

        return left;
    }

    /**
     * @brief   Parses bitwise AND expressions (`&`).
     * 
     * Higher precedence than bitwise XOR. The grammar is:
     *   bitwise_and_expr := shift_expr ( '&' shift_expr )*
     * 
     * Left-associative: `a & b & c` parses as `(a & b) & c`.
     */
    auto parser::parse_bitwise_and_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the left operand (higher precedence expression).
        auto left_result = parse_shift_expression(lex);
        if (left_result.has_value() == false)
        {
            return g10::error(left_result.error());
        }

        auto left = std::move(left_result.value());

        // - While we see a bitwise AND operator, consume it and parse the
        //   right operand.
        while (true)
        {
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == false)
            {
                break;
            }

            const token& op_tk = peek_result.value();
            if (op_tk.type != token_type::bitwise_and)
            {
                break;
            }

            // - Consume the operator token.
            lex.skip_tokens(1);

            // - Parse the right operand.
            auto right_result = parse_shift_expression(lex);
            if (right_result.has_value() == false)
            {
                return g10::error(right_result.error());
            }

            // - Create the binary expression node.
            auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
            binary_node->operator_type = op_tk.type;
            binary_node->left_operand = std::move(left);
            binary_node->right_operand = std::move(right_result.value());

            left = std::move(binary_node);
        }

        return left;
    }

    /**
     * @brief   Parses shift expressions (`<<`, `>>`).
     * 
     * Higher precedence than bitwise AND. The grammar is:
     *   shift_expr := additive_expr ( ('<<' | '>>') additive_expr )*
     * 
     * Left-associative: `a << b >> c` parses as `(a << b) >> c`.
     */
    auto parser::parse_shift_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the left operand (higher precedence expression).
        auto left_result = parse_additive_expression(lex);
        if (left_result.has_value() == false)
        {
            return g10::error(left_result.error());
        }

        auto left = std::move(left_result.value());

        // - While we see a shift operator, consume it and parse the
        //   right operand.
        while (true)
        {
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == false)
            {
                break;
            }

            const token& op_tk = peek_result.value();
            if (op_tk.type != token_type::bitwise_shift_left &&
                op_tk.type != token_type::bitwise_shift_right)
            {
                break;
            }

            // - Consume the operator token.
            lex.skip_tokens(1);

            // - Parse the right operand.
            auto right_result = parse_additive_expression(lex);
            if (right_result.has_value() == false)
            {
                return g10::error(right_result.error());
            }

            // - Create the binary expression node.
            auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
            binary_node->operator_type = op_tk.type;
            binary_node->left_operand = std::move(left);
            binary_node->right_operand = std::move(right_result.value());

            left = std::move(binary_node);
        }

        return left;
    }

    /**
     * @brief   Parses additive expressions (`+`, `-`).
     * 
     * Higher precedence than shift operators. The grammar is:
     *   additive_expr := multiplicative_expr ( ('+' | '-') multiplicative_expr )*
     * 
     * Left-associative: `a + b - c` parses as `(a + b) - c`.
     */
    auto parser::parse_additive_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the left operand (higher precedence expression).
        auto left_result = parse_multiplicative_expression(lex);
        if (left_result.has_value() == false)
        {
            return g10::error(left_result.error());
        }

        auto left = std::move(left_result.value());

        // - While we see an additive operator, consume it and parse the
        //   right operand.
        while (true)
        {
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == false)
            {
                break;
            }

            const token& op_tk = peek_result.value();
            if (op_tk.type != token_type::plus &&
                op_tk.type != token_type::minus)
            {
                break;
            }

            // - Consume the operator token.
            lex.skip_tokens(1);

            // - Parse the right operand.
            auto right_result = parse_multiplicative_expression(lex);
            if (right_result.has_value() == false)
            {
                return g10::error(right_result.error());
            }

            // - Create the binary expression node.
            auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
            binary_node->operator_type = op_tk.type;
            binary_node->left_operand = std::move(left);
            binary_node->right_operand = std::move(right_result.value());

            left = std::move(binary_node);
        }

        return left;
    }

    /**
     * @brief   Parses multiplicative expressions (`*`, `/`, `%`).
     * 
     * Higher precedence than additive operators. The grammar is:
     *   multiplicative_expr := exponent_expr ( ('*' | '/' | '%') exponent_expr )*
     * 
     * Left-associative: `a * b / c` parses as `(a * b) / c`.
     */
    auto parser::parse_multiplicative_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the left operand (higher precedence expression).
        auto left_result = parse_exponent_expression(lex);
        if (left_result.has_value() == false)
        {
            return g10::error(left_result.error());
        }

        auto left = std::move(left_result.value());

        // - While we see a multiplicative operator, consume it and parse the
        //   right operand.
        while (true)
        {
            auto peek_result = lex.peek_token(0);
            if (peek_result.has_value() == false)
            {
                break;
            }

            const token& op_tk = peek_result.value();
            if (op_tk.type != token_type::times &&
                op_tk.type != token_type::divide &&
                op_tk.type != token_type::modulo)
            {
                break;
            }

            // - Consume the operator token.
            lex.skip_tokens(1);

            // - Parse the right operand.
            auto right_result = parse_exponent_expression(lex);
            if (right_result.has_value() == false)
            {
                return g10::error(right_result.error());
            }

            // - Create the binary expression node.
            auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
            binary_node->operator_type = op_tk.type;
            binary_node->left_operand = std::move(left);
            binary_node->right_operand = std::move(right_result.value());

            left = std::move(binary_node);
        }

        return left;
    }

    /**
     * @brief   Parses exponentiation expressions (`**`).
     * 
     * Higher precedence than multiplicative operators. The grammar is:
     *   exponent_expr := unary_expr ( '**' exponent_expr )?
     * 
     * Right-associative: `2 ** 3 ** 4` parses as `2 ** (3 ** 4)`.
     * This is achieved by recursively calling parse_exponent_expression
     * for the right operand instead of a higher-precedence parser.
     */
    auto parser::parse_exponent_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Parse the base (left operand) as a unary expression.
        auto base_result = parse_unary_expression(lex);
        if (base_result.has_value() == false)
        {
            return g10::error(base_result.error());
        }

        // - Check if the next token is an exponentiation operator.
        auto peek_result = lex.peek_token(0);
        if (peek_result.has_value() == false)
        {
            return base_result;
        }

        const token& op_tk = peek_result.value();
        if (op_tk.type != token_type::exponent)
        {
            return base_result;
        }

        // - Consume the exponentiation operator.
        lex.skip_tokens(1);

        // - Recursively parse the exponent (right operand) as another
        //   exponent expression to achieve right-associativity.
        auto exponent_result = parse_exponent_expression(lex);
        if (exponent_result.has_value() == false)
        {
            return g10::error(exponent_result.error());
        }

        // - Create the binary expression node.
        auto binary_node = std::make_unique<ast_expr_binary>(op_tk);
        binary_node->operator_type = op_tk.type;
        binary_node->left_operand = std::move(base_result.value());
        binary_node->right_operand = std::move(exponent_result.value());

        return binary_node;
    }

    /**
     * @brief   Parses unary expressions (`-`, `~`, `!`).
     * 
     * Unary operators have the highest precedence among operators. The grammar is:
     *   unary_expr := ('-' | '~' | '!') unary_expr | primary_expr
     * 
     * Unary expressions can be nested: `--x` parses as `-(-x)`.
     * 
     * Supported unary operators:
     * - `-` : Arithmetic negation (e.g., `-5`, `-label`)
     * - `~` : Bitwise NOT/complement (e.g., `~0xFF`, `~mask`)
     * - `!` : Logical NOT (e.g., `!flag`)
     */
    auto parser::parse_unary_expression (lexer& lex)
        -> g10::result_uptr<ast_expression>
    {
        // - Peek at the current token to check for unary operators.
        auto peek_result = lex.peek_token(0);
        if (peek_result.has_value() == false)
        {
            return g10::error(peek_result.error());
        }

        const token& op_tk = peek_result.value();

        // - Check if this is a unary operator.
        if (op_tk.type == token_type::minus ||
            op_tk.type == token_type::bitwise_not ||
            op_tk.type == token_type::logical_not)
        {
            // - Consume the unary operator token.
            lex.skip_tokens(1);

            // - Recursively parse the operand as another unary expression.
            //   This allows for nested unary operators like `--x` or `~~y`.
            auto operand_result = parse_unary_expression(lex);
            if (operand_result.has_value() == false)
            {
                return g10::error(operand_result.error());
            }

            // - Create the unary expression node.
            auto unary_node = std::make_unique<ast_expr_unary>(op_tk);
            unary_node->operator_type = op_tk.type;
            unary_node->operand = std::move(operand_result.value());

            return unary_node;
        }

        // - If no unary operator is found, parse a primary expression.
        return parse_primary_expression(lex);
    }

    /**
     * @brief   Parses primary expressions (literals, identifiers, grouped).
     * 
     * Primary expressions are the atomic building blocks of all expressions.
     * The grammar is:
     *   primary_expr := INTEGER | NUMBER | CHAR | STRING | IDENTIFIER |
     *                   VARIABLE | PLACEHOLDER | '(' expression ')'
     */
    auto parser::parse_primary_expression (lexer& lex) 
        -> g10::result_uptr<ast_expression>
    {
        // - Consume the next token.
        auto primary_tk_result = lex.consume_token();
        if (primary_tk_result.has_value() == false)
        {
            return g10::error(primary_tk_result.error());
        }
        const token& primary_tk = primary_tk_result.value();

        // - Create the AST node for the primary expression.
        auto primary_node = std::make_unique<ast_expr_primary>(primary_tk);

        // - Determine the type of primary expression based on the token type.
        switch (primary_tk.type)
        {
            case token_type::integer_literal:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::integer_literal;
                primary_node->value = primary_tk.int_value.value();
                break;

            case token_type::number_literal:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::number_literal;
                primary_node->value = primary_tk.number_value.value();
                break;

            case token_type::character_literal:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::char_literal;
                primary_node->value = 
                    static_cast<char>(primary_tk.int_value.value());
                break;

            case token_type::string_literal:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::string_literal;
                primary_node->value = primary_tk.lexeme;
                break;

            case token_type::identifier:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::identifier;
                primary_node->value = primary_tk.lexeme;
                break;

            case token_type::variable:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::variable;
                primary_node->value = primary_tk.lexeme;
                break;

            case token_type::placeholder:
                primary_node->expr_type =
                    ast_expr_primary::primary_type::placeholder;
                primary_node->value = primary_tk.lexeme;
                break;

            case token_type::left_parenthesis:
            {
                // - A left parenthesis indicates the start of a grouped
                //   expression. We need to parse the inner expression
                //   and expect a right parenthesis to close it.
                auto expr_result = parse_expression(lex);
                if (expr_result.has_value() == false)
                {
                    return g10::error(expr_result.error());
                }

                // - Expect a right parenthesis to close the grouped expression.
                //   We actually don't need to use it, so we can just consume it.
                auto rparen_tk_result = lex.consume_token(
                    token_type::right_parenthesis,
                    " - Expected ')' to close grouped expression.\n"
                    " - In file '{}:{}:{}'",
                    primary_tk.source_file,
                    primary_tk.source_line,
                    primary_tk.source_column
                );

                // - If we failed to find the right parenthesis, return the error.
                if (rparen_tk_result.has_value() == false)
                {
                    return g10::error(rparen_tk_result.error());
                }

                // - Create the grouping expression AST node.
                auto grouping_node = 
                    std::make_unique<ast_expr_grouping>(primary_tk);
                grouping_node->inner_expression = 
                    std::move(expr_result.value());

                return grouping_node;
            }

            default:
                return g10::error(
                    " - Unsupported token type '{}' ('{}') for primary expression.\n"
                    " - In file '{}:{}:{}'",
                    primary_tk.type_to_string(),
                    primary_tk.lexeme,
                    primary_tk.source_file,
                    primary_tk.source_line,
                    primary_tk.source_column
                );
        }

        return primary_node;
    }
}