Set correct Platform and Options
- ln -s build-configurations/Options-stm32discoveryf4-linux

Options
- ln -s build-configurations/Platform-cortexm4-linux Platform

Build stack
- make rebuild

After building specific ZBOSS applications or tests
you will see generated .hex files

Make sure https://github.com/texane/stlink is
installed on your computer/virtual machine

You may flash these hex files to an STM32F4 board:
- st-flash write zdo_start_zc.bin 0x8000000
