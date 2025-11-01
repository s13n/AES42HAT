/** @file
 * newlib-specific definitions, mainly for startup/exit handling
 * @addtogroup arch
 * @{
 */
#pragma once

/** Application entry point.
 * Must be called by the startup routine after initialization.
 */
extern int main(void);

/** Exit function.
 * Must be called by the startup routine after main() returns. Can also be called
 * by the application to terminate the program.
 */
extern void _exit(int) __attribute__((noreturn));

/** Entry point for C++ library startup.
 * This routine is provided by newlib. It must be called by the startup routine
 * before calling main(), to run the constructors of global variables.
 */
extern void __libc_init_array(void);

/** User initialization routine.
 * Newlib's __libc_init_array() function calls this routine after having gone through
 * the preinit array, and before it goes through the init array. This routine must be
 * provided by the user. It can be used to initialize more hardware that wasn't handled
 * by sysinit().
 */
extern void _init(void);

/** Entry point for C++ library teardown.
 * This routine is provided by newlib. It should be called by the _exit routine, to
 * run the destructors of global variables. In an embedded system, it may not be
 * desirable to destroy global variables, if the system just gets switched off. Not
 * calling this function may save some code associated with global object deletion.
 */
extern void __libc_fini_array(void);

/** User teardown routine.
 * Newlib's __libc_fini_array() function calls this routine after having gone through
 * the fini array. This routine must be provided by the user, unless __libc_fini_array()
 * is never called. It can be used to bring down hardware before shutting down the system.
 */
extern void _fini(void);

/* The following symbols must be provided by the linker script.
 * They define three lists of function pointers that must be called on initialization
 * or teardown. Compiler and linker collaborate to assemble those lists, which are
 * made to point to constructors/destructors of global objects. Two functions in
 * newlib, __libc_init_array() and __libc_fini_array(), scan through the lists and
 * call each function in turn.
 *
 * GCC has function attributes that may be used to cause inclusion of a function in one
 * of those lists, i.e. __attribute__((constructor)) and __attribute__((destructor)).
 * The C++ compiler uses them automatically on constructors/destructors of global
 * object instances.
 *
 * The arrays are constant and may be placed in ROM or Flash.
 */
extern void (*__preinit_array_start []) (void);
extern void (*__preinit_array_end []) (void);
extern void (*__init_array_start []) (void);
extern void (*__init_array_end []) (void);
extern void (*__fini_array_start []) (void);
extern void (*__fini_array_end []) (void);

/** Pointer to the stack base.
 * This symbol needs to be provided by the linker script.
 *
 * In systems with a stack that grows downward, the stack base is the highest address
 * used by the stack.
 *
 * We treat this as a function pointer, so that it is compatible with the vector
 * table for exceptions.
 */
extern void __stack(void);

/** Pointer to the stack limit.
 * This symbol needs to be provided by the linker script.
 *
 * In systems with a stack that grows downward, the stack limit is the lowest address
 * that the stack may use. A stack growing beyond this point is likely to lead to a crash.
 *
 * We treat this as a function pointer, so that it is compatible with __stack
 */
extern void __stacklimit(void);


/* The following symbols permit access to the location and size of segments.
 * Use them by taking their address. Don't read or write from/to the variables
 * themselves.
 */
/** Start of .text segment.
 * The linker script must provide this symbol.
 */
extern char stext;

/** End of .text segment.
 * The linker script must provide this symbol.
 */
extern char etext;

/** Start of .data segment.
 * The linker script must provide this symbol.
 */
extern char _data;

/** End of .data segment.
 * The linker script must provide this symbol.
 */
extern char _edata;

/** Start of .bss segment.
 * The linker script must provide this symbol.
 */
extern char _bss;

/** End of .bss segment.
 * The linker script must provide this symbol.
 */
extern char _ebss;

/** Tables for data and bss sections, in case of multisegmented RAM.
 * TODO: implement support for this.
 */
extern unsigned __data_section_table;       //!< start of data section table
extern unsigned __data_section_table_end;   //!< end of data section table
extern unsigned __bss_section_table;        //!< start of bss section table
extern unsigned __bss_section_table_end;    //!< end of bss section table

/** Function to initialize global variables in .data and .bss segments.
 * Call this in startup code after calling sysinit() and before calling __libc_init_array().
 */
__attribute__((always_inline)) inline void startup_meminit(void) {
    // Initialize the .data segment from .text
    for(unsigned *dp = &__data_section_table; dp < &__data_section_table_end; ) {
        unsigned const *src = (unsigned const *)(*dp++);
        unsigned *dest = (unsigned *)(*dp++);
        unsigned len = *dp++;
        for(unsigned i = 0; i < len; i += 4)
            *dest++ = *src++;
    }
    // Zero fill the .bss segment
    for(unsigned *bp = &__bss_section_table; bp < &__bss_section_table_end; ) {
        unsigned *dest = (unsigned *)(*bp++);
        unsigned len = *bp++;
        for(unsigned i = 0; i < len; i += 4)
            *dest++ = 0;
    }
}

/** Reset entry point.
 * This routine must be defined by the user. It is called directly by hardware when the
 * CPU leaves reset. It is responsible for initializing the processor and the hardware,
 * before calling __libc_init_array() to initialize the library, and main() to start the
 * application.
 *
 * We put it in a section .after_vectors, so that the linker script can control its
 * placement in code memory. This is helpful in cases where right after reset, only a
 * small amount of memory is accessible, until pins and memory interface have been
 * initialized. For example, when booting from external Flash, some microcontrollers
 * configure only a limited number of address lines, corresponding to only a few kBytes
 * of addressable memory. The reset entry point, and the early startup code, must be
 * placed in this small memory area. Only after the startup code has initialized more
 * address pins, which allows the full extent of external Flash memory to be accessed,
 * it will be possible to call code outside of this section. The user must make the
 * necessary arrangements in the linker script to ensure the section gets placed
 * correctly. Typically, the right place is immediately after the exception vector
 * table, but this ultimately depends on the hardware design.
 */
extern __attribute__ ((noreturn, naked, section(".after_vectors"))) void ResetISR(void);

/** System initialization.
 * This function is called immediately at the beginning of the startup routine.
 * Memory and global variables are not yet initialized at this time, but a stack
 * is available. The user must provide this function. Its purpose is to bring up
 * the hardware to a point where all relevant memory is available, so that global
 * variables can be initialized.
 *
 * The user may choose to initialize other hardware at this point, too. For example,
 * it is customary to also initialize clocking and pin configuration to suit the
 * board's wiring.
 *
 * See the description of ResetISR for the rationale behind section placement.
 */
extern __attribute__ ((section(".after_vectors"))) int sysinit(void);

//!@}
