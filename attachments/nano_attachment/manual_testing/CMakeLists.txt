include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/core/include/attachments)
include_directories(${PROJECT_SOURCE_DIR}/attachments/nano_attachment)

add_executable(nano_attachment_test main.c)

target_link_libraries(nano_attachment_test nano_attachment)

# install(TARGETS nano_attachment_test DESTINATION bin)
