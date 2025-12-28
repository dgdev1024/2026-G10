/**
 * @file    g10asm/parser.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-22
 *
 * @brief   Contains declarations for the G10 Assembler Tool's syntax analysis
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/lexer.hpp>
#include <g10asm/ast.hpp>

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a static class representing the G10 assembler tool's
     *          syntax analysis component (henceforth, the "parser").
     *
     * The parser is responsible for analyzing the sequence of tokens produced
     * by a lexical analyzer (lexer) to determine the grammatical structure
     * of the source code. It ensures that the code adheres to the syntax rules
     * of the G10 assembly language and constructs an abstract syntax tree (AST)
     * or other intermediate representations as needed for further processing.
     */
    class parser final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Parses the sequence of tokens provided by the given lexer,
         *          constructing and returning the corresponding abstract syntax
         *          tree (AST) for the G10 assembly module (object file).
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns the root AST node representing the
         *          entire G10 assembly module;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse (lexer& lex) -> g10::result<ast_module>;

        /**
         * @brief   Indicates whether or not the parser has encountered an
         *          error during any of its parsing operations, or whether it
         *          parsed anything at all.
         *
         * @return  If the parser has performed any parsing operations, and has
         *          performed them without encountering any errors, returns
         *          `true`;
         *          Otherwise, returns `false`.
         */
        static auto is_good () -> bool
            { return s_good; }

    private: /* Private Methods - Statements **********************************/

        /**
         * @brief   Parses a single G10 assembly statement from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed statement;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_statement (lexer& lex) -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a G10 assembly label definition from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed label definition;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_label_definition (lexer& lex) 
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a single G10 assembly instruction from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed instruction;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_instruction (lexer& lex) 
            -> g10::result_uptr<ast_node>;

    private: /* Private Methods - Directives **********************************/

        /**
         * @brief   Parses a single G10 assembly directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_directive (lexer& lex) -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.org` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.org` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.org` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_org (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.rom` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.rom` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.rom` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_rom (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.ram` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.ram` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.ram` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_ram (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.int` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.int` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.int` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_int (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.byte` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.byte` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.byte` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_byte (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.word` assembler directive from the token
         *          stream provided by the given lexer.
         *
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.word` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.word` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_word (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.dword` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.dword` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.dword` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_dword (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.global` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.global` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.global` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_global (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.extern` assembler directive from the token
         *          stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.extern` directive
         *                  keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.extern` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_extern (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.let` variable declaration directive from the
         *          token stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.let` directive keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.let` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_let (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a `.const` constant declaration directive from the
         *          token stream provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * @param   dir_tk  The token representing the `.const` directive keyword.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed `.const` directive;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_dir_const (lexer& lex, const token& dir_tk)
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a variable assignment statement from the token stream
         *          provided by the given lexer.
         * 
         * A variable assignment statement starts with a variable token (`$name`)
         * followed by an assignment operator (`=`, `+=`, `-=`, `*=`, etc.) and
         * a value expression.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed variable assignment;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_var_assignment (lexer& lex)
            -> g10::result_uptr<ast_node>;

    private: /* Private Methods - Operands ************************************/

        /**
         * @brief   Parses a single G10 assembly operand from the token stream
         *          provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed operand;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_operand (lexer& lex) -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses an immediate operand from the token stream provided
         *          by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed immediate operand;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_opr_immediate (lexer& lex) 
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses a direct memory address operand from the token stream
         *          provided by the given lexer.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed direct memory address operand;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_opr_direct (lexer& lex) 
            -> g10::result_uptr<ast_node>;

        /**
         * @brief   Parses an indirect memory address operand from the token
         *          stream provided by the given lexer.
         * 
         * This function is called by `parse_opr_direct` when it detects that
         * the memory operand contains a register instead of an expression.
         * 
         * @param   lex         The lexer instance providing the sequence of 
         *                      tokens to be parsed.
         * @param   bracket_tk  The opening bracket token that was already
         *                      consumed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed indirect memory address operand;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_opr_indirect (lexer& lex, const token& bracket_tk) 
            -> g10::result_uptr<ast_node>;

    private: /* Private Methods - Expressions *********************************/

        /**
         * @brief   Parses a single G10 assembly expression from the token
         *          stream provided by the given lexer.
         * 
         * This method serves as the entry point for expression parsing and
         * delegates to the lowest-precedence binary expression parser
         * (`parse_bitwise_or_expression`), which in turn chains to
         * higher-precedence parsers, ultimately reaching unary and primary
         * expression parsing.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_expression (lexer& lex) 
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a bitwise OR expression (`|`) from the token stream.
         * 
         * This is the lowest-precedence binary operator. It parses left-to-right,
         * building a left-associative tree of `ast_expr_binary` nodes.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_bitwise_or_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a bitwise XOR expression (`^`) from the token stream.
         * 
         * Higher precedence than bitwise OR. Parses left-to-right.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_bitwise_xor_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a bitwise AND expression (`&`) from the token stream.
         * 
         * Higher precedence than bitwise XOR. Parses left-to-right.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_bitwise_and_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a shift expression (`<<`, `>>`) from the token stream.
         * 
         * Higher precedence than bitwise AND. Parses left-to-right.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_shift_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses an additive expression (`+`, `-`) from the token stream.
         * 
         * Higher precedence than shift operators. Parses left-to-right.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_additive_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a multiplicative expression (`*`, `/`, `%`) from the
         *          token stream.
         * 
         * Higher precedence than additive operators. Parses left-to-right.
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_multiplicative_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses an exponentiation expression (`**`) from the token
         *          stream.
         * 
         * Higher precedence than multiplicative operators. Unlike other binary
         * operators, exponentiation is right-associative (e.g., `2 ** 3 ** 4`
         * is parsed as `2 ** (3 ** 4)`).
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_exponent_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a unary expression (`-`, `~`, `!`) from the token
         *          stream.
         * 
         * Unary operators have the highest precedence among expression operators.
         * They apply to the operand immediately following them. Unary expressions
         * can be nested (e.g., `--x`, `~~y`).
         * 
         * Supported unary operators:
         * - `-` : Arithmetic negation
         * - `~` : Bitwise NOT (complement)
         * - `!` : Logical NOT
         * 
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         * 
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed expression;
         *          Otherwise, returns an error.
         */
        static auto parse_unary_expression (lexer& lex)
            -> g10::result_uptr<ast_expression>;

        /**
         * @brief   Parses a primary G10 assembly expression from the token
         *          stream provided by the given lexer.
         * 
         * Primary expressions are the atomic building blocks of all expressions.
         * They include literals (integer, number, character, string),
         * identifiers, variables, placeholders, and grouped expressions
         * (parenthesized sub-expressions).
         *
         * @param   lex     The lexer instance providing the sequence of tokens
         *                  to be parsed.
         *
         * @return  If successful, returns a unique pointer to the AST node
         *          representing the parsed primary expression;
         *          Otherwise, returns an error indicating that a parsing error
         *          occurred.
         */
        static auto parse_primary_expression (lexer& lex) 
            -> g10::result_uptr<ast_expression>;

    private: /* Private Members ***********************************************/

        /**
         * @brief   Indicates whether or not the parser has encountered an
         *          error during any of its parsing operations, or whether it
         *          parsed anything at all.
         */
        static bool s_good;

    };
}
