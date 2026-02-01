add_executable(lcd_uart
    lcd_uart.c
)

# incluir dependências comuns e suporte adicional ao hardware UART
target_link_libraries(lcd_uart pico_stdlib hardware_uart)

# habilitar saída USB e saída UART
# modifique aqui conforme necessário
pico_enable_stdio_usb(lcd_uart 1)
pico_enable_stdio_uart(lcd_uart 1)

# criar arquivos map/bin/hex etc.
pico_add_extra_outputs(lcd_uart)

# adicionar URL via pico_set_program_url
example_auto_set_url(lcd_uart)