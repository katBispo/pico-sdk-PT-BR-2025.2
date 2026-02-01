# Define um executável chamado "uart_advanced"
add_executable(uart_advanced
    uart_advanced.c
)

# Vincula as dependências comuns do Pico SDK
# e o suporte adicional ao hardware de UART
target_link_libraries(uart_advanced pico_stdlib hardware_uart)

# Gera arquivos extras de saída (map, bin, hex, uf2, etc.)
pico_add_extra_outputs(uart_advanced)

# Adiciona a URL do exemplo usando pico_set_program_url
example_auto_set_url(uart_advanced)
