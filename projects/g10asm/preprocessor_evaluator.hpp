/**
 * @file    g10asm/preprocessor_evaluator.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-03
 * 
 * @brief   Contains definitions for the expression evaluator used by the G10
 *          assembler's preprocessor component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10asm/lexer.hpp>
#include <g10asm/preprocessor_contexts.hpp>
#include <g10asm/preprocessor_macros.hpp>
#include <g10asm/preprocessor_values.hpp>

/* Public Types and Forward Declarations **************************************/

namespace g10asm
{
    /**
     * @brief   Forward declaration of the `preprocessor` class, which
     *          represents the G10 assembler's preprocessor component.
     */
    class preprocessor;
}

/* Public Constants and Enumerations ******************************************/

namespace g10asm
{

}

/* Public Unions and Structures ***********************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing the expression evaluator used by
     *          the G10 assembler's preprocessor component.
     * 
     * This subcomponent is responsible for evaluating expressions found
     * within the preprocessing language, such as those used in conditional
     * directives and macro arguments.
     */
    class pp_evaluator final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a new preprocessor expression evaluator
         *          instance, providing it with the tokens that make up the
         *          expression to be evaluated.
         * 
         * @param   tokens  A vector of tokens representing the expression
         *                  to be evaluated.
         */
        explicit pp_evaluator (const std::vector<token>& tokens) noexcept;

        /**
         * @brief   Indicates whether or not the expression was successfully
         *          evaluated.
         * 
         * @return  `true` if the expression was successfully evaluated;
         *          `false` otherwise.
         */
        inline auto is_good () const noexcept -> bool
            { return m_good; }

    private: /* Private Methods ***********************************************/



    private: /* Private Members ***********************************************/

        /**
         * @brief   A span of tokens representing the expression to be
         *          evaluated.
         */
        std::span<const token> m_tokens;

        /**
         * @brief   Indicates whether or not the provided expression was
         *          successfully evaluated.
         */
        bool m_good { false };

    };
}
