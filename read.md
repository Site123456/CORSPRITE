Install: 
https://vulkan.lunarg.com/doc/sdk/1.4.341.1/windows/getting_started.html
https://www.glfw.org/download.html

Run command:

Build run:
g++ -Wall src/main.cpp imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp backends/imgui_impl_glfw.cpp backends/imgui_impl_vulkan.cpp -I. -Iinclude -Ibackends -IC:/VulkanSDK/1.4.341.1/Include -LC:/VulkanSDK/1.4.341.1/Lib -lglfw3 -lvulkan-1 -lgdi32 -o app.exe