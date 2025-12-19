/**
 * @file    g10-asm/common.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains includes and definitions commonly used by the G10 CPU's
 *          assembler tool project.
 */

#pragma once

/* Public Includes ************************************************************/

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <print>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <cctype>
#include <g10/common.hpp>

/* Public Types ***************************************************************/

namespace fs = std::filesystem;

namespace g10asm
{
    using namespace g10;
}
