add_executable(main
        main.c
        ../../raspberry_pi_1/cpelight.c
        ../../Jack_Coroutines/jack_coroutines.S
        )

target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../Jack_Coroutines
        )

# pull in common dependencies and additional spi hardware support
target_link_libraries(main pico_stdlib hardware_spi)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)
