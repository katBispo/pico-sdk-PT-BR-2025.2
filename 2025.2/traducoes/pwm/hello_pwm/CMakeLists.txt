add_executable(hello_pwm
        hello_pwm.c
        )

# adiciona dependências comuns e suporte adicional ao hardware de PWM
target_link_libraries(hello_pwm pico_stdlib hardware_pwm)

# cria arquivos map/bin/hex etc.
pico_add_extra_outputs(hello_pwm)

# adiciona URL via pico_set_program_url
example_auto_set_url(hello_pwm)
