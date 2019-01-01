
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(jasterix
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/jasterix_files.h"
        "${CMAKE_CURRENT_LIST_DIR}/base64.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/jasterix_files.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base64.cpp"
)

