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
#include <functional>
#include <memory>
#include <print>
#include <span>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>

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

/* Public Macros **************************************************************/

#define G10_BIT_ENUM(ec) \
    inline constexpr auto operator| (ec a, ec b) noexcept -> ec \
        { return static_cast<ec>( \
            std::to_underlying(a) | std::to_underlying(b)); } \
    inline constexpr auto operator& (ec a, ec b) noexcept -> ec \
        { return static_cast<ec>( \
            std::to_underlying(a) & std::to_underlying(b)); } \
    inline constexpr auto operator~ (ec a) noexcept -> ec \
        { return static_cast<ec>(~std::to_underlying(a)); }

/* Public Constants ***********************************************************/

namespace g10
{
    /**
     * @brief   Defines the major, minor, and patch version numbers of the G10
     *          library.
     */
    constexpr std::uint8_t  MAJOR_VERSION   = 1;
    constexpr std::uint8_t  MINOR_VERSION   = 0;
    constexpr std::uint16_t PATCH_VERSION   = 0;

    /**
     * @brief   Defines the combined version number of the G10 library, as a
     *          32-bit integer laid out as follows: `0xMMmmPPPP`, where
     *          `MM` is the major version, `mm` is the minor version, and
     *          `PPPP` is the patch version.
     */
    constexpr std::uint32_t VERSION_NUMBER =
        (static_cast<std::uint32_t>(MAJOR_VERSION) << 24) |
        (static_cast<std::uint32_t>(MINOR_VERSION) << 16) |
        (static_cast<std::uint32_t>(PATCH_VERSION));

    /**
     * @brief   Defines the version string of the G10 library in the format
     *          "MAJOR.MINOR.PATCH".
     */
    const std::string VERSION_STRING = 
        std::format(
            "{}.{}.{}",
            MAJOR_VERSION,
            MINOR_VERSION,
            PATCH_VERSION
        );
}

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

    /**
     * @brief   Defines a variant type representing references to multiple
     *          types.
     * 
     * @tparam  Ts  The types of the referenced values.
     */
    template <typename... Ts>
    using variant_ref = std::variant<ref<Ts>...>;

    /**
     * @brief   Defines a variant type representing constant references to
     *          multiple types.
     * 
     * @tparam  Ts  The types of the referenced values.
     */
    template <typename... Ts>
    using variant_cref = std::variant<cref<Ts>...>;

}

/* Public Functions ***********************************************************/

namespace g10
{
    template <typename T, typename... Args>
    inline auto ok (Args&&... args) -> result<T>
    {
        return result<T> { T(std::forward<Args>(args)...) };
    }

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

    /**
     * @brief   Reads a little-endian 16-bit value from a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to read from.
     * @param   offset  The offset in bytes within the buffer.
     * 
     * @return  The 16-bit value in native byte order.
     */
    inline auto read_u16_le (
        std::span<const std::uint8_t> buffer,
        std::size_t offset
    ) -> std::uint16_t
    {
        return static_cast<std::uint16_t>(buffer[offset]) |
               (static_cast<std::uint16_t>(buffer[offset + 1]) << 8);
    }

    /**
     * @brief   Reads a little-endian 32-bit value from a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to read from.
     * @param   offset  The offset in bytes within the buffer.
     * 
     * @return  The 32-bit value in native byte order.
     */
    inline auto read_u32_le (
        std::span<const std::uint8_t> buffer,
        std::size_t offset
    ) -> std::uint32_t
    {
        return static_cast<std::uint32_t>(buffer[offset]) |
               (static_cast<std::uint32_t>(buffer[offset + 1]) << 8) |
               (static_cast<std::uint32_t>(buffer[offset + 2]) << 16) |
               (static_cast<std::uint32_t>(buffer[offset + 3]) << 24);
    }

    /**
     * @brief   Writes a little-endian 16-bit value to a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to write to.
     * @param   offset  The offset in bytes within the buffer.
     * @param   value   The 16-bit value to write in native byte order.
     */
    inline auto write_u16_le (
        std::span<std::uint8_t> buffer,
        std::size_t offset,
        std::uint16_t value
    ) -> void
    {
        buffer[offset]     = static_cast<std::uint8_t>(value & 0xFF);
        buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    }

    /**
     * @brief   Writes a little-endian 32-bit value to a byte span at the
     *          given offset.
     * 
     * @param   buffer  The buffer to write to.
     * @param   offset  The offset in bytes within the buffer.
     * @param   value   The 32-bit value to write in native byte order.
     */
    inline auto write_u32_le (
        std::span<std::uint8_t> buffer,
        std::size_t offset,
        std::uint32_t value
    ) -> void
    {
        buffer[offset]     = static_cast<std::uint8_t>(value & 0xFF);
        buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        buffer[offset + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
        buffer[offset + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
    }

}
