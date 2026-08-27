file(
    GLOB_RECURSE
    LVGL_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)

add_library(
    cryptomachine_lvgl
    STATIC
    ${LVGL_SOURCES}
)

target_include_directories(
    cryptomachine_lvgl
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../..
)

target_compile_definitions(
    cryptomachine_lvgl
    PUBLIC
        LV_CONF_INCLUDE_SIMPLE
)