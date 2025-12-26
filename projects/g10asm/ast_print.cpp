/**
 * @file    g10asm/ast_print.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-22
 * 
 * @brief   Provides definitions for the G10 Assembler Tool's abstract syntax
 *          tree (AST) `print` methods.
 */

/* Private Includes ***********************************************************/

#include <g10asm/ast.hpp>

/* Private Macros *************************************************************/

#define i(l) std::string(l * 4, ' ')

/* Public Methods *************************************************************/

namespace g10asm
{
    auto ast_to_string (const ast_module& node, int indent) 
        -> std::string
    {
        std::string result = std::format("{}module \n", i(indent));
        for (const auto& child : node.children)
        {
            result += ast_to_string(*child, indent + 1);
        }
        
        return result;
    }

    auto ast_to_string (const ast_label_definition& node, 
        int indent) -> std::string
    {
        return std::format("{}label_definition: '{}'\n", i(indent), 
            node.label_name);
    }

    auto ast_to_string (const ast_instruction& node, 
        int indent) -> std::string
    {
        if (node.operands.empty())
        {
            return std::format("{}instruction: {}\n", i(indent), 
                node.lexeme);
        }
        else
        {
            std::string result = std::format("{}instruction: {} \n", 
                i(indent), node.lexeme);
            for (const auto& operand : node.operands)
            {
                result += ast_to_string(*operand, indent + 1);
            }
            
            return result;
        }
    }

    auto ast_to_string (const ast_dir_org& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}.org directive: \n", i(indent));
        result += ast_to_string(*node.address_expression, indent + 1);
        
        return result;
    }

    auto ast_to_string (const ast_dir_byte& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}.byte directive: \n", i(indent));
        for (std::size_t i = 0; i < node.values.size(); ++i)
        {
            result += ast_to_string(*node.values[i], indent + 1);
        }
        
        return result;
    }

    auto ast_to_string (const ast_dir_word& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}.word directive: \n", i(indent));
        for (std::size_t i = 0; i < node.values.size(); ++i)
        {
            result += ast_to_string(*node.values[i], indent + 1);
        }
        
        return result;
    }

    auto ast_to_string (const ast_dir_dword& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}.dword directive: \n", i(indent));
        for (std::size_t i = 0; i < node.values.size(); ++i)
        {
            result += ast_to_string(*node.values[i], indent + 1);
        }
        
        return result;
    }

    auto ast_to_string (const ast_dir_global& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}.global directive:\n", i(indent));
        for (std::size_t i = 0; i < node.symbols.size(); ++i)
        {
            result += std::format("{}{}\n", i(indent + 1), 
                node.symbols[i]);
        }
        return result;
    }

    auto ast_to_string (const ast_dir_extern& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}.extern directive:\n", i(indent));
        for (std::size_t i = 0; i < node.symbols.size(); ++i)
        {
            result += std::format("{}{}\n", i(indent + 1), 
                node.symbols[i]);
        }
        return result;
    }   

    auto ast_to_string (const ast_opr_immediate& node, 
        int indent) -> std::string
    {
        std::string result = std::format("{}immediate operand: \n", 
            i(indent));
        result += ast_to_string(*node.value, indent + 1);
        
        return result;
    }

    auto ast_to_string (const ast_opr_register& node, 
        int indent) -> std::string
    {
        return std::format("{}register operand: {}\n", i(indent), 
            node.lexeme);
    }

    auto ast_to_string (const ast_opr_condition& node, 
        int indent) -> std::string
    {
        return std::format("{}condition operand: {}\n", i(indent), 
            node.lexeme);
    }

    auto ast_to_string (const ast_opr_direct& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}direct operand: \n", 
            i(indent));
        result += ast_to_string(*node.address, indent + 1);
        
        return result;
    }

    auto ast_to_string (const ast_opr_indirect& node, 
        int indent) -> std::string
    {
        return std::format("{}indirect operand: [{}]\n", i(indent), 
            node.lexeme);
    }

    auto ast_to_string (const ast_expr_binary& node, 
        int indent) -> std::string
    {
        std::string result = std::format("{}binary expression: \n", 
            i(indent));
        result += std::format("{}operator: {}\n", i(indent + 1), 
            node.lexeme);
        if (node.left_operand != nullptr)
        {
            result += std::format("{}left_operand: \n", i(indent + 1));
            result += ast_to_string(*node.left_operand, indent + 2);
        }

        if (node.right_operand != nullptr)
        {
            result += std::format("{}right_operand: \n", i(indent + 1));
            result += ast_to_string(*node.right_operand, indent + 2);
        }
        
        return result;
    }

    auto ast_to_string (const ast_expr_unary& node, int indent)
        -> std::string
    {
        std::string result = std::format("{}unary expression: \n", 
            i(indent));
        result += std::format("{}operator: {}\n", i(indent + 1), 
            node.lexeme);
        if (node.operand != nullptr)
        {
            result += std::format("{}operand: \n", i(indent + 1));
            result += ast_to_string(*node.operand, indent + 2);
        }
        
        return result;
    }

    auto ast_to_string (const ast_expr_grouping& node, 
        int indent) -> std::string
    {
        std::string result = std::format("{}grouping expression: \n", 
            i(indent));
        if (node.inner_expression != nullptr)
        {
            result += ast_to_string(*node.inner_expression, indent + 2);
        }
        
        return result;
    }

    auto ast_to_string (const ast_expr_primary& node, 
        int indent) -> std::string
    {
        std::string result = "";
        switch (node.expr_type)
        {
            case ast_expr_primary::primary_type::integer_literal:
                result = std::format("{}integer: {}\n", i(indent), 
                    std::get<int64_t>(node.value));
                break;
            case ast_expr_primary::primary_type::number_literal:
                result = std::format("{}number: {}\n", i(indent), 
                    std::get<double>(node.value));
                break;
            case ast_expr_primary::primary_type::char_literal:
                result = std::format("{}char: '{}'\n", i(indent), 
                    std::get<char>(node.value));
                break;
            case ast_expr_primary::primary_type::string_literal:
                result = std::format("{}string: \"{}\"\n", i(indent), 
                    std::get<std::string_view>(node.value));
                break;
            case ast_expr_primary::primary_type::identifier:
                result = std::format("{}identifier: {}\n", i(indent), 
                    std::get<std::string_view>(node.value));
                break;
            case ast_expr_primary::primary_type::variable:
                result = std::format("{}variable: {}\n", i(indent), 
                    std::get<std::string_view>(node.value));
                break;
            case ast_expr_primary::primary_type::placeholder:
                result = std::format("{}placeholder\n", i(indent));
                break;
            default:
                result = std::format("{}unknown primary type\n", i(indent));
                break;
        }
        return result;
    }

    auto ast_to_string (const ast_node& node, int indent) -> std::string
    {
        switch (node.type)
        {
            case ast_node_type::module:
                return ast_to_string(static_cast<const ast_module&>(node), indent);
            case ast_node_type::label_definition:
                return ast_to_string(static_cast<const ast_label_definition&>(node), indent);
            case ast_node_type::instruction:
                return ast_to_string(static_cast<const ast_instruction&>(node), indent);
            case ast_node_type::dir_org:
                return ast_to_string(static_cast<const ast_dir_org&>(node), indent);
            case ast_node_type::dir_byte:
                return ast_to_string(static_cast<const ast_dir_byte&>(node), indent);
            case ast_node_type::dir_word:
                return ast_to_string(static_cast<const ast_dir_word&>(node), indent);
            case ast_node_type::dir_dword:
                return ast_to_string(static_cast<const ast_dir_dword&>(node), indent);
            case ast_node_type::dir_global:
                return ast_to_string(static_cast<const ast_dir_global&>(node), indent);
            case ast_node_type::dir_extern:
                return ast_to_string(static_cast<const ast_dir_extern&>(node), indent);
            case ast_node_type::opr_immediate:
                return ast_to_string(static_cast<const ast_opr_immediate&>(node), indent);
            case ast_node_type::opr_register:
                return ast_to_string(static_cast<const ast_opr_register&>(node), indent);
            case ast_node_type::opr_condition:
                return ast_to_string(static_cast<const ast_opr_condition&>(node), indent);
            case ast_node_type::opr_direct:
                return ast_to_string(static_cast<const ast_opr_direct&>(node), indent);
            case ast_node_type::opr_indirect:
                return ast_to_string(static_cast<const ast_opr_indirect&>(node), indent);
            
            // - Expression types - cast to ast_expression and call expression version
            case ast_node_type::expr_binary:
            case ast_node_type::expr_unary:
            case ast_node_type::expr_grouping:
            case ast_node_type::expr_primary:
                return ast_to_string(static_cast<const ast_expression&>(node), indent);
            
            default:
                return std::format("{}unknown node type\n", i(indent));
        }
    }

    auto ast_to_string (const ast_expression& node, int indent) -> std::string
    {
        switch (node.type)
        {
            case ast_node_type::expr_binary:
                return ast_to_string(static_cast<const ast_expr_binary&>(node), indent);
            case ast_node_type::expr_unary:
                return ast_to_string(static_cast<const ast_expr_unary&>(node), indent);
            case ast_node_type::expr_grouping:
                return ast_to_string(static_cast<const ast_expr_grouping&>(node), indent);
            case ast_node_type::expr_primary:
                return ast_to_string(static_cast<const ast_expr_primary&>(node), indent);
            default:
                return std::format("{}unknown expression type\n", i(indent));
        }
    }
}
