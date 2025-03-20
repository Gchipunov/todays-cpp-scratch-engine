cmake_minimum_required(VERSION 3.10)
project(IntMainLauncher)

# Set C standard (since we're using C, not C++)
set(CMAKE_C_STANDARD 11)

# Add launcher source
add_executable(IntMainLauncher
    main.c
)

# Link against Windows libraries
target_link_libraries(IntMainLauncher PRIVATE
    user32
)

# Ensure the DLL is built before the launcher
add_dependencies(IntMainLauncher IntMainClientDLL)
