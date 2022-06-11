// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <assert.h>
#define Assert(x) assert(x)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;

#include "math.h"
#include "editor.cpp"

// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 18.0f, NULL);
    IM_ASSERT(font != NULL);

    // Our state
    ImVec4 clear_color = ImVec4(0, 0, 0, 1);
    ImVec4 WindowBgColor = ImVec4(0.3f, 0.315f, 0.33f, 1.00f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, WindowBgColor);

    u32 MemorySize = 1024*1024*512;
    void* MemoryBase = malloc(MemorySize);

    editor Editor;
    InitializePool(&Editor.BitmapPool, (u8* )MemoryBase, MemorySize);
    Editor.CameraOffset = V2(0, 0);
    Editor.PixelDim = V2(16, 16);
    Editor.ViewportVisible = true;
    Editor.ColorPickerVisible = true;
    Editor.CurrentColor = 0xFF000000;
    Editor.FloatColor[0] = 0.0;
    Editor.FloatColor[1] = 0.0;
    Editor.FloatColor[2] = 0.0;
    Editor.FloatColor[3] = 1.0;

    bitmap Bitmap = AllocateEmptyBitmap(&Editor.BitmapPool, 12, 12);
    FillBitmap(&Bitmap);

    // Main loop
    bool Running = true;
    while(Running)
    {
        SDL_Event Event;
        while(SDL_PollEvent(&Event))
        {
            ImGui_ImplSDL2_ProcessEvent(&Event);
            switch(Event.type)
            {
                case SDL_QUIT:
                {
                    Running = false;
                } break;

                case SDL_KEYDOWN:
                {
                    if(Event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        Running = false;
                    }

                    if(Event.key.keysym.sym == SDLK_SPACE)
                    {
                        Editor.CameraOffset = V2(0, 0);
                    }
                } break;
            }

            if(Event.type == SDL_WINDOWEVENT && Event.window.event == SDL_WINDOWEVENT_CLOSE && Event.window.windowID == SDL_GetWindowID(window))
                Running = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        if(ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("New"))
                {
                    // TODO: New
                }

                if(ImGui::MenuItem("Save"))
                {
                    // TODO: save
                    printf("Saved (no lul)\n");
                }
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Windows"))
            {
                if(ImGui::MenuItem("Viewport", NULL, &Editor.ViewportVisible)) {}
                if(ImGui::MenuItem("Color Picker", NULL, &Editor.ColorPickerVisible)) {}
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if(Editor.ViewportVisible)
        {
            if(ImGui::Begin("Viewport", &Editor.ViewportVisible))
            {
                ImGuiIO& IO = ImGui::GetIO();
                if(ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0))
                {
                    Editor.CameraOffset.x += IO.MouseDelta.x;
                    Editor.CameraOffset.y += IO.MouseDelta.y;
                }

                ImVec2 WindowPos = ImGui::GetWindowPos();
                v2 PixelDim = Editor.PixelDim;
                v2 Offset = Editor.CameraOffset + WindowPos + V2(30, 30);

                v2 MousePos = ImGui::GetMousePos();
                rect2 BitmapRect = GetBitmapRectTranslated(&Bitmap, PixelDim, Offset);
                bool MouseInRect = InRect(MousePos, BitmapRect);

                ImDrawList* DrawList = ImGui::GetWindowDrawList();
                for(u32 Y = 0; Y < Bitmap.Height; Y++)
                {
                    for(u32 X = 0; X < Bitmap.Width; X++)
                    {
                        v2 DrawOffset = Offset + X * V2(PixelDim.x, 0) + Y * V2(0, PixelDim.y);
                        u32 Color = GetPixelColor(&Bitmap, X, Y);
                        DrawList->AddRectFilled(DrawOffset, DrawOffset + PixelDim, Color);
                    }
                }

                if(InRect(MousePos, BitmapRect))
                {
                    rect2 Tmp = BitmapRect;
                    Tmp.Min = Tmp.Min - Offset;
                    Tmp.Max = Tmp.Max - Offset;
                    MousePos.x -= Offset.x;
                    MousePos.y -= Offset.y;

                    u32 X = MousePos.x / PixelDim.x;
                    u32 Y = MousePos.y / PixelDim.y;
                    Assert(X >= 0);
                    Assert(Y >= 0);

                    v2 HighlightOffset = Offset + X * V2(PixelDim.x, 0) + Y * V2(0, PixelDim.y);
                    u32 Thickness = 2;
                    DrawList->AddRect(HighlightOffset, HighlightOffset + PixelDim, 0xFF0000FF, 0, 0, Thickness);
                    if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        SetPixelColor(&Bitmap, X, Y, Editor.CurrentColor);
                    }
                }
            }
            ImGui::End();
        }

        if(Editor.ColorPickerVisible)
        {
            if(ImGui::Begin("Color Picker", &Editor.ColorPickerVisible))
            {
                ImGui::ColorPicker4("MyColor##4", (f32 *)&Editor.FloatColor);
                u32 Red = Editor.FloatColor[0] * 255;
                u32 Blue = Editor.FloatColor[1] * 255;
                u32 Green = Editor.FloatColor[2] * 255;
                u32 Alpha = Editor.FloatColor[3] * 255;

                Editor.CurrentColor = (Red << 0) |
                                      (Blue << 8) |
                                      (Green << 16) |
                                      (Alpha << 24);
            }
            ImGui::End();
        }

        // ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
