add_executable(c3do geometry.c obj.c main.c graphics_context.c ../include/geometry.h ../include/obj.h ../include/graphics_context.h)
target_link_libraries(c3do nano_bmp)

if (UNIX)
	target_link_libraries(c3do m)
endif (UNIX)