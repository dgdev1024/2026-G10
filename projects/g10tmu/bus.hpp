/**
 * @file    g10tmu/bus.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-27
 * 
 * @brief   Contains declarations for the G10 Testbed Emulator's system bus.
 */

/* Public Includes ************************************************************/

#include <g10/cpu.hpp>
#include <g10/bus.hpp>
#include <g10/program.hpp>
#include <g10tmu/timer.hpp>

/* Public Classes *************************************************************/

namespace g10tmu
{
    class bus final : public g10::bus
    {
    public:
        /**
         * @brief   Constructs a new G10 Testbed Emulator system bus,
         *          loading the specified program file into memory.
         * 
         * @param   program_path    The path to the G10 program file to load
         *                          into memory.
         * @param   ram_size        The size of the system RAM to allocate, in
         *                          bytes. Defaults to `0x10` (16 bytes).
         */
        explicit bus (const fs::path& program_path,
            const std::size_t ram_size = 0x10);

        /**
         * @brief   The G10 Testbed Emulator system bus's destructor.
         */
        ~bus () override;

        /**
         * @brief   Resets the CPU's connected system bus, setting all buffers,
         *          registers and internal states of all connected devices to
         *          their default, power-on values.
         * 
         * This method is called internally by the G10 CPU's @a `reset` method;
         * it should not be called directly.
         */
        auto reset () -> void override;

        /**
         * @brief   Ticks the CPU's connected system bus, advancing the internal
         *          clocks and states of all connected devices by one T-cycle.
         * 
         * This method is called internally by the G10 CPU's @a `consume_tick_cycles`
         * method; it should not be called directly.
         * 
         * @return  If all connected devices ticked without errors, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto tick () -> bool override;

        /**
         * @brief   Reads one byte of data from the specified address on the
         *          CPU's connected system bus.
         * 
         * @param   address     The absolute address of the byte to read.
         * 
         * @return  If the read was successful, returns the byte read from the
         *          specified address; Otherwise, returns `0xFF`.
         */
        auto read (std::uint32_t address) -> std::uint8_t override;

        /**
         * @brief   Writes one byte of data to the specified address on the
         *          CPU's connected system bus.
         * 
         * @param   address     The absolute address to which to write the
         *                      specified byte.
         * @param   value       The byte to write to the specified address.
         * 
         * @return  If the write was successful, returns the byte which was
         *          actually written to the specified address, which may differ
         *          from the requested value due to bus behavior; Otherwise,
         *          returns `0xFF`.
         */
        auto write (std::uint32_t address, std::uint8_t value)
            -> std::uint8_t override;

        /**
         * @brief   Starts the G10 Testbed Emulator, running the loaded program.
         * 
         * @return  The emulator's exit code.
         */
        auto start () -> std::int32_t;

        /**
         * @brief   Gets a reference to the system RAM.
         * 
         * @return  A reference to the system RAM vector.
         */
        inline auto get_ram () -> std::vector<std::uint8_t>&
            { return m_ram; }
        inline auto get_ram () const -> const std::vector<std::uint8_t>&
            { return m_ram; }

        /**
         * @brief   Gets a reference to the loaded G10 program file.
         * 
         * @return  A reference to the loaded G10 program file.
         */
        inline auto get_program () -> g10::program&
            { return m_program; }
        inline auto get_program () const -> const g10::program&
            { return m_program; }

        /**
         * @brief   Gets a reference to the G10 CPU connected to this system bus.
         * 
         * @return  A reference to the G10 CPU connected to this system bus.
         */
        inline auto get_cpu () -> g10::cpu&
            { return m_cpu; }
        inline auto get_cpu () const -> const g10::cpu&
            { return m_cpu; }

        /**
         * @brief   Function call operator overload to get a reference to the
         *          G10 CPU connected to this system bus.
         * 
         * @return  A reference to the G10 CPU connected to this system bus.
         */
        inline auto operator() () -> g10::cpu&
            { return m_cpu; }
        inline auto operator() () const -> const g10::cpu&
            { return m_cpu; }

        /**
         * @brief   Subscript operator overload to read one byte of data from
         *          the specified address on the CPU's connected system bus.
         * 
         * @param   address     The absolute address of the byte to read.
         * 
         * @return  The byte read from the specified address.
         */
        inline auto operator[] (std::uint32_t address) -> std::uint8_t
            { return read(address); }

        /**
         * @brief   Gets a reference to the timer component connected to this
         *          system bus.
         * 
         * @return  A reference to the timer component.
         */
        inline auto get_timer () -> timer&
            { return m_timer; }
        inline auto get_timer () const -> const timer&
            { return m_timer; }

    private:
        std::vector<std::uint8_t> m_ram;
        g10::program m_program;
        g10::cpu m_cpu;
        timer m_timer;

    };
}