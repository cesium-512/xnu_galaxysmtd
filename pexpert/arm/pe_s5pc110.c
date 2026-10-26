/*
 * Initial Samsung S5PC110 platform support for Galaxy S bring-up.
 *
 * UART2 is expected to be initialized by the bootloader. Interrupt and timer
 * support must be implemented before the kernel can continue past early init.
 */

#if defined(BOARD_CONFIG_S5PC110)

#include <mach/mach_types.h>
#include <kern/debug.h>
#include <machine/machine_routines.h>
#include <pexpert/pexpert.h>
#include <pexpert/arm/protos.h>
#include <vm/pmap.h>
#include <arm/pmap.h>

#define S5PC110_UART_BASE	0xE2900000
#define S5PC110_UART2_OFFSET	0x800

#define UART_UTRSTAT		0x10
#define UART_UTXH		0x20
#define UART_URXH		0x24

#define UART_UTRSTAT_RX_READY	(1 << 0)
#define UART_UTRSTAT_TX_READY	(1 << 1)

static vm_offset_t s5pc110_uart_base;

static volatile uint32_t *s5pc110_uart_reg(uint32_t offset)
{
	return (volatile uint32_t *)(s5pc110_uart_base +
				     S5PC110_UART2_OFFSET + offset);
}

static void s5pc110_uart_init(void)
{
	if (!s5pc110_uart_base) {
		s5pc110_uart_base = ml_io_map(S5PC110_UART_BASE, PAGE_SIZE);
		if (!s5pc110_uart_base)
			panic("S5PC110: unable to map UART2");
	}
}

static void s5pc110_uart_putc(char c)
{
	if (!s5pc110_uart_base)
		panic("S5PC110: UART2 used before initialization");

	if (c == '\n')
		s5pc110_uart_putc('\r');

	while (!(*s5pc110_uart_reg(UART_UTRSTAT) & UART_UTRSTAT_TX_READY))
		__asm__ volatile ("" ::: "memory");

	*(volatile uint8_t *)(s5pc110_uart_base + S5PC110_UART2_OFFSET +
			      UART_UTXH) = (uint8_t)c;
}

static int s5pc110_uart_getc(void)
{
	if (!s5pc110_uart_base)
		panic("S5PC110: UART2 used before initialization");

	if (!(*s5pc110_uart_reg(UART_UTRSTAT) & UART_UTRSTAT_RX_READY))
		return -1;

	return (int)*(volatile uint8_t *)(s5pc110_uart_base +
					 S5PC110_UART2_OFFSET + UART_URXH);
}

static void s5pc110_interrupt_init(void)
{
	ml_set_interrupts_enabled(FALSE);
}

static void s5pc110_unimplemented(const char *service)
	__attribute__((noreturn));

static void s5pc110_unimplemented(const char *service)
{
	panic("S5PC110: %s is not implemented", service);
}

static void s5pc110_timebase_init(void)
{
	s5pc110_unimplemented("timebase");
}

static void s5pc110_handle_interrupt(__unused void *context)
{
	s5pc110_unimplemented("interrupt handling");
}

static uint64_t s5pc110_get_timebase(void)
{
	s5pc110_unimplemented("timebase");
}

static uint64_t s5pc110_timer_value(void)
{
	s5pc110_unimplemented("timer");
}

static void s5pc110_timer_enabled(__unused int enable)
{
	s5pc110_unimplemented("timer");
}

void PE_init_SocSupport_stub(void)
{
	static const char banner[] = "\nS5PC110 XNU UART2 ready\n";
	unsigned int i;

	gPESocDispatch.uart_getc = s5pc110_uart_getc;
	gPESocDispatch.uart_putc = s5pc110_uart_putc;
	gPESocDispatch.uart_init = s5pc110_uart_init;
	gPESocDispatch.interrupt_init = s5pc110_interrupt_init;
	gPESocDispatch.timebase_init = s5pc110_timebase_init;
	gPESocDispatch.handle_interrupt = s5pc110_handle_interrupt;
	gPESocDispatch.get_timebase = s5pc110_get_timebase;
	gPESocDispatch.timer_value = s5pc110_timer_value;
	gPESocDispatch.timer_enabled = s5pc110_timer_enabled;

	s5pc110_uart_init();
	for (i = 0; i < sizeof(banner) - 1; i++)
		s5pc110_uart_putc(banner[i]);
}

#endif
