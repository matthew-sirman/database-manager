# myproject-config.cmake

# Set the include directory
set(database_manager_core_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../include")

set(database_manager_core_LIB
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../out/build/x64-debug/database_manager_core_d.lib>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_LIST_DIR}/../out/build/x64-release/database_manager_core.lib>
)
# Optionally, you can include any other CMake logic, such as handling different OSes, debug/release variants, etc.
