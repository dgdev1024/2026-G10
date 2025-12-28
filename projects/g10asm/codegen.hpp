/**
 * @file    g10asm/codegen.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-26
 * 
 * @brief   Contains definitions for the G10 assembler's syntax interpretation,
 *          expression evaluation and code generation component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/object.hpp>
#include <g10asm/ast.hpp>

/* Public Types ***************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a type representing a value (or lack thereof) which can
     *          be produced by an expression evaluation.
     * 
     * This variant can hold one of the following types, depending upon the
     * evaluation and its result:
     * 
     * - `std::monostate`: Returned by expressions that do not yield a value,
     *   such as directives that affect assembly state but do not produce
     *   immediate values.
     * 
     * - `std::int64_t`: Returned by expressions that evaluate to integer or
     *   single-character values. Integer values are represented as signed
     *   64-bit integers to accommodate the full range of possible immediate
     *   values in the G10 assembly language.
     * 
     * - `std::uint64_t`: Returned by expressions which evaluate to fixed-point
     *   numbers. Numbers with fractional components in the G10 assembly language
     *   are represented in `32.32` fixed-point format, in which the upper 32 bits
     *   represent the integer part and the lower 32 bits represent the fractional
     *   part. Both parts are unsigned, and the integer part should be cast to
     *   signed when used in signed contexts.
     * 
     * - `std::uint32_t`: Returned by expressions which evaluate to memory
     *   addresses. Addresses in the G10 assembly language are represented as
     *   unsigned 32-bit integers, and may be absolute 32-bit addresses,
     *   relative 16-bit offsets relative to `$FFFF0000` or `8-bit` offsets
     *   relative to `$FFFFFF00`.
     * 
     * - `std::string`: Returned by expressions which evaluate to string values.
     *   String values are represented as standard C++ strings.
     */
    using value = std::variant<
        std::monostate,
        std::int64_t,
        std::uint64_t,
        std::uint32_t,
        std::string
    >;
}

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{

}

/* Private Types **************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing the current state and context
     *          of the code generation process.
     */
    struct codegen_state final
    {
        g10::object         object;                 /** @brief The object file being built. */
        std::uint32_t       location_counter;       /** @brief The current state of the location counter. */
        std::uint32_t       rom_location_counter;   /** @brief The current location counter within the ROM region. */
        std::uint32_t       ram_location_counter;   /** @brief The current location counter within the RAM region. */
        std::size_t         current_section_index;  /** @brief Index of current section. */
        bool                in_rom_region;          /** @brief Indicates whether the location counter is in the ROM region (`< $80000000`). */
        
        /**
         * @brief   A map of label names to their section index and offset.
         */
        std::unordered_map<
            std::string, 
            std::pair<std::size_t, std::uint32_t>
        > label_map;
        
        /** @brief Set of global symbol names (for duplicate checking). */

        /**
         * @brief   A set of symbol names marked global, via the `.global` 
         *          directive.
         * 
         * This is needed to check for duplicate global definitions and to
         * ensure symbols are not marked both global and extern.
         */
        std::unordered_set<std::string> global_symbols;
        
        /**
         * @brief   A set of symbol names marked extern, via the `.extern`
         *          directive.
         * 
         * This is needed to check for conflicts with global symbols, and to
         * ensure that extern symbols are not defined within the object file.
         */
        std::unordered_set<std::string> extern_symbols;

    public:

        /**
         * @brief   This default constructor initializes the code generation
         *          context with default values.
         */
        codegen_state () :
            location_counter { 0x00002000 },
            rom_location_counter { 0x00002000 },
            ram_location_counter { 0x80000000 },
            current_section_index { 0 },
            in_rom_region { true }
        {}

    };
}

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a static class representing the G10 assembler's code
     *          generation ("codegen") component.
     * 
     * This component is responsible for interpreting the abstract syntax tree
     * (AST) produced by the parser, evaluating expressions, emitting machine
     * code and building the G10 object file to be output.
     * 
     * @sa      @a `g10::object`
     */
    class codegen final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Processes the given AST module, generating a G10 object
         *          file.
         * 
         * Processing involves interpreting the AST nodes, evaluating any
         * expressions, emitting machine code, building sections, symbols and
         * relocations, and assembling everything into a valid G10 object file.
         * 
         * This process occurs in two passes:
         * 
         * - First Pass: (...)
         * 
         * - Second Pass: (...)
         * 
         * - Finalization: (...)
         * 
         * @param   module  The AST module to process.
         * 
         * @return  If successful, returns the generated G10 object file;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto process (ast_module& module) -> g10::result<g10::object>;

    private: /* Private Types *************************************************/

    private: /* Private Methods - Variable Pass *******************************/

        /**
         * @brief   Performs the variable pass of the assembly process.
         * 
         * This pass processes all `.let` and `.const` directives, as well as
         * variable assignment statements, populating the environment with
         * variable and constant values before the first pass.
         * 
         * This must be done before the first pass because variables can be
         * used in `.org` expressions and other places that affect address
         * calculation.
         * 
         * @param   state   The codegen state.
         * @param   module  The AST module to process.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto variable_pass (codegen_state& state, ast_module& module)
            -> g10::result<void>;

        /**
         * @brief   Processes a `.let` directive in the variable pass.
         * 
         * @param   state   The codegen state.
         * @param   let_dir The AST `.let` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto variable_pass_let (
            codegen_state& state,
            ast_dir_let& let_dir
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.const` directive in the variable pass.
         * 
         * @param   state       The codegen state.
         * @param   const_dir   The AST `.const` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto variable_pass_const (
            codegen_state& state,
            ast_dir_const& const_dir
        ) -> g10::result<void>;

        /**
         * @brief   Processes a variable assignment statement in the variable pass.
         * 
         * @param   state       The codegen state.
         * @param   assign_stmt The AST variable assignment statement node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto variable_pass_assignment (
            codegen_state& state,
            ast_stmt_var_assignment& assign_stmt
        ) -> g10::result<void>;

    private: /* Private Methods - First Pass **********************************/

        /**
         * @brief   Performs the first pass of the assembly process.
         * 
         * @param   state   The codegen state.
         * @param   module  The AST module to process.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass (codegen_state& state, ast_module& module)
            -> g10::result<void>;

        /**
         * @brief   Processes a label definition in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   label   The AST label definition node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_label (
            codegen_state& state,
            ast_label_definition& label
        ) -> g10::result<void>;

        /**
         * @brief   Processes an instruction in the first pass.
         * 
         * During the first pass, this involves calculating the size of the
         * instruction (including its operands) and updating the location
         * counter accordingly.
         * 
         * @param   state   The codegen state.
         * @param   instr   The AST instruction node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Processes an `.org` directive in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   org     The AST `.org` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_org (
            codegen_state& state,
            ast_dir_org& org
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.rom` directive in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   rom     The AST `.rom` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_rom (
            codegen_state& state,
            ast_dir_rom& rom
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.ram` directive in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   ram     The AST `.ram` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_ram (
            codegen_state& state,
            ast_dir_ram& ram
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.int` directive in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   int_    The AST `.int` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_int (
            codegen_state& state,
            ast_dir_int& int_
        ) -> g10::result<void>;

        /**
         * @brief   Processes a data directive (`.byte`, `.word`, or `.dword`)
         *          in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   node    The AST data directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_data (
            codegen_state& state,
            ast_node& node
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.global` directive in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   global  The AST `.global` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_global (
            codegen_state& state,
            ast_dir_global& global
        ) -> g10::result<void>;

        /**
         * @brief   Processes an `.extern` directive in the first pass.
         * 
         * @param   state   The codegen state.
         * @param   extern_ The AST `.extern` directive node.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto first_pass_extern (
            codegen_state& state,
            ast_dir_extern& extern_
        ) -> g10::result<void>;

    private: /* Private Methods - Second Pass *********************************/

        /**
         * @brief   Performs the second pass of assembly.
         * 
         * The second pass is responsible for:
         * - Emitting machine code for instructions
         * - Evaluating expressions to final values
         * - Emitting data bytes for data directives
         * - Generating relocations for external symbol references
         * 
         * @param   state   The codegen state.
         * @param   module  The AST module to process.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass (codegen_state& state, ast_module& module)
            -> g10::result<void>;

        /**
         * @brief   Processes an instruction in the second pass (emit code).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Processes an `.org` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   org     The .org directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_org (
            codegen_state& state,
            ast_dir_org& org
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.rom` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   rom     The .rom directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_rom (
            codegen_state& state,
            ast_dir_rom& rom
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.ram` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   ram     The .ram directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_ram (
            codegen_state& state,
            ast_dir_ram& ram
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.int` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   int_    The .int directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_int (
            codegen_state& state,
            ast_dir_int& int_
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.byte` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   dir     The .byte directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_byte (
            codegen_state& state,
            ast_dir_byte& dir
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.word` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   dir     The .word directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_word (
            codegen_state& state,
            ast_dir_word& dir
        ) -> g10::result<void>;

        /**
         * @brief   Processes a `.dword` directive in the second pass.
         * 
         * @param   state   The codegen state.
         * @param   dir     The .dword directive node.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto second_pass_dword (
            codegen_state& state,
            ast_dir_dword& dir
        ) -> g10::result<void>;

    private: /* Private Methods - Finalization ********************************/

        /**
         * @brief   Performs the finalization step of the assembly process.
         * 
         * This step validates the assembled object, ensures consistency,
         * and prepares it for serialization to disk. It includes:
         * - Setting appropriate object flags
         * - Verifying all global symbols are defined
         * - Validating relocations and sections
         * - Running the object's internal validation
         * 
         * @param   state   The codegen state.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto finalize (codegen_state& state) -> g10::result<void>;

        /**
         * @brief   Sets the object flags based on assembled content.
         * 
         * @param   state   The codegen state.
         */
        static auto finalize_flags (codegen_state& state) -> void;

        /**
         * @brief   Verifies that all global symbols have been defined.
         * 
         * @param   state   The codegen state.
         * 
         * @return  If successful, returns void; otherwise an error listing
         *          undefined global symbols.
         */
        static auto verify_global_symbols (codegen_state& state)
            -> g10::result<void>;

        /**
         * @brief   Verifies that all relocations are valid.
         * 
         * Checks that each relocation references a valid symbol and
         * has a valid offset within its section.
         * 
         * @param   state   The codegen state.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto verify_relocations (codegen_state& state)
            -> g10::result<void>;

    private: /* Private Methods - Expression Evaluation ***********************/

        /**
         * @brief   Evaluates an AST expression and returns its value.
         * 
         * This is the main entry point for expression evaluation. It dispatches
         * to the appropriate evaluation method based on the expression type
         * (primary, binary, unary, or grouping).
         * 
         * @param   state   The codegen state (for symbol lookup).
         * @param   expr    The expression to evaluate.
         * 
         * @return  If successful, returns the evaluated value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_expression (
            codegen_state& state,
            const ast_expression& expr
        ) -> g10::result<value>;

        /**
         * @brief   Evaluates a primary expression (literals and identifiers).
         * 
         * @param   state   The codegen state (for symbol lookup).
         * @param   expr    The primary expression to evaluate.
         * 
         * @return  If successful, returns the evaluated value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_primary (
            codegen_state& state,
            const ast_expr_primary& expr
        ) -> g10::result<value>;

        /**
         * @brief   Evaluates a binary expression (e.g., a + b, x * y).
         * 
         * @param   state   The codegen state.
         * @param   expr    The binary expression to evaluate.
         * 
         * @return  If successful, returns the evaluated value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_binary (
            codegen_state& state,
            const ast_expr_binary& expr
        ) -> g10::result<value>;

        /**
         * @brief   Evaluates a unary expression (e.g., -a, ~b).
         * 
         * @param   state   The codegen state.
         * @param   expr    The unary expression to evaluate.
         * 
         * @return  If successful, returns the evaluated value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_unary (
            codegen_state& state,
            const ast_expr_unary& expr
        ) -> g10::result<value>;

        /**
         * @brief   Evaluates a grouping expression (parenthesized expression).
         * 
         * @param   state   The codegen state.
         * @param   expr    The grouping expression to evaluate.
         * 
         * @return  If successful, returns the evaluated value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_grouping (
            codegen_state& state,
            const ast_expr_grouping& expr
        ) -> g10::result<value>;

        /**
         * @brief   Converts a value to an integer if possible.
         * 
         * @param   val     The value to convert.
         * 
         * @return  If convertible, returns the integer value;
         *          Otherwise, returns an error message.
         */
        static auto value_to_integer (const value& val) -> g10::result<std::int64_t>;

        /**
         * @brief   Checks if a value represents an address (uint32_t).
         * 
         * @param   val     The value to check.
         * 
         * @return  True if the value is an address; false otherwise.
         */
        static auto is_address_value (const value& val) -> bool;

        /**
         * @brief   Evaluates an expression and returns it as an integer.
         * 
         * This is a convenience wrapper that evaluates an expression and
         * converts the result to an integer in a single step.
         * 
         * @param   state   The codegen state.
         * @param   expr    The expression to evaluate.
         * 
         * @return  If successful, returns the integer value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_as_integer (
            codegen_state& state,
            const ast_expression& expr
        ) -> g10::result<std::int64_t>;

        /**
         * @brief   Evaluates an expression and returns it as an address.
         * 
         * This evaluates an expression and ensures the result fits in a
         * 32-bit unsigned address.
         * 
         * @param   state   The codegen state.
         * @param   expr    The expression to evaluate.
         * 
         * @return  If successful, returns the address value;
         *          Otherwise, returns an error message.
         */
        static auto evaluate_as_address (
            codegen_state& state,
            const ast_expression& expr
        ) -> g10::result<std::uint32_t>;

        /**
         * @brief   Checks if an expression references external symbols.
         * 
         * Used to determine if a relocation is needed.
         * 
         * @param   state   The codegen state.
         * @param   expr    The expression to check.
         * 
         * @return  True if the expression references external symbols.
         */
        static auto references_external (
            codegen_state& state,
            const ast_expression& expr
        ) -> bool;

    private: /* Private Methods - Code Emission *******************************/

        /**
         * @brief   Emits a single byte to the current section.
         * 
         * @param   state   The codegen state.
         * @param   byte    The byte to emit.
         */
        static auto emit_byte (codegen_state& state, std::uint8_t byte) -> void;

        /**
         * @brief   Emits a 16-bit word (little-endian) to the current section.
         * 
         * @param   state   The codegen state.
         * @param   word    The word to emit.
         */
        static auto emit_word (codegen_state& state, std::uint16_t word) -> void;

        /**
         * @brief   Emits a 32-bit dword (little-endian) to the current section.
         * 
         * @param   state   The codegen state.
         * @param   dword   The dword to emit.
         */
        static auto emit_dword (codegen_state& state, std::uint32_t dword) -> void;

        /**
         * @brief   Emits bytes from a buffer to the current section.
         * 
         * @param   state   The codegen state.
         * @param   data    The buffer to emit (as a span).
         */
        static auto emit_bytes (
            codegen_state& state,
            std::span<const std::uint8_t> data
        ) -> void;

        /**
         * @brief   Gets the current offset within the current section.
         * 
         * @param   state   The codegen state.
         * 
         * @return  The current offset within the section.
         */
        static auto current_section_offset (const codegen_state& state)
            -> std::uint32_t;

        /**
         * @brief   Creates a relocation entry for the current position.
         * 
         * @param   state           The codegen state.
         * @param   symbol_name     The name of the symbol to relocate.
         * @param   type            The relocation type.
         * @param   addend          The addend value.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto create_relocation (
            codegen_state& state,
            const std::string& symbol_name,
            g10::relocation_type type,
            std::int16_t addend
        ) -> g10::result<void>;

    private: /* Private Methods - Instruction Emission ************************/

        /**
         * @brief   Emits the machine code for a CPU instruction.
         * 
         * This is the main entry point for instruction encoding. It determines
         * the opcode based on the instruction mnemonic and operands, then
         * emits the appropriate bytes.
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a no-operand (zero-operand) instruction.
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_no_operand_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a load instruction (LD, LDQ, LDP).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_load_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a store instruction (ST, STQ, STP).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_store_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a move instruction (MV, MWH, MWL).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_move_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a stack instruction (PUSH, POP, LSP, SSP, SPO, SPI).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_stack_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a branch instruction (JMP, JPB, CALL, RET, RETI, INT).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_branch_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits an ALU instruction (ADD, ADC, SUB, SBC, INC, DEC,
         *          AND, OR, XOR, NOT, CMP).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_alu_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a shift/rotate instruction (SLA, SRA, SRL, SWAP,
         *          RLA, RL, RLCA, RLC, RRA, RR, RRCA, RRC).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_shift_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Emits a bit manipulation instruction (BIT, SET, RES, TOG).
         * 
         * @param   state   The codegen state.
         * @param   instr   The instruction to emit.
         * 
         * @return  If successful, returns void; otherwise an error.
         */
        static auto emit_bit_instruction (
            codegen_state& state,
            ast_instruction& instr
        ) -> g10::result<void>;

        /**
         * @brief   Gets the register index (0-15) from a register type.
         * 
         * @param   reg     The register type.
         * 
         * @return  The 4-bit register index.
         */
        static auto get_register_index (g10::register_type reg) -> std::uint8_t;

        /**
         * @brief   Gets the register size class (0=byte, 1=word, 2=dword).
         * 
         * @param   reg     The register type.
         * 
         * @return  The size class (0, 1, or 2).
         */
        static auto get_register_size_class (g10::register_type reg) -> std::uint8_t;

    private: /* Private Methods - Helper Methods ******************************/

        /**
         * @brief   Ensures that a section exists at the given address, creating
         *          one if necessary; or switches to the existing section if one
         *          is already present.
         * 
         * @param   state   The codegen state.
         * @param   address The address at which to ensure a section exists.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error message describing the failure.
         */
        static auto ensure_section (
            codegen_state& state,
            std::uint32_t address
        ) -> g10::result<void>;

        /**
         * @brief   Retrieves the size, in bytes, of the given instruction,
         *          including its operands.
         * 
         * @param   instr   The AST instruction node.
         * 
         * @return  The size of the instruction in bytes.
         */
        static auto calculate_instruction_size (
            const ast_instruction& instr
        ) -> std::size_t;

    private: /* Private Members ***********************************************/



    };
}