add_library(nano_bmp nano_bmp.c)

install(FILES "${Nano_BMP_SOURCE_DIR}/include/nano_bmp.h" DESTINATION include)
install(TARGETS nano_bmp
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

