TINYIIOD=y
LIBRARIES += iio

ifneq (,$(PLATFORM))
SRC_DIRS += $(PROJECT)/src/platform/$(PLATFORM)
endif

SRC_DIRS += $(NO-OS)/iio/iio_app \
	+= $(PROJECT)/src/app \
	+= $(DRIVERS)/rf-transceiver/hmc630x

SRCS += $(DRIVERS)/api/no_os_gpio.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.c \
	$(DRIVERS)/api/no_os_spi.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.c \
	$(DRIVERS)/api/no_os_irq.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_irq.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_rtc.c \
	$(DRIVERS)/api/no_os_uart.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart_stdio.c \
	$(DRIVERS)/api/no_os_mdio.c \
	$(DRIVERS)/net/mdio_bitbang.c \
	$(NO-OS)/util/no_os_lf256fifo.c \
	$(NO-OS)/util/no_os_util.c \
	$(NO-OS)/util/no_os_alloc.c \
	$(NO-OS)/util/no_os_list.c \
	$(NO-OS)/util/no_os_pid.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_delay.c

INCS += $(INCLUDE)/no_os_gpio.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.h \
	$(INCLUDE)/no_os_spi.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.h \
	$(INCLUDE)/no_os_irq.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_irq.h \
	$(INCLUDE)/no_os_rtc.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_rtc.h \
	$(INCLUDE)/no_os_uart.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart_stdio.h \
	$(INCLUDE)/no_os_mdio.h \
	$(DRIVERS)/net/mdio_bitbang.h \
	$(INCLUDE)/no_os_error.h \
	$(INCLUDE)/no_os_util.h \
	$(INCLUDE)/no_os_alloc.h \
	$(INCLUDE)/no_os_units.h \
	$(INCLUDE)/no_os_list.h \
	$(INCLUDE)/no_os_pid.h \
	$(INCLUDE)/no_os_print_log.h \
	$(INCLUDE)/no_os_delay.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_delay.h \
	$(INCLUDE)/no_os_lf256fifo.h


