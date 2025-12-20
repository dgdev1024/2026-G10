/**
 * @file    g10-asm/codegen.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-20
 * 
 * @brief   Contains definitions for the G10 CPU assembler's code generation
 *          component, which transforms parsed AST into binary machine code.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/ast.hpp>
#include <g10-asm/object_file.hpp>

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing the code generation component of the
     *          G10 CPU assembler tool.
     * 
     * The code generator operates in two passes:
     * 
     * Pass 1 (Address Resolution):
     * 
     * - Calculate the address of each statement based on .ORG directives
     * 
     * - Determine the size of each instruction and directive
     * 
     * - Build a mapping of label names to their addresses
     * 
     * 
     * Pass 2 (Code Emission):
     * 
     * - Emit binary opcodes and operand bytes for each instruction
     * 
     * - Emit data values for directives
     * 
     * - Create relocation entries for unresolved symbols
     * 
     * 
     * The output is a relocatable object file that can be processed by the linker.
     */
    class codegen final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a code generator for the given parsed program.
         * 
         * @param   prog        Reference to the parsed program AST.
         * @param   source_name The name of the source file (for debugging).
         */
        codegen (const program& prog, const std::string& source_name = "");

        /**
         * @brief   Generates machine code from the parsed program.
         * 
         * This method performs two passes:
         * 
         * - Pass 1: Resolve all label addresses
         * 
         * - Pass 2: Emit code and generate relocations
         * 
         * @return  If successful, returns the generated object file;
         *          otherwise, returns an error description.
         */
        auto generate () -> g10::result<g10obj::object_file>;

    private: /* Private Methods - Pass 1: Address Resolution ******************/

        /**
         * @brief   Performs the first pass: address resolution.
         * 
         * This pass:
         * 
         * - Tracks the current address through the program
         * 
         * - Creates a section for each .ORG directive
         * 
         * - Records the address of each label
         * 
         * - Calculates total code size
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto resolve_addresses () -> g10::result<void>;

        /**
         * @brief   Calculates the size in bytes of an instruction statement.
         * 
         * The size depends on the instruction type and operand sizes:
         * 
         * - Zero-operand instructions: 2 bytes (opcode only)
         * 
         * - Register-register operations: 2 bytes
         * 
         * - Instructions with immediate values: 2 + (1/2/4) bytes
         * 
         * - Jump/Call instructions: 2 + 4 bytes (absolute) or 2 + 1 bytes (relative)
         * 
         * @param   stmt    The instruction statement to measure.
         * 
         * @return  The size in bytes.
         */
        auto calculate_instruction_size (const statement& stmt) -> std::size_t;

        /**
         * @brief   Calculates the size in bytes of a directive statement.
         * 
         * The size depends on the directive type:
         * 
         * - `.ORG`: 0 bytes (doesn't generate code)
         * 
         * - `.BYTE`: 1 byte per value
         * 
         * - `.WORD`: 2 bytes per value
         * 
         * - `.DWORD`: 4 bytes per value
         * 
         * - `.GLOBAL` / `.EXTERN`: 0 bytes (doesn't generate code)
         * 
         * @param   stmt    The directive statement to measure.
         * 
         * @return  The size in bytes.
         */
        auto calculate_directive_size (const statement& stmt) -> std::size_t;

    private: /* Private Methods - Pass 2: Code Emission ***********************/

        /**
         * @brief   Performs the second pass: code emission.
         * 
         * This pass:
         * 
         * - Emits binary opcodes and operand data
         * 
         * - Creates relocation entries for external symbols
         * 
         * - Records symbol addresses in the symbol table
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto emit_code () -> g10::result<void>;

        /**
         * @brief   Emits binary code for an instruction statement.
         * 
         * @param   stmt    The instruction statement to emit.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto emit_instruction (const statement& stmt) -> g10::result<void>;

        /**
         * @brief   Emits data for a directive statement.
         * 
         * @param   stmt    The directive statement to emit.
         * 
         * @return  If successful, returns nothing;
         *          otherwise, returns an error description.
         */
        auto emit_directive (const statement& stmt) -> g10::result<void>;

    private: /* Private Methods - Instruction Emission ***********************/

        /**
         * @brief   Emits a 16-bit opcode word.
         * 
         * @param   opcode  The opcode value.
         */
        auto emit_opcode (std::uint16_t opcode) -> void;

        /**
         * @brief   Emits a single byte.
         * 
         * @param   value   The byte value.
         */
        auto emit_byte (std::uint8_t value) -> void;

        /**
         * @brief   Emits a 16-bit word in little-endian format.
         * 
         * @param   value   The word value.
         */
        auto emit_word (std::uint16_t value) -> void;

        /**
         * @brief   Emits a 32-bit double word in little-endian format.
         * 
         * @param   value   The double word value.
         */
        auto emit_dword (std::uint32_t value) -> void;

    private: /* Private Methods - Operand Encoding ***************************/

        /**
         * @brief   Encodes a register operand into its binary representation.
         * 
         * Register encoding: `SSSS RRRR`
         * 
         * - `SSSS`: Register size/type (0-7)
         * 
         * - `RRRR`: Register index (0-15)
         * 
         * @param   op  The register operand.
         * 
         * @return  The encoded byte value.
         */
        auto encode_register (const operand& op) -> std::uint8_t;

        /**
         * @brief   Encodes a condition code operand.
         * 
         * Condition codes are encoded as:
         * 
         * - `NC` (no condition): 0x0
         * 
         * - `ZS` (zero set): 0x1
         * 
         * - `ZC` (zero clear): 0x2
         * 
         * - `CS` (carry set): 0x3
         * 
         * - `CC` (carry clear): 0x4
         * 
         * - `VS` (overflow set): 0x5
         * 
         * - `VC` (overflow clear): 0x6
         * 
         * @param   op  The condition code operand.
         * 
         * @return  The encoded condition code value.
         */
        auto encode_condition (const operand& op) -> std::uint8_t;

    private: /* Private Methods - Symbol Resolution **************************/

        /**
         * @brief   Resolves a label reference to an address or creates a relocation.
         * 
         * If the label is defined locally, returns its address directly.
         * If the label is an external symbol, creates a relocation entry.
         * 
         * @param   label_name  The label name to resolve.
         * 
         * @return  The resolved address (or 0 for external symbols).
         * 
         * @note    May add a relocation entry to m_output.relocations.
         */
        auto resolve_label (const std::string& label_name) -> std::uint32_t;

        /**
         * @brief   Registers a label and its address in the symbol table.
         * 
         * @param   label_name  The label name.
         * @param   address     The label's address.
         * @param   is_global   Whether the label is global (.GLOBAL directive).
         */
        auto register_symbol (const std::string& label_name, std::uint32_t address,
            bool is_global) -> void;

    private: /* Private Methods - Output Helpers *****************************/

        /**
         * @brief   Adds a relocation entry for an external symbol.
         * 
         * @param   symbol_name     The name of the symbol to resolve.
         * @param   reloc_type      The type of relocation.
         * @param   addend          Additional value to add after resolution.
         */
        auto add_relocation (const std::string& symbol_name,
            g10obj::relocation_type reloc_type, std::int32_t addend = 0) -> void;

        /**
         * @brief   Returns the current emission address (relative to section base).
         * 
         * @return  The current address offset within the current section.
         */
        inline auto current_offset () const noexcept -> std::uint32_t
        {
            return static_cast<std::uint32_t>(
                m_output.sections[m_current_section].code.size()
            );
        }

        /**
         * @brief   Returns the current emission address (absolute).
         * 
         * @return  The absolute address (base + offset).
         */
        inline auto current_address () const noexcept -> std::uint32_t
        {
            return m_output.sections[m_current_section].base_address +
                   current_offset();
        }

    private: /* Private Member Variables *************************************/

        const program&              m_program;           /** @brief Reference to parsed program */
        g10obj::object_file         m_output;            /** @brief Generated object file */
        std::size_t                 m_current_section;   /** @brief Index of current section */
        
        /**
         * @brief   Maps label names to their resolved addresses.
         * 
         * Populated during Pass 1 (address resolution).
         */
        std::unordered_map<std::string, std::uint32_t> m_label_addresses;

        /**
         * @brief   Maps statement indices to their absolute addresses.
         * 
         * Populated during Pass 1 (address resolution).
         */
        std::vector<std::uint32_t> m_statement_addresses;

        /**
         * @brief   Tracks which labels have been registered as symbols.
         * 
         * Used to avoid duplicate symbol entries.
         */
        std::unordered_map<std::string, std::size_t> m_symbol_index_map;

        /**
         * @brief   Tracks which external symbols have been seen.
         * 
         * Used to create symbol entries for referenced but undefined symbols.
         */
        std::unordered_map<std::string, bool> m_external_symbols_seen;
    };

}
