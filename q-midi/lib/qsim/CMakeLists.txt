# Finds all source files in the current directory
# and save the name to the DIR_examples_SRCS variable
#aux_source_directory(.)

include_directories(.)

# Generate the link library
add_library(qsim measure.c qsim.c simulate.c)
target_link_libraries(qsim pico_stdlib hardware_spi)
