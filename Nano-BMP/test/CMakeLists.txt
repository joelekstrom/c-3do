add_executable(nano_test test.c)
target_link_libraries(nano_test nano_bmp)
install(TARGETS nano_test DESTINATION "${Nano_BMP_SOURCE_DIR}/bin")

