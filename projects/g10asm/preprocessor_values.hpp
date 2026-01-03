/**
 * @file    g10asm/preprocessor_values.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2026-01-03
 * 
 * @brief   Contains definitions for value types represented in the G10
 *          assembler's preprocessing language.
 */

#pragma once

/* Public Includes ************************************************************/

#include <g10/common.hpp>

/* Public Types and Forward Declarations **************************************/

namespace g10asm
{
    /**
     * @brief   Forward declaration of the `pp_number` class, which represents
     *          the `number` type in the G10 assembler's preprocessing language.
     */
    class pp_number;

    /**
     * @brief   Defines a type representing the `void`/`undefined` type in the
     *          G10 assembler's preprocessing language.
     */
    using pp_void = std::monostate;

    /**
     * @brief   Defines a type representing the integer type in the G10 assembler's
     *          preprocessing language.
     */
    using pp_integer = std::int64_t;

    /**
     * @brief   Defines a type representing the boolean type in the G10 assembler's
     *          preprocessing language.
     */
    using pp_boolean = bool;

    /**
     * @brief   Defines a type representing the string type in the G10 assembler's
     *          preprocessing language.
     */
    using pp_string = std::string;

    /**
     * @brief   Defines a variant type that can hold any value type in the G10
     *          assembler's preprocessing language.
     */
    using pp_value = std::variant<
        pp_void,
        pp_integer,
        pp_number,
        pp_boolean,
        pp_string
    >;
}

/* Public Classes *************************************************************/

namespace g10asm
{
    /**
     * @brief   Defines a class representing the `number` type in the G10
     *          assembler's preprocessing language.
     * 
     * A `number` in the G10 assembler's preprocessing language is represented
     * in `32.32` fixed-point format, in which the upper 32 bits represent the
     * signed integer portion of the number, and the lower 32 bits represent
     * the fractional portion of the number.
     */
    class pp_number final
    {
    public:

        /**
         * @brief   The default constructor initializes the `number` to zero.
         */
        pp_number () = default;

        /**
         * @brief   Constructs a `number` from a floating-point value, converting
         *          it to the necessary `32.32` fixed-point representation.
         * 
         * @param   value   The floating-point value to convert to a `number`.
         */
        inline explicit pp_number (const double value) :
            m_float { value }
        {
            double int_part = 0.0;
            double frac_part = std::modf(value, &int_part);

            m_raw =
                (
                    static_cast<std::uint64_t>(
                        static_cast<std::int32_t>(int_part)
                    ) << 32
                ) |
                static_cast<std::uint64_t>(
                    frac_part * static_cast<double>(1ULL << 32)
                );
        }

        /**
         * @brief   Retrieves the `number`'s signed integer portion.
         *
         * @return  The `number`'s signed integer portion.
         */
        inline auto get_signed_integer () const -> pp_integer
        {
            return static_cast<pp_integer>(m_raw >> 32) & 0xFFFFFFFFLL;
        }

        /**
         * @brief   Calculates the `number`'s floating-point value from its
         *          `32.32` fixed-point representation.
         * 
         * @return  The `number`'s calculated floating-point value.
         */
        inline auto get_calculated_float () const -> double
        {
            return (
                static_cast<double>(get_signed_integer()) +
                (
                    static_cast<double>(m_raw & 0xFFFFFFFFULL) /
                    static_cast<double>(1ULL << 32)
                )
            );
        }

        /**
         * @brief   Retrieves the `number`'s raw, 64-bit integer representation.
         * 
         * @return  The `number`'s raw, 64-bit integer representation.
         */
        inline auto get_raw () const -> std::uint64_t
        {
            return m_raw;
        }

        /**
         * @brief   Retrieves the `number`'s actual floating-point value.
         * 
         * @return  The `number`'s actual floating-point value.
         */
        inline auto get_float () const -> double
        {
            return m_float;
        }

        /**
         * @brief   Retrieves the `number`'s unsigned integer portion (the
         *          upper 32 bits).
         * 
         * @return  The `number`'s unsigned integer portion.
         */
        inline auto get_integer_part () const -> std::uint32_t
        {
            return static_cast<std::uint32_t>(m_raw >> 32);
        }

        /**
         * @brief   Retrieves the `number`'s fractional portion (the lower 32
         *          bits).
         * 
         * @return  The `number`'s fractional portion.
         */
        inline auto get_fractional_part () const -> std::uint32_t
        {
            return static_cast<std::uint32_t>(m_raw & 0xFFFFFFFFULL);
        }
        
        /**
         * @brief   Assignment operator for assigning a `double` value to
         *          a `number` instance.
         */
        inline auto operator= (const double value) -> pp_number&
        {
            *this = pp_number { value };
            return *this;
        }

        /**
         * @brief   Equality operator for comparing two `number` instances.
         * 
         * @param   other   The other `number` instance to compare against.
         * 
         * @return  `true` if the two `number` instances are equal; `false`
         *          otherwise.
         */
        inline auto operator== (const pp_number& other) const -> bool
        {
            return m_raw == other.m_raw;
        }

        /**
         * @brief   Three-way comparison operator for comparing two `number`
         *          instances.
         * 
         * @param   other   The other `number` instance to compare against.
         * 
         * @return  The result of the three-way comparison.
         */
        inline auto operator<=> (const pp_number& other) const 
        {
            return get_calculated_float() <=> other.get_calculated_float();
        }

    private:
        std::uint64_t m_raw { 0 };  /** @brief The `number`'s raw, 64-bit integer representation. */
        double m_float { 0.0 };     /** @brief The `number`'s actual floating-point value. */

    };
}

/* Public Functions ***********************************************************/

namespace g10asm::pp_values
{
    /**
     * @brief   Determines if the given `pp_value` is of type `pp_void`, which
     *          holds a void or undefined value.
     * 
     * @param   value   The `pp_value` to check.
     * 
     * @return  If `value` is of type `pp_void`, returns `true`; 
     *          Otherwise, returns `false`.
     */
    inline constexpr auto is_void (const pp_value& value) -> bool
        { return std::holds_alternative<pp_void>(value); }

    /**
     * @brief   Determines if the given `pp_value` is of type `pp_integer`, 
     *          which holds an integer value.
     * 
     * @param   value   The `pp_value` to check.
     * 
     * @return  If `value` is of type `pp_integer`, returns `true`; 
     *          Otherwise, returns `false`.
     */
    inline constexpr auto is_integer (const pp_value& value) -> bool
        { return std::holds_alternative<pp_integer>(value); }

    /**
     * @brief   Determines if the given `pp_value` is of type `pp_number`, 
     *          which holds a fixed-point number value.
     * 
     * @param   value   The `pp_value` to check.
     * 
     * @return  If `value` is of type `pp_number`, returns `true`; 
     *          Otherwise, returns `false`.
     */
    inline constexpr auto is_number (const pp_value& value) -> bool
        { return std::holds_alternative<pp_number>(value); }

    /**
     * @brief   Determines if the given `pp_value` is of a numeric type,
     *          which includes both `pp_integer` and `pp_number`.
     * 
     * @param   value   The `pp_value` to check.
     * 
     * @return  If `value` is of type `pp_integer` or `pp_number`, returns `true`; 
     *          Otherwise, returns `false`.
     */
    inline constexpr auto is_numeric (const pp_value& value) -> bool
        { return is_integer(value) || is_number(value); }

    /**
     * @brief   Determines if the given `pp_value` is of type `pp_boolean`, 
     *          which holds a boolean value.
     * 
     * @param   value   The `pp_value` to check.
     * 
     * @return  If `value` is of type `pp_boolean`, returns `true`; 
     *          Otherwise, returns `false`.
     */
    inline constexpr auto is_boolean (const pp_value& value) -> bool
        { return std::holds_alternative<pp_boolean>(value); }

    /**
     * @brief   Determines if the given `pp_value` is of type `pp_string`, 
     *          which holds a string value.
     * 
     * @param   value   The `pp_value` to check.
     * 
     * @return  If `value` is of type `pp_string`, returns `true`; 
     *          Otherwise, returns `false`.
     */
    inline constexpr auto is_string (const pp_value& value) -> bool
        { return std::holds_alternative<pp_string>(value); }

    /**
     * @brief   Retrieves a string representation of the type of the given
     *          `pp_value`.
     *
     * @param   value   The `pp_value` whose type is to be determined.
     *
     * @return  A string view indicating the type of the `pp_value`.
     */
    inline constexpr auto type_of (const pp_value& value) -> std::string_view
    {
        if (is_void(value))
            { return "void"; }
        else if (is_integer(value))
            { return "integer"; }
        else if (is_number(value))
            { return "fixed-point"; }
        else if (is_boolean(value))
            { return "boolean"; }
        else if (is_string(value))
            { return "string"; }
        else
            { return "unknown"; }
    }

    /**
     * @brief   Attempts to extract an integer value from the given
     *          `pp_value`.
     * 
     * If the `pp_value` is of type `pp_integer`, the integer value is returned
     * directly. If the `pp_value` is of type `pp_number`, the signed integer
     * portion of the fixed-point number is extracted and returned. Otherwise,
     * an empty optional is returned.
     * 
     * @param   value   The `pp_value` from which to extract the integer value.
     * 
     * @return  An optional containing the extracted integer value if successful;
     *          otherwise, an empty optional.
     */
    inline constexpr auto as_integer (const pp_value& value) 
        -> std::optional<pp_integer>
    {
        if (auto* int_ptr = std::get_if<pp_integer>(&value))
            { return *int_ptr; }
        else if (auto* fp_ptr = std::get_if<pp_number>(&value))
            { return fp_ptr->get_signed_integer(); }
        else
            { return std::nullopt; }
    }

    /**
     * @brief   Attempts to extract a fixed-point number value from the given
     *          `pp_value`.
     * 
     * If the `pp_value` is of type `pp_number`, the fixed-point number value
     * is returned. Otherwise, an empty optional is returned.
     * 
     * @param   value   The `pp_value` from which to extract the fixed-point
     *                  number value.
     * 
     * @return  An optional containing the extracted fixed-point number value
     *          if successful; otherwise, an empty optional.
     */
    inline constexpr auto as_number (const pp_value& value) 
        -> std::optional<pp_number>
    {
        if (auto* num_ptr = std::get_if<pp_number>(&value))
            { return *num_ptr; }
        else
            { return std::nullopt; }
    }

    /**
     * @brief   Attempts to extract a boolean value from the given
     *          `pp_value`.
     * 
     * If the `pp_value` is of type `pp_boolean`, the boolean value is returned
     * directly. Otherwise, an empty optional is returned.
     * 
     * @param   value   The `pp_value` from which to extract the boolean value.
     * 
     * @return  An optional containing the extracted boolean value if successful;
     *          otherwise, an empty optional.
     * 
     * @note    This function does not perform any type coercion or truthiness
     *          evaluation. It only extracts the value if it is explicitly of
     *          type `pp_boolean`.
     */
    inline constexpr auto as_boolean (const pp_value& value) 
        -> std::optional<pp_boolean>
    {
        if (auto* bool_ptr = std::get_if<pp_boolean>(&value))
            { return *bool_ptr; }
        else
            { return std::nullopt; }
    }

    /**
     * @brief   Attempts to extract a string value from the given
     *          `pp_value`.
     * 
     * If the `pp_value` is of type `pp_string`, the string value is returned
     * directly. Otherwise, an empty optional is returned.
     * 
     * @param   value   The `pp_value` from which to extract the string value.
     * 
     * @return  An optional containing the extracted string value if successful;
     *          otherwise, an empty optional.
     * 
     * @note    This function does not perform any stringification or conversion.
     *          It only extracts the value if it is explicitly of type `pp_string`.
     */
    inline constexpr auto as_string (const pp_value& value) 
        -> std::optional<pp_string>
    {
        if (auto* str_ptr = std::get_if<pp_string>(&value))
            { return *str_ptr; }
        else
            { return std::nullopt; }
    }

    /**
     * @brief   Evaluates the "truthiness" of the given `pp_value` according
     *          to the rules of the G10 assembler's preprocessing language.
     * 
     * The truthiness evaluation follows these rules:
     * 
     * - `pp_void`: Always `false`.
     * 
     * - `pp_integer`: `false` if the integer is `0`; `true` otherwise.
     * 
     * - `pp_number`: `false` if the raw, 64-bit representation is `0`; 
     *   `true` otherwise.
     * 
     * - `pp_boolean`: The boolean value itself.
     * 
     * - `pp_string`: `false` if the string is empty; `true` otherwise.
     * 
     * @param   value   The `pp_value` to evaluate for truthiness.
     * 
     * @return  `true` if the `pp_value` is considered truthy; `false` otherwise.
     */
    inline constexpr auto is_truthy (const pp_value& value) -> bool
    {
        if (is_void(value))
            { return false; }
        else if (auto *int_ptr = std::get_if<pp_integer>(&value))
            { return *int_ptr != 0; }
        else if (auto *num_ptr = std::get_if<pp_number>(&value))
            { return num_ptr->get_raw() != 0; }
        else if (auto *bool_ptr = std::get_if<pp_boolean>(&value))
            { return *bool_ptr; }
        else if (auto *str_ptr = std::get_if<pp_string>(&value))
            { return !str_ptr->empty(); }
        else
            { return false; }
    }

    /**
     * @brief   Converts the given `pp_value` to its string representation.
     * 
     * @param   value   The `pp_value` to convert to a string.
     * 
     * @return  A string representation of the `pp_value`.
     */
    inline constexpr auto to_string (const pp_value& value) -> std::string
    {
        if (is_void(value))
            { return "void"; }
        else if (auto *int_ptr = std::get_if<pp_integer>(&value))
            { return std::to_string(*int_ptr); }
        else if (auto *num_ptr = std::get_if<pp_number>(&value))
            { return std::to_string(num_ptr->get_calculated_float()); }
        else if (auto *bool_ptr = std::get_if<pp_boolean>(&value))
            { return *bool_ptr ? "true" : "false"; }
        else if (auto *str_ptr = std::get_if<pp_string>(&value))
            { return *str_ptr; }
        else
            { return "unknown"; }
    }
}
