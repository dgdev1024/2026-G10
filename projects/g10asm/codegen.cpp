/**
 * @file    g10asm/codegen.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-26
 * 
 * @brief   Contains implementations for the G10 assembler's syntax interpretation,
 *          expression evaluation and code generation component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/environment.hpp>
#include <g10asm/codegen.hpp>

/* Private Constants and Enumerations *****************************************/

namespace g10asm
{

}

/* Private Unions and Structures **********************************************/

namespace g10asm
{
    
}

/* Private Static Members *****************************************************/

namespace g10asm
{

}

/* Public Methods *************************************************************/

namespace g10asm
{
    auto codegen::process (ast_module& module) -> g10::result<g10::object>
    {
        // - Create the codegen state.
        codegen_state state;

        // - Clear the environment from any previous assembly runs.
        environment::clear();

        // - Set the object file flags (will be finalized later).
        state.object.set_flags(g10::object_flags::relocatable);

        // Variable Pass:
        // - Process all `.let`, `.const`, and variable assignment statements.
        // - This must be done before the first pass because variables can be
        //   used in `.org` expressions and other places that affect address
        //   calculation.
        if (auto result = variable_pass(state, module); !result.has_value())
        {
            std::println(stderr,
                "Variable pass code generation failed: {}", result.error());
            return g10::error(result.error());
        }

        // First Pass: 
        // - Collect symbols, create sections, assign addresses.
        if (auto result = first_pass(state, module); !result.has_value())
        {
            std::println(stderr,
                "First pass code generation failed: {}", result.error());
            return g10::error(result.error());
        }

        // Second Pass: 
        // - Emit code, evaluate expressions, generate relocations.
        if (auto result = second_pass(state, module); !result.has_value())
        {
            std::println(stderr,
                "Second pass code generation failed: {}", result.error());
            return g10::error(result.error());
        }

        // Finalization: 
        // - Validate the object, set final flags, verify symbols and relocations.
        if (auto result = finalize(state); !result.has_value())
        {
            std::println(stderr,
                "Finalization failed: {}", result.error());
            return g10::error(result.error());
        }

        // - Return the generated object file.
        return std::move(state.object);
    }
}

/* Private Methods - Variable Pass ********************************************/

namespace g10asm
{
    auto codegen::variable_pass (codegen_state& state, ast_module& module)
        -> g10::result<void>
    {
        // - Process each node in the module, looking for variable-related
        //   statements.
        for (auto& child : module.children)
        {
            if (!child || !child->valid)
            {
                continue;
            }

            switch (child->type)
            {
                case ast_node_type::dir_let:
                {
                    auto& let_dir = static_cast<ast_dir_let&>(*child);
                    if (auto result = variable_pass_let(state, let_dir);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_const:
                {
                    auto& const_dir = static_cast<ast_dir_const&>(*child);
                    if (auto result = variable_pass_const(state, const_dir);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::stmt_var_assignment:
                {
                    auto& assign_stmt = static_cast<ast_stmt_var_assignment&>(*child);
                    if (auto result = variable_pass_assignment(state, assign_stmt);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                default:
                    // - Other node types are handled in subsequent passes.
                    break;
            }
        }

        return {};
    }

    auto codegen::variable_pass_let (
        codegen_state& state,
        ast_dir_let& let_dir
    ) -> g10::result<void>
    {
        // - Evaluate the initialization expression.
        auto init_result = evaluate_expression(state, *let_dir.init_expression);
        if (!init_result.has_value())
        {
            return g10::error(
                " - Failed to evaluate initialization expression for variable '${}': {}\n"
                " - In file '{}:{}'",
                let_dir.variable_name,
                init_result.error(),
                let_dir.source_file,
                let_dir.source_line
            );
        }

        // - Define the variable in the environment.
        auto define_result = environment::define_variable(
            std::string(let_dir.variable_name),
            init_result.value(),
            let_dir.source_file,
            let_dir.source_line
        );

        if (!define_result.has_value())
        {
            return g10::error(
                " - Failed to define variable '${}': {}\n"
                " - In file '{}:{}'",
                let_dir.variable_name,
                define_result.error(),
                let_dir.source_file,
                let_dir.source_line
            );
        }

        return {};
    }

    auto codegen::variable_pass_const (
        codegen_state& state,
        ast_dir_const& const_dir
    ) -> g10::result<void>
    {
        // - Evaluate the value expression.
        auto value_result = evaluate_expression(state, *const_dir.value_expression);
        if (!value_result.has_value())
        {
            return g10::error(
                " - Failed to evaluate value expression for constant '${}': {}\n"
                " - In file '{}:{}'",
                const_dir.constant_name,
                value_result.error(),
                const_dir.source_file,
                const_dir.source_line
            );
        }

        // - Define the constant in the environment.
        auto define_result = environment::define_constant(
            std::string(const_dir.constant_name),
            value_result.value(),
            const_dir.source_file,
            const_dir.source_line
        );

        if (!define_result.has_value())
        {
            return g10::error(
                " - Failed to define constant '${}': {}\n"
                " - In file '{}:{}'",
                const_dir.constant_name,
                define_result.error(),
                const_dir.source_file,
                const_dir.source_line
            );
        }

        return {};
    }

    auto codegen::variable_pass_assignment (
        codegen_state& state,
        ast_stmt_var_assignment& assign_stmt
    ) -> g10::result<void>
    {
        const std::string var_name(assign_stmt.variable_name);

        // - Check if the variable exists.
        if (!environment::exists(var_name))
        {
            return g10::error(
                " - Undefined variable '${}' in assignment.\n"
                " - In file '{}:{}'",
                var_name,
                assign_stmt.source_file,
                assign_stmt.source_line
            );
        }

        // - Check if it's a constant (cannot be modified).
        if (environment::is_constant(var_name))
        {
            return g10::error(
                " - Cannot modify constant '${}' in assignment.\n"
                " - In file '{}:{}'",
                var_name,
                assign_stmt.source_file,
                assign_stmt.source_line
            );
        }

        // - Get the current value.
        auto current_result = environment::get_value(var_name);
        if (!current_result.has_value())
        {
            return g10::error(current_result.error());
        }

        // - Evaluate the right-hand side expression.
        auto rhs_result = evaluate_expression(state, *assign_stmt.value_expression);
        if (!rhs_result.has_value())
        {
            return g10::error(
                " - Failed to evaluate assignment expression for '${}': {}\n"
                " - In file '{}:{}'",
                var_name,
                rhs_result.error(),
                assign_stmt.source_file,
                assign_stmt.source_line
            );
        }

        // - Convert both values to integers for compound operations.
        auto current_int_result = value_to_integer(current_result.value());
        auto rhs_int_result = value_to_integer(rhs_result.value());

        if (!current_int_result.has_value())
        {
            return g10::error(
                " - Variable '${}' does not hold a numeric value.\n"
                " - In file '{}:{}'",
                var_name,
                assign_stmt.source_file,
                assign_stmt.source_line
            );
        }

        if (!rhs_int_result.has_value())
        {
            return g10::error(
                " - Assignment expression for '${}' does not evaluate to a numeric value.\n"
                " - In file '{}:{}'",
                var_name,
                assign_stmt.source_file,
                assign_stmt.source_line
            );
        }

        std::int64_t current_int = current_int_result.value();
        std::int64_t rhs_int = rhs_int_result.value();
        std::int64_t new_value = 0;

        // - Apply the assignment operator.
        switch (assign_stmt.assignment_operator)
        {
            case token_type::assign_equal:
                new_value = rhs_int;
                break;
            case token_type::assign_plus:
                new_value = current_int + rhs_int;
                break;
            case token_type::assign_minus:
                new_value = current_int - rhs_int;
                break;
            case token_type::assign_times:
                new_value = current_int * rhs_int;
                break;
            case token_type::assign_divide:
                if (rhs_int == 0)
                {
                    return g10::error(
                        " - Division by zero in assignment to '${}'\n"
                        " - In file '{}:{}'",
                        var_name,
                        assign_stmt.source_file,
                        assign_stmt.source_line
                    );
                }
                new_value = current_int / rhs_int;
                break;
            case token_type::assign_modulo:
                if (rhs_int == 0)
                {
                    return g10::error(
                        " - Modulo by zero in assignment to '${}'\n"
                        " - In file '{}:{}'",
                        var_name,
                        assign_stmt.source_file,
                        assign_stmt.source_line
                    );
                }
                new_value = current_int % rhs_int;
                break;
            case token_type::assign_and:
                new_value = current_int & rhs_int;
                break;
            case token_type::assign_or:
                new_value = current_int | rhs_int;
                break;
            case token_type::assign_xor:
                new_value = current_int ^ rhs_int;
                break;
            case token_type::assign_shift_left:
                new_value = current_int << rhs_int;
                break;
            case token_type::assign_shift_right:
                new_value = current_int >> rhs_int;
                break;
            case token_type::assign_exponent:
            {
                // - Compute exponentiation.
                new_value = 1;
                for (std::int64_t i = 0; i < rhs_int; ++i)
                {
                    new_value *= current_int;
                }
                break;
            }
            default:
                return g10::error(
                    " - Unknown assignment operator in assignment to '${}'\n"
                    " - In file '{}:{}'",
                    var_name,
                    assign_stmt.source_file,
                    assign_stmt.source_line
                );
        }

        // - Update the variable in the environment.
        auto set_result = environment::set_value(var_name, value{new_value});
        if (!set_result.has_value())
        {
            return g10::error(set_result.error());
        }

        return {};
    }
}

/* Private Methods - First Pass ***********************************************/

namespace g10asm
{
    auto codegen::first_pass (codegen_state& state, ast_module& module)
        -> g10::result<void>
    {
        // - Create initial section at the default location (`$2000` in ROM).
        if (auto result = ensure_section(state, state.location_counter);
            !result.has_value())
        {
            return result;
        }

        // - Process each node in the module.
        for (auto& child : module.children)
        {
            if (!child || !child->valid)
            {
                continue;
            }

            // - Dispatch based on node type.
            switch (child->type)
            {
                case ast_node_type::label_definition:
                {
                    auto& label = static_cast<ast_label_definition&>(*child);
                    if (auto result = first_pass_label(state, label);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::instruction:
                {
                    auto& instr = static_cast<ast_instruction&>(*child);
                    if (auto result = first_pass_instruction(state, instr);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_org:
                {
                    auto& org = static_cast<ast_dir_org&>(*child);
                    if (auto result = first_pass_org(state, org);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_rom:
                {
                    auto& rom = static_cast<ast_dir_rom&>(*child);
                    if (auto result = first_pass_rom(state, rom);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_ram:
                {
                    auto& ram = static_cast<ast_dir_ram&>(*child);
                    if (auto result = first_pass_ram(state, ram);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_int:
                {
                    auto& int_ = static_cast<ast_dir_int&>(*child);
                    if (auto result = first_pass_int(state, int_);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_byte:
                case ast_node_type::dir_word:
                case ast_node_type::dir_dword:
                {
                    if (auto result = first_pass_data(state, *child);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_global:
                {
                    auto& global = static_cast<ast_dir_global&>(*child);
                    if (auto result = first_pass_global(state, global);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_extern:
                {
                    auto& extern_ = static_cast<ast_dir_extern&>(*child);
                    if (auto result = first_pass_extern(state, extern_);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                default:
                    // Ignore other node types (expressions, operands, etc.)
                    break;
            }
        }

        return {};
    }

    auto codegen::first_pass_label (
        codegen_state& state,
        ast_label_definition& label
    ) -> g10::result<void>
    {
        // - Extract the label name.
        const std::string label_name { label.label_name };

        // - Check if label already exists.
        if (state.label_map.contains(label_name))
        {
            return g10::error("Label '{}' redefined ({}:{}:{})",
                label_name,
                label.source_file,
                label.source_line,
                label.source_column);
        }

        // - Store label location (section index and address).
        state.label_map[label_name] = {
            state.current_section_index,
            state.location_counter
        };

        // - Create a symbol for this label.
        // - Check if the label was declared global before definition.
        g10::object_symbol symbol;
        symbol.name = label_name;
        symbol.value = state.location_counter;
        symbol.section_index = static_cast<std::uint32_t>(state.current_section_index);
        symbol.type = g10::symbol_type::label;
        // - If label was declared global before, use global binding.
        symbol.binding = state.global_symbols.contains(label_name) ?
            g10::symbol_binding::global : g10::symbol_binding::local_;
        symbol.flags = g10::symbol_flags::none;

        // - Add the symbol to the object file.
        if (auto result = state.object.add_symbol(symbol); !result.has_value())
        {
            return g10::error("Failed to add symbol '{}': {} ({}:{}:{})",
                label_name,
                result.error(),
                label.source_file,
                label.source_line,
                label.source_column);
        }

        return {};
    }

    auto codegen::first_pass_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // - Calculate the size of this instruction.
        const std::size_t instr_size = calculate_instruction_size(instr);

        // - Advance the location counter.
        state.location_counter += static_cast<std::uint32_t>(instr_size);

        return {};
    }

    auto codegen::first_pass_org (
        codegen_state& state,
        ast_dir_org& org
    ) -> g10::result<void>
    {
        // Validate the address expression is present.
        if (!org.address_expression)
        {
            return g10::error(".org directive missing address expression ({}:{}:{})",
                org.source_file,
                org.source_line,
                org.source_column);
        }

        // Evaluate the address expression.
        auto result = evaluate_expression(state, *org.address_expression);
        if (!result.has_value())
        {
            return g10::error(".org directive: {} ({}:{}:{})",
                result.error(),
                org.source_file,
                org.source_line,
                org.source_column);
        }

        // Convert the result to an address.
        std::uint32_t new_address = 0;
        const value& val = result.value();

        if (std::holds_alternative<std::uint32_t>(val))
        {
            // Already an address.
            new_address = std::get<std::uint32_t>(val);
        }
        else if (std::holds_alternative<std::int64_t>(val))
        {
            // Integer value: validate range.
            std::int64_t addr = std::get<std::int64_t>(val);
            if (addr < 0 || addr > 0xFFFFFFFF)
            {
                return g10::error(".org address out of range: {} ({}:{}:{})",
                    addr,
                    org.source_file,
                    org.source_line,
                    org.source_column);
            }
            new_address = static_cast<std::uint32_t>(addr);
        }
        else
        {
            return g10::error(".org directive requires an integer or address "
                "expression ({}:{}:{})",
                org.source_file,
                org.source_line,
                org.source_column);
        }

        // - Save current location counter to appropriate region counter.
        if (state.in_rom_region)
        {
            state.rom_location_counter = state.location_counter;
        }
        else
        {
            state.ram_location_counter = state.location_counter;
        }

        // Update location counter.
        state.location_counter = new_address;
        state.in_rom_region = (new_address & 0x80000000) == 0;

        // - Update region-specific location counter.
        if (state.in_rom_region)
        {
            state.rom_location_counter = new_address;
        }
        else
        {
            state.ram_location_counter = new_address;
        }

        // Create or switch to section at this address.
        return ensure_section(state, new_address);
    }

    auto codegen::first_pass_rom (
        codegen_state& state,
        ast_dir_rom& rom
    ) -> g10::result<void>
    {
        // - If not in the ROM region, save the current RAM location counter.
        if (!state.in_rom_region)
        {
            state.ram_location_counter = state.location_counter;
        }

        // - Switch to ROM region.
        state.in_rom_region = true;
        state.location_counter = state.rom_location_counter;

        // - Ensure section exists at the new location.
        return ensure_section(state, state.location_counter);
    }

    auto codegen::first_pass_ram (
        codegen_state& state,
        ast_dir_ram& ram
    ) -> g10::result<void>
    {
        // - If in the ROM region, save the current ROM location counter.
        if (state.in_rom_region)
        {
            state.rom_location_counter = state.location_counter;
        }

        // - Switch to RAM region.
        state.in_rom_region = false;
        state.location_counter = state.ram_location_counter;

        // - Ensure section exists at the new location.
        return ensure_section(state, state.location_counter);
    }

    auto codegen::first_pass_int (
        codegen_state& state,
        ast_dir_int& int_
    ) -> g10::result<void>
    {
        // - Interrupt vectors are in ROM region ($1000 - $1FFF).
        //   Each vector occupies 0x80 bytes, starting at $1000.

        // - If not in the ROM region, save the current RAM location counter.
        if (!state.in_rom_region)
        {
            state.ram_location_counter = state.location_counter;
        }

        // - Evaluate the vector number expression.
        if (!int_.vector_expression)
        {
            return g10::error(".int directive missing vector number ({}:{}:{})",
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        auto result = evaluate_expression(state, *int_.vector_expression);
        if (!result.has_value())
        {
            return g10::error(".int directive: {} ({}:{}:{})",
                result.error(),
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        // - Convert to integer.
        std::int64_t vector_num = 0;
        const value& val = result.value();

        if (std::holds_alternative<std::int64_t>(val))
        {
            vector_num = std::get<std::int64_t>(val);
        }
        else if (std::holds_alternative<std::uint32_t>(val))
        {
            vector_num = static_cast<std::int64_t>(std::get<std::uint32_t>(val));
        }
        else
        {
            return g10::error(".int requires integer vector number ({}:{}:{})",
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        // - Validate vector number is in range [0, 31].
        if (vector_num < 0 || vector_num > 31)
        {
            return g10::error(".int vector number must be 0-31, got {} ({}:{}:{})",
                vector_num,
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        // - Calculate the interrupt vector address: $1000 + (vector * 0x80).
        constexpr std::uint32_t IVT_START = 0x00001000;
        constexpr std::uint32_t VECTOR_SIZE = 0x80;
        std::uint32_t new_address = IVT_START + 
            (static_cast<std::uint32_t>(vector_num) * VECTOR_SIZE);

        // - Switch to ROM region and set location counter.
        state.in_rom_region = true;
        state.location_counter = new_address;
        state.rom_location_counter = new_address;

        // - Ensure section exists at the new location.
        return ensure_section(state, new_address);
    }

    auto codegen::first_pass_data (
        codegen_state& state,
        ast_node& node
    ) -> g10::result<void>
    {
        // - Determine data size based on directive type.
        std::size_t element_size = 0;
        std::size_t element_count = 0;

        if (node.type == ast_node_type::dir_byte)
        {
            element_size = 1;
            auto& dir = static_cast<ast_dir_byte&>(node);
            element_count = dir.values.size();
        }
        else if (node.type == ast_node_type::dir_word)
        {
            element_size = 2;
            auto& dir = static_cast<ast_dir_word&>(node);
            element_count = dir.values.size();
        }
        else if (node.type == ast_node_type::dir_dword)
        {
            element_size = 4;
            auto& dir = static_cast<ast_dir_dword&>(node);
            element_count = dir.values.size();
        }

        // In ROM region: emit data directly (size = element_size * element_count)
        // In RAM region: reserve BSS space (size depends on evaluated expressions)
        if (state.in_rom_region)
        {
            // ROM: Each value contributes element_size bytes.
            state.location_counter += static_cast<std::uint32_t>(
                element_size * element_count);
        }
        else
        {
            // RAM: Values are counts, need to sum them (simplified for now).
            // In first pass, we'll assume each value = 1 for size calculation.
            // Full expression evaluation happens in second pass.
            state.location_counter += static_cast<std::uint32_t>(
                element_size * element_count);
        }

        return {};
    }

    auto codegen::first_pass_global (
        codegen_state& state,
        ast_dir_global& global
    ) -> g10::result<void>
    {
        // - Process each symbol declared as global.
        for (const auto& symbol_name_view : global.symbols)
        {
            const std::string symbol_name { symbol_name_view };

            // - Check for duplicate global declarations.
            if (state.global_symbols.contains(symbol_name))
            {
                return g10::error("Symbol '{}' already declared as global ({}:{}:{})",
                    symbol_name,
                    global.source_file,
                    global.source_line,
                    global.source_column);
            }

            // - Check if symbol is extern.
            if (state.extern_symbols.contains(symbol_name))
            {
                return g10::error("Symbol '{}' cannot be both global and extern ({}:{}:{})",
                    symbol_name,
                    global.source_file,
                    global.source_line,
                    global.source_column);
            }

            // - Add to global symbols set.
            state.global_symbols.insert(symbol_name);

            // - Find the symbol in the object and promote it to global.
            auto symbol_index = state.object.find_symbol(symbol_name);
            if (symbol_index.has_value())
            {
                // Symbol exists - update its binding to global.
                auto& symbols = const_cast<std::vector<g10::object_symbol>&>(
                    state.object.get_symbols());
                symbols[*symbol_index].binding = g10::symbol_binding::global;
            }
            // - If symbol doesn't exist yet, it will be created as global when
            //   the label is encountered.
        }

        return {};
    }

    auto codegen::first_pass_extern (
        codegen_state& state,
        ast_dir_extern& extern_
    ) -> g10::result<void>
    {
        // - Process each symbol declared as extern.
        for (const auto& symbol_name_view : extern_.symbols)
        {
            const std::string symbol_name { symbol_name_view };

            // - Check for duplicate extern declarations.
            if (state.extern_symbols.contains(symbol_name))
            {
                continue; // Already declared extern, skip.
            }

            // - Check if symbol is global.
            if (state.global_symbols.contains(symbol_name))
            {
                return g10::error("Symbol '{}' cannot be both extern and global ({}:{}:{})",
                    symbol_name,
                    extern_.source_file,
                    extern_.source_line,
                    extern_.source_column);
            }

            // - Add to extern symbols set.
            state.extern_symbols.insert(symbol_name);

            // - Create an extern symbol entry.
            g10::object_symbol symbol;
            symbol.name = symbol_name;
            symbol.value = 0;
            symbol.section_index = g10::SECTION_INDEX_UNDEF;
            symbol.type = g10::symbol_type::label;
            symbol.binding = g10::symbol_binding::extern_;
            symbol.flags = g10::symbol_flags::none;

            // - Add the symbol to the object file.
            if (auto result = state.object.add_symbol(symbol); !result.has_value())
            {
                return g10::error("Failed to add extern symbol '{}': {} ({}:{}:{})",
                    symbol_name,
                    result.error(),
                    extern_.source_file,
                    extern_.source_line,
                    extern_.source_column);
            }
        }

        return {};
    }
}

/* Private Methods - Second Pass **********************************************/

namespace g10asm
{
    auto codegen::second_pass (codegen_state& state, ast_module& module)
        -> g10::result<void>
    {
        // Reset the location counter and section index for second pass.
        state.location_counter = 0x00002000;
        state.rom_location_counter = 0x00002000;
        state.ram_location_counter = 0x80000000;
        state.current_section_index = 0;
        state.in_rom_region = true;

        // Process each node in the module.
        for (auto& child : module.children)
        {
            if (!child || !child->valid)
            {
                continue;
            }

            // Dispatch based on node type.
            switch (child->type)
            {
                case ast_node_type::label_definition:
                    // Labels were processed in first pass; skip.
                    break;

                case ast_node_type::instruction:
                {
                    auto& instr = static_cast<ast_instruction&>(*child);
                    if (auto result = second_pass_instruction(state, instr);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_org:
                {
                    auto& org = static_cast<ast_dir_org&>(*child);
                    if (auto result = second_pass_org(state, org);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_rom:
                {
                    auto& rom = static_cast<ast_dir_rom&>(*child);
                    if (auto result = second_pass_rom(state, rom);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_ram:
                {
                    auto& ram = static_cast<ast_dir_ram&>(*child);
                    if (auto result = second_pass_ram(state, ram);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_int:
                {
                    auto& int_ = static_cast<ast_dir_int&>(*child);
                    if (auto result = second_pass_int(state, int_);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_byte:
                {
                    auto& dir = static_cast<ast_dir_byte&>(*child);
                    if (auto result = second_pass_byte(state, dir);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_word:
                {
                    auto& dir = static_cast<ast_dir_word&>(*child);
                    if (auto result = second_pass_word(state, dir);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_dword:
                {
                    auto& dir = static_cast<ast_dir_dword&>(*child);
                    if (auto result = second_pass_dword(state, dir);
                        !result.has_value())
                    {
                        return result;
                    }
                    break;
                }

                case ast_node_type::dir_global:
                case ast_node_type::dir_extern:
                    // Symbol directives were processed in first pass; skip.
                    break;

                default:
                    // Ignore other node types.
                    break;
            }
        }

        return {};
    }

    auto codegen::second_pass_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // Emit the instruction's machine code.
        return emit_instruction(state, instr);
    }

    auto codegen::second_pass_org (
        codegen_state& state,
        ast_dir_org& org
    ) -> g10::result<void>
    {
        // Evaluate the address expression (same as first pass).
        if (!org.address_expression)
        {
            return g10::error(".org directive missing address expression ({}:{}:{})",
                org.source_file,
                org.source_line,
                org.source_column);
        }

        auto result = evaluate_expression(state, *org.address_expression);
        if (!result.has_value())
        {
            return g10::error(".org directive: {} ({}:{}:{})",
                result.error(),
                org.source_file,
                org.source_line,
                org.source_column);
        }

        // Convert to address.
        std::uint32_t new_address = 0;
        const value& val = result.value();

        if (std::holds_alternative<std::uint32_t>(val))
        {
            new_address = std::get<std::uint32_t>(val);
        }
        else if (std::holds_alternative<std::int64_t>(val))
        {
            std::int64_t addr = std::get<std::int64_t>(val);
            if (addr < 0 || addr > 0xFFFFFFFF)
            {
                return g10::error(".org address out of range ({}:{}:{})",
                    org.source_file,
                    org.source_line,
                    org.source_column);
            }
            new_address = static_cast<std::uint32_t>(addr);
        }
        else
        {
            return g10::error(".org requires integer address ({}:{}:{})",
                org.source_file,
                org.source_line,
                org.source_column);
        }

        // Update location counter and region flag.
        state.location_counter = new_address;
        state.in_rom_region = (new_address & 0x80000000) == 0;

        // Find the section that contains this address.
        const auto& sections = state.object.get_sections();
        for (std::size_t i = 0; i < sections.size(); ++i)
        {
            const auto& section = sections[i];
            if (section.virtual_address == new_address)
            {
                state.current_section_index = i;
                return {};
            }
        }

        // Section not found - this shouldn't happen if first pass ran correctly.
        return g10::error("Internal error: section at 0x{:08X} not found ({}:{}:{})",
            new_address,
            org.source_file,
            org.source_line,
            org.source_column);
    }

    auto codegen::second_pass_rom (
        codegen_state& state,
        ast_dir_rom& rom
    ) -> g10::result<void>
    {
        // - If not in the ROM region, save the current RAM location counter.
        if (!state.in_rom_region)
        {
            state.ram_location_counter = state.location_counter;
        }

        // - Switch to ROM region.
        state.in_rom_region = true;
        state.location_counter = state.rom_location_counter;

        // - Find the section that contains this address.
        const auto& sections = state.object.get_sections();
        for (std::size_t i = 0; i < sections.size(); ++i)
        {
            const auto& section = sections[i];
            if (section.virtual_address == state.location_counter)
            {
                state.current_section_index = i;
                return {};
            }
        }

        // Section not found - this shouldn't happen if first pass ran correctly.
        return g10::error("Internal error: section at 0x{:08X} not found for .rom ({}:{}:{})",
            state.location_counter,
            rom.source_file,
            rom.source_line,
            rom.source_column);
    }

    auto codegen::second_pass_ram (
        codegen_state& state,
        ast_dir_ram& ram
    ) -> g10::result<void>
    {
        // - If in the ROM region, save the current ROM location counter.
        if (state.in_rom_region)
        {
            state.rom_location_counter = state.location_counter;
        }

        // - Switch to RAM region.
        state.in_rom_region = false;
        state.location_counter = state.ram_location_counter;

        // - Find the section that contains this address.
        const auto& sections = state.object.get_sections();
        for (std::size_t i = 0; i < sections.size(); ++i)
        {
            const auto& section = sections[i];
            if (section.virtual_address == state.location_counter)
            {
                state.current_section_index = i;
                return {};
            }
        }

        // Section not found - this shouldn't happen if first pass ran correctly.
        return g10::error("Internal error: section at 0x{:08X} not found for .ram ({}:{}:{})",
            state.location_counter,
            ram.source_file,
            ram.source_line,
            ram.source_column);
    }

    auto codegen::second_pass_int (
        codegen_state& state,
        ast_dir_int& int_
    ) -> g10::result<void>
    {
        // - If not in the ROM region, save the current RAM location counter.
        if (!state.in_rom_region)
        {
            state.ram_location_counter = state.location_counter;
        }

        // - Evaluate the vector number expression (same as first pass).
        if (!int_.vector_expression)
        {
            return g10::error(".int directive missing vector number ({}:{}:{})",
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        auto result = evaluate_expression(state, *int_.vector_expression);
        if (!result.has_value())
        {
            return g10::error(".int directive: {} ({}:{}:{})",
                result.error(),
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        // - Convert to integer.
        std::int64_t vector_num = 0;
        const value& val = result.value();

        if (std::holds_alternative<std::int64_t>(val))
        {
            vector_num = std::get<std::int64_t>(val);
        }
        else if (std::holds_alternative<std::uint32_t>(val))
        {
            vector_num = static_cast<std::int64_t>(std::get<std::uint32_t>(val));
        }
        else
        {
            return g10::error(".int requires integer vector number ({}:{}:{})",
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        // - Validate vector number is in range [0, 31].
        if (vector_num < 0 || vector_num > 31)
        {
            return g10::error(".int vector number must be 0-31, got {} ({}:{}:{})",
                vector_num,
                int_.source_file,
                int_.source_line,
                int_.source_column);
        }

        // - Calculate the interrupt vector address: $1000 + (vector * 0x80).
        constexpr std::uint32_t IVT_START = 0x00001000;
        constexpr std::uint32_t VECTOR_SIZE = 0x80;
        std::uint32_t new_address = IVT_START + 
            (static_cast<std::uint32_t>(vector_num) * VECTOR_SIZE);

        // - Switch to ROM region and set location counter.
        state.in_rom_region = true;
        state.location_counter = new_address;
        state.rom_location_counter = new_address;

        // - Find the section that contains this address.
        const auto& sections = state.object.get_sections();
        for (std::size_t i = 0; i < sections.size(); ++i)
        {
            const auto& section = sections[i];
            if (section.virtual_address == new_address)
            {
                state.current_section_index = i;
                return {};
            }
        }

        // Section not found - this shouldn't happen if first pass ran correctly.
        return g10::error("Internal error: section at 0x{:08X} not found for .int ({}:{}:{})",
            new_address,
            int_.source_file,
            int_.source_line,
            int_.source_column);
    }

    auto codegen::second_pass_byte (
        codegen_state& state,
        ast_dir_byte& dir
    ) -> g10::result<void>
    {
        if (state.in_rom_region)
        {
            // ROM region: emit actual byte values.
            for (const auto& value_node : dir.values)
            {
                if (!value_node)
                {
                    continue;
                }

                // Check if this is a string literal (emit each character).
                if (value_node->type == ast_node_type::expr_primary)
                {
                    const auto& primary = 
                        static_cast<const ast_expr_primary&>(*value_node);
                    if (primary.expr_type == 
                        ast_expr_primary::primary_type::string_literal)
                    {
                        // Emit each character of the string.
                        if (std::holds_alternative<std::string_view>(primary.value))
                        {
                            std::string_view str = 
                                std::get<std::string_view>(primary.value);
                            for (char c : str)
                            {
                                emit_byte(state, static_cast<std::uint8_t>(c));
                            }
                            continue;
                        }
                    }
                }

                // Evaluate as expression and emit as byte.
                auto result = evaluate_expression(
                    state, static_cast<const ast_expression&>(*value_node));
                if (!result.has_value())
                {
                    return g10::error(".byte: {} ({}:{}:{})",
                        result.error(),
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                auto int_result = value_to_integer(result.value());
                if (!int_result.has_value())
                {
                    return g10::error(".byte requires integer value ({}:{}:{})",
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                emit_byte(state, static_cast<std::uint8_t>(
                    int_result.value() & 0xFF));
            }
        }
        else
        {
            // RAM region (BSS): reserve space, don't emit values.
            // Sum all values to get count.
            std::size_t total_count = 0;
            for (const auto& value_node : dir.values)
            {
                if (!value_node)
                {
                    total_count += 1;
                    continue;
                }

                auto result = evaluate_expression(
                    state, static_cast<const ast_expression&>(*value_node));
                if (!result.has_value())
                {
                    return g10::error(".byte: {} ({}:{}:{})",
                        result.error(),
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                auto int_result = value_to_integer(result.value());
                if (!int_result.has_value() || int_result.value() < 0)
                {
                    return g10::error(".byte count must be positive ({}:{}:{})",
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                total_count += static_cast<std::size_t>(int_result.value());
            }

            // Advance location counter without emitting data.
            state.location_counter += static_cast<std::uint32_t>(total_count);
        }

        return {};
    }

    auto codegen::second_pass_word (
        codegen_state& state,
        ast_dir_word& dir
    ) -> g10::result<void>
    {
        if (state.in_rom_region)
        {
            // ROM region: emit actual word values.
            for (const auto& value_node : dir.values)
            {
                if (!value_node)
                {
                    continue;
                }

                // Check for external symbol references (need relocation).
                const auto& expr = static_cast<const ast_expression&>(*value_node);
                bool needs_reloc = references_external(state, expr);

                if (needs_reloc)
                {
                    // Find the external symbol name.
                    std::string symbol_name;
                    if (expr.type == ast_node_type::expr_primary)
                    {
                        const auto& primary = 
                            static_cast<const ast_expr_primary&>(expr);
                        if (std::holds_alternative<std::string_view>(primary.value))
                        {
                            symbol_name = std::string { 
                                std::get<std::string_view>(primary.value) };
                        }
                        else
                        {
                            symbol_name = std::string { primary.lexeme };
                        }
                    }

                    // Create relocation for 16-bit absolute.
                    if (auto result = create_relocation(state, symbol_name,
                        g10::relocation_type::abs16, 0); !result.has_value())
                    {
                        return result;
                    }

                    // Emit placeholder.
                    emit_word(state, 0x0000);
                }
                else
                {
                    // Evaluate and emit.
                    auto result = evaluate_expression(state, expr);
                    if (!result.has_value())
                    {
                        return g10::error(".word: {} ({}:{}:{})",
                            result.error(),
                            dir.source_file,
                            dir.source_line,
                            dir.source_column);
                    }

                    auto int_result = value_to_integer(result.value());
                    if (!int_result.has_value())
                    {
                        return g10::error(".word requires integer value ({}:{}:{})",
                            dir.source_file,
                            dir.source_line,
                            dir.source_column);
                    }

                    emit_word(state, static_cast<std::uint16_t>(
                        int_result.value() & 0xFFFF));
                }
            }
        }
        else
        {
            // RAM region (BSS): reserve space.
            std::size_t total_count = 0;
            for (const auto& value_node : dir.values)
            {
                if (!value_node)
                {
                    total_count += 1;
                    continue;
                }

                auto result = evaluate_expression(
                    state, static_cast<const ast_expression&>(*value_node));
                if (!result.has_value())
                {
                    return g10::error(".word: {} ({}:{}:{})",
                        result.error(),
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                auto int_result = value_to_integer(result.value());
                if (!int_result.has_value() || int_result.value() < 0)
                {
                    return g10::error(".word count must be positive ({}:{}:{})",
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                total_count += static_cast<std::size_t>(int_result.value());
            }

            // Advance location counter (2 bytes per word).
            state.location_counter += static_cast<std::uint32_t>(total_count * 2);
        }

        return {};
    }

    auto codegen::second_pass_dword (
        codegen_state& state,
        ast_dir_dword& dir
    ) -> g10::result<void>
    {
        if (state.in_rom_region)
        {
            // ROM region: emit actual dword values.
            for (const auto& value_node : dir.values)
            {
                if (!value_node)
                {
                    continue;
                }

                // Check for external symbol references (need relocation).
                const auto& expr = static_cast<const ast_expression&>(*value_node);
                bool needs_reloc = references_external(state, expr);

                if (needs_reloc)
                {
                    // Find the external symbol name.
                    std::string symbol_name;
                    if (expr.type == ast_node_type::expr_primary)
                    {
                        const auto& primary = 
                            static_cast<const ast_expr_primary&>(expr);
                        if (std::holds_alternative<std::string_view>(primary.value))
                        {
                            symbol_name = std::string { 
                                std::get<std::string_view>(primary.value) };
                        }
                        else
                        {
                            symbol_name = std::string { primary.lexeme };
                        }
                    }

                    // Create relocation for 32-bit absolute.
                    if (auto result = create_relocation(state, symbol_name,
                        g10::relocation_type::abs32, 0); !result.has_value())
                    {
                        return result;
                    }

                    // Emit placeholder.
                    emit_dword(state, 0x00000000);
                }
                else
                {
                    // Evaluate and emit.
                    auto result = evaluate_expression(state, expr);
                    if (!result.has_value())
                    {
                        return g10::error(".dword: {} ({}:{}:{})",
                            result.error(),
                            dir.source_file,
                            dir.source_line,
                            dir.source_column);
                    }

                    auto int_result = value_to_integer(result.value());
                    if (!int_result.has_value())
                    {
                        return g10::error(".dword requires integer value ({}:{}:{})",
                            dir.source_file,
                            dir.source_line,
                            dir.source_column);
                    }

                    emit_dword(state, static_cast<std::uint32_t>(
                        int_result.value() & 0xFFFFFFFF));
                }
            }
        }
        else
        {
            // RAM region (BSS): reserve space.
            std::size_t total_count = 0;
            for (const auto& value_node : dir.values)
            {
                if (!value_node)
                {
                    total_count += 1;
                    continue;
                }

                auto result = evaluate_expression(
                    state, static_cast<const ast_expression&>(*value_node));
                if (!result.has_value())
                {
                    return g10::error(".dword: {} ({}:{}:{})",
                        result.error(),
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                auto int_result = value_to_integer(result.value());
                if (!int_result.has_value() || int_result.value() < 0)
                {
                    return g10::error(".dword count must be positive ({}:{}:{})",
                        dir.source_file,
                        dir.source_line,
                        dir.source_column);
                }

                total_count += static_cast<std::size_t>(int_result.value());
            }

            // Advance location counter (4 bytes per dword).
            state.location_counter += static_cast<std::uint32_t>(total_count * 4);
        }

        return {};
    }
}

/* Private Methods - Finalization *********************************************/

namespace g10asm
{
    auto codegen::finalize (codegen_state& state) -> g10::result<void>
    {
        // Step 1: Set appropriate object flags based on content.
        finalize_flags(state);

        // Step 2: Verify all global symbols have been defined.
        if (auto result = verify_global_symbols(state); !result.has_value())
        {
            return g10::error(result.error());
        }

        // Step 3: Verify all relocations are valid.
        if (auto result = verify_relocations(state); !result.has_value())
        {
            return g10::error(result.error());
        }

        // Step 4: Run the object's internal validation.
        // The object class has its own validate() method that checks for:
        // - Section overlaps
        // - Valid symbol references
        // - Relocation bounds
        // This is called automatically when saving, but we can validate early.
        
        return {};
    }

    auto codegen::finalize_flags (codegen_state& state) -> void
    {
        g10::object_flags flags = g10::object_flags::none;

        // Mark as relocatable if there are any relocations.
        if (!state.object.get_relocations().empty())
        {
            flags = flags | g10::object_flags::relocatable;
        }

        // Check for common entry point symbols.
        // If symbols like "_start", "main", or "start" exist, mark has_entry.
        static const std::vector<std::string> entry_point_names = {
            "_start", "main", "start", "_main"
        };

        for (const auto& name : entry_point_names)
        {
            if (auto sym_idx = state.object.find_symbol(name); sym_idx.has_value())
            {
                const auto& sym = state.object.get_symbols()[sym_idx.value()];
                // Only count as entry if it's defined (not extern).
                if (sym.section_index != g10::SECTION_INDEX_UNDEF)
                {
                    flags = flags | g10::object_flags::has_entry;
                    break;
                }
            }
        }

        state.object.set_flags(flags);
    }

    auto codegen::verify_global_symbols (codegen_state& state)
        -> g10::result<void>
    {
        std::vector<std::string> undefined_globals;

        // Check each global symbol to ensure it's defined.
        for (const auto& global_name : state.global_symbols)
        {
            // Look up the symbol in the object.
            auto sym_idx = state.object.find_symbol(global_name);
            if (!sym_idx.has_value())
            {
                // Symbol was declared global but never added to symbol table.
                undefined_globals.push_back(global_name);
                continue;
            }

            const auto& sym = state.object.get_symbols()[sym_idx.value()];

            // Check that the symbol is actually defined (has a section).
            if (sym.section_index == g10::SECTION_INDEX_UNDEF)
            {
                // Symbol is in table but not defined (treated as extern).
                undefined_globals.push_back(global_name);
            }
        }

        if (!undefined_globals.empty())
        {
            std::string error_msg = "Undefined global symbol(s): ";
            for (std::size_t i = 0; i < undefined_globals.size(); ++i)
            {
                if (i > 0) { error_msg += ", "; }
                error_msg += undefined_globals[i];
            }
            return g10::error(error_msg);
        }

        return {};
    }

    auto codegen::verify_relocations (codegen_state& state)
        -> g10::result<void>
    {
        const auto& relocations = state.object.get_relocations();
        const auto& symbols = state.object.get_symbols();
        const auto& sections = state.object.get_sections();

        for (std::size_t i = 0; i < relocations.size(); ++i)
        {
            const auto& reloc = relocations[i];

            // Verify symbol index is valid.
            if (reloc.symbol_index >= symbols.size())
            {
                return g10::error("Relocation {} references invalid symbol index {}",
                    i, reloc.symbol_index);
            }

            // Verify section index is valid.
            if (reloc.section_index >= sections.size())
            {
                return g10::error("Relocation {} references invalid section index {}",
                    i, reloc.section_index);
            }

            const auto& section = sections[reloc.section_index];

            // Get section size - for BSS sections with no data, use the
            // section's actual allocated size from data vector.
            // BSS sections might have empty data but still have valid
            // relocations pending linker resolution.
            std::size_t section_size = section.data.size();

            // Determine relocation size based on type.
            std::size_t reloc_size = 0;
            switch (reloc.type)
            {
                case g10::relocation_type::abs8:
                case g10::relocation_type::rel8:
                    reloc_size = 1;
                    break;
                case g10::relocation_type::abs16:
                case g10::relocation_type::rel16:
                    reloc_size = 2;
                    break;
                case g10::relocation_type::abs32:
                case g10::relocation_type::rel32:
                    reloc_size = 4;
                    break;
                default:
                    reloc_size = 1;
                    break;
            }

            // Check if relocation fits within section.
            if (reloc.offset + reloc_size > section_size)
            {
                return g10::error("Relocation {} at offset {} extends beyond "
                    "section '{}' (size {})",
                    i, reloc.offset, section.name, section_size);
            }

            // Verify the referenced symbol makes sense.
            const auto& sym = symbols[reloc.symbol_index];
            
            // For relocations, we typically expect extern symbols or
            // symbols that need address patching.
            // Warn if relocating against a defined local symbol
            // (though this might be valid for position-independent code).
        }

        return {};
    }
}

/* Private Methods - Expression Evaluation ************************************/

namespace g10asm
{
    auto codegen::evaluate_expression (
        codegen_state& state,
        const ast_expression& expr
    ) -> g10::result<value>
    {
        // Dispatch based on the expression type.
        switch (expr.type)
        {
            case ast_node_type::expr_primary:
            {
                const auto& primary = static_cast<const ast_expr_primary&>(expr);
                return evaluate_primary(state, primary);
            }

            case ast_node_type::expr_binary:
            {
                const auto& binary = static_cast<const ast_expr_binary&>(expr);
                return evaluate_binary(state, binary);
            }

            case ast_node_type::expr_unary:
            {
                const auto& unary = static_cast<const ast_expr_unary&>(expr);
                return evaluate_unary(state, unary);
            }

            case ast_node_type::expr_grouping:
            {
                const auto& grouping = static_cast<const ast_expr_grouping&>(expr);
                return evaluate_grouping(state, grouping);
            }

            default:
                return g10::error("Unknown expression type at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
        }
    }

    auto codegen::evaluate_primary (
        codegen_state& state,
        const ast_expr_primary& expr
    ) -> g10::result<value>
    {
        switch (expr.expr_type)
        {
            case ast_expr_primary::primary_type::integer_literal:
            {
                // Integer literal: already parsed in the AST.
                if (std::holds_alternative<std::int64_t>(expr.value))
                {
                    return std::get<std::int64_t>(expr.value);
                }
                return g10::error("Invalid integer literal at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }

            case ast_expr_primary::primary_type::number_literal:
            {
                // Number literal: convert to 32.32 fixed-point.
                if (std::holds_alternative<double>(expr.value))
                {
                    double num = std::get<double>(expr.value);
                    // Convert to 32.32 fixed-point: multiply by 2^32.
                    std::uint64_t fixed = static_cast<std::uint64_t>(
                        std::fabs(num) * 4294967296.0);
                    if (num < 0)
                    {
                        // Handle negative: two's complement of the full 64-bit.
                        fixed = ~fixed + 1;
                    }
                    return fixed;
                }
                return g10::error("Invalid number literal at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }

            case ast_expr_primary::primary_type::char_literal:
            {
                // Character literal: convert to integer.
                if (std::holds_alternative<char>(expr.value))
                {
                    return static_cast<std::int64_t>(
                        static_cast<unsigned char>(std::get<char>(expr.value)));
                }
                return g10::error("Invalid character literal at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }

            case ast_expr_primary::primary_type::string_literal:
            {
                // String literal: return as string.
                if (std::holds_alternative<std::string_view>(expr.value))
                {
                    return std::string { std::get<std::string_view>(expr.value) };
                }
                return g10::error("Invalid string literal at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }

            case ast_expr_primary::primary_type::identifier:
            {
                // Identifier: look up as a label.
                std::string name;
                if (std::holds_alternative<std::string_view>(expr.value))
                {
                    name = std::string { std::get<std::string_view>(expr.value) };
                }
                else
                {
                    name = std::string { expr.lexeme };
                }

                // Check if it's a known label.
                if (auto it = state.label_map.find(name);
                    it != state.label_map.end())
                {
                    // Return as address (uint32_t).
                    return it->second.second;
                }

                // Check if it's an extern symbol.
                if (state.extern_symbols.contains(name))
                {
                    // Extern symbols have unknown addresses at assembly time.
                    // Return 0 as placeholder; relocation will fix it.
                    return static_cast<std::uint32_t>(0);
                }

                return g10::error("Undefined symbol '{}' at {}:{}:{}",
                    name,
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }

            case ast_expr_primary::primary_type::variable:
            {
                // Variable: look up in the environment.
                std::string var_name;
                if (std::holds_alternative<std::string_view>(expr.value))
                {
                    var_name = std::string { std::get<std::string_view>(expr.value) };
                }
                else
                {
                    var_name = std::string { expr.lexeme };
                }

                // Remove the '$' prefix if present.
                if (!var_name.empty() && var_name[0] == '$')
                {
                    var_name = var_name.substr(1);
                }

                // Look up in the environment.
                auto value_result = environment::get_value(var_name);
                if (!value_result.has_value())
                {
                    return g10::error("Undefined variable '${}' at {}:{}:{}",
                        var_name,
                        expr.source_file,
                        expr.source_line,
                        expr.source_column);
                }

                return value_result.value();
            }

            case ast_expr_primary::primary_type::placeholder:
            {
                // Placeholders are for macro expansion.
                // Not supported in expression evaluation context.
                return g10::error("Placeholders not supported in "
                    "expressions at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }

            default:
                return g10::error("Unknown primary expression type at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
        }
    }

    auto codegen::evaluate_binary (
        codegen_state& state,
        const ast_expr_binary& expr
    ) -> g10::result<value>
    {
        // Evaluate left and right operands.
        if (!expr.left_operand || !expr.right_operand)
        {
            return g10::error("Binary expression missing operand at {}:{}:{}",
                expr.source_file,
                expr.source_line,
                expr.source_column);
        }

        auto left_result = evaluate_expression(state, *expr.left_operand);
        if (!left_result.has_value())
        {
            return g10::error(left_result.error());
        }

        auto right_result = evaluate_expression(state, *expr.right_operand);
        if (!right_result.has_value())
        {
            return g10::error(right_result.error());
        }

        // Convert both operands to integers for arithmetic/bitwise operations.
        auto left_int = value_to_integer(left_result.value());
        auto right_int = value_to_integer(right_result.value());

        if (!left_int.has_value() || !right_int.has_value())
        {
            return g10::error("Binary operation requires integer operands "
                "at {}:{}:{}",
                expr.source_file,
                expr.source_line,
                expr.source_column);
        }

        const std::int64_t left = left_int.value();
        const std::int64_t right = right_int.value();

        // Perform the operation based on operator type.
        switch (expr.operator_type)
        {
            // Arithmetic operators
            case token_type::plus:
                return left + right;

            case token_type::minus:
                return left - right;

            case token_type::times:
                return left * right;

            case token_type::divide:
                if (right == 0)
                {
                    return g10::error("Division by zero at {}:{}:{}",
                        expr.source_file,
                        expr.source_line,
                        expr.source_column);
                }
                return left / right;

            case token_type::modulo:
                if (right == 0)
                {
                    return g10::error("Modulo by zero at {}:{}:{}",
                        expr.source_file,
                        expr.source_line,
                        expr.source_column);
                }
                return left % right;

            case token_type::exponent:
            {
                // Integer exponentiation.
                if (right < 0)
                {
                    return g10::error("Negative exponent not supported at {}:{}:{}",
                        expr.source_file,
                        expr.source_line,
                        expr.source_column);
                }
                std::int64_t result = 1;
                std::int64_t base = left;
                std::int64_t exp = right;
                while (exp > 0)
                {
                    if (exp & 1)
                    {
                        result *= base;
                    }
                    base *= base;
                    exp >>= 1;
                }
                return result;
            }

            // Bitwise operators
            case token_type::bitwise_and:
                return left & right;

            case token_type::bitwise_or:
                return left | right;

            case token_type::bitwise_xor:
                return left ^ right;

            case token_type::bitwise_shift_left:
                if (right < 0 || right >= 64)
                {
                    return g10::error("Shift amount out of range at {}:{}:{}",
                        expr.source_file,
                        expr.source_line,
                        expr.source_column);
                }
                return left << right;

            case token_type::bitwise_shift_right:
                if (right < 0 || right >= 64)
                {
                    return g10::error("Shift amount out of range at {}:{}:{}",
                        expr.source_file,
                        expr.source_line,
                        expr.source_column);
                }
                return left >> right;

            // Comparison operators (return 1 for true, 0 for false)
            case token_type::compare_equal:
                return static_cast<std::int64_t>(left == right ? 1 : 0);

            case token_type::compare_not_equal:
                return static_cast<std::int64_t>(left != right ? 1 : 0);

            case token_type::compare_less:
                return static_cast<std::int64_t>(left < right ? 1 : 0);

            case token_type::compare_less_equal:
                return static_cast<std::int64_t>(left <= right ? 1 : 0);

            case token_type::compare_greater:
                return static_cast<std::int64_t>(left > right ? 1 : 0);

            case token_type::compare_greater_equal:
                return static_cast<std::int64_t>(left >= right ? 1 : 0);

            // Logical operators
            case token_type::logical_and:
                return static_cast<std::int64_t>((left != 0 && right != 0) ? 1 : 0);

            case token_type::logical_or:
                return static_cast<std::int64_t>((left != 0 || right != 0) ? 1 : 0);

            default:
                return g10::error("Unknown binary operator at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
        }
    }

    auto codegen::evaluate_unary (
        codegen_state& state,
        const ast_expr_unary& expr
    ) -> g10::result<value>
    {
        // Evaluate the operand.
        if (!expr.operand)
        {
            return g10::error("Unary expression missing operand at {}:{}:{}",
                expr.source_file,
                expr.source_line,
                expr.source_column);
        }

        auto operand_result = evaluate_expression(state, *expr.operand);
        if (!operand_result.has_value())
        {
            return g10::error(operand_result.error());
        }

        // Convert operand to integer.
        auto operand_int = value_to_integer(operand_result.value());
        if (!operand_int.has_value())
        {
            return g10::error("Unary operation requires integer operand "
                "at {}:{}:{}",
                expr.source_file,
                expr.source_line,
                expr.source_column);
        }

        const std::int64_t operand = operand_int.value();

        // Perform the operation based on operator type.
        switch (expr.operator_type)
        {
            case token_type::minus:
                return -operand;

            case token_type::plus:
                return operand; // Unary plus is a no-op.

            case token_type::bitwise_not:
                return ~operand;

            case token_type::logical_not:
                return static_cast<std::int64_t>(operand == 0 ? 1 : 0);

            default:
                return g10::error("Unknown unary operator at {}:{}:{}",
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
        }
    }

    auto codegen::evaluate_grouping (
        codegen_state& state,
        const ast_expr_grouping& expr
    ) -> g10::result<value>
    {
        // Simply evaluate the inner expression.
        if (!expr.inner_expression)
        {
            return g10::error("Grouping expression missing inner expression "
                "at {}:{}:{}",
                expr.source_file,
                expr.source_line,
                expr.source_column);
        }

        return evaluate_expression(state, *expr.inner_expression);
    }

    auto codegen::value_to_integer (const value& val) -> g10::result<std::int64_t>
    {
        // Handle each variant type.
        if (std::holds_alternative<std::int64_t>(val))
        {
            return std::get<std::int64_t>(val);
        }

        if (std::holds_alternative<std::uint64_t>(val))
        {
            // Fixed-point: take the integer part (upper 32 bits).
            std::uint64_t fixed = std::get<std::uint64_t>(val);
            return static_cast<std::int64_t>(fixed >> 32);
        }

        if (std::holds_alternative<std::uint32_t>(val))
        {
            // Address: convert to signed integer.
            return static_cast<std::int64_t>(std::get<std::uint32_t>(val));
        }

        if (std::holds_alternative<std::monostate>(val))
        {
            return g10::error("Cannot convert void value to integer");
        }

        if (std::holds_alternative<std::string>(val))
        {
            return g10::error("Cannot convert string value to integer");
        }

        return g10::error("Unknown value type");
    }

    auto codegen::is_address_value (const value& val) -> bool
    {
        return std::holds_alternative<std::uint32_t>(val);
    }

    auto codegen::evaluate_as_integer (
        codegen_state& state,
        const ast_expression& expr
    ) -> g10::result<std::int64_t>
    {
        // Evaluate the expression.
        auto result = evaluate_expression(state, expr);
        if (!result.has_value())
        {
            return g10::error(result.error());
        }

        // Convert to integer.
        auto int_result = value_to_integer(result.value());
        if (!int_result.has_value())
        {
            return g10::error("Expression does not evaluate to an integer "
                "at {}:{}:{}",
                expr.source_file,
                expr.source_line,
                expr.source_column);
        }

        return int_result.value();
    }

    auto codegen::evaluate_as_address (
        codegen_state& state,
        const ast_expression& expr
    ) -> g10::result<std::uint32_t>
    {
        // Evaluate the expression.
        auto result = evaluate_expression(state, expr);
        if (!result.has_value())
        {
            return g10::error(result.error());
        }

        const value& val = result.value();

        // If already an address, return directly.
        if (std::holds_alternative<std::uint32_t>(val))
        {
            return std::get<std::uint32_t>(val);
        }

        // If fixed-point, extract integer part and validate range.
        if (std::holds_alternative<std::uint64_t>(val))
        {
            std::uint64_t fixed_val = std::get<std::uint64_t>(val);
            std::uint32_t int_part = static_cast<std::uint32_t>(fixed_val >> 32);
            return int_part;
        }

        // If integer, validate range and convert.
        if (std::holds_alternative<std::int64_t>(val))
        {
            std::int64_t int_val = std::get<std::int64_t>(val);
            if (int_val < 0 || int_val > 0xFFFFFFFF)
            {
                return g10::error("Address value out of range: {} at {}:{}:{}",
                    int_val,
                    expr.source_file,
                    expr.source_line,
                    expr.source_column);
            }
            return static_cast<std::uint32_t>(int_val);
        }

        return g10::error("Expression does not evaluate to an address "
            "at {}:{}:{}",
            expr.source_file,
            expr.source_line,
            expr.source_column);
    }

    auto codegen::references_external (
        codegen_state& state,
        const ast_expression& expr
    ) -> bool
    {
        switch (expr.type)
        {
            case ast_node_type::expr_primary:
            {
                const auto& primary = static_cast<const ast_expr_primary&>(expr);
                if (primary.expr_type == ast_expr_primary::primary_type::identifier)
                {
                    std::string name;
                    if (std::holds_alternative<std::string_view>(primary.value))
                    {
                        name = std::string { std::get<std::string_view>(primary.value) };
                    }
                    else
                    {
                        name = std::string { primary.lexeme };
                    }
                    return state.extern_symbols.contains(name);
                }
                return false;
            }

            case ast_node_type::expr_binary:
            {
                const auto& binary = static_cast<const ast_expr_binary&>(expr);
                bool left_ext = binary.left_operand && 
                    references_external(state, *binary.left_operand);
                bool right_ext = binary.right_operand &&
                    references_external(state, *binary.right_operand);
                return left_ext || right_ext;
            }

            case ast_node_type::expr_unary:
            {
                const auto& unary = static_cast<const ast_expr_unary&>(expr);
                return unary.operand && references_external(state, *unary.operand);
            }

            case ast_node_type::expr_grouping:
            {
                const auto& grouping = static_cast<const ast_expr_grouping&>(expr);
                return grouping.inner_expression &&
                    references_external(state, *grouping.inner_expression);
            }

            default:
                return false;
        }
    }
}

/* Private Methods - Code Emission ********************************************/

namespace g10asm
{
    auto codegen::emit_byte (codegen_state& state, std::uint8_t byte) -> void
    {
        // Get the current section's data vector.
        auto& sections = const_cast<std::vector<g10::object_section>&>(
            state.object.get_sections());
        
        if (state.current_section_index < sections.size())
        {
            auto& section = sections[state.current_section_index];
            section.data.push_back(byte);
        }

        // Advance the location counter.
        state.location_counter += 1;
    }

    auto codegen::emit_word (codegen_state& state, std::uint16_t word) -> void
    {
        // Emit in little-endian order.
        emit_byte(state, static_cast<std::uint8_t>(word & 0xFF));
        emit_byte(state, static_cast<std::uint8_t>((word >> 8) & 0xFF));
    }

    auto codegen::emit_dword (codegen_state& state, std::uint32_t dword) -> void
    {
        // Emit in little-endian order.
        emit_byte(state, static_cast<std::uint8_t>(dword & 0xFF));
        emit_byte(state, static_cast<std::uint8_t>((dword >> 8) & 0xFF));
        emit_byte(state, static_cast<std::uint8_t>((dword >> 16) & 0xFF));
        emit_byte(state, static_cast<std::uint8_t>((dword >> 24) & 0xFF));
    }

    auto codegen::emit_bytes (
        codegen_state& state,
        std::span<const std::uint8_t> data
    ) -> void
    {
        for (const auto byte : data)
        {
            emit_byte(state, byte);
        }
    }

    auto codegen::current_section_offset (const codegen_state& state)
        -> std::uint32_t
    {
        const auto& sections = state.object.get_sections();
        if (state.current_section_index < sections.size())
        {
            const auto& section = sections[state.current_section_index];
            return static_cast<std::uint32_t>(section.data.size());
        }
        return 0;
    }

    auto codegen::create_relocation (
        codegen_state& state,
        const std::string& symbol_name,
        g10::relocation_type type,
        std::int16_t addend
    ) -> g10::result<void>
    {
        // Find the symbol index.
        auto symbol_index = state.object.find_symbol(symbol_name);
        if (!symbol_index.has_value())
        {
            return g10::error("Cannot create relocation: symbol '{}' not found",
                symbol_name);
        }

        // Create the relocation entry.
        g10::object_relocation reloc;
        reloc.offset = current_section_offset(state);
        reloc.symbol_index = static_cast<std::uint32_t>(symbol_index.value());
        reloc.section_index = static_cast<std::uint32_t>(state.current_section_index);
        reloc.type = type;
        reloc.addend = static_cast<std::int32_t>(addend);

        // Add to the object file.
        auto result = state.object.add_relocation(reloc);
        if (!result.has_value())
        {
            return g10::error("Failed to add relocation: {}", result.error());
        }

        return {};
    }
}

/* Private Methods - Instruction Emission *************************************/

namespace g10asm
{
    auto codegen::get_register_index (g10::register_type reg) -> std::uint8_t
    {
        // Register index is the lower 4 bits of the register type.
        return static_cast<std::uint8_t>(std::to_underlying(reg) & 0x0F);
    }

    auto codegen::get_register_size_class (g10::register_type reg) -> std::uint8_t
    {
        // Determine size class from the register type encoding:
        // - Dn (0x00-0x0F): dword (32-bit) = 2
        // - Wn (0x10-0x1F): word (16-bit) = 1
        // - Hn (0x20-0x2F): byte high = 0
        // - Ln (0x40-0x4F): byte low = 0
        const std::uint8_t type_bits = 
            (std::to_underlying(reg) >> 4) & 0x07;
        switch (type_bits)
        {
            case 0: return 2;   // Dn = dword
            case 1: return 1;   // Wn = word
            case 2:             // Hn = byte (high)
            case 4: return 0;   // Ln = byte (low)
            default: return 2;  // Default to dword for special regs
        }
    }

    auto codegen::emit_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // Dispatch based on instruction type.
        switch (instr.instruction)
        {
            // CPU Control Instructions (no operands)
            case g10::instruction::nop:
            case g10::instruction::stop:
            case g10::instruction::halt:
            case g10::instruction::di:
            case g10::instruction::ei:
            case g10::instruction::eii:
            case g10::instruction::daa:
            case g10::instruction::scf:
            case g10::instruction::ccf:
            case g10::instruction::tcf:
            case g10::instruction::clv:
            case g10::instruction::sev:
            case g10::instruction::reti:
                return emit_no_operand_instruction(state, instr);

            // Load Instructions
            case g10::instruction::ld:
            case g10::instruction::ldq:
            case g10::instruction::ldp:
                return emit_load_instruction(state, instr);

            // Store Instructions
            case g10::instruction::st:
            case g10::instruction::stq:
            case g10::instruction::stp:
                return emit_store_instruction(state, instr);

            // Move Instructions
            case g10::instruction::mv:
            case g10::instruction::mwh:
            case g10::instruction::mwl:
                return emit_move_instruction(state, instr);

            // Stack Instructions
            case g10::instruction::lsp:
            case g10::instruction::pop:
            case g10::instruction::ssp:
            case g10::instruction::push:
            case g10::instruction::spo:
            case g10::instruction::spi:
                return emit_stack_instruction(state, instr);

            // Branch Instructions
            case g10::instruction::jmp:
            case g10::instruction::jp:
            case g10::instruction::jpb:
            case g10::instruction::jr:
            case g10::instruction::call:
            case g10::instruction::int_:
            case g10::instruction::ret:
                return emit_branch_instruction(state, instr);

            // ALU Instructions
            case g10::instruction::add:
            case g10::instruction::adc:
            case g10::instruction::sub:
            case g10::instruction::sbc:
            case g10::instruction::inc:
            case g10::instruction::dec:
            case g10::instruction::and_:
            case g10::instruction::or_:
            case g10::instruction::xor_:
            case g10::instruction::not_:
            case g10::instruction::cmp:
            case g10::instruction::cp:
            case g10::instruction::cpl:
                return emit_alu_instruction(state, instr);

            // Shift/Rotate Instructions
            case g10::instruction::sla:
            case g10::instruction::sra:
            case g10::instruction::srl:
            case g10::instruction::swap:
            case g10::instruction::rla:
            case g10::instruction::rl:
            case g10::instruction::rlca:
            case g10::instruction::rlc:
            case g10::instruction::rra:
            case g10::instruction::rr:
            case g10::instruction::rrca:
            case g10::instruction::rrc:
                return emit_shift_instruction(state, instr);

            // Bit Manipulation Instructions
            case g10::instruction::bit:
            case g10::instruction::set:
            case g10::instruction::res:
            case g10::instruction::tog:
                return emit_bit_instruction(state, instr);

            default:
                return g10::error("Unknown instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }
    }

    auto codegen::emit_no_operand_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // CPU Control Instructions: opcodes are 0x0I00 where I is the
        // instruction index.
        std::uint16_t opcode = 0x0000;

        switch (instr.instruction)
        {
            case g10::instruction::nop:   opcode = 0x0000; break;
            case g10::instruction::stop:  opcode = 0x0100; break;
            case g10::instruction::halt:  opcode = 0x0200; break;
            case g10::instruction::di:    opcode = 0x0300; break;
            case g10::instruction::ei:    opcode = 0x0400; break;
            case g10::instruction::eii:   opcode = 0x0500; break;
            case g10::instruction::daa:   opcode = 0x0600; break;
            case g10::instruction::scf:   opcode = 0x0700; break;
            case g10::instruction::ccf:
            case g10::instruction::tcf:   opcode = 0x0800; break;
            case g10::instruction::clv:   opcode = 0x0900; break;
            case g10::instruction::sev:   opcode = 0x0A00; break;
            case g10::instruction::reti:  opcode = 0x4600; break;

            default:
                return g10::error("Invalid no-operand instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }

        emit_word(state, opcode);
        return {};
    }

    auto codegen::emit_load_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // Load instructions require at least 2 operands: destination, source.
        if (instr.operands.size() < 2)
        {
            return g10::error("Load instruction requires 2 operands at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        // First operand must be a register (destination).
        if (instr.operands[0]->type != ast_node_type::opr_register)
        {
            return g10::error("Load destination must be a register at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        const auto& dest_reg_node = 
            static_cast<const ast_opr_register&>(*instr.operands[0]);
        const g10::register_type dest_reg = dest_reg_node.reg;
        const std::uint8_t dest_idx = get_register_index(dest_reg);
        const std::uint8_t size_class = get_register_size_class(dest_reg);

        // Second operand determines the addressing mode.
        const auto& src_node = *instr.operands[1];
        std::uint16_t opcode = 0x0000;

        switch (src_node.type)
        {
            case ast_node_type::opr_immediate:
            {
                // LD reg, IMM - Load immediate
                const auto& imm_node = 
                    static_cast<const ast_opr_immediate&>(src_node);
                if (!imm_node.value)
                {
                    return g10::error("Immediate operand missing value at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                // Evaluate immediate value.
                auto result = evaluate_as_integer(state, *imm_node.value);
                if (!result.has_value())
                {
                    return g10::error("Invalid immediate value: {} at {}:{}:{}",
                        result.error(),
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                // Determine opcode based on register size.
                switch (size_class)
                {
                    case 0: // 8-bit: 0x10X0
                        opcode = 0x1000 | (dest_idx << 4);
                        emit_word(state, opcode);
                        emit_byte(state, static_cast<std::uint8_t>(
                            result.value() & 0xFF));
                        break;

                    case 1: // 16-bit: 0x20X0
                        opcode = 0x2000 | (dest_idx << 4);
                        emit_word(state, opcode);
                        emit_word(state, static_cast<std::uint16_t>(
                            result.value() & 0xFFFF));
                        break;

                    case 2: // 32-bit: 0x30X0
                        opcode = 0x3000 | (dest_idx << 4);
                        emit_word(state, opcode);
                        emit_dword(state, static_cast<std::uint32_t>(
                            result.value() & 0xFFFFFFFF));
                        break;
                }
                return {};
            }

            case ast_node_type::opr_direct:
            {
                // LD reg, [ADDR] - Load from memory address
                const auto& dir_node = 
                    static_cast<const ast_opr_direct&>(src_node);
                if (!dir_node.address)
                {
                    return g10::error("Direct operand missing address at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                // Check for Quick (LDQ) or Port (LDP) addressing.
                if (instr.instruction == g10::instruction::ldq)
                {
                    // LDQ: 16-bit relative to $FFFF0000
                    // Evaluate address.
                    auto result = evaluate_as_address(state, *dir_node.address);
                    if (!result.has_value())
                    {
                        return g10::error("Invalid address: {} at {}:{}:{}",
                            result.error(),
                            instr.source_file,
                            instr.source_line,
                            instr.source_column);
                    }
                    std::uint32_t addr = result.value();
                    switch (size_class)
                    {
                        case 0: opcode = 0x1300 | (dest_idx << 4); break;
                        case 1: opcode = 0x2300 | (dest_idx << 4); break;
                        case 2: opcode = 0x3300 | (dest_idx << 4); break;
                    }
                    emit_word(state, opcode);
                    emit_word(state, static_cast<std::uint16_t>(addr & 0xFFFF));
                }
                else if (instr.instruction == g10::instruction::ldp)
                {
                    // LDP: 8-bit relative to $FFFFFF00 (byte only)
                    auto result = evaluate_as_address(state, *dir_node.address);
                    if (!result.has_value())
                    {
                        return g10::error("Invalid address: {} at {}:{}:{}",
                            result.error(),
                            instr.source_file,
                            instr.source_line,
                            instr.source_column);
                    }
                    std::uint32_t addr = result.value();
                    opcode = 0x1500 | (dest_idx << 4);
                    emit_word(state, opcode);
                    emit_byte(state, static_cast<std::uint8_t>(addr & 0xFF));
                }
                else
                {
                    // LD: 32-bit absolute
                    switch (size_class)
                    {
                        case 0: opcode = 0x1100 | (dest_idx << 4); break;
                        case 1: opcode = 0x2100 | (dest_idx << 4); break;
                        case 2: opcode = 0x3100 | (dest_idx << 4); break;
                    }
                    emit_word(state, opcode);

                    // Check if the address references an external symbol.
                    bool needs_reloc = references_external(state, *dir_node.address);
                    if (needs_reloc)
                    {
                        // Find the external symbol name.
                        std::string symbol_name;
                        if (dir_node.address->type == ast_node_type::expr_primary)
                        {
                            const auto& primary = 
                                static_cast<const ast_expr_primary&>(*dir_node.address);
                            if (std::holds_alternative<std::string_view>(primary.value))
                            {
                                symbol_name = std::string { 
                                    std::get<std::string_view>(primary.value) };
                            }
                            else
                            {
                                symbol_name = std::string { primary.lexeme };
                            }
                        }

                        // Emit placeholder FIRST (so offset is valid for relocation).
                        emit_dword(state, 0x00000000);

                        // Create relocation for 32-bit absolute address.
                        auto symbol_index = state.object.find_symbol(symbol_name);
                        if (!symbol_index.has_value())
                        {
                            return g10::error("Cannot create relocation: symbol '{}' not found",
                                symbol_name);
                        }

                        g10::object_relocation reloc;
                        reloc.offset = current_section_offset(state) - 4;
                        reloc.symbol_index = static_cast<std::uint32_t>(symbol_index.value());
                        reloc.section_index = static_cast<std::uint32_t>(state.current_section_index);
                        reloc.type = g10::relocation_type::abs32;
                        reloc.addend = 0;

                        auto result = state.object.add_relocation(reloc);
                        if (!result.has_value())
                        {
                            return g10::error("Failed to add relocation: {}", result.error());
                        }
                    }
                    else
                    {
                        // Evaluate address.
                        auto result = evaluate_as_address(state, *dir_node.address);
                        if (!result.has_value())
                        {
                            return g10::error("Invalid address: {} at {}:{}:{}",
                                result.error(),
                                instr.source_file,
                                instr.source_line,
                                instr.source_column);
                        }
                        emit_dword(state, result.value());
                    }
                }
                return {};
            }

            case ast_node_type::opr_indirect:
            {
                // LD reg, [reg] - Load from address in register
                const auto& ind_node = 
                    static_cast<const ast_opr_indirect&>(src_node);
                const std::uint8_t base_idx = get_register_index(ind_node.base_register);
                const std::uint8_t base_size = get_register_size_class(ind_node.base_register);

                if (instr.instruction == g10::instruction::ldq)
                {
                    // LDQ LX, [WY] - Quick indirect (word register base)
                    switch (size_class)
                    {
                        case 0: opcode = 0x1400; break;
                        case 1: opcode = 0x2400; break;
                        case 2: opcode = 0x3400; break;
                    }
                }
                else if (instr.instruction == g10::instruction::ldp)
                {
                    // LDP LX, [LY] - Port indirect (byte register base)
                    opcode = 0x1600;
                }
                else
                {
                    // LD reg, [Dreg] - Indirect (dword register base)
                    switch (size_class)
                    {
                        case 0: opcode = 0x1200; break;
                        case 1: opcode = 0x2200; break;
                        case 2: opcode = 0x3200; break;
                    }
                }

                opcode |= (dest_idx << 4) | base_idx;
                emit_word(state, opcode);
                return {};
            }

            default:
                return g10::error("Invalid source operand for load at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }
    }

    auto codegen::emit_store_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // Store instructions: ST [dest], src_reg
        if (instr.operands.size() < 2)
        {
            return g10::error("Store instruction requires 2 operands at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        // Second operand is the source register.
        if (instr.operands[1]->type != ast_node_type::opr_register)
        {
            return g10::error("Store source must be a register at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        const auto& src_reg_node = 
            static_cast<const ast_opr_register&>(*instr.operands[1]);
        const g10::register_type src_reg = src_reg_node.reg;
        const std::uint8_t src_idx = get_register_index(src_reg);
        const std::uint8_t size_class = get_register_size_class(src_reg);

        // First operand is the destination (memory).
        const auto& dest_node = *instr.operands[0];
        std::uint16_t opcode = 0x0000;

        switch (dest_node.type)
        {
            case ast_node_type::opr_direct:
            {
                // ST [ADDR], reg - Store to memory address
                const auto& dir_node = 
                    static_cast<const ast_opr_direct&>(dest_node);
                if (!dir_node.address)
                {
                    return g10::error("Direct operand missing address at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                auto result = evaluate_as_address(state, *dir_node.address);
                if (!result.has_value())
                {
                    return g10::error("Invalid address: {} at {}:{}:{}",
                        result.error(),
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                std::uint32_t addr = result.value();

                if (instr.instruction == g10::instruction::stq)
                {
                    // STQ: 16-bit relative to $FFFF0000
                    switch (size_class)
                    {
                        case 0: opcode = 0x1900 | src_idx; break;
                        case 1: opcode = 0x2900 | src_idx; break;
                        case 2: opcode = 0x3900 | src_idx; break;
                    }
                    emit_word(state, opcode);
                    emit_word(state, static_cast<std::uint16_t>(addr & 0xFFFF));
                }
                else if (instr.instruction == g10::instruction::stp)
                {
                    // STP: 8-bit relative to $FFFFFF00 (byte only)
                    opcode = 0x1B00 | src_idx;
                    emit_word(state, opcode);
                    emit_byte(state, static_cast<std::uint8_t>(addr & 0xFF));
                }
                else
                {
                    // ST: 32-bit absolute
                    switch (size_class)
                    {
                        case 0: opcode = 0x1700 | src_idx; break;
                        case 1: opcode = 0x2700 | src_idx; break;
                        case 2: opcode = 0x3700 | src_idx; break;
                    }
                    emit_word(state, opcode);
                    emit_dword(state, addr);
                }
                return {};
            }

            case ast_node_type::opr_indirect:
            {
                // ST [reg], src_reg - Store to address in register
                const auto& ind_node = 
                    static_cast<const ast_opr_indirect&>(dest_node);
                const std::uint8_t base_idx = get_register_index(ind_node.base_register);

                if (instr.instruction == g10::instruction::stq)
                {
                    // STQ [WX], LY
                    switch (size_class)
                    {
                        case 0: opcode = 0x1A00; break;
                        case 1: opcode = 0x2A00; break;
                        case 2: opcode = 0x3A00; break;
                    }
                }
                else if (instr.instruction == g10::instruction::stp)
                {
                    // STP [LX], LY
                    opcode = 0x1C00;
                }
                else
                {
                    // ST [DX], reg
                    switch (size_class)
                    {
                        case 0: opcode = 0x1800; break;
                        case 1: opcode = 0x2800; break;
                        case 2: opcode = 0x3800; break;
                    }
                }

                opcode |= (base_idx << 4) | src_idx;
                emit_word(state, opcode);
                return {};
            }

            default:
                return g10::error("Invalid destination for store at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }
    }

    auto codegen::emit_move_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // Move instructions: MV dest, src
        if (instr.operands.size() < 2)
        {
            return g10::error("Move instruction requires 2 operands at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        // Both operands must be registers.
        if (instr.operands[0]->type != ast_node_type::opr_register ||
            instr.operands[1]->type != ast_node_type::opr_register)
        {
            return g10::error("Move operands must be registers at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        const auto& dest_node = 
            static_cast<const ast_opr_register&>(*instr.operands[0]);
        const auto& src_node = 
            static_cast<const ast_opr_register&>(*instr.operands[1]);

        const std::uint8_t dest_idx = get_register_index(dest_node.reg);
        const std::uint8_t src_idx = get_register_index(src_node.reg);
        const std::uint8_t dest_size = get_register_size_class(dest_node.reg);
        const std::uint8_t src_size = get_register_size_class(src_node.reg);

        std::uint16_t opcode = 0x0000;

        switch (instr.instruction)
        {
            case g10::instruction::mv:
            {
                // MV reg, reg - Move same-size registers
                switch (dest_size)
                {
                    case 0:
                    {
                        // Byte moves: MV LX, LY or MV HX, LY or MV LX, HY
                        const std::uint8_t dest_type = 
                            (std::to_underlying(dest_node.reg) >> 4) & 0x07;
                        const std::uint8_t src_type = 
                            (std::to_underlying(src_node.reg) >> 4) & 0x07;
                        
                        if (dest_type == 4 && src_type == 4)
                        {
                            // MV LX, LY
                            opcode = 0x1D00;
                        }
                        else if (dest_type == 2 && src_type == 4)
                        {
                            // MV HX, LY
                            opcode = 0x1E00;
                        }
                        else if (dest_type == 4 && src_type == 2)
                        {
                            // MV LX, HY
                            opcode = 0x1F00;
                        }
                        else
                        {
                            return g10::error("Invalid byte move combination at {}:{}:{}",
                                instr.source_file,
                                instr.source_line,
                                instr.source_column);
                        }
                        break;
                    }
                    case 1:
                        opcode = 0x2D00; // MV WX, WY
                        break;
                    case 2:
                        opcode = 0x3D00; // MV DX, DY
                        break;
                }
                break;
            }

            case g10::instruction::mwh:
                // MWH DX, WY - Move word to high half of dword
                opcode = 0x2E00;
                break;

            case g10::instruction::mwl:
                // MWL WX, DY - Move high half of dword to word
                opcode = 0x2F00;
                break;

            default:
                return g10::error("Invalid move instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }

        opcode |= (dest_idx << 4) | src_idx;
        emit_word(state, opcode);
        return {};
    }

    auto codegen::emit_stack_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        std::uint16_t opcode = 0x0000;

        switch (instr.instruction)
        {
            case g10::instruction::lsp:
            {
                // LSP IMM32 - Load stack pointer
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_immediate)
                {
                    return g10::error("LSP requires immediate operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& imm_node = 
                    static_cast<const ast_opr_immediate&>(*instr.operands[0]);
                if (!imm_node.value)
                {
                    return g10::error("Immediate operand missing value at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                auto result = evaluate_as_address(state, *imm_node.value);
                if (!result.has_value())
                {
                    return g10::error("Invalid SP value: {} at {}:{}:{}",
                        result.error(),
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x3500;
                emit_word(state, opcode);
                emit_dword(state, result.value());
                return {};
            }

            case g10::instruction::pop:
            {
                // POP DX - Pop dword from stack
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_register)
                {
                    return g10::error("POP requires register operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& reg_node = 
                    static_cast<const ast_opr_register&>(*instr.operands[0]);
                const std::uint8_t reg_idx = get_register_index(reg_node.reg);

                // Make sure the register is a dword register.
                const std::uint8_t reg_size = get_register_size_class(reg_node.reg);
                if (reg_size != 2)
                {
                    return g10::error("POP requires dword register at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x3600 | (reg_idx << 4);
                emit_word(state, opcode);
                return {};
            }

            case g10::instruction::ssp:
            {
                // SSP [ADDR32] - Store stack pointer
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_direct)
                {
                    return g10::error("SSP requires direct memory operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& dir_node = 
                    static_cast<const ast_opr_direct&>(*instr.operands[0]);
                if (!dir_node.address)
                {
                    return g10::error("Direct operand missing address at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                auto result = evaluate_as_address(state, *dir_node.address);
                if (!result.has_value())
                {
                    return g10::error("Invalid address: {} at {}:{}:{}",
                        result.error(),
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x3B00;
                emit_word(state, opcode);
                emit_dword(state, result.value());
                return {};
            }

            case g10::instruction::push:
            {
                // PUSH DY - Push dword to stack
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_register)
                {
                    return g10::error("PUSH requires register operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& reg_node = 
                    static_cast<const ast_opr_register&>(*instr.operands[0]);
                const std::uint8_t reg_idx = get_register_index(reg_node.reg);

                // Make sure the register is a dword register.
                const std::uint8_t reg_size = get_register_size_class(reg_node.reg);
                if (reg_size != 2)  
                {
                    return g10::error("PUSH requires dword register at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x3C00 | reg_idx;
                emit_word(state, opcode);
                return {};
            }

            case g10::instruction::spo:
            {
                // SPO DX - Stack pointer out (move SP to register)
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_register)
                {
                    return g10::error("SPO requires register operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& reg_node = 
                    static_cast<const ast_opr_register&>(*instr.operands[0]);
                const std::uint8_t reg_idx = get_register_index(reg_node.reg);

                opcode = 0x3E00 | (reg_idx << 4);
                emit_word(state, opcode);
                return {};
            }

            case g10::instruction::spi:
            {
                // SPI DY - Stack pointer in (move register to SP)
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_register)
                {
                    return g10::error("SPI requires register operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& reg_node = 
                    static_cast<const ast_opr_register&>(*instr.operands[0]);
                const std::uint8_t reg_idx = get_register_index(reg_node.reg);

                opcode = 0x3F00 | reg_idx;
                emit_word(state, opcode);
                return {};
            }

            default:
                return g10::error("Invalid stack instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }
    }

    auto codegen::emit_branch_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        std::uint16_t opcode = 0x0000;
        std::uint8_t condition = g10::CC_NO_CONDITION;
        std::size_t operand_start = 0;

        // Check if first operand is a condition code.
        if (!instr.operands.empty() &&
            instr.operands[0]->type == ast_node_type::opr_condition)
        {
            const auto& cond_node = 
                static_cast<const ast_opr_condition&>(*instr.operands[0]);
            condition = static_cast<std::uint8_t>(cond_node.condition);
            operand_start = 1;
        }

        switch (instr.instruction)
        {
            case g10::instruction::jmp:
            case g10::instruction::jp:
            {
                if (operand_start >= instr.operands.size())
                {
                    return g10::error("JMP requires target operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& target_node = *instr.operands[operand_start];

                if (target_node.type == ast_node_type::opr_immediate)
                {
                    // JMP cond, IMM32
                    const auto& imm_node = 
                        static_cast<const ast_opr_immediate&>(target_node);
                    if (!imm_node.value)
                    {
                        return g10::error("Immediate missing value at {}:{}:{}",
                            instr.source_file,
                            instr.source_line,
                            instr.source_column);
                    }

                    auto result = evaluate_as_address(state, *imm_node.value);
                    if (!result.has_value())
                    {
                        return g10::error("Invalid jump target: {} at {}:{}:{}",
                            result.error(),
                            instr.source_file,
                            instr.source_line,
                            instr.source_column);
                    }

                    opcode = 0x4000 | (condition << 4);
                    emit_word(state, opcode);
                    emit_dword(state, result.value());
                }
                else if (target_node.type == ast_node_type::opr_register)
                {
                    // JMP cond, DY
                    const auto& reg_node = 
                        static_cast<const ast_opr_register&>(target_node);
                    const std::uint8_t reg_idx = get_register_index(reg_node.reg);

                    opcode = 0x4100 | (condition << 4) | reg_idx;
                    emit_word(state, opcode);
                }
                else
                {
                    return g10::error("Invalid jump target at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }
                return {};
            }

            case g10::instruction::jpb:
            case g10::instruction::jr:
            {
                // JPB cond, SIMM16 - Relative jump
                if (operand_start >= instr.operands.size())
                {
                    return g10::error("JPB requires offset operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& target_node = *instr.operands[operand_start];
                if (target_node.type != ast_node_type::opr_immediate)
                {
                    return g10::error("JPB requires immediate offset at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& imm_node = 
                    static_cast<const ast_opr_immediate&>(target_node);
                if (!imm_node.value)
                {
                    return g10::error("Immediate missing value at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                auto result = evaluate_as_integer(state, *imm_node.value);
                if (!result.has_value())
                {
                    return g10::error("Invalid offset: {} at {}:{}:{}",
                        result.error(),
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                std::int64_t offset = result.value();
                
                // Check if this is a label reference (need to calculate relative offset).
                // If the value is an address (uint32_t), convert to relative offset.
                auto val_result = evaluate_expression(state, *imm_node.value);
                if (val_result.has_value() && is_address_value(val_result.value()))
                {
                    // Label reference: calculate relative offset.
                    std::uint32_t target_addr = 
                        std::get<std::uint32_t>(val_result.value());
                    // Offset is from the address AFTER this instruction (PC + 4).
                    std::uint32_t next_pc = state.location_counter + 4;
                    offset = static_cast<std::int64_t>(target_addr) - 
                             static_cast<std::int64_t>(next_pc);
                }

                // Validate offset range (-32768 to 32767).
                if (offset < -32768 || offset > 32767)
                {
                    return g10::error("Relative offset out of range: {} at {}:{}:{}",
                        offset,
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x4200 | (condition << 4);
                emit_word(state, opcode);
                emit_word(state, static_cast<std::uint16_t>(
                    static_cast<std::int16_t>(offset)));
                return {};
            }

            case g10::instruction::call:
            {
                // CALL cond, IMM32
                if (operand_start >= instr.operands.size())
                {
                    return g10::error("CALL requires target operand at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& target_node = *instr.operands[operand_start];
                if (target_node.type != ast_node_type::opr_immediate)
                {
                    return g10::error("CALL requires immediate address at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& imm_node = 
                    static_cast<const ast_opr_immediate&>(target_node);
                if (!imm_node.value)
                {
                    return g10::error("Immediate missing value at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x4300 | (condition << 4);
                emit_word(state, opcode);

                // Check if the address references an external symbol.
                bool needs_reloc = references_external(state, *imm_node.value);
                if (needs_reloc)
                {
                    // Find the external symbol name.
                    std::string symbol_name;
                    if (imm_node.value->type == ast_node_type::expr_primary)
                    {
                        const auto& primary = 
                            static_cast<const ast_expr_primary&>(*imm_node.value);
                        if (std::holds_alternative<std::string_view>(primary.value))
                        {
                            symbol_name = std::string { 
                                std::get<std::string_view>(primary.value) };
                        }
                        else
                        {
                            symbol_name = std::string { primary.lexeme };
                        }
                    }

                    // Emit placeholder FIRST (so offset is valid for relocation).
                    emit_dword(state, 0x00000000);

                    // Create relocation for 32-bit absolute address.
                    // Offset is 4 bytes back from current position.
                    auto symbol_index = state.object.find_symbol(symbol_name);
                    if (!symbol_index.has_value())
                    {
                        return g10::error("Cannot create relocation: symbol '{}' not found",
                            symbol_name);
                    }

                    g10::object_relocation reloc;
                    reloc.offset = current_section_offset(state) - 4;  // Point to the dword we just emitted
                    reloc.symbol_index = static_cast<std::uint32_t>(symbol_index.value());
                    reloc.section_index = static_cast<std::uint32_t>(state.current_section_index);
                    reloc.type = g10::relocation_type::abs32;
                    reloc.addend = 0;

                    auto result = state.object.add_relocation(reloc);
                    if (!result.has_value())
                    {
                        return g10::error("Failed to add relocation: {}", result.error());
                    }
                }
                else
                {
                    auto result = evaluate_as_address(state, *imm_node.value);
                    if (!result.has_value())
                    {
                        return g10::error("Invalid call target: {} at {}:{}:{}",
                            result.error(),
                            instr.source_file,
                            instr.source_line,
                            instr.source_column);
                    }
                    emit_dword(state, result.value());
                }
                return {};
            }

            case g10::instruction::int_:
            {
                // INT XX - Software interrupt
                if (instr.operands.empty() ||
                    instr.operands[0]->type != ast_node_type::opr_immediate)
                {
                    return g10::error("INT requires interrupt number at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                const auto& imm_node = 
                    static_cast<const ast_opr_immediate&>(*instr.operands[0]);
                if (!imm_node.value)
                {
                    return g10::error("Immediate missing value at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                auto result = evaluate_as_integer(state, *imm_node.value);
                if (!result.has_value())
                {
                    return g10::error("Invalid interrupt number: {} at {}:{}:{}",
                        result.error(),
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                std::int64_t int_num = result.value();
                if (int_num < 0 || int_num > 31)
                {
                    return g10::error("Interrupt number out of range (0-31): {} at {}:{}:{}",
                        int_num,
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }

                opcode = 0x4400 | (static_cast<std::uint8_t>(int_num) << 4) |
                         static_cast<std::uint8_t>(int_num);
                emit_word(state, opcode);
                return {};
            }

            case g10::instruction::ret:
            {
                // RET cond
                opcode = 0x4500 | (condition << 4);
                emit_word(state, opcode);
                return {};
            }

            default:
                return g10::error("Invalid branch instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }
    }

    auto codegen::emit_alu_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        std::uint16_t opcode = 0x0000;

        // Handle CPL alias (NOT L0)
        if (instr.instruction == g10::instruction::cpl)
        {
            // CPL is an alias for NOT L0 (8-bit)
            opcode = 0x7900;
            emit_word(state, opcode);
            return {};
        }

        // Check for operands.
        if (instr.operands.empty())
        {
            return g10::error("ALU instruction requires operands at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        // Handle unary operations (INC, DEC, NOT)
        if (instr.instruction == g10::instruction::inc ||
            instr.instruction == g10::instruction::dec ||
            instr.instruction == g10::instruction::not_)
        {
            const auto& operand = *instr.operands[0];

            if (operand.type == ast_node_type::opr_register)
            {
                const auto& reg_node = 
                    static_cast<const ast_opr_register&>(operand);
                const std::uint8_t reg_idx = get_register_index(reg_node.reg);
                const std::uint8_t size_class = get_register_size_class(reg_node.reg);

                if (instr.instruction == g10::instruction::inc)
                {
                    switch (size_class)
                    {
                        case 0: opcode = 0x5C00 | (reg_idx << 4); break; // INC LX
                        case 1: opcode = 0x6C00 | (reg_idx << 4); break; // INC WX
                        case 2: opcode = 0x6D00 | (reg_idx << 4); break; // INC DX
                    }
                }
                else if (instr.instruction == g10::instruction::dec)
                {
                    switch (size_class)
                    {
                        case 0: opcode = 0x5E00 | (reg_idx << 4); break; // DEC LX
                        case 1: opcode = 0x6E00 | (reg_idx << 4); break; // DEC WX
                        case 2: opcode = 0x6F00 | (reg_idx << 4); break; // DEC DX
                    }
                }
                else // NOT
                {
                    switch (size_class)
                    {
                        case 0: opcode = 0x7900 | (reg_idx << 4); break; // NOT LX
                        case 1: opcode = 0x7A00 | (reg_idx << 4); break; // NOT WX
                        case 2: opcode = 0x7B00 | (reg_idx << 4); break; // NOT DX
                    }
                }

                emit_word(state, opcode);
                return {};
            }
            else if (operand.type == ast_node_type::opr_indirect)
            {
                // INC [DX], DEC [DX], or NOT [DX]
                const auto& ind_node = 
                    static_cast<const ast_opr_indirect&>(operand);
                const std::uint8_t reg_idx = get_register_index(ind_node.base_register);

                if (instr.instruction == g10::instruction::inc)
                {
                    opcode = 0x5D00 | (reg_idx << 4); // INC [DX]
                }
                else if (instr.instruction == g10::instruction::dec)
                {
                    opcode = 0x5F00 | (reg_idx << 4); // DEC [DX]
                }
                else // NOT [DX]
                {
                    opcode = 0x7A00 | (reg_idx << 4); // NOT [DX]
                }

                emit_word(state, opcode);
                return {};
            }
            else
            {
                return g10::error("Invalid operand for unary ALU instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
            }
        }

        // Binary ALU operations (ADD, ADC, SUB, SBC, AND, OR, XOR, CMP)
        // First operand is accumulator, second is source.
        if (instr.operands.size() < 2)
        {
            return g10::error("ALU instruction requires 2 operands at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        // First operand should be the accumulator register.
        if (instr.operands[0]->type != ast_node_type::opr_register)
        {
            return g10::error("ALU destination must be accumulator register at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        const auto& dest_reg_node = 
            static_cast<const ast_opr_register&>(*instr.operands[0]);
        const std::uint8_t size_class = get_register_size_class(dest_reg_node.reg);

        switch (instr.instruction)
        {
            // `ADD` and `SUB` require accumulator destination register.
            case g10::instruction::add:
            case g10::instruction::sub:
            {
                if (
                    dest_reg_node.reg != g10::register_type::l0 &&
                    dest_reg_node.reg != g10::register_type::w0 &&
                    dest_reg_node.reg != g10::register_type::d0
                )
                {
                    return g10::error("ALU destination must be accumulator register at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }
            } break;

            // `ADC`, `SBC`, `AND`, `OR`, `XOR`, and `CMP` require the low
            // byte accumulator register `L0` for 8-bit operations.
            case g10::instruction::adc:
            case g10::instruction::sbc:
            case g10::instruction::and_:
            case g10::instruction::or_:
            case g10::instruction::xor_:
            case g10::instruction::cmp:
            case g10::instruction::cp:
            {
                if (
                    dest_reg_node.reg != g10::register_type::l0
                )
                {
                    return g10::error("8-bit ALU destination must be L0 register at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
                }
            }
            default: break;
        }

        // Determine base opcode based on instruction and size.
        std::uint8_t base_offset = 0;
        switch (instr.instruction)
        {
            case g10::instruction::add: base_offset = 0x00; break;
            case g10::instruction::adc: base_offset = 0x03; break;
            case g10::instruction::sub: base_offset = 0x06; break;
            case g10::instruction::sbc: base_offset = 0x09; break;
            case g10::instruction::and_: base_offset = 0x00; break;
            case g10::instruction::or_: base_offset = 0x03; break;
            case g10::instruction::xor_: base_offset = 0x06; break;
            case g10::instruction::cmp:
            case g10::instruction::cp: base_offset = 0x00; break;
            default: break;
        }

        // Get source operand.
        const auto& src_node = *instr.operands[1];

        // Determine opcode category (0x5xxx for 8-bit, 0x6xxx for 16/32-bit,
        // 0x7xxx for logical)
        bool is_logical = (instr.instruction == g10::instruction::and_ ||
                           instr.instruction == g10::instruction::or_ ||
                           instr.instruction == g10::instruction::xor_);
        bool is_compare = (instr.instruction == g10::instruction::cmp ||
                           instr.instruction == g10::instruction::cp);

        if (src_node.type == ast_node_type::opr_immediate)
        {
            // Immediate source
            const auto& imm_node = static_cast<const ast_opr_immediate&>(src_node);
            if (!imm_node.value)
            {
                return g10::error("Immediate missing value at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
            }

            auto result = evaluate_as_integer(state, *imm_node.value);
            if (!result.has_value())
            {
                return g10::error("Invalid immediate: {} at {}:{}:{}",
                    result.error(),
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
            }

            if (is_logical)
            {
                switch (size_class)
                {
                    case 0: opcode = 0x7000 + base_offset * 0x100; break; // AND/OR/XOR L0, IMM8
                    case 1: opcode = 0x7400 + base_offset * 0x100; break; // AND/OR/XOR W0, IMM16
                    case 2: opcode = 0x7800 + base_offset * 0x100; break; // AND/OR/XOR D0, IMM32 (if exists)
                }
            }
            else if (is_compare)
            {
                switch (size_class)
                {
                    case 0: opcode = 0x7D00; break; // CMP L0, IMM8
                    case 1: opcode = 0x7D00; break; // CMP W0, IMM16 (if exists)
                    case 2: opcode = 0x7D00; break; // CMP D0, IMM32 (if exists)
                }
            }
            else
            {
                // Arithmetic - Note: 16/32-bit have different offset patterns than 8-bit
                switch (size_class)
                {
                    case 0: 
                        // 8-bit: ADD=0x5000, ADC=0x5300, SUB=0x5600, SBC=0x5900
                        opcode = 0x5000 + base_offset * 0x100; 
                        break;
                    case 1: 
                        // 16-bit: ADD=0x6000, SUB=0x6400 (offset is base_offset/1.5 * 0x100)
                        // base_offset: 0(ADD), 6(SUB) -> opcode_offset: 0(ADD), 4(SUB)
                        opcode = 0x6000 + (base_offset == 0x06 ? 0x400 : 0x000);
                        break;
                    case 2: 
                        // 32-bit: ADD=0x6200, SUB=0x6600 (offset is base_offset/1.5 * 0x100)
                        // base_offset: 0(ADD), 6(SUB) -> opcode_offset: 0(ADD), 4(SUB)
                        opcode = 0x6200 + (base_offset == 0x06 ? 0x400 : 0x000);
                        break;
                }
            }

            emit_word(state, opcode);

            // Emit immediate value.
            switch (size_class)
            {
                case 0:
                    emit_byte(state, static_cast<std::uint8_t>(result.value() & 0xFF));
                    break;
                case 1:
                    emit_word(state, static_cast<std::uint16_t>(result.value() & 0xFFFF));
                    break;
                case 2:
                    emit_dword(state, static_cast<std::uint32_t>(result.value() & 0xFFFFFFFF));
                    break;
            }
        }
        else if (src_node.type == ast_node_type::opr_register)
        {
            // Register source
            const auto& src_reg_node = 
                static_cast<const ast_opr_register&>(src_node);
            const std::uint8_t src_idx = get_register_index(src_reg_node.reg);

            if (is_logical)
            {
                switch (size_class)
                {
                    case 0: opcode = 0x7100 + base_offset * 0x100 + src_idx; break;
                    case 1: opcode = 0x7500 + base_offset * 0x100 + src_idx; break;
                    case 2: opcode = 0x7800 + base_offset * 0x100 + src_idx; break;
                }
            }
            else if (is_compare)
            {
                switch (size_class)
                {
                    case 0: opcode = 0x7E00 | src_idx; break; // CMP L0, LY
                    case 1: opcode = 0x7E00 | src_idx; break; // CMP W0, WY (if exists)
                    case 2: opcode = 0x7E00 | src_idx; break; // CMP D0, DY (if exists)
                }
            }
            else
            {
                // Arithmetic
                switch (size_class)
                {
                    case 0: 
                        // 8-bit: ADD=0x510Y, ADC=0x540Y, SUB=0x570Y, SBC=0x5A0Y
                        opcode = 0x5100 + base_offset * 0x100 + src_idx; 
                        break;
                    case 1: 
                        // 16-bit: ADD=0x610Y, SUB=0x650Y
                        opcode = 0x6100 + (base_offset == 0x06 ? 0x400 : 0x000) + src_idx;
                        break;
                    case 2: 
                        // 32-bit: ADD=0x630Y, SUB=0x670Y
                        opcode = 0x6300 + (base_offset == 0x06 ? 0x400 : 0x000) + src_idx;
                        break;
                }
            }

            emit_word(state, opcode);
        }
        else if (src_node.type == ast_node_type::opr_indirect)
        {
            // Indirect source (memory via register)
            const auto& ind_node = 
                static_cast<const ast_opr_indirect&>(src_node);
            const std::uint8_t base_idx = get_register_index(ind_node.base_register);

            if (is_logical)
            {
                switch (size_class)
                {
                    case 0: opcode = 0x7200 + base_offset * 0x100 + base_idx; break;
                    case 1: opcode = 0x7600 + base_offset * 0x100 + base_idx; break;
                    case 2: opcode = 0x7800 + base_offset * 0x100 + base_idx; break;
                }
            }
            else if (is_compare)
            {
                switch (size_class)
                {
                    case 0: opcode = 0x7F00 | base_idx; break; // CMP L0, [DY]
                    case 1: opcode = 0x7F00 | base_idx; break; // CMP W0, [DY] (if exists)
                    case 2: opcode = 0x7F00 | base_idx; break; // CMP D0, [DY] (if exists)
                }
            }
            else
            {
                // Arithmetic indirect (only 8-bit has this in spec)
                opcode = 0x5200 + base_offset * 0x100 + base_idx;
            }

            emit_word(state, opcode);
        }
        else
        {
            return g10::error("Invalid source operand for ALU instruction at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        return {};
    }

    auto codegen::emit_shift_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        std::uint16_t opcode = 0x0000;

        // Accumulator-only rotates (no operands)
        switch (instr.instruction)
        {
            case g10::instruction::rla:
                opcode = 0x9000; // RLA (rotate L0 left through carry)
                emit_word(state, opcode);
                return {};

            case g10::instruction::rlca:
                opcode = 0x9300; // RLCA (rotate L0 left circular)
                emit_word(state, opcode);
                return {};

            case g10::instruction::rra:
                opcode = 0x9600; // RRA (rotate L0 right through carry)
                emit_word(state, opcode);
                return {};

            case g10::instruction::rrca:
                opcode = 0x9900; // RRCA (rotate L0 right circular)
                emit_word(state, opcode);
                return {};

            default:
                break; // Continue to register-operand versions
        }

        // Check for indirect operand for shift/swap instructions.
        // Spec supports: SLA [DX], SRA [DX], SRL [DX], SWAP [DX]
        if (!instr.operands.empty() &&
            instr.operands[0]->type == ast_node_type::opr_indirect)
        {
            const auto& ind_node = 
                static_cast<const ast_opr_indirect&>(*instr.operands[0]);
            const std::uint8_t reg_idx = get_register_index(ind_node.base_register);

            switch (instr.instruction)
            {
                case g10::instruction::sla:
                    opcode = 0x8100 | (reg_idx << 4); // SLA [DX]
                    break;
                case g10::instruction::sra:
                    opcode = 0x8300 | (reg_idx << 4); // SRA [DX]
                    break;
                case g10::instruction::srl:
                    opcode = 0x8500 | (reg_idx << 4); // SRL [DX]
                    break;
                case g10::instruction::swap:
                    opcode = 0x8700 | (reg_idx << 4); // SWAP [DX]
                    break;
                case g10::instruction::rl:
                    opcode = 0x9200 | (reg_idx << 4); // RL [DX]
                    break;
                case g10::instruction::rlc:
                    opcode = 0x9500 | (reg_idx << 4); // RLC [DX]
                    break;
                case g10::instruction::rr:
                    opcode = 0x9800 | (reg_idx << 4); // RR [DX]
                    break;
                case g10::instruction::rrc:
                    opcode = 0x9B00 | (reg_idx << 4); // RRC [DX]
                    break;
                default:
                    return g10::error("This instruction does not support indirect mode at {}:{}:{}",
                        instr.source_file,
                        instr.source_line,
                        instr.source_column);
            }

            emit_word(state, opcode);
            return {};
        }

        // Register-operand shifts and rotates.
        if (instr.operands.empty() ||
            instr.operands[0]->type != ast_node_type::opr_register)
        {
            return g10::error("Shift/rotate requires register or indirect operand at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        const auto& reg_node = 
            static_cast<const ast_opr_register&>(*instr.operands[0]);
        const std::uint8_t reg_idx = get_register_index(reg_node.reg);
        const std::uint8_t size_class = get_register_size_class(reg_node.reg);

        switch (instr.instruction)
        {
            case g10::instruction::sla:
                // SLA only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("SLA only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x8000 | (reg_idx << 4); // SLA LX
                break;

            case g10::instruction::sra:
                // SRA only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("SRA only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x8200 | (reg_idx << 4); // SRA LX
                break;

            case g10::instruction::srl:
                // SRL only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("SRL only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x8400 | (reg_idx << 4); // SRL LX
                break;

            case g10::instruction::swap:
                // SWAP works on L, W, and D registers with different opcodes
                switch (size_class)
                {
                    case 0: opcode = 0x8600 | (reg_idx << 4); break; // SWAP LX (nibbles)
                    case 1: opcode = 0x8800 | (reg_idx << 4); break; // SWAP WX (bytes)
                    case 2: opcode = 0x8900 | (reg_idx << 4); break; // SWAP DX (words)
                }
                break;

            case g10::instruction::rl:
                // RL only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("RL only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x9100 | (reg_idx << 4); // RL LX
                break;

            case g10::instruction::rlc:
                // RLC only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("RLC only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x9400 | (reg_idx << 4); // RLC LX
                break;

            case g10::instruction::rr:
                // RR only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("RR only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x9700 | (reg_idx << 4); // RR LX
                break;

            case g10::instruction::rrc:
                // RRC only works on L registers (8-bit)
                if (size_class != 0)
                {
                    return g10::error("RRC only supports L registers at {}:{}:{}",
                        instr.source_file, instr.source_line, instr.source_column);
                }
                opcode = 0x9A00 | (reg_idx << 4); // RRC LX
                break;

            default:
                return g10::error("Invalid shift instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }

        emit_word(state, opcode);
        return {};
    }

    auto codegen::emit_bit_instruction (
        codegen_state& state,
        ast_instruction& instr
    ) -> g10::result<void>
    {
        // Bit instructions: BIT/SET/RES/TOG bit, reg
        if (instr.operands.size() < 2)
        {
            return g10::error("Bit instruction requires 2 operands at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        // First operand is the bit number (immediate).
        if (instr.operands[0]->type != ast_node_type::opr_immediate)
        {
            return g10::error("Bit number must be immediate at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        const auto& bit_node = 
            static_cast<const ast_opr_immediate&>(*instr.operands[0]);
        if (!bit_node.value)
        {
            return g10::error("Bit number missing value at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        auto bit_result = evaluate_as_integer(state, *bit_node.value);
        if (!bit_result.has_value())
        {
            return g10::error("Invalid bit number: {} at {}:{}:{}",
                bit_result.error(),
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        std::int64_t bit_num = bit_result.value();

        // Second operand can be a register or indirect memory.
        bool is_indirect = false;
        std::uint8_t reg_idx = 0;
        std::int64_t max_bit = 7; // Default for 8-bit operations

        if (instr.operands[1]->type == ast_node_type::opr_register)
        {
            const auto& reg_node = 
                static_cast<const ast_opr_register&>(*instr.operands[1]);
            reg_idx = get_register_index(reg_node.reg);
            const std::uint8_t size_class = get_register_size_class(reg_node.reg);

            // Validate bit number based on register size.
            switch (size_class)
            {
                case 0: max_bit = 7; break;
                case 1: max_bit = 15; break;
                case 2: max_bit = 31; break;
            }

            // Note: Spec only defines 8-bit register bit ops (LX), not WX/DX
            if (size_class != 0)
            {
                return g10::error("Bit operations only support L registers at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
            }
        }
        else if (instr.operands[1]->type == ast_node_type::opr_indirect)
        {
            const auto& ind_node = 
                static_cast<const ast_opr_indirect&>(*instr.operands[1]);
            reg_idx = get_register_index(ind_node.base_register);
            is_indirect = true;
            max_bit = 7; // Indirect bit ops work on bytes
        }
        else
        {
            return g10::error("Bit target must be register or [DX] at {}:{}:{}",
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        if (bit_num < 0 || bit_num > max_bit)
        {
            return g10::error("Bit number {} out of range (0-{}) at {}:{}:{}",
                bit_num, max_bit,
                instr.source_file,
                instr.source_line,
                instr.source_column);
        }

        std::uint16_t opcode = 0x0000;

        // Determine base opcode based on instruction type and addressing mode.
        // Format: 0xA0XY BIT Y, LX  / 0xA1XY BIT Y, [DX]
        //         0xA2XY SET Y, LX  / 0xA3XY SET Y, [DX]
        //         0xA4XY RES Y, LX  / 0xA5XY RES Y, [DX]
        //         0xA6XY TOG Y, LX  / 0xA7XY TOG Y, [DX]
        switch (instr.instruction)
        {
            case g10::instruction::bit:
                opcode = is_indirect ? 0xA100 : 0xA000;
                break;

            case g10::instruction::set:
                opcode = is_indirect ? 0xA300 : 0xA200;
                break;

            case g10::instruction::res:
                opcode = is_indirect ? 0xA500 : 0xA400;
                break;

            case g10::instruction::tog:
                opcode = is_indirect ? 0xA700 : 0xA600;
                break;

            default:
                return g10::error("Invalid bit instruction at {}:{}:{}",
                    instr.source_file,
                    instr.source_line,
                    instr.source_column);
        }

        // Encode: opcode | (reg_idx << 4) | bit_num
        // Format is 0xAnXY where X=register, Y=bit number
        opcode |= (reg_idx << 4) | static_cast<std::uint8_t>(bit_num);

        emit_word(state, opcode);
        return {};
    }
}

/* Private Methods - Helper Methods *******************************************/

namespace g10asm
{
    auto codegen::ensure_section (
        codegen_state& state,
        std::uint32_t address
    ) -> g10::result<void>
    {
        // - Determine section type based on address (ROM vs RAM).
        const bool is_rom = (address & 0x80000000) == 0;
        const g10::section_type sec_type = is_rom ?
            g10::section_type::code : g10::section_type::bss;
        const g10::section_flags sec_flags = is_rom ?
            (g10::section_flags::alloc | g10::section_flags::load |
             g10::section_flags::exec) :
            (g10::section_flags::alloc | g10::section_flags::write);

        // - Check if we can reuse the current section.
        const auto& sections = state.object.get_sections();
        if (!sections.empty())
        {
            const auto& current = sections[state.current_section_index];
            
            // - Can reuse if same type and contiguous address.
            if (current.type == sec_type &&
                address == (current.virtual_address +
                    static_cast<std::uint32_t>(current.data.size())))
            {
                return {}; // Continue using current section.
            }
        }

        // - Need to create a new section.
        g10::object_section new_section;
        new_section.name = is_rom ? ".text" : ".bss";
        new_section.virtual_address = address;
        new_section.type = sec_type;
        new_section.flags = sec_flags;
        new_section.data.clear(); // Will be filled in second pass.

        state.current_section_index = state.object.add_section(new_section);
        
        return {};
    }

    auto codegen::calculate_instruction_size (
        const ast_instruction& instr
    ) -> std::size_t
    {
        // - G10 instructions have a 2-byte opcode.
        std::size_t size = 2;

        // - Determine the immediate operand size based on instruction type.
        //   Most instructions use 32-bit immediates, but some use smaller sizes:
        //   - JPB/JR: 16-bit signed offset
        //   - LDQ/STQ: 16-bit address (relative to $FFFF0000)
        //   - LDP/STP: 8-bit address (relative to $FFFFFF00)
        //   - Arithmetic/Logic with immediate: depends on register size
        std::size_t immediate_size = 4;  // Default to 32-bit
        
        switch (instr.instruction)
        {
            case g10::instruction::ld:
            {
                // If operand two is an immediate, then the immediate size will
                // depend on the size of the destination register.
                if (
                    instr.operands.size() >= 2 &&
                    instr.operands[1]->type == ast_node_type::opr_immediate
                )
                {
                    const auto& dest_operand = *instr.operands[0];
                    if (dest_operand.type == ast_node_type::opr_register)
                    {
                        const auto& reg_node = 
                            static_cast<const ast_opr_register&>(dest_operand);
                        const std::uint8_t size_class = get_register_size_class(reg_node.reg);

                        switch (size_class)
                        {
                            case 0: immediate_size = 1; break; // 8-bit
                            case 1: immediate_size = 2; break; // 16-bit
                            case 2: immediate_size = 4; break; // 32-bit
                        }
                    }
                }

                // If operand two is an address, then immediate size is 4 bytes.
                else if (
                    instr.operands.size() >= 2 &&
                    instr.operands[1]->type == ast_node_type::opr_direct
                )
                {
                    immediate_size = 4;
                }
            } break;

            case g10::instruction::st:
            {
                // ST [ADDR32], reg - 4 bytes for address
                // ST [indirect], reg - 0 bytes (register in opcode)
                if (
                    instr.operands.size() >= 1 &&
                    instr.operands[0]->type == ast_node_type::opr_direct
                )
                {
                    immediate_size = 4;
                }
                else
                {
                    immediate_size = 0;
                }
            } break;

            // Arithmetic and logic instructions with immediate operand
            // size depends on the accumulator register size.
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
                // Check if second operand is an immediate.
                if (
                    instr.operands.size() >= 2 &&
                    instr.operands[1]->type == ast_node_type::opr_immediate
                )
                {
                    // Size depends on the destination register.
                    const auto& dest_operand = *instr.operands[0];
                    if (dest_operand.type == ast_node_type::opr_register)
                    {
                        const auto& reg_node = 
                            static_cast<const ast_opr_register&>(dest_operand);
                        const std::uint8_t size_class = get_register_size_class(reg_node.reg);

                        switch (size_class)
                        {
                            case 0: immediate_size = 1; break; // 8-bit
                            case 1: immediate_size = 2; break; // 16-bit
                            case 2: immediate_size = 4; break; // 32-bit
                        }
                    }
                }
                else
                {
                    // Register-to-register: no immediate.
                    immediate_size = 0;
                }
            } break;

            // Move instructions: no immediate data.
            case g10::instruction::mv:
            case g10::instruction::mwh:
            case g10::instruction::mwl:
            case g10::instruction::swap:
                immediate_size = 0;
                break;

            // Increment/Decrement: no immediate data.
            case g10::instruction::inc:
            case g10::instruction::dec:
                immediate_size = 0;
                break;

            // Shift/Rotate instructions: no immediate data.
            case g10::instruction::sla:
            case g10::instruction::sra:
            case g10::instruction::srl:
            case g10::instruction::rl:
            case g10::instruction::rr:
            case g10::instruction::rlc:
            case g10::instruction::rrc:
            case g10::instruction::rla:
            case g10::instruction::rra:
            case g10::instruction::rlca:
            case g10::instruction::rrca:
                immediate_size = 0;
                break;

            // Bitwise NOT: no immediate data.
            case g10::instruction::not_:
            case g10::instruction::cpl:
                immediate_size = 0;
                break;

            // Stack operations: no immediate data.
            case g10::instruction::push:
            case g10::instruction::pop:
            case g10::instruction::spo:
            case g10::instruction::spi:
                immediate_size = 0;
                break;

            // Return instructions: no immediate data.
            case g10::instruction::ret:
            case g10::instruction::reti:
                immediate_size = 0;
                break;

            // Control instructions: no immediate data.
            case g10::instruction::nop:
            case g10::instruction::stop:
            case g10::instruction::halt:
            case g10::instruction::ei:
            case g10::instruction::di:
            case g10::instruction::daa:
            case g10::instruction::scf:
            case g10::instruction::ccf:
                immediate_size = 0;
                break;

            // JPB/JR: 16-bit signed offset.
            case g10::instruction::jpb:
            case g10::instruction::jr:
                immediate_size = 2;
                break;

            // JMP: 32-bit address if not register indirect.
            case g10::instruction::jmp:
                if (instr.operands.size() >= 1)
                {
                    const auto& target_operand = *instr.operands[0];
                    if (target_operand.type == ast_node_type::opr_direct)
                    {
                        immediate_size = 4;
                    }
                    else
                    {
                        immediate_size = 0; // Register indirect
                    }
                }
                break;

            // CALL: 32-bit address.
            case g10::instruction::call:
                immediate_size = 4;
                break;

            // INT: 8-bit interrupt number (but encoded in opcode for some variants).
            case g10::instruction::int_:
                immediate_size = 0;  // Interrupt number is in the opcode.
                break;
            
            case g10::instruction::ldq:
            case g10::instruction::stq:
                // LDQ/STQ use 16-bit addresses relative to $FFFF0000.
                immediate_size = 2;
                break;
            
            case g10::instruction::ldp:
            case g10::instruction::stp:
                // LDP/STP use 8-bit addresses relative to $FFFFFF00.
                // The 8-bit port address is encoded in the opcode itself.
                immediate_size = 0;
                break;

            // Bit operations: immediate bit number may be encoded in opcode.
            case g10::instruction::bit:
            case g10::instruction::set:
            case g10::instruction::res:
                immediate_size = 0;
                break;
            
            default:
                // Default to 32-bit for unknown instructions.
                break;
        }

        // - Add operand sizes.
        for (const auto& operand : instr.operands)
        {
            // - Skip invalid operands.
            if (!operand || !operand->valid)
            {
                continue;
            }

            switch (operand->type)
            {
                case ast_node_type::opr_immediate:
                    // - Immediate values: size depends on instruction type.
                    size += immediate_size;
                    break;

                case ast_node_type::opr_register:
                case ast_node_type::opr_condition:
                    // - Registers and conditions are encoded in the opcode.
                    break;

                case ast_node_type::opr_direct:
                    // - Direct memory address: size depends on instruction type.
                    //   - LD/ST: 4 bytes (32-bit address)
                    //   - LDQ/STQ: 2 bytes (16-bit relative address)
                    //   - LDP/STP: 0 bytes (8-bit port in opcode itself)
                    switch (instr.instruction)
                    {
                        case g10::instruction::ldq:
                        case g10::instruction::stq:
                            size += 2;
                            break;
                        case g10::instruction::ldp:
                        case g10::instruction::stp:
                            // Port address is encoded in the opcode itself.
                            break;
                        default:
                            size += 4;
                            break;
                    }
                    break;

                case ast_node_type::opr_indirect:
                    // - Indirect addressing: register encoded in opcode.
                    break;

                default:
                    break;
            }
        }

        return size;
    }
}
