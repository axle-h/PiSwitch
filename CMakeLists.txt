cmake_minimum_required(VERSION 2.8)
project(PiSwitch)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

configure_file ("service/PiSwitch.in" "${PROJECT_BINARY_DIR}/service/PiSwitch")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

# Source files
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.c")
add_executable(PiSwitch ${SOURCE_FILES})

find_package(BCM2835 REQUIRED)
find_package(Confuse REQUIRED)

include_directories(include
    ${BCM2835_INCLUDE_DIR}
    ${CONFUSE_INCLUDE_DIR})

target_link_libraries(PiSwitch
    ${BCM2835_LIBRARIES}
    ${CONFUSE_LIBRARIES})

# install target
install(TARGETS PiSwitch
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_READ
    DESTINATION sbin)

install(FILES "${PROJECT_SOURCE_DIR}/service/piswitch.cfg"
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
    DESTINATION /etc/gpio)

install(FILES "${PROJECT_BINARY_DIR}/service/PiSwitch"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_READ
    DESTINATION /etc/init.d)

install(CODE "execute_process(COMMAND systemctl daemon-reload)")
install(CODE "execute_process(COMMAND service PiSwitch start)")
install(CODE "execute_process(COMMAND update-rc.d PiSwitch defaults)")

# uninstall target
SET(DAEMON_SCRIPT PiSwitch)
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/cmake_uninstall.cmake)