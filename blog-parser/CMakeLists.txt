set(APP_NAME "blog-parser")
add_executable(${APP_NAME})

file(GLOB SOURCES src/*.cpp)
target_sources(${APP_NAME} PRIVATE ${SOURCES})

target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${APP_NAME}
    PRIVATE
        fmt::fmt
        cmark::cmark
)