/**
 * @file    g10asm/preprocessor.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2023-01-03
 * 
 * @brief   Contains definitions for the G10 assembler's preprocessor
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/lexer.hpp>
#include <g10asm/preprocessor_macros.hpp>
#include <g10asm/preprocessor_evaluator.hpp>

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{
    /**
     * @brief   The default maximum recursion depth for the preprocessor.
     * 
     * This value limits how deeply nested macro expansions can be.
     */
    constexpr std::size_t DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH = 256;

    /**
     * @brief   The default maximum include depth for the preprocessor.
     * 
     * This value limits how deeply nested file includes can be.
     */
    constexpr std::size_t DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH = 16;
}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a structure representing the configuration options
     *          for the G10 assembler's preprocessor component.
     */
    struct preprocessor_config final
    {
        /**
         * @brief   The maximum recursion depth for macro expansions.
         * 
         * This value limits how deeply nested macro expansions can be, and is
         * used to prevent excessive (or infinite) recursion during
         * preprocessing.
         */
        std::size_t max_recursion_depth = 
            DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH;

        /**
         * @brief   The maximum include depth for file includes.
         * 
         * This value limits how deeply nested file includes can be, and is
         * used to prevent excessive (or infinite) recursion during
         * preprocessing.
         * 
         * Recursive file inclusions can also be limited via the `.pragma once`
         * preprocessor directive.
         */
        std::size_t max_include_depth = 
            DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH;

        /**
         * @brief   A list of directories to search for included files.
         * 
         * When the preprocessor encounters an `.include` directive, it will
         * search these directories (in order) to locate the specified file, in
         * addition to the current working directory and the directory of the
         * file containing the directive.
         * 
         * Use the `-i` or `--include` command-line options to add directories
         * to this list.
         */
        std::vector<std::string> include_dirs {};
    };

    /**
     * @brief   Represents the state of a conditional assembly block.
     * 
     * This structure tracks whether a condition has been met, whether the
     * current branch is active, and whether an `.else` directive has been
     * encountered.
     */
    struct conditional_state final
    {
        /**
         * @brief   Indicates whether any branch in this conditional block
         *          has been taken (condition was true).
         */
        bool condition_met { false };

        /**
         * @brief   Indicates whether the current branch is being processed
         *          (tokens should be emitted).
         */
        bool currently_active { false };

        /**
         * @brief   Indicates whether an `.else` directive has been seen
         *          in this conditional block.
         */
        bool else_seen { false };

        /**
         * @brief   Source file where the conditional block started.
         */
        std::string source_file {};

        /**
         * @brief   Source line where the conditional block started.
         */
        std::size_t source_line { 0 };
    };

    /**
     * @brief   Enumerates the types of loop assembly blocks.
     */
    enum class loop_type
    {
        repeat,     ///< .repeat/.rept loop
        for_loop,   ///< .for loop
        while_loop  ///< .while loop
    };

    /**
     * @brief   Represents the state of a loop assembly block.
     * 
     * This structure tracks the loop type, iteration count, loop variable,
     * and the collected body tokens for re-processing.
     */
    struct loop_state final
    {
        /**
         * @brief   The type of loop (repeat, for, or while).
         */
        loop_type type { loop_type::repeat };

        /**
         * @brief   Name of the loop variable (optional for repeat/while).
         */
        std::string variable_name {};

        /**
         * @brief   Current value of the loop variable.
         */
        std::int64_t current_value { 0 };

        /**
         * @brief   End value for .for loops (exclusive).
         */
        std::int64_t end_value { 0 };

        /**
         * @brief   Step value for .for loops.
         */
        std::int64_t step_value { 1 };

        /**
         * @brief   Current iteration count (0-based).
         */
        std::size_t iteration_count { 0 };

        /**
         * @brief   Maximum number of iterations for .repeat loops.
         */
        std::size_t max_iterations { 0 };

        /**
         * @brief   Collected tokens of the loop body.
         */
        std::vector<token> body_tokens {};

        /**
         * @brief   Condition tokens for .while loops.
         */
        std::vector<token> condition_tokens {};

        /**
         * @brief   Source file where the loop started.
         */
        std::string source_file {};

        /**
         * @brief   Source line where the loop started.
         */
        std::size_t source_line { 0 };

        /**
         * @brief   Flag to indicate loop should break.
         */
        bool should_break { false };

        /**
         * @brief   Flag to indicate loop should continue to next iteration.
         */
        bool should_continue { false };
    };
}

/* Public Classes *************************************************************/

namespace g10asm
{
    class preprocessor final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a new instance of the G10 assembler preprocessor
         *          with the specified configuration and token from the given
         *          lexer component.
         * 
         * @param   config  The preprocessor configuration options.
         * @param   lexer   The lexer instance providing the input tokens.
         */
        preprocessor (
            const preprocessor_config&  config,
            const lexer&                lexer
        );

        /**
         * @brief   Retrieves the output string resulting from preprocessing.
         * 
         * This string contains the preprocessed assembly source code, which
         * can be sent back to the lexer for further processing before parsing.
         * 
         * @return  The preprocessed assembly source code as a string.
         */
        inline auto get_output () const -> std::string_view
            { return m_output_string; }

        /**
         * @brief   Indicates whether the preprocessor is in a good state.
         *
         * If preprocessing completed without errors, this method returns `true`.
         * If any errors occurred during preprocessing, this method returns `false`.
         * 
         * @return  If the preprocessor is in a good state, returns `true`;
         *          otherwise, returns `false`.
         */        
        inline auto is_good () const -> bool
            { return m_good; }

    private: /* Private Methods - Initialization ******************************/

        /**
         * @brief   Resolves the include paths specified in the preprocessor
         *          component's configuration.
         * 
         * This initialization step processes the list of include directories,
         * resolving them to absolute, lexically-normalized paths, ensuring that
         * they exist and are valid directories, and preparing them for use
         * in file inclusion during preprocessing.
         * 
         * @param   include_dirs    The list of include directories to resolve.
         * 
         * @return  If all include paths (if any) are successfully resolved,
         *          returns `void`;
         *          Otherwise, returns an error indicating the reason for the
         *          failure.
         */
        auto resolve_include_paths (
            const std::vector<std::string>& include_dirs)
                -> g10::result<void>;

        /**
         * @brief   Filters the input tokens obtained from the lexer, removing
         *          any unnecessary tokens and preparing them for preprocessing.
         * 
         * @param   tokens  The list of tokens obtained from the lexer.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error indicating the reason for the
         *          failure.
         */
        auto filter_input_tokens (const std::vector<token>& tokens)
            -> g10::result<void>;

        /**
         * @brief   Performs the preprocessing of the input tokens, handling
         *          directives such as includes, macros, and conditionals.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error indicating the reason for the
         *          failure.
         */
        auto preprocess () -> g10::result<void>;

    private: /* Private Methods - Token Navigation ****************************/

        /**
         * @brief   Retrieves the current token at the current index position.
         *
         * @return  If successful, returns a constant reference to the current
         *          token in the input token list;
         *          Otherwise, returns an error indicating the reason for the
         *          failure.
         */
        auto current_token () const -> g10::result_cref<token>;

        /**
         * @brief   Peeks at a token at the specified offset from the current
         *          position. The offset can be positive (lookahead) or negative
         *          (lookbehind).
         *
         * @param   offset  The offset from the current position. Specify `0` to
         *                  get the current token, positive values to look ahead,
         *                  and negative values to look behind.
         *
         * @return  If `offset` is within bounds, returns a constant reference to
         *          the token at the specified offset;
         *          Otherwise, returns an error indicating the reason for the
         *          failure.
         */
        auto peek_token (std::int64_t offset = 0) const 
            -> g10::result_cref<token>;

        /**
         * @brief   Advances the current index position by the specified count.
         *
         * If advancing would move the index beyond the end of the token list,
         * the index is set to the end of the list instead.
         *
         * @param   count   The number of positions to advance.
         */
        auto advance (std::size_t count = 1) -> void;

        /**
         * @brief   Checks if the current position is at the end of the token
         *          list.
         *
         * @return  If the current position is at or beyond the end of the token
         *          list, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto is_at_end () const -> bool;

    private: /* Private Methods - Output Building *****************************/

        /**
         * @brief   Determines if a space should be added before the given
         *          token type.
         *
         * @param   type    The token type to check.
         *
         * @return  If no space should precede this token type, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto is_no_space_before (token_type type) const -> bool;

        /**
         * @brief   Determines if a space should be added after the given
         *          token type.
         *
         * @param   type    The token type to check.
         *
         * @return  If no space should follow this token type, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto is_no_space_after (token_type type) const -> bool;

        /**
         * @brief   Appends a token's lexeme to the output string with
         *          appropriate spacing.
         *
         * @param   tok     The token to append.
         */
        auto append_token (const token& tok) -> void;

        /**
         * @brief   Appends a string view directly to the output.
         *
         * @param   str     The string view to append.
         * 
         * @warning If appending a temporary string view, ensure that the
         *          underlying string data remains valid for the lifetime of
         *          the preprocessor's output string. Otherwise, this may lead to
         *          dangling references and undefined behavior. Use
         *          @a `append_string` for temporary strings.
         */
        auto append_string_view (std::string_view str) -> void;

        /**
         * @brief   Appends a string directly to the output.
         *
         * @param   str     The string to append.
         */
        auto append_string (const std::string& str) -> void;

        /**
         * @brief   Appends a newline to the output and resets spacing state.
         */
        auto append_newline () -> void;

    private: /* Private Methods - Line Continuation ***************************/

        /**
         * @brief   Handles line continuation (backslash followed by newline).
         * 
         * Line continuations in the G10's preprocessing language allow a
         * logical line of code to be split across multiple physical lines in
         * the source file. This is achieved by placing a backslash (`\`) at the
         * end of a line, immediately followed by a newline character. The
         * preprocessor recognizes this pattern and treats the subsequent line
         * as a continuation of the current line, effectively removing the
         * backslash and newline from the output.
         *
         * @return  If a line continuation was handled, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto handle_line_continuation () -> bool;

    private: /* Private Methods - Directive Handling **************************/

        /**
         * @brief   Checks if the current token is a preprocessor directive
         *          and handles it if so.
         * 
         * @return  If a directive was handled, returns `true`;
         *          Otherwise, returns `false`. Returns an error if directive
         *          processing failed.
         */
        auto handle_directive () -> g10::result<bool>;

        /**
         * @brief   Handles the `.define` directive to create a text-substitution
         *          macro.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_define_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.undef` or `.purge` directive to remove a macro.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_undef_directive () -> g10::result<void>;

    private: /* Private Methods - Conditional Assembly ************************/

        /**
         * @brief   Checks if the preprocessor is currently in an active
         *          conditional branch (i.e., tokens should be emitted).
         * 
         * @return  `true` if currently active (or no conditionals);
         *          `false` if in an inactive branch.
         */
        auto is_conditionally_active () const -> bool;

        /**
         * @brief   Skips tokens until the next relevant conditional directive
         *          is found (.elif, .else, .endif, or nested .if/.endif pairs).
         * 
         * This method is called when a conditional branch is inactive and
         * tokens need to be skipped until the next branch or end of block.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error (e.g., unmatched conditional).
         */
        auto skip_conditional_block () -> g10::result<void>;

        /**
         * @brief   Validates that a token list does not contain braces.
         * 
         * In directive expression contexts (`.if`, `.elif`, `.repeat`, `.for`,
         * `.while`), braces `{}` are not allowed since the entire argument is
         * already an expression handled by the preprocessor.
         * 
         * @param   tokens          The tokens to validate.
         * @param   directive_name  The name of the directive (for error messages).
         * 
         * @return  If no braces are found, returns `void`;
         *          Otherwise, returns an error.
         */
        auto validate_no_braces (
            const std::vector<token>& tokens,
            std::string_view directive_name
        ) -> g10::result<void>;

        /**
         * @brief   Evaluates a condition expression for `.if` or `.elif`.
         * 
         * Collects tokens until newline and evaluates them as an expression.
         * 
         * @return  If successful, returns the boolean result of the condition;
         *          Otherwise, returns an error.
         */
        auto evaluate_condition () -> g10::result<bool>;

        /**
         * @brief   Handles the `.if` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_if_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.ifdef` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_ifdef_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.ifndef` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_ifndef_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.elif` or `.elseif` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_elif_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.else` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_else_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.endif` or `.endc` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_endif_directive () -> g10::result<void>;

    private: /* Private Methods - Loop Assembly *******************************/

        /**
         * @brief   Checks if the preprocessor is currently inside a loop.
         * 
         * @return  `true` if inside a loop; `false` otherwise.
         */
        auto is_in_loop () const -> bool;

        /**
         * @brief   Collects tokens for a loop body until the matching end
         *          directive is found.
         * 
         * @param   end_directive   The directive type that ends this loop.
         * 
         * @return  If successful, returns the collected tokens;
         *          Otherwise, returns an error.
         */
        auto collect_loop_body (directive_type end_directive) 
            -> g10::result<std::vector<token>>;

        /**
         * @brief   Processes a single iteration of a loop.
         * 
         * @param   body_tokens The tokens of the loop body.
         * @param   loop        Reference to the current loop state.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto process_loop_iteration (
            const std::vector<token>& body_tokens,
            loop_state& loop
        ) -> g10::result<void>;

        /**
         * @brief   Handles the `.repeat` or `.rept` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_repeat_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.endrepeat` or `.endr` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_endrepeat_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.for` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_for_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.endfor` or `.endf` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_endfor_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.while` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_while_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.endwhile` or `.endw` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_endwhile_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.continue` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_continue_directive () -> g10::result<void>;

        /**
         * @brief   Handles the `.break` directive.
         * 
         * @return  If successful, returns `void`;
         *          Otherwise, returns an error.
         */
        auto handle_break_directive () -> g10::result<void>;

    private: /* Private Methods - Macro Expansion *****************************/

        /**
         * @brief   Attempts to expand a macro if the current token is an
         *          identifier that matches a defined macro.
         * 
         * @return  If macro expansion occurred, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto try_expand_macro () -> bool;

    private: /* Private Methods - Expression Evaluation ***********************/

        /**
         * @brief   Handles braced expression interpolation.
         * 
         * When the preprocessor encounters a left brace `{`, it collects
         * all tokens until the matching right brace `}`, evaluates the
         * expression, and emits the result to the output string.
         * 
         * @return  If a braced expression was handled, returns `true`;
         *          Otherwise, returns `false`. Returns an error if evaluation
         *          failed.
         */
        auto handle_braced_expression () -> g10::result<bool>;

        /**
         * @brief   Handles identifier interpolation with braced expressions.
         * 
         * This handles cases where identifiers contain or are adjacent to
         * braced expressions, such as:
         * - `{expr}_suffix` → `result_suffix`
         * - `prefix_{expr}` → `prefix_result`
         * - `prefix_{expr}_suffix` → `prefix_result_suffix`
         * 
         * @return  If identifier interpolation was handled, returns `true`;
         *          Otherwise, returns `false`. Returns an error if evaluation
         *          failed.
         */
        auto handle_identifier_interpolation () -> g10::result<bool>;

        /**
         * @brief   Handles string literal interpolation with braced expressions.
         * 
         * This processes string literals to find and evaluate embedded
         * expressions like `"The answer is {42}."`.
         * 
         * @return  If string interpolation was handled, returns `true`;
         *          Otherwise, returns `false`. Returns an error if evaluation
         *          failed.
         */
        auto handle_string_interpolation () -> g10::result<bool>;

        /**
         * @brief   Evaluates a braced expression given a string containing
         *          the expression content (without the braces).
         * 
         * @param   expr_content    The expression content to evaluate.
         * @param   source_file     Source file for error reporting.
         * @param   source_line     Source line for error reporting.
         * 
         * @return  If successful, returns the string representation of the
         *          evaluated expression;
         *          Otherwise, returns an error.
         */
        auto evaluate_inline_expression (
            const std::string& expr_content,
            const std::string& source_file,
            std::size_t source_line
        ) -> g10::result<std::string>;

        /**
         * @brief   Checks if two tokens are adjacent (no whitespace between them).
         * 
         * @param   first   The first token.
         * @param   second  The second token.
         * 
         * @return  `true` if the tokens are adjacent; `false` otherwise.
         */
        auto are_tokens_adjacent (const token& first, const token& second) const
            -> bool;

    private: /* Private Members ***********************************************/

        /**
         * @brief   The list of tokens received from the lexer, filtered and ready
         *          for preprocessing.
         */
        std::vector<token> m_input_tokens {};

        /**
         * @brief   The output string resulting from the preprocessing stage.
         *          This is the final preprocessed assembly source code, sent
         *          back to the lexer for further processing before parsing.
         */
        std::string m_output_string {};

        /**
         * @brief   Indicates whether the preprocessor is in a good state.
         * 
         * If preprocessing completes without errors, this flag is set to `true`.
         * If any errors occur during preprocessing, this flag is set to `false`.
         */
        bool m_good { false };

        /**
         * @brief   The current index position in the input token list.
         */
        std::size_t m_current_index { 0 };

        /**
         * @brief   Indicates whether a space should be added before the next
         *          token in the output.
         */
        bool m_needs_space { false };

        /**
         * @brief   The maximum recursion depth for macro expansions.
         */
        std::size_t m_max_recursion_depth
            { DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH };

        /**
         * @brief   The maximum include depth for file includes.
         */
        std::size_t m_max_include_depth
            { DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH };

        /**
         * @brief   The macro table for storing defined macros.
         */
        pp_macro_table m_macro_table {};

        /**
         * @brief   Stack of conditional assembly states for tracking nested
         *          `.if`/`.endif` blocks.
         */
        std::vector<conditional_state> m_conditional_stack {};

        /**
         * @brief   Stack of loop states for tracking nested loops.
         */
        std::vector<loop_state> m_loop_stack {};

    };
}
