add_executable(c3do geometry.c obj.c main.c graphics_context.c color.c textures.c shaders.c)
target_link_libraries(c3do SDL2)

if (UNIX)
	target_link_libraries(c3do m)
endif (UNIX)
