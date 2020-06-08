IMGUI_OBJECTS = imgui/imgui.o imgui/imgui_draw.o imgui/imgui_impl_sdl.o imgui/imgui_demo.o imgui/imgui_impl_opengl3.o imgui/imgui_widgets.o

OBJECTS = main.o keyboard.o math3d.o rendering.o sim.o glew.o $(IMGUI_OBJECTS)

CPPFLAGS = -g -isystem. -isystemSDL -DGLEW_STATIC -DGLEW_NO_GLU -DIMGUI_IMPL_OPENGL_LOADER_GLEW

test_image_generator: $(OBJECTS)
	$(CXX) $(OBJECTS) -o test_image_generator $(CPPFLAGS) -L. -pthread -lGL -lSDL2 -ldl

clean:
	rm -f test_image_generator
	rm -f $(OBJECTS)
