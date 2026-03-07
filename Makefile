CXX = g++
CXXFLAGS = -Wall -I. -Iinclude -Ibackends

LIBS = -lglfw3 -lopengl32 -lgdi32 -ldxgi -ld3d11 -lole32 -luuid -lavrt -lwinmm

SRC = src/main.cpp imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp


TARGET = Corsprite_launcher.exe

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	del $(TARGET)