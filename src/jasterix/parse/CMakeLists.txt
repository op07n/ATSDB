
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(jasterix
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/frameparser.h"
        "${CMAKE_CURRENT_LIST_DIR}/itemparsing.h"
        "${CMAKE_CURRENT_LIST_DIR}/frameparsertask.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/frameparser.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/itemparsing.cpp"
)

