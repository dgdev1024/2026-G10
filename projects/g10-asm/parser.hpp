/**
 * @file    g10-asm/parser.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-19
 * 
 * @brief   Contains definitions for the G10 CPU assembler's syntax analyzer
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10-asm/ast.hpp>
#include <g10-asm/lexer.hpp>

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing a syntax analyzer (parser) used by
     *          the G10 assembler tool to parse tokens produced by a lexer
     *          into an abstract syntax tree (AST) representing the structure
     *          of the assembly program.
     */
    class parser final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a new @a `parser` instance, initializing it
         *          with the provided @a `lexer` instance from which tokens
         *          will be consumed during parsing.
         * 
         * @param   lexer   A reference to the @a `lexer` instance from which
         *                  tokens will be consumed during parsing.
         */
        explicit parser (lexer& lex);

        /**
         * @brief   Parses the token stream produced by the associated @a
         *          `lexer` instance into an abstract syntax tree (AST)
         *          representing the structure of the assembly program.
         * 
         * @return  If successful, returns a reference to the parsed @a
         *          `program` AST;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_program () -> g10::result_ref<program>;

        /**
         * @brief   Indicates whether the parser is in a good state.
         * 
         * The parser is considered "good" if it has successfully parsed the
         * token stream without encountering any errors.
         * 
         * @return  `true` if the parser is in a good state;
         *          Otherwise, `false`.
         */
        inline auto is_good () const noexcept -> bool
            { return m_good; }

    private: /* Private Methods - Token Consumption ***************************/

        /**
         * @brief   Retrieves the current token from the associated @a
         *          `lexer` instance without advancing the internal token
         *          pointer.
         * 
         * @return  If successful, returns a reference to the current @a
         *          `token` in the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto current_token () -> g10::result_ref<token>;

        /**
         * @brief   Peeks at a token in the associated @a `lexer` instance's
         *          token stream at the specified offset from the current
         *          token pointer, without advancing the internal token pointer.
         * 
         * @param   offset  The number of tokens ahead of the current token
         *                  pointer to peek at. Defaults to `1`, which peeks at
         *                  the next token.
         * 
         * @return  If successful, returns a reference to the @a `token` at the
         *          specified offset in the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto peek_token (std::int64_t offset = 1) -> g10::result_ref<token>;

        /**
         * @brief   Advances the internal token pointer of the associated @a `lexer`
         *          to the next token in the token stream, then retrieves that token.
         * 
         * @return  If successful, returns a reference to the next @a `token` in
         *          the token stream;
         *          otherwise, returns an error describing the failure.
         */
        auto advance_token () -> g10::result_ref<token>;

        /**
         * @brief   Checks if the current token in the token stream matches
         *          the expected token type.
         * 
         * @param   expected    The expected @a `token_type` to check against.
         * 
         * @return  `true` if the current token matches the expected type;
         *          Otherwise, `false`.
         */
        auto check_token (token_type expected) -> bool;

        /**
         * @brief   Checks if the current token in the token stream matches
         *          the expected token type. If it does, advances the internal
         *          token pointer to the next token.
         * 
         * @param   expected    The expected @a `token_type` to match against.
         * 
         * @return  `true` if the current token matches the expected type and
         *          the internal token pointer was advanced;
         *          Otherwise, `false`.
         */
        auto match_token (token_type expected) -> bool;

        /**
         * @brief   Checks if the parser has reached the end its @a `lexer`'s
         *          token stream.
         * 
         * @return  `true` if the end of the token stream has been reached;
         *          Otherwise, `false`.
         */
        auto is_at_end () -> bool;

        /**
         * @brief   Checks if the current token in the token stream matches
         *          the expected token type. If it does, advances the internal
         *          token pointer to the next token, and retrieves that token.
         *          If it does not match, returns an error with a formatted
         *          message.
         * 
         * @tparam  Args        The types of additional arguments to format into
         *                      the error message.
         * 
         * @param   expected    The expected @a `token_type` to match against.
         * @param   error_fmt   A format string for the error message if the
         *                      token does not match the expected type.
         * @param   error_args  Additional arguments to format into the error
         *                      message.
         * 
         * @return  If successful, returns a reference to the consumed @a `token`;
         *          Otherwise, returns an error describing the failure.
         */
        template <typename... Args>
        constexpr auto consume_token (token_type expected,
            const std::string& error_fmt, Args&&... error_args)
                -> g10::result_ref<token>
        {
            // - If the current token matches the expected type, advance to
            //   the next token and return the consumed token.
            if (check_token(expected) == true)
            {
                return advance_token();
            }

            // - Otherwise, return an error with the formatted message.
            auto token_result = current_token();
            if (token_result.has_value() == true)
            {
                return std::unexpected {
                    panic(token_result->get(), error_fmt,
                        std::forward<Args>(error_args)...)
                };
            }

            return std::unexpected { 
                panic(error_fmt, std::forward<Args>(error_args)...)  
            };
        }

    private: /* Private Methods - Statement Parsing ***************************/

        /**
         * @brief   Parses a single assembly statement from the token stream.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_statement () -> g10::result<statement>;

        /**
         * @brief   Parses an address label definition statement from the token
         *          stream.
         * 
         * Address label definitions consist of an identifier token followed by
         * a colon token (e.g., `start_loop:`), and define a label which can
         * be referenced elsewhere in the assembly program, like in jump
         * and branch instructions.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_label_definition () -> g10::result<statement>;

        /**
         * @brief   Parses an instruction statement from the token stream.
         * 
         * Instruction statements consist of an instruction mnemonic token
         * followed by anywhere between zero and two operands, depending on the
         * specific instruction being parsed.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_instruction () -> g10::result<statement>;

        /**
         * @brief   Parses a directive statement from the token stream.
         * 
         * Directive statements consist of a directive keyword token
         * (e.g., `.ORG`, `.BYTE`, etc.) followed by directive-specific
         * parameters.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive () -> g10::result<statement>;

    private: /* Private Methods - Directive Parsing ***************************/

        /**
         * @brief   Parses an `.ORG` directive statement from the token stream.
         * 
         * The `.ORG` directive sets the location counter (origin) to the specified
         * address for subsequent instructions and data.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive_org () -> g10::result<statement>;

        /**
         * @brief   Parses a `.BYTE` directive statement from the token stream.
         * 
         * The `.BYTE` directive defines one or more byte-sized data values to
         * be included in the assembled program.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive_byte () -> g10::result<statement>;

        /**
         * @brief   Parses a `.WORD` directive statement from the token stream.
         * 
         * The `.WORD` directive defines one or more word-sized (2 bytes)
         * data values to be included in the assembled program.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive_word () -> g10::result<statement>;

        /**
         * @brief   Parses a `.DWORD` directive statement from the token stream.
         * 
         * The `.DWORD` directive defines one or more double-word-sized (4 bytes)
         * data values to be included in the assembled program.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive_dword () -> g10::result<statement>;

        /**
         * @brief   Parses a `.GLOBAL` directive statement from the token stream.
         * 
         * The `.GLOBAL` directive declares one or more symbols as global,
         * making them accessible from other modules.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive_global () -> g10::result<statement>;

        /**
         * @brief   Parses an `.EXTERN` directive statement from the token stream.
         * 
         * The `.EXTERN` directive declares one or more symbols as external,
         * indicating that they are defined in other modules.
         * 
         * @return  If successful, returns the parsed @a `statement`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_directive_extern () -> g10::result<statement>;

    private: /* Private Methods - Operand Parsing *****************************/

        /**
         * @brief   Parses a single operand from the token stream.
         * 
         * @return  If successful, returns the parsed @a `operand`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_operand () -> g10::result<operand>;

        /**
         * @brief   Parses a register operand from the token stream.
         * 
         * A register operand consists of a register token (e.g., `D0`, `W1`, etc.)
         * 
         * @return  If successful, returns the parsed @a `operand`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_register_operand () -> g10::result<operand>;

        /**
         * @brief   Parses an immediate or address operand from the token stream.
         * 
         * An immediate/address operand can be an immediate value (e.g., `42`,
         * `0x1234`, etc.).
         * 
         * @return  If successful, returns the parsed @a `operand`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_immediate_operand () -> g10::result<operand>;

        /**
         * @brief   Parses a memory address operand from the token stream.
         * 
         * A memory address operand can be either a direct memory address
         * (e.g., `[$8000]`, `[0xFFFF]`) or an indirect register address
         * (e.g., `[D0]`, `[W1]`).
         * 
         * @return  If successful, returns the parsed @a `operand`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_indirect_operand () -> g10::result<operand>;

        /**
         * @brief   Parses an address operand (wrapper for parse_indirect_operand).
         * 
         * @return  If successful, returns the parsed @a `operand`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_address_operand () -> g10::result<operand>;

        /**
         * @brief   Parses a label operand from the token stream.
         * 
         * A label operand consists of an identifier token representing a
         * previously-defined label in the assembly program.
         * 
         * @return  If successful, returns the parsed @a `operand`;
         *          otherwise, returns an error describing the failure.
         */
        auto parse_label_operand () -> g10::result<operand>;

    private: /* Private Methods - Validation **********************************/

        /**
         * @brief   Validates that the provided instruction statement is
         *          semantically correct according to the G10 assembly language
         *          specification.
         * 
         * @param   instr_stmt  The instruction @a `statement` to validate.
         * 
         * @return  If the instruction statement is valid, returns nothing;
         *          otherwise, returns an error describing the failure.
         */
        auto validate_instruction (const statement& instr_stmt) -> g10::result<void>;

        /**
         * @brief   Validates that the provided instruction statement has the
         *          expected number of operands.
         * 
         * @param   instr_stmt      The instruction @a `statement` to validate.
         * @param   expected_count  The expected number of operands for the
         *                          instruction.
         * 
         * @return  If the instruction statement has the expected number of
         *          operands, returns nothing;
         *          otherwise, returns an error describing the failure.
         */
        auto validate_operand_count (const statement& instr_stmt,
            std::size_t expected_count) -> g10::result<void>;

        /**
         * @brief   Validates that the provided operand is of the expected type.
         * 
         * @param   op              The @a `operand` to validate.
         * @param   expected_type   The expected @a `operand_type` of the
         *                          operand.
         *
         * @return  If the operand is of the expected type, returns nothing;
         *          otherwise, returns an error describing the failure.
         */
        auto validate_operand_type (const operand& op,
            operand_type expected_type) -> g10::result<void>;

        /**
         * @brief   Validates jump and call instructions (JMP, JP, JPB, JR, CALL)
         *          which can have 1 or 2 operands.
         * 
         * When 1 operand is provided, it's the address/label and condition is
         * assumed to be NC (no condition).
         * When 2 operands are provided, the first is the condition code and
         * the second is the address/label.
         * 
         * @param   instr_stmt  The instruction @a `statement` to validate.
         * 
         * @return  If the instruction is valid, returns nothing;
         *          otherwise, returns an error describing the failure.
         */
        auto validate_jump_or_call (const statement& instr_stmt) -> g10::result<void>;

        /**
         * @brief   Validates the RET instruction which can have 0 or 1 operands.
         * 
         * When 0 operands are provided, it's an unconditional return and
         * condition is assumed to be NC (no condition).
         * When 1 operand is provided, it must be a condition code.
         * 
         * @param   instr_stmt  The instruction @a `statement` to validate.
         * 
         * @return  If the instruction is valid, returns nothing;
         *          otherwise, returns an error describing the failure.
         */
        auto validate_ret (const statement& instr_stmt) -> g10::result<void>;

    private: /* Private Methods - Error Handling and Recovery *****************/

        /**
         * @brief   Synchronizes the parser after encountering a syntax error.
         * 
         * The parser enters panic mode upon encountering an error, skipping
         * tokens until a known synchronization point is reached, allowing
         * parsing to continue from a stable state.
         */
        auto synchronize () -> void;

        /**
         * @brief   Enters panic mode due to a syntax error, returning an
         *          error with a formatted message.
         * 
         * @tparam  Args        The types of additional arguments to format into
         *                      the error message.
         * 
         * @param   error_fmt   A format string for the error message.
         * @param   error_args  Additional arguments to format into the error
         *                      message.
         * 
         * @return  An error describing the failure.
         */
        template <typename... Args>
        constexpr auto panic (const std::string& error_fmt, Args&&... error_args)
            -> std::string
        {
            m_panic = true;
            return std::vformat(
                "Parser error:\n - " + error_fmt,
                std::make_format_args(error_args...)
            );
        }

        /**
         * @brief   Enters panic mode due to a syntax error at a specific token,
         *          returning an error with a formatted message.
         * 
         * @tparam  Args        The types of additional arguments to format into
         *                      the error message.
         * 
         * @param   tok         The @a `token` where the error occurred.
         * @param   error_fmt   A format string for the error message.
         * @param   error_args  Additional arguments to format into the error
         *                      message.
         * 
         * @return  An error describing the failure.
         */
        template <typename... Args>
        constexpr auto panic (const token& tok, const std::string& error_fmt,
            Args&&... error_args)
                -> std::string
        {
            m_panic = true;
            return std::vformat(
                "Parser error in file '{}:{}' ('{}'):\n - " + error_fmt,
                std::make_format_args(
                    tok.source_file,
                    tok.line_number,
                    tok.lexeme,
                    error_args...
                )
            );
        }

    private: /* Private Members ***********************************************/

        /**
         * @brief   A reference to the @a `lexer` instance from which tokens
         *          will be consumed during parsing.
         */
        lexer& m_lexer;

        /**
         * @brief   Indicates whether the parser is in a good state.
         * 
         * The parser is considered "good" if it has successfully parsed the
         * token stream without encountering any errors.
         */
        bool m_good { false };

        /**
         * @brief   Indicates whether the parser is currently in panic mode.
         * 
         * When in panic mode, the parser will attempt to recover from errors
         * by skipping tokens until a known synchronization point is reached.
         */
        bool m_panic { false };
        
        /**
         * @brief   The abstract syntax tree (AST) representing the parsed
         *          assembly program.
         */
        program m_program;

    };
}
