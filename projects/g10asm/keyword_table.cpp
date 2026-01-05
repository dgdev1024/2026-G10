/**
 * @file    g10asm/keyword_table.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains definitions for the assembler's keyword lookup table.
 */

/* Private Includes ***********************************************************/

#include <g10/cpu.hpp>
#include <g10asm/keyword_table.hpp>

/* Private Static Members *****************************************************/

namespace g10asm
{
    const std::vector<keyword> keyword_table::s_keywords =
    {
        // Instruction Mnemonics
        // - `param1` holds the underlying value of the `g10::instruction` enum.
        // - `param2` holds the minimum number of operands required.
        // - `param3` holds the maximum number of operands allowed.
        { "nop", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::nop), 0 },
        { "stop", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::stop), 0 },
        { "halt", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::halt), 0 },
        { "di", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::di), 0 },
        { "ei", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ei), 0 },
        { "eii", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::eii), 0 },
        { "daa", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::daa), 0 },
        { "scf", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::scf), 0 },
        { "ccf", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ccf), 0 },
        { "clv", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::clv), 0 },
        { "sev", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::sev), 0 },
        { "ld", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ld), 0 },
        { "ldq", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ldq), 0 },
        { "ldp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ldp), 0 },
        { "st", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::st), 0 },
        { "stq", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::stq), 0 },
        { "stp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::stp), 0 },
        { "mv", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::mv), 0 },
        { "mwh", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::mwh), 0 },
        { "mwl", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::mwl), 0 },
        { "lsp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::lsp), 0 },
        { "pop", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::pop), 0 },
        { "ssp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ssp), 0 },
        { "push", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::push), 0 },
        { "spo", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::spo), 0 },
        { "spi", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::spi), 0 },
        { "jmp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::jmp), 0 },
        { "jpb", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::jpb), 0 },
        { "call", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::call), 0 },
        { "int", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::int_), 0 },
        { "ret", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::ret), 0 },
        { "reti", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::reti), 0 },
        { "add", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::add), 0 },
        { "adc", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::adc), 0 },
        { "sub", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::sub), 0 },
        { "sbc", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::sbc), 0 },
        { "inc", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::inc), 0 },
        { "dec", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::dec), 0 },
        { "and", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::and_), 0 },
        { "or", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::or_), 0 },
        { "xor", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::xor_), 0 },
        { "not", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::not_), 0 },
        { "cmp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::cmp), 0 },
        { "sla", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::sla), 0 },
        { "sra", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::sra), 0 },
        { "srl", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::srl), 0 },
        { "swap", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::swap), 0 },
        { "rla", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rla), 0 },
        { "rl", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rl), 0 },
        { "rlca", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rlca), 0 },
        { "rlc", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rlc), 0 },
        { "rra", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rra), 0 },
        { "rr", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rr), 0 },
        { "rrca", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rrca), 0 },
        { "rrc", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::rrc), 0 },
        { "bit", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::bit), 0 },
        { "set", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::set), 0 },
        { "res", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::res), 0 },
        { "tog", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::tog), 0 },

        // Instruction Mnemonic Aliases
        { "tcf", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::tcf), 0 },
        { "jp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::jp), 0 },
        { "jr", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::jr), 0 },
        { "cpl", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::cpl), 0 },
        { "cp", keyword_type::instruction_mnemonic, std::to_underlying(g10::instruction::cp), 0 },

        // Preprocessor Functions
        { "fint", keyword_type::preprocessor_function, std::to_underlying(function_type::fint), 0 },
        { "ffrac", keyword_type::preprocessor_function, std::to_underlying(function_type::ffrac), 0 },
        { "strlen", keyword_type::preprocessor_function, std::to_underlying(function_type::strlen_), 0 },
        { "strcmp", keyword_type::preprocessor_function, std::to_underlying(function_type::strcmp_), 0 },
        { "substr", keyword_type::preprocessor_function, std::to_underlying(function_type::substr_), 0 },
        { "indexof", keyword_type::preprocessor_function, std::to_underlying(function_type::indexof), 0 },
        { "toupper", keyword_type::preprocessor_function, std::to_underlying(function_type::toupper_), 0 },
        { "tolower", keyword_type::preprocessor_function, std::to_underlying(function_type::tolower_), 0 },
        { "concat", keyword_type::preprocessor_function, std::to_underlying(function_type::concat), 0 },
        { "defined", keyword_type::preprocessor_function, std::to_underlying(function_type::defined), 0 },
        { "typeof", keyword_type::preprocessor_function, std::to_underlying(function_type::typeof_), 0 },

        // Preprocessor Directives
        { ".pragma", keyword_type::preprocessor_directive, std::to_underlying(directive_type::pragma), 0 },
        { ".include", keyword_type::preprocessor_directive, std::to_underlying(directive_type::include), 0 },
        { ".define", keyword_type::preprocessor_directive, std::to_underlying(directive_type::define), 0 },
        { ".macro", keyword_type::preprocessor_directive, std::to_underlying(directive_type::macro), 0 },
        { ".shift", keyword_type::preprocessor_directive, std::to_underlying(directive_type::shift), 0 },
        { ".endm", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endmacro), 0 },
        { ".undef", keyword_type::preprocessor_directive, std::to_underlying(directive_type::undef), 0 },
        { ".purge", keyword_type::preprocessor_directive, std::to_underlying(directive_type::undef), 0 },
        { ".ifdef", keyword_type::preprocessor_directive, std::to_underlying(directive_type::ifdef_), 0 },
        { ".ifndef", keyword_type::preprocessor_directive, std::to_underlying(directive_type::ifndef_), 0 },
        { ".if", keyword_type::preprocessor_directive, std::to_underlying(directive_type::if_), 0 },
        { ".elseif", keyword_type::preprocessor_directive, std::to_underlying(directive_type::elseif), 0 },
        { ".elif", keyword_type::preprocessor_directive, std::to_underlying(directive_type::elseif), 0 },
        { ".else", keyword_type::preprocessor_directive, std::to_underlying(directive_type::else_), 0 },
        { ".endif", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endif), 0 },
        { ".endc", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endif), 0 },
        { ".repeat", keyword_type::preprocessor_directive, std::to_underlying(directive_type::repeat), 0 },
        { ".rept", keyword_type::preprocessor_directive, std::to_underlying(directive_type::repeat), 0 },
        { ".endrepeat", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endrepeat), 0 },
        { ".endr", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endrepeat), 0 },
        { ".for", keyword_type::preprocessor_directive, std::to_underlying(directive_type::for_), 0 },
        { ".endfor", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endfor), 0 },
        { ".endf", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endfor), 0 },
        { ".while", keyword_type::preprocessor_directive, std::to_underlying(directive_type::while_), 0 },
        { ".endwhile", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endwhile), 0 },
        { ".endw", keyword_type::preprocessor_directive, std::to_underlying(directive_type::endwhile), 0 },
        { ".continue", keyword_type::preprocessor_directive, std::to_underlying(directive_type::continue_), 0 },
        { ".break", keyword_type::preprocessor_directive, std::to_underlying(directive_type::break_), 0 },
        { ".info", keyword_type::preprocessor_directive, std::to_underlying(directive_type::info), 0 },
        { ".warning", keyword_type::preprocessor_directive, std::to_underlying(directive_type::warning), 0 },
        { ".warn", keyword_type::preprocessor_directive, std::to_underlying(directive_type::warning), 0 },
        { ".error", keyword_type::preprocessor_directive, std::to_underlying(directive_type::error), 0 },
        { ".err", keyword_type::preprocessor_directive, std::to_underlying(directive_type::error), 0 },
        { ".fatal", keyword_type::preprocessor_directive, std::to_underlying(directive_type::fatal), 0 },
        { ".fail", keyword_type::preprocessor_directive, std::to_underlying(directive_type::fatal), 0 },
        { ".critical", keyword_type::preprocessor_directive, std::to_underlying(directive_type::fatal), 0 },
        { ".assert", keyword_type::preprocessor_directive, std::to_underlying(directive_type::assert_), 0 },

        // Assembler Directives
        { ".metadata", keyword_type::assembler_directive, std::to_underlying(directive_type::metadata), 0 },
        { ".meta", keyword_type::assembler_directive, std::to_underlying(directive_type::metadata), 0 },
        { ".interrupt", keyword_type::assembler_directive, std::to_underlying(directive_type::int_), 0 },
        { ".int", keyword_type::assembler_directive, std::to_underlying(directive_type::int_), 0 },
        { ".code", keyword_type::assembler_directive, std::to_underlying(directive_type::code), 0 },
        { ".text", keyword_type::assembler_directive, std::to_underlying(directive_type::code), 0 },
        { ".data", keyword_type::assembler_directive, std::to_underlying(directive_type::data), 0 },
        { ".rodata", keyword_type::assembler_directive, std::to_underlying(directive_type::data), 0 },
        { ".bss", keyword_type::assembler_directive, std::to_underlying(directive_type::bss), 0 },
        { ".org", keyword_type::assembler_directive, std::to_underlying(directive_type::org), 0 },
        { ".byte", keyword_type::assembler_directive, std::to_underlying(directive_type::byte), 0 },
        { ".db", keyword_type::assembler_directive, std::to_underlying(directive_type::byte), 0 },
        { ".word", keyword_type::assembler_directive, std::to_underlying(directive_type::word), 0 },
        { ".dw", keyword_type::assembler_directive, std::to_underlying(directive_type::word), 0 },
        { ".dword", keyword_type::assembler_directive, std::to_underlying(directive_type::dword), 0 },
        { ".dd", keyword_type::assembler_directive, std::to_underlying(directive_type::dword), 0 },
        { ".space", keyword_type::assembler_directive, std::to_underlying(directive_type::space), 0 },
        { ".ds", keyword_type::assembler_directive, std::to_underlying(directive_type::space), 0 },
        { ".global", keyword_type::assembler_directive, std::to_underlying(directive_type::global), 0 },
        { ".extern", keyword_type::assembler_directive, std::to_underlying(directive_type::extern_), 0 },

        // Preprocessor Pragmas
        { "once", keyword_type::pragma, std::to_underlying(pragma_type::once), 0 },
        { "max_recursion_depth", keyword_type::pragma, std::to_underlying(pragma_type::max_recursion_depth), 1 },
        { "max_include_depth", keyword_type::pragma, std::to_underlying(pragma_type::max_include_depth), 1 },

        // Inserted (Automatic) Pragmas
        { "push_file", keyword_type::pragma, std::to_underlying(pragma_type::push_file), 0 },
        { "pop_file", keyword_type::pragma, std::to_underlying(pragma_type::pop_file), 0 },

        // CPU Registers
        { "d0", keyword_type::register_name, std::to_underlying(g10::register_type::d0), 0 },
        { "d1", keyword_type::register_name, std::to_underlying(g10::register_type::d1), 0 },
        { "d2", keyword_type::register_name, std::to_underlying(g10::register_type::d2), 0 },
        { "d3", keyword_type::register_name, std::to_underlying(g10::register_type::d3), 0 },
        { "d4", keyword_type::register_name, std::to_underlying(g10::register_type::d4), 0 },
        { "d5", keyword_type::register_name, std::to_underlying(g10::register_type::d5), 0 },
        { "d6", keyword_type::register_name, std::to_underlying(g10::register_type::d6), 0 },
        { "d7", keyword_type::register_name, std::to_underlying(g10::register_type::d7), 0 },
        { "d8", keyword_type::register_name, std::to_underlying(g10::register_type::d8), 0 },
        { "d9", keyword_type::register_name, std::to_underlying(g10::register_type::d9), 0 },
        { "d10", keyword_type::register_name, std::to_underlying(g10::register_type::d10), 0 },
        { "d11", keyword_type::register_name, std::to_underlying(g10::register_type::d11), 0 },
        { "d12", keyword_type::register_name, std::to_underlying(g10::register_type::d12), 0 },
        { "d13", keyword_type::register_name, std::to_underlying(g10::register_type::d13), 0 },
        { "d14", keyword_type::register_name, std::to_underlying(g10::register_type::d14), 0 },
        { "d15", keyword_type::register_name, std::to_underlying(g10::register_type::d15), 0 },
        { "w0", keyword_type::register_name, std::to_underlying(g10::register_type::w0), 0 },
        { "w1", keyword_type::register_name, std::to_underlying(g10::register_type::w1), 0 },
        { "w2", keyword_type::register_name, std::to_underlying(g10::register_type::w2), 0 },
        { "w3", keyword_type::register_name, std::to_underlying(g10::register_type::w3), 0 },
        { "w4", keyword_type::register_name, std::to_underlying(g10::register_type::w4), 0 },
        { "w5", keyword_type::register_name, std::to_underlying(g10::register_type::w5), 0 },
        { "w6", keyword_type::register_name, std::to_underlying(g10::register_type::w6), 0 },
        { "w7", keyword_type::register_name, std::to_underlying(g10::register_type::w7), 0 },
        { "w8", keyword_type::register_name, std::to_underlying(g10::register_type::w8), 0 },
        { "w9", keyword_type::register_name, std::to_underlying(g10::register_type::w9), 0 },
        { "w10", keyword_type::register_name, std::to_underlying(g10::register_type::w10), 0 },
        { "w11", keyword_type::register_name, std::to_underlying(g10::register_type::w11), 0 },
        { "w12", keyword_type::register_name, std::to_underlying(g10::register_type::w12), 0 },
        { "w13", keyword_type::register_name, std::to_underlying(g10::register_type::w13), 0 },
        { "w14", keyword_type::register_name, std::to_underlying(g10::register_type::w14), 0 },
        { "w15", keyword_type::register_name, std::to_underlying(g10::register_type::w15), 0 },
        { "h0", keyword_type::register_name, std::to_underlying(g10::register_type::h0), 0 },
        { "h1", keyword_type::register_name, std::to_underlying(g10::register_type::h1), 0 },
        { "h2", keyword_type::register_name, std::to_underlying(g10::register_type::h2), 0 },
        { "h3", keyword_type::register_name, std::to_underlying(g10::register_type::h3), 0 },
        { "h4", keyword_type::register_name, std::to_underlying(g10::register_type::h4), 0 },
        { "h5", keyword_type::register_name, std::to_underlying(g10::register_type::h5), 0 },
        { "h6", keyword_type::register_name, std::to_underlying(g10::register_type::h6), 0 },
        { "h7", keyword_type::register_name, std::to_underlying(g10::register_type::h7), 0 },
        { "h8", keyword_type::register_name, std::to_underlying(g10::register_type::h8), 0 },
        { "h9", keyword_type::register_name, std::to_underlying(g10::register_type::h9), 0 },
        { "h10", keyword_type::register_name, std::to_underlying(g10::register_type::h10), 0 },
        { "h11", keyword_type::register_name, std::to_underlying(g10::register_type::h11), 0 },
        { "h12", keyword_type::register_name, std::to_underlying(g10::register_type::h12), 0 },
        { "h13", keyword_type::register_name, std::to_underlying(g10::register_type::h13), 0 },
        { "h14", keyword_type::register_name, std::to_underlying(g10::register_type::h14), 0 },
        { "h15", keyword_type::register_name, std::to_underlying(g10::register_type::h15), 0 },
        { "l0", keyword_type::register_name, std::to_underlying(g10::register_type::l0), 0 },
        { "l1", keyword_type::register_name, std::to_underlying(g10::register_type::l1), 0 },
        { "l2", keyword_type::register_name, std::to_underlying(g10::register_type::l2), 0 },
        { "l3", keyword_type::register_name, std::to_underlying(g10::register_type::l3), 0 },
        { "l4", keyword_type::register_name, std::to_underlying(g10::register_type::l4), 0 },
        { "l5", keyword_type::register_name, std::to_underlying(g10::register_type::l5), 0 },
        { "l6", keyword_type::register_name, std::to_underlying(g10::register_type::l6), 0 },
        { "l7", keyword_type::register_name, std::to_underlying(g10::register_type::l7), 0 },
        { "l8", keyword_type::register_name, std::to_underlying(g10::register_type::l8), 0 },
        { "l9", keyword_type::register_name, std::to_underlying(g10::register_type::l9), 0 },
        { "l10", keyword_type::register_name, std::to_underlying(g10::register_type::l10), 0 },
        { "l11", keyword_type::register_name, std::to_underlying(g10::register_type::l11), 0 },
        { "l12", keyword_type::register_name, std::to_underlying(g10::register_type::l12), 0 },
        { "l13", keyword_type::register_name, std::to_underlying(g10::register_type::l13), 0 },
        { "l14", keyword_type::register_name, std::to_underlying(g10::register_type::l14), 0 },
        { "l15", keyword_type::register_name, std::to_underlying(g10::register_type::l15), 0 },

        // Branching Conditions
        { "nc", keyword_type::branching_condition, g10::CC_NO_CONDITION, 0 },
        { "zs", keyword_type::branching_condition, g10::CC_ZERO_SET, 0 },
        { "zc", keyword_type::branching_condition, g10::CC_ZERO_CLEAR, 0 },
        { "cs", keyword_type::branching_condition, g10::CC_CARRY_SET, 0 },
        { "cc", keyword_type::branching_condition, g10::CC_CARRY_CLEAR, 0 },
        { "vs", keyword_type::branching_condition, g10::CC_OVERFLOW_SET, 0 },
        { "vc", keyword_type::branching_condition, g10::CC_OVERFLOW_CLEAR, 0 }
    };
}

/* Public Methods *************************************************************/

namespace g10asm
{
    auto keyword_table::lookup_keyword (std::string_view name)
        -> g10::result_cref<keyword>
    {
        // - Make sure the string given is not empty.
        if (name.empty() == true)
        {
            return g10::error("Keyword name cannot be empty.");
        }

        // - Convert the name to lowercase for case-insensitive lookup.
        std::string lower { name };
        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            [] (unsigned char c) { return std::tolower(c); }
        );

        // - Search for the keyword in the table.
        auto it = std::find_if(
            s_keywords.begin(),
            s_keywords.end(),
            [&lower] (const keyword& entry)
            {
                return entry.name == lower;
            }
        );
        if (it == s_keywords.end())
        {
            return g10::error("'{}' is not a keyword.", name);
        }

        return std::cref(*it);
    }

    auto keyword_table::stringify_keyword (keyword_type type) -> std::string_view
    {
        switch (type)
        {
            case keyword_type::instruction_mnemonic:
                return "instruction mnemonic";
            case keyword_type::preprocessor_function:
                return "preprocessor function";
            case keyword_type::preprocessor_directive:
                return "preprocessor directive";
            case keyword_type::assembler_directive:
                return "assembler directive";
            case keyword_type::pragma:
                return "pragma";
            case keyword_type::register_name:
                return "register name";
            case keyword_type::branching_condition:
                return "branching condition";
            default:
                return "unknown keyword type";
        }
    }
}
