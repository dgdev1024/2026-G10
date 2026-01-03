/**
 * @file    g10asm/preprocessor_evaluator.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-01
 * 
 * @brief   Contains definitions for the expression evaluator used by the G10
 *          assembler's preprocessor component.
 */

/* Private Includes ***********************************************************/

#include <g10asm/preprocessor.hpp>
#include <g10asm/preprocessor_evaluator.hpp>

/* Private Macros *************************************************************/

#define G10ASM_ERR_TOK(tok) \
    tok.source_file, \
    tok.source_line, \
    tok.source_column

/* Private Constants and Enumerations *****************************************/

namespace g10asm
{

}

/* Private Unions and Structures **********************************************/

namespace g10asm
{

}

/* Public Methods *************************************************************/

namespace g10asm
{
    pp_evaluator::pp_evaluator (
        const std::vector<token>& tokens
    ) noexcept :
        m_tokens { tokens }
    {
    }
}

/* Private Methods ************************************************************/

namespace g10asm
{

}
