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
         * @brief   The maximum recursion depth for macro expansions.
         */
        std::size_t m_max_recursion_depth
            { DEFAULT_MAX_PREPROCESSOR_RECURSION_DEPTH };

        /**
         * @brief   The maximum include depth for file includes.
         */
        std::size_t m_max_include_depth
            { DEFAULT_MAX_PREPROCESSOR_INCLUDE_DEPTH };

    };
}
