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
         * @brief   Parses a primary G10 assembly expression from the token
         *          stream provided by the given lexer.
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
