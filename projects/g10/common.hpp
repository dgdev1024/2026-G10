/**
 * @file    g10/common.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-11
 * 
 * @brief   Contains includes and definitions commonly used by the G10 library
 *          and its client applications.
 */

#pragma once

/* Public Includes ************************************************************/

#include <algorithm>
#include <expected>
#include <filesystem>
#include <fstream>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <cctype>
#include <cstdint>

/* Public Macros - Library Import/Export **************************************/

#if defined(G10_BUILD_STATIC)
    #define g10api
#else
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(G10_BUILDING_SHARED)
            #define g10api __declspec(dllexport)
        #else
            #define g10api __declspec(dllimport)
        #endif
    #else
        #if __GNUC__ >= 4
            #define g10api __attribute__ ((visibility ("default")))
        #else
            #define g10api
        #endif
    #endif
#endif

/* Public Types ***************************************************************/

namespace fs = std::filesystem;

namespace g10
{

    /**
     * @brief   Defines a type representing a reference to a value of type T.
     * 
     * @tparam  T   The type of the referenced value.
     */
    template <typename T>
    using ref = std::reference_wrapper<T>;

    template <typename T>
    using cref = std::reference_wrapper<const T>;
    
    /**
     * @brief   Defines a type representing the result of an operation that
     *          can either succeed with a value of type T or fail with an
     *          error message.
     * 
     * @tparam  T   The type of the successful result value.
     */
    template <typename T, typename U = std::string>
    using result = std::expected<T, U>;

    /**
     * @brief   Defines a type representing the result of an operation that
     *          can either succeed with a reference to a value of type T or
     *          fail with an error message.
     * 
     * @tparam  T   The type of the successful result value.
     */
    template <typename T, typename U = std::string>
    using result_ref = std::expected<ref<T>, U>;

    /**
     * @brief   Defines a type representing the result of an operation that
     *          can either succeed with a constant reference to a value of type
     *          T or fail with an error message.
     * 
     * @tparam  T   The type of the successful result value.
     */
    template <typename T, typename U = std::string>
    using result_cref = std::expected<cref<T>, U>;

    /**
     * @brief   Defines a type representing the result of an operation that
     *          can either succeed with a unique pointer to a value of type T
     *          or fail with an error message.
     * 
     * @tparam  T   The type of the successful result value.
     */
    template <typename T, typename U = std::string>
    using result_uptr = std::expected<std::unique_ptr<T>, U>;

    /**
     * @brief   Defines a type representing the result of an operation that
     *          can either succeed with a shared pointer to a value of type T
     *          or fail with an error message.
     * 
     * @tparam  T   The type of the successful result value.
     */
    template <typename T, typename U = std::string>
    using result_sptr = std::expected<std::shared_ptr<T>, U>;

    /**
     * @brief   Defines a type representing an optional reference to a value
     *          of type T.
     * 
     * @tparam  T   The type of the referenced value.
     */
    template <typename T>
    using optional_ref = std::optional<ref<T>>;

    /**
     * @brief   Defines a type representing an optional constant reference
     *          to a value of type T.
     * 
     * @tparam  T   The type of the referenced value.
     */
    template <typename T>
    using optional_cref = std::optional<cref<T>>;

}

/* Public Functions ***********************************************************/

namespace g10
{

    /**
     * @brief   Helper function to create an unexpected result containing
     *          a formatted error message. This function is intended to be used
     *          with the @a `g10::result` type.
     * 
     * @tparam  Args    The types of the arguments to format into the string.
     * 
     * @param   format  The format string.
     * @param   args    The arguments to format into the string.
     * 
     * @return  An unexpected result containing the formatted error message.
     */
    template <typename... Args>
    inline auto error (const std::string& format, Args&&... args)
        -> std::unexpected<std::string>
    {
        return std::unexpected<std::string> {
            std::vformat(
                format,
                std::make_format_args(args...)
            )
        };
    }

}
