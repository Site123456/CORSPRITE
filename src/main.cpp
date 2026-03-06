#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <windows.h>
#include <stdio.h>
void OpenURL(const char* url) {
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}
int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // No title bar
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // Transparent framebuffer
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // --- GET SCREEN SIZE BEFORE CREATING WINDOW ---
    RECT screen;
    GetWindowRect(GetDesktopWindow(), &screen);

    int screenWidth  = screen.right  - screen.left;
    int screenHeight = screen.bottom - screen.top - 40; // taskbar height compensation (optional)

    // --- CREATE FULLSCREEN-SIZED WINDOW ---
    GLFWwindow* window = glfwCreateWindow(
        screenWidth,
        screenHeight,
        "Transparent Overlay",
        NULL,
        NULL
    );

    if (!window)
        return -1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Win32 handle
    HWND hwnd = glfwGetWin32Window(window);

    // Always on top
    SetWindowPos(
        hwnd,
        HWND_TOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE
    );

    // Transparent layered window (click-through capable)
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(
        hwnd,
        GWL_EXSTYLE,
        style | WS_EX_LAYERED | WS_EX_TRANSPARENT
    );
    SetLayeredWindowAttributes(hwnd, RGB(0,0,0), 255, LWA_ALPHA);

    // --- INIT IMGUI ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImFont* fontSmallest  = io.Fonts->AddFontFromFileTTF("fonts/Roboto-VariableFont_wdth,wght.ttf", 12.0f);
    ImFont* fontSmall  = io.Fonts->AddFontFromFileTTF("fonts/Roboto-VariableFont_wdth,wght.ttf", 18.0f);
    ImFont* fontMedium = io.Fonts->AddFontFromFileTTF("fonts/Roboto-VariableFont_wdth,wght.ttf", 22.0f);
    ImFont* fontLarge  = io.Fonts->AddFontFromFileTTF("fonts/Roboto-VariableFont_wdth,wght.ttf", 32.0f);
    io.FontDefault = fontMedium;



    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 6.0f;
    s.FrameRounding  = 5.0f;
    s.GrabRounding   = 5.0f;
    s.WindowBorderSize = 0.0f;
    s.Colors[ImGuiCol_WindowBg].w = 0.90f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // UI state
    ImVec4 circleColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
    float radius = 8.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLuint myImageLogoTexture = 0;
    int imgWidth = 0, imgHeight = 0;

    {
        int channels;
        unsigned char* data = stbi_load("images/logo.png", &imgWidth, &imgHeight, &channels, 4);

        glGenTextures(1, &myImageLogoTexture);
        glBindTexture(GL_TEXTURE_2D, myImageLogoTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Detect if mouse is over ImGui
        bool hoveringUI =
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
            ImGui::IsAnyItemHovered();

        // Click-through logic
        LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
        if (hoveringUI)
            SetWindowLong(hwnd, GWL_EXSTYLE, ex & ~WS_EX_TRANSPARENT);
        else
            SetWindowLong(hwnd, GWL_EXSTYLE, ex | WS_EX_TRANSPARENT);

        // --- START IMGUI FRAME ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- LEFT PANEL (INTERACTIVE) ---
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGuiStyle& s = ImGui::GetStyle();

        // --- WINDOW SHAPE ---
        s.WindowRounding = 18.0f;     // big visible rounding
        s.FrameRounding  = 10.0f;
        s.GrabRounding   = 10.0f;
        s.ChildRounding  = 14.0f;

        // --- PADDING ---
        s.WindowPadding  = ImVec2(20, 20);   // internal padding
        s.FramePadding   = ImVec2(12, 8);    // widget padding
        s.ItemSpacing    = ImVec2(12, 10);   // spacing between items
        s.ItemInnerSpacing = ImVec2(8, 6);

        // --- BORDERS ---
        s.WindowBorderSize = 0.0f;
        s.FrameBorderSize  = 0.0f;

        // --- COLORS ---
        s.Colors[ImGuiCol_WindowBg]        = ImVec4(0.10f, 0.10f, 0.10f, 0.92f);
        s.Colors[ImGuiCol_ChildBg]         = ImVec4(0.12f, 0.12f, 0.12f, 0.00f);
        s.Colors[ImGuiCol_PopupBg]         = ImVec4(0.10f, 0.10f, 0.10f, 0.94f);

        s.Colors[ImGuiCol_FrameBg]         = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
        s.Colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.25f, 0.25f, 0.25f, 0.90f);
        s.Colors[ImGuiCol_FrameBgActive]   = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

        s.Colors[ImGuiCol_Button]          = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
        s.Colors[ImGuiCol_ButtonHovered]   = ImVec4(0.30f, 0.30f, 0.30f, 0.90f);
        s.Colors[ImGuiCol_ButtonActive]    = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

        s.Colors[ImGuiCol_Header]          = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
        s.Colors[ImGuiCol_HeaderHovered]   = ImVec4(0.35f, 0.35f, 0.35f, 0.90f);
        s.Colors[ImGuiCol_HeaderActive]    = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

        s.Colors[ImGuiCol_Separator]       = ImVec4(0.35f, 0.35f, 0.35f, 0.50f);
        s.Colors[ImGuiCol_SeparatorHovered]= ImVec4(0.50f, 0.50f, 0.50f, 0.70f);
        s.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.60f, 0.60f, 0.80f);

        s.Colors[ImGuiCol_Text]            = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        s.Colors[ImGuiCol_TextDisabled]    = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(360, io.DisplaySize.y - 40), ImGuiCond_Always);

        ImGui::Begin("MainUI", NULL,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar
        );
        // Logo (200x200)
        ImGui::Image(
            (void*)(intptr_t)myImageLogoTexture,
            ImVec2(60, 60)
        );

        // Put next item on the same horizontal line
        ImGui::SameLine();

        // Add spacing so the text isn't glued to the image
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 14);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 20); // horizontal alignment tweak

        // Big title font
        ImGui::PushFont(fontLarge);
        ImGui::Text("ORSPRITE");
        ImGui::PopFont();

        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("View Documentation", ImVec2(-1, 40))) {
            OpenURL("https://corsprite-docs.vercel.app/");
        }


        ImGui::PushFont(fontMedium);
        ImGui::Text("Circle Settings");
        ImGui::PopFont();
        
        ImGui::PushFont(fontSmall);
        ImGui::Text("Enabled when corsprite is on.");
        ImGui::PopFont();
        ImGui::PushFont(fontSmall);
        ImGui::ColorEdit3("Color", (float*)&circleColor);
        ImGui::SliderFloat("Radius", &radius, 1.0f, 100.0f);
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        
        ImGui::PushFont(fontMedium);
        if (ImGui::Button("Quit CORSPRITE", ImVec2(-1, 40)))
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        ImGui::PopFont();
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushFont(fontMedium);
        ImGui::Text("© 2026 CORSPRITE");
        ImGui::PopFont();
        ImGui::PushFont(fontSmallest);
        ImGui::Text("Version Alpha 0.1");
        ImGui::PopFont();
        ImGui::End();


        // --- DRAW CIRCLE CENTERED ---
        ImVec2 center(
            io.DisplaySize.x - radius - 10.0f,   // X = right side - radius - margin
            radius + 10.0f                       // Y = top + radius + margin
        );


        ImGui::GetForegroundDrawList()->AddCircleFilled(
            center,
            radius,
            ImColor(circleColor),
            64
        );

        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0, 0, 0, 0); // transparent
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
