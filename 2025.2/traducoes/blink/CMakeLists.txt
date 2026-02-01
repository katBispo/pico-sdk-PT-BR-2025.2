add_executable(blink
    blink.c
    )

# adiciona dependências comuns
target_link_libraries(blink pico_stdlib)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(blink pico_cyw43_arch_none)
endif()

# cria arquivos map/bin/hex etc.
pico_add_extra_outputs(blink)

# adiciona URL via pico_set_program_url
example_auto_set_url(blink)
