add_executable(pwm_measure_duty_cycle
        measure_duty_cycle.c
        )

# adiciona dependências comuns e suporte adicional ao hardware de PWM
target_link_libraries(pwm_measure_duty_cycle pico_stdlib hardware_pwm)

# cria arquivos map/bin/hex etc.
pico_add_extra_outputs(pwm_measure_duty_cycle)

# adiciona URL via pico_set_program_url
example_auto_set_url(pwm_measure_duty_cycle)