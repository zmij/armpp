#pragma once

#include <armpp/hal/common_types.hpp>
#include <armpp/hal/handle_base.hpp>
#include <armpp/hal/registers.hpp>

namespace armpp::hal::scb {

union cpu_id_base_register {
    raw_read_only_register_field<0, 4>  revision;
    raw_read_only_register_field<4, 12> partno;
    raw_read_only_register_field<16, 4> constant;
    raw_read_only_register_field<20, 4> variant;
    raw_read_only_register_field<24, 8> implementer;

    raw_register volatile raw;
};
static_assert(sizeof(cpu_id_base_register) == sizeof(raw_register));

/**
 * @brief Interrupt Control State Register
 *
 * Use the Interrupt Control State Register to:
 *
 * - set a pending Non-Maskable Interrupt (NMI)
 * - set or clear a pending SVC
 * - set or clear a pending SysTick
 * - check for pending exceptions
 * - check the vector number of the highest priority pended exception
 * - check the vector number of the active exception.
 */
union interrupt_control_state_register {
    /**
     * Active ISR number field. VECTACTIVE contains the interrupt number of the currently running
     * ISR, including NMI and Hard Fault. A shared handler can use VECTACTIVE to determine which
     * interrupt invoked it. You can subtract 16 from the VECTACTIVE field to index into the
     * Interrupt Clear/Set Enable, Interrupt Clear Pending/SetPending and Interrupt Priority
     * Registers. INTISR[0] has vector number 16.
     *
     * Reset clears the VECTACTIVE field.
     */
    raw_read_only_register_field<0, 9> vectactive;
    /**
     * This bit is 1 when the set of all active exceptions minus the IPSR_current_exception yields
     * the empty set.
     */
    bit_read_only_register_field<11> rettobase;
    /**
     * Pending ISR number field. VECTPENDING contains the interrupt number of the highest priority
     * pending ISR.
     */
    raw_read_only_register_field<12, 9> vectpending;
    /**
     * Interrupt pending flag. Excludes NMI and Faults:
     *
     * 1 = interrupt pending
     * 0 = interrupt not pending.
     */
    bool_read_only_register_field<22> isrpending;
    /**
     * You must only use this at debug time. It indicates that a pending interrupt becomes active in
     * the next running cycle. If C_MASKINTS is clear in the Debug Halting Control and Status
     * Register, the interrupt is serviced.
     */
    bool_read_only_register_field<23> isrpreemt;
    /**
     * Clear pending SysTick bit:
     *
     * 1 = clear pending SysTick
     * 0 = do not clear pending SysTick.
     */
    write_only_register_field<clear_t, 25, 1> pendstclr;
    /**
     * Set a pending SysTick bit
     *
     * 1 = set pending SysTick
     * 0 = do not set pending SysTick.
     */
    read_write_register_field<set_t, 26, 1> pendstset;
    /**
     * Clear pending pendSV bit:
     *
     * 1 = clear pending pendSV
     * 0 = do not clear pending pendSV.
     */
    write_only_register_field<clear_t, 25, 1> pendsvclr;
    /**
     * Set a pending pendSV bit
     *
     * 1 = set pending pendSV
     * 0 = do not set pending pendSV.
     */
    read_write_register_field<set_t, 26, 1> pendsvset;
    /**
     * Set pending NMI bit:
     *
     * 1 = set pending NMI
     * 0 = do not set pending NMI.
     *
     * NMIPENDSET pends and activates an NMI. Because NMI is the highest-priority interrupt, it
     * takes effect as soon as it registers.
     */
    read_write_register_field<set_t, 26, 1> nmipendset;
};
static_assert(sizeof(interrupt_control_state_register) == sizeof(raw_register));

enum class vector_table_location_t { code = 0, ram = 1 };

/**
 * @brief Vector Table Offset Register
 *
 * Use the Vector Table Offset Register to determine:
 *
 * - if the vector table is in RAM or code memory
 * - the vector table offset.
 */
union vector_table_offset_register {
    raw_read_write_register_field<7, 22>                      tbloff;
    read_write_register_field<vector_table_location_t, 29, 1> tblbase;
};
static_assert(sizeof(vector_table_offset_register) == sizeof(raw_register));

enum class system_reset_t { no_effect = 0, reset = 1 };
enum class priority_split_t {
    split_7_1 = 0, /*<! 7.1 indicates 7 bits of pre-emption priority, 1 bit of subpriority */
    split_6_2 = 1, /*<! 6.2 indicates 6 bits of pre-emption priority, 2 bits of subpriority */
    split_5_3 = 2, /*<! 5.3 indicates 5 bits of pre-emption priority, 3 bits of subpriority */
    split_4_5 = 3, /*<! 5.3 indicates 4 bits of pre-emption priority, 4 bits of subpriority */
    split_3_5 = 4, /*<! 5.3 indicates 3 bits of pre-emption priority, 5 bits of subpriority */
    split_2_6 = 5, /*<! 5.3 indicates 2 bits of pre-emption priority, 6 bits of subpriority */
    split_1_7 = 6, /*<! 5.3 indicates 1 bits of pre-emption priority, 7 bits of subpriority */
    split_0_8 = 7  /*<! 0.8 indicates no pre-emption priority, 8 bits of subpriority.*/
};
enum class endiannes_t { little = 0, big = 1 };

/**
 * @brief Application Interrupt and Reset Control Register
 * Use the Application Interrupt and Reset Control Register to:
 *
 * - determine data endianness
 * - clear all active state information for debug or to recover from a hard failure
 * - execute a system reset
 * - alter the priority grouping position (binary point).
 *
 * Note
 * SYSRESETREQ is cleared by a system reset, which means that asserting VECTRESET at the same time
 * may cause SYSRESETREQ to be cleared in the same cycle as it is written to. This may prevent the
 * external system from seeing SYSRESETREQ. It is therefore recommended that VECTRESET and
 * SYSRESETREQ be used exclusively and never both written to 1 at the same time.
 */
union app_interrupt_and_reset_control_register {
    /**
     * System Reset bit. Resets the system, with the exception of debug components
     *
     * The VECTRESET bit self-clears. Reset clears the VECTRESET bit.
     *
     * For debugging, only write this bit when the core is halted.
     */
    read_write_register_field<system_reset_t, 0, 1> vectreset;
    /**
     * @brief Clear active vector bit:
     *
     * clear = clear all state information for active NMI, fault, and interrupts
     * no_effect = do not clear.
     *
     * It is the responsibility of the application to reinitialize the stack.
     *
     * The VECTCLRACTIVE bit is for returning to a known state during debug. The VECTCLRACTIVE bit
     * self-clears.
     *
     * IPSR is not cleared by this operation. So, if used by an application, it must only be used at
     * the base level of activation, or within a system handler whose active bit can be set.
     */
    read_write_register_field<clear_t, 1, 1> vectclractive;
    /**
     * @brief Causes a signal to be asserted to the outer system that indicates a reset is
     * requested. Intended to force a large system reset of all major components except for debug.
     * Setting this bit does not prevent Halting Debug from running.
     */
    read_write_register_field<system_reset_t, 2, 1> sysresetreq;
    /**
     * @brief Interrupt priority grouping field
     */
    read_write_register_field<priority_split_t, 8, 3> prigroup;
    /**
     * @brief Data endianness bit
     *
     * ENDIANESS is sampled from the BIGEND input port during reset. You cannot change ENDIANESS
     * outside of reset.
     */
    read_only_register_field<endiannes_t, 2, 1> edniannes;

    raw_read_only_register_field<16, 16>  vectkeystat;
    raw_read_write_register_field<16, 16> vectkey;
};
static_assert(sizeof(app_interrupt_and_reset_control_register) == sizeof(raw_register));

/**
 * @brief System Control Register
 *
 * Use the System Control Register for power-management functions:
 *
 * - signal to the system when the processor can enter a low power state
 * - control how the processor enters and exits low power states.
 */
union system_control_register {
    /**
     * Sleep on exit when returning from Handler mode to Thread mode:
     *
     * 1 = sleep on ISR exit.
     * 0 = do not sleep when returning to Thread mode.
     *
     * Enables interrupt driven applications to avoid returning to empty main application.
     */
    bit_read_write_register_field<1> sleeponexit;
    /**
     * Sleep deep bit:
     *
     * 1 = indicates to the system that Cortex-M3 clock can be stopped. Setting this bit causes the
     * SLEEPDEEP port to be asserted when the processor can be stopped.
     * 0 = not OK to turn off system clock.
     */
    bit_read_write_register_field<2> sleepdeep;
    /**
     * When enabled, this causes WFE to wake up when an interrupt moves from inactive to pended.
     * Otherwise, WFE only wakes up from an event signal, external and SEV instruction generated.
     * The event input, RXEV, is registered even when not waiting for an event, and so effects the
     * next WFE.
     */
    read_write_register_field<enabled_t, 3, 1> sevonpend;
};
static_assert(sizeof(system_control_register) == sizeof(raw_register));

/**
 * @brief Configuration Control Register
 *
 * Use the Configuration Control Register to:
 *
 * - enable NMI, Hard Fault and FAULTMASK to ignore bus fault
 * - trap divide by zero, and unaligned accesses
 * - enable user access to the Software Trigger Exception Register
 * - control entry to Thread Mode.
 */
union configuration_control_register {
    /**
     * When 0, default, It is only possible to enter Thread mode when returning from the last
     * exception. When set to 1, Thread mode can be entered from any level in Handler mode by
     * controlled return value (EXC_RETURN).
     */
    read_write_register_field<enabled_t, 0, 1> nonebasethrdena;
    /**
     * If written as 1, enables user code to write the Software Trigger Interrupt register to
     * trigger (pend) a Main exception, which is one associated with the Main stack pointer.
     */
    read_write_register_field<enabled_t, 1, 1> usersetmpend;
    /**
     * Trap for unaligned access. This enables faulting/halting on any unaligned half or full
     * word access. Unaligned load-store multiples always fault. The relevant Usage Fault Status
     * Register bit is UNALIGNED, see Usage Fault Status Register.
     */
    read_write_register_field<enabled_t, 2, 1> unalign_trp;
    /**
     * Trap on Divide by 0. This enables faulting/halting when an attempt is made to divide by 0.
     * The relevant Usage Fault Status Register bit is DIVBYZERO, see Usage Fault Status Register.
     */
    read_write_register_field<enabled_t, 3, 1> div_0_trp;
    /**
     * When enabled, this causes handlers running at priority -1 and -2 (Hard Fault, NMI, and
     * FAULTMASK escalated handlers) to ignore Data Bus faults caused by load and store
     * instructions. When disabled, these bus faults cause a lock-up. You must only use this enable
     * with extreme caution. All data bus faults are ignored - you must only use it when the handler
     * and its data are in absolutely safe memory. Its normal use is to probe system devices and
     * bridges to detect control path problems and fix them.
     */
    read_write_register_field<enabled_t, 8, 1> bfhfnmign;
    /**
     * 1 = on exception entry, the SP used prior to the exception is adjusted to be 8-byte aligned
     * and the context to restore it is saved. The SP is restored on the associated exception
     * return.
     *
     * 0 = only 4-byte alignment is guaranteed for the SP used prior to the exception on exception
     * entry.
     */
    bit_read_only_register_field<9> stkalign;
};

static_assert(sizeof(configuration_control_register) == sizeof(raw_register));

enum class system_handler_index_t {
    mem_manage_fault = 0,
    bus_fault        = 1,
    usage_fault      = 2,
    sv_call          = 7,
    debug_monitor    = 8,
    pend_sv          = 10,
    sys_tick         = 11,
};
using system_handler_priority_register = read_write_register_field_array<std::uint32_t, 8, 12, 3>;
static_assert(sizeof(system_handler_priority_register) == sizeof(raw_register) * 3);

/**
 * @brief System Handler Control and State Register
 *
 * Use the System Handler Control and State Register to:
 *
 * - enable or disable the system handlers
 * - determine the pending status of bus fault, mem manage fault, and SVC
 * - determine the active status of the system handlers.
 *
 * If a fault condition occurs while its fault handler is disabled, the fault escalates to a Hard
 * Fault.
 *
 * The active bits indicate if any of the system handlers are active, running now, or stacked
 * because of pre-emption. This information is used for debugging and is also used by the
 * application handlers. The pend bits are only set when a fault that cannot be retried has been
 * deferred because of late arrival of a higher priority interrupt.
 *
 * Caution
 *
 * You can write, clear, or set the active bits, but you must only do this with extreme caution.
 * Clearing and setting these bits does not repair stack contents nor clean up other data
 * structures. It is intended that context switchers use clearing and setting to save a thread's
 * context, even when in a fault handler. The most common case is to save the context of a thread
 * that is in an SVCall handler or UsageFault handler, for undefined instruction and coprocessor
 * emulation.
 *
 * The model for doing this is to save the current state, switch out the stack containing the
 * handler's context, load the state of the new thread, switch in the new thread's stacks, and then
 * return to the thread. The active bit of the current handler must never be cleared, because the
 * IPSR is not changed to reflect this. Only use it to change stacked active handlers.
 *
 * As indicated, the SVCALLPENDED and BUSFAULTPENDED bits are set when the corresponding handler is
 * held off by a late arriving interrupt. These bits are not cleared until the underlying handler is
 * actually invoked. That is, if a stack error or vector read error occurs before the SVCall or
 * BusFault handler is started, the bits are not cleared. This enables the push-error or
 * vector-read-error handler to choose to clear them or retry.
 */
union system_handler_control_and_state_register {
    read_write_register_field<active_t, 0, 1>   memfaultact;
    read_write_register_field<active_t, 1, 1>   busfaultact;
    read_write_register_field<active_t, 2, 1>   usgfaultact;
    read_write_register_field<active_t, 7, 1>   svcallact;
    read_write_register_field<active_t, 10, 1>  pednsvact;
    read_write_register_field<active_t, 11, 1>  sestickact;
    read_write_register_field<pended_t, 12, 1>  usgfaultpended;
    read_write_register_field<pended_t, 13, 1>  memfaultpended;
    read_write_register_field<pended_t, 14, 1>  busfaultpended;
    read_write_register_field<pended_t, 15, 1>  svcallpended;
    read_write_register_field<enabled_t, 16, 1> memfaultena;
    read_write_register_field<enabled_t, 17, 1> busfaultena;
    read_write_register_field<enabled_t, 18, 1> usgfaultena;
};
static_assert(sizeof(system_handler_control_and_state_register) == sizeof(raw_register));

/**
 * @brief Configurable Fault Status Registers
 *
 * Use the three Configurable Fault Status Registers to obtain information about local faults. These
 * registers include:
 *
 * - Memory Manage Fault Status Register
 * - Bus Fault Status Register
 * - Usage Fault Status Register.
 *
 * The flags in these registers indicate the causes of local faults. Multiple flags can be set if
 * more than one fault occurs. These register are read/write-clear. This means that they can be read
 * normally, but writing a 1 to any bit clears that bit.
 *
 * @todo Add separate fields for distinct registers
 * @todo separate fields for reading and resetting
 */
union configurable_fault_status_register {
    //@{
    /** @name Memory Manage Fault Status Register */
    /**
     * Instruction access violation flag. Attempting to fetch an instruction from a location that
     * does not permit execution sets the IACCVIOL flag. This occurs on any access to an XN region,
     * even when the MPU is disabled or not present. The return PC points to the faulting
     * instruction. The MMAR is not written.
     */
    bit_read_write_register_field<0> iaccviol;
    /**
     * Data access violation flag. Attempting to load or store at a location that does not permit
     * the operation sets the DACCVIOL flag. The return PC points to the faulting instruction. This
     * error loads MMAR with the address of the attempted access.
     */
    bit_read_write_register_field<1> daccviol;
    /**
     * Unstack from exception return has caused one or more access violations. This is chained to
     * the handler, so that the original return stack is still present. SP is not adjusted from
     * failing return and new save is not performed. The MMAR is not written.
     */
    bit_read_write_register_field<3> munstkerr;
    /**
     * Stacking from exception has caused one or more access violations. The SP is still adjusted
     * and the values in the context area on the stack might be incorrect. The MMAR is not written.
     */
    bit_read_write_register_field<4> mstkerr;
    /**
     * Memory Manage Address Register (MMAR) address valid flag:
     *
     * 1 = valid fault address in MMAR. A later-arriving fault, such as a bus fault, can clear a
     * memory manage fault.
     * 0 = no valid fault address in MMAR.
     *
     * If a MemManage fault occurs that is escalated to a Hard Fault because of priority, the Hard
     * Fault handler must clear this bit. This prevents problems on return to a stacked active
     * MemManage handler whose MMAR value has been overwritten.
     */
    bit_read_write_register_field<7> mmarvalid;
    //@}
    //@{
    /** @name Bus Fault Status Register */
    /**
     * Instruction bus error flag:
     *
     * 1 = instruction bus error
     * 0 = no instruction bus error.
     *
     * The IBUSERR flag is set by a prefetch error. The fault stops on the instruction, so if the
     * error occurs under a branch shadow, no fault occurs. The BFAR is not written.
     */
    bit_read_write_register_field<8> ibuserr;
    /**
     * Precise data bus error return.
     */
    bit_read_write_register_field<9> precierr;
    /**
     * Imprecise data bus error. It is a BusFault, but the Return PC is not related to the causing
     * instruction. This is not a synchronous fault. So, if detected when the priority of the
     * current activation is higher than the Bus Fault, it only pends. Bus fault activates when
     * returning to a lower priority activation. If a precise fault occurs before returning to a
     * lower priority exception, the handler detects both IMPRECISERR set and one of the precise
     * fault status bits set at the same time. The BFAR is not written.
     */
    bit_read_write_register_field<10> ipreciserr;
    /**
     * Unstack from exception return has caused one or more bus faults. This is chained to the
     * handler, so that the original return stack is still present. SP is not adjusted from failing
     * return and new save is not performed. The BFAR is not written.
     */
    bit_read_write_register_field<11> unstkerr;
    /**
     * Stacking from exception has caused one or more bus faults. The SP is still adjusted and the
     * values in the context area on the stack might be incorrect. The BFAR is not written.
     */
    bit_read_write_register_field<12> stkerr;
    /**
     * This bit is set if the Bus Fault Address Register (BFAR) contains a valid address. This is
     * true after a bus fault where the address is known. Other faults can clear this bit, such as a
     * Mem Manage fault occurring later.
     *
     * If a Bus fault occurs that is escalated to a Hard Fault because of priority, the Hard Fault
     * handler must clear this bit. This prevents problems if returning to a stacked active Bus
     * fault handler whose BFAR value has been overwritten.
     */
    bit_read_write_register_field<15> bfarvalid;
    //@}
    //@{
    /** @name Usage Fault Status Register */
    /**
     * The UNDEFINSTR flag is set when the processor attempts to execute an undefined instruction.
     * This is an instruction that the processor cannot decode. The return PC points to the
     * undefined instruction.
     */
    bit_read_write_register_field<16> undefinstr;
    /**
     * Invalid combination of EPSR and instruction, for reasons other than UNDEFINED instruction.
     * Return PC points to faulting instruction, with the invalid state.
     */
    bit_read_write_register_field<17> invstate;
    /**
     * Attempt to load EXC_RETURN into PC illegally. Invalid instruction, invalid context, invalid
     * value. The return PC points to the instruction that tried to set the PC.
     */
    bit_read_write_register_field<18> invpc;
    /**
     * Attempt to use a coprocessor instruction. The processor does not support coprocessor
     * instructions.
     */
    bit_read_write_register_field<19> nocp;
    /**
     * When UNALIGN_TRP is enabled (see Configuration Control Register), and there is an attempt to
     * make an unaligned memory access, then this fault occurs.Unaligned LDM/STM/LDRD/STRD
     * instructions always fault irrespective of the setting of UNALIGN_TRP.
     */
    bit_read_write_register_field<24> unaligned;
    /**
     * When DIV_0_TRP (see Configuration Control Register) is enabled and an SDIV or UDIV
     * instruction is used with a divisor of 0, this fault occurs The instruction is executed and
     * the return PC points to it. If DIV_0_TRP is not set, then the divide returns a quotient of 0.
     */
    bit_read_write_register_field<15> dibyzero;
    //@}
};
static_assert(sizeof(configurable_fault_status_register) == sizeof(raw_register));

/**
 * @brief Hard Fault Status Register
 *
 * Use the Hard Fault Status Register (HFSR) to obtain information about events that activate the
 * Hard Fault handler.
 */
union hard_fault_status_register {
    /**
     * This bit is set if there is a fault because of vector table read on exception processing (Bus
     * Fault). This case is always a Hard Fault. The return PC points to the pre-empted instruction.
     */
    bit_read_write_register_field<1> vecttbl;
    /**
     * Hard Fault activated because a Configurable Fault was received and cannot activate because of
     * priority or because the Configurable Fault is disabled.The Hard Fault handler then has to
     * read the other fault status registers to determine cause.
     */
    bit_read_write_register_field<30> forced;
    /**
     * This bit is set if there is a fault related to debug.
     *
     * This is only possible when halting debug is not enabled. For monitor enabled debug, it only
     * happens for BKPT when the current priority is higher than the monitor. When both halting and
     * monitor debug are disabled, it only happens for debug events that are not ignored (minimally,
     * BKPT). The Debug Fault Status Register is updated.
     */
    bit_read_write_register_field<31> debugevt;
};

/**
 * @brief Debug Fault Status Register
 *
 * Use the Debug Fault Status Register to monitor:
 *
 * - external debug requests
 * - vector catches
 * - data watchpoint match
 * - BKPT instruction execution
 * - halt requests.
 *
 * Multiple flags in the Debug Fault Status Register can be set when multiple fault conditions
 * occur. The register is read/write clear. This means that it can be read normally. Writing a 1 to
 * a bit clears that bit.
 *
 * Note
 * These bits are not set unless the event is caught. This means that it causes a stop of some sort.
 * If halting debug is enabled, these events stop the processor into debug. If debug is disabled and
 * the debug monitor is enabled, then this becomes a debug monitor handler call, if priority
 * permits. If debug and the monitor are both disabled, some of these events are Hard Faults, and
 * the DBGEVT bit is set in the Hard Fault status register, and some are ignored.
 *
 */
union debug_fault_status_register {
    /**
     * Halt request flag:
     *
     * 1 = halt requested by NVIC, including step. The processor is halted on the next instruction.
     * 0 = no halt request.
     */
    bit_read_write_register_field<0> halted;
    /**
     * BKPT flag:
     *
     * 1 = BKPT instruction execution
     * 0 = no BKPT instruction execution.
     *
     * The BKPT flag is set by a BKPT instruction in flash patch code, and also by normal code.
     * Return PC points to breakpoint containing instruction.
     */
    bit_read_write_register_field<1> bkpt;
    /**
     * Data Watchpoint and Trace (DWT) flag:
     *
     * 1 = DWT match
     * 0 = no DWT match.
     *
     * The processor stops at the current instruction or at the next instruction.
     */
    bit_read_write_register_field<2> dwttrap;
    /**
     * Vector catch flag:
     *
     * 1 = vector catch occurred
     * 0 = no vector catch occurred.
     *
     * When the VCATCH flag is set, a flag in one of the local fault status registers is also set to
     * indicate the type of fault.
     */
    bit_read_write_register_field<3> vcatch;
    /**
     * External debug request flag:
     *
     * 1 = EDBGRQ signal asserted
     * 0 = EDBGRQ signal not asserted.
     *
     * The processor stops on next instruction boundary.
     */
    bit_read_write_register_field<4> external;
};
static_assert(sizeof(debug_fault_status_register) == sizeof(raw_register));

/**
 * @brief Memory Manage Fault Address Register
 *
 * Use the Memory Manage Fault Address Register to read the address of the location that caused a
 * Memory Manage Fault.
 */
using memmanage_fault_address_register = raw_read_write_register_field<0, 32>;
/**
 * @brief Bus Fault Address Register
 *
 * Use the Bus Fault Address Register to read the address of the location that generated a Bus
 * Fault.
 */
using bus_fault_address_register = raw_read_write_register_field<0, 32>;

/**
 * @brief Auxiliary Fault Status Register
 *
 * Use the Auxiliary Fault Status Register (AFSR) to determine additional system fault information
 * to software.
 *
 * The AFSR flags map directly onto the AUXFAULT inputs of the processor, and a single-cycle high
 * level on an external pin causes the corresponding AFSR bit to become latched as one. The bit can
 * only be cleared by writing a one to the corresponding AFSR bit.
 *
 * When an AFSR bit is written or latched as one, an exception does not occur. If you require an
 * exception, you must use an interrupt.
 *
 */
using auxilary_fault_address_register = read_write_register_field_array<raw_register, 1, 32, 1>;
static_assert(sizeof(auxilary_fault_address_register) == sizeof(raw_register));

class scb {
public:
    static constexpr address base_address = 0xe000ed00;
    static constexpr address end_address  = 0xe000ed3c + sizeof(auxilary_fault_address_register);

    cpu_id
    get_cpu_id() const
    {
        return {.raw = cpuid_.raw};
    }

private:
    cpu_id_base_register                      cpuid_;    // 0xe000ed00
    interrupt_control_state_register          icsr_;     // 0xe000ed04
    vector_table_offset_register              voff_;     // 0xe000ed08
    app_interrupt_and_reset_control_register  aircr_;    // 0xe000ed0c
    system_control_register                   scr_;      // 0xe000ed10
    configuration_control_register            ccr_;      // 0xe000ed14
    system_handler_priority_register          shp_;      // 0xe000ed18, 0xe000ed1c , 0xe000ed20
    system_handler_control_and_state_register shcsr_;    // 0xe000ed24
    configurable_fault_status_register        cfsr_;     // 0xe000ed28
    hard_fault_status_register                hfsr_;     // 0xe000ed2c
    debug_fault_status_register               dfsr_;     // 0xe000ed30
    memmanage_fault_address_register          mmfar_;    // 0xe000ed34
    bus_fault_address_register                bfar_;     // 0xe000ed38
    auxilary_fault_address_register           afsr_;     // 0xe000ed3c
};

static_assert(sizeof(scb) == scb::end_address - scb::base_address);

class scb_handle : public handle_base<scb> {
public:
    using base_type = handle_base<scb>;

    scb_handle() : base_type{scb::base_address} {}
};

}    // namespace armpp::hal::scb
