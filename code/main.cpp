// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
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

enum
{
    MaxProjectNameLength = 255,
    MaxLayerNameLength = 32,
};

#include "math.h"
#include "editor.cpp"
#include "paf.cpp"

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
    ImVec4 WindowBgColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, WindowBgColor);

    u32 MemorySize = 1024*1024*512;
    void* MemoryBase = malloc(MemorySize);
    u32 BitmapMemorySize = 1024*1024*400;

    editor Editor = {};
    InitializePool(&Editor.BitmapPool, (u8* )MemoryBase, BitmapMemorySize);
    InitializePool(&Editor.LayerPool, (u8* )MemoryBase + BitmapMemorySize, MemorySize - BitmapMemorySize);

    Editor.CameraOffset = V2(0, 0);
    Editor.PixelDim = V2(16, 16);
    Editor.ViewportVisible = true;
    Editor.ColorPickerVisible = true;
    Editor.TimelineVisible = true;

    Editor.BitmapWidth = 16;
    Editor.BitmapHeight = 16;
    
    Editor.FloatColor[0] = 0.0;
    Editor.FloatColor[1] = 0.0;
    Editor.FloatColor[2] = 0.0;
    Editor.FloatColor[3] = 1.0;
    Editor.CurrentColor = ColorFloatToU32(Editor.FloatColor);

    Editor.FloatBitmapBgColor[0] = 0.0;
    Editor.FloatBitmapBgColor[1] = 0.3;
    Editor.FloatBitmapBgColor[2] = 0.3;
    Editor.FloatBitmapBgColor[3] = 1.0;
    Editor.BitmapBgColor = ColorFloatToU32(Editor.FloatBitmapBgColor);

    Editor.Ticks = SDL_GetTicks();

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

        // NOTE: TODO: workaround for ImGui bug with popups not opening in menus
        bool NewProject = false;
        if(ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("New"))
                {
                    NewProject = true;
                }

                if(ImGui::MenuItem("Save"))
                {
                    // TODO: save
                    // TODO: Disallow saving if we have no layer
                    ImGuiFileDialog::Instance()->OpenModal("Save project", "Save file", ".paf,.*", ".");
                    printf("Saved (no lul)\n");
                }

                if(ImGui::MenuItem("Load"))
                {
                    ImGuiFileDialog::Instance()->OpenModal("Load project", "Load file", ".paf,.*", ".");
                    printf("Loaded (no lul)\n");
                }
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Windows"))
            {
                if(ImGui::MenuItem("Viewport", NULL, &Editor.ViewportVisible)) {}
                if(ImGui::MenuItem("Color Picker", NULL, &Editor.ColorPickerVisible)) {}
                if(ImGui::MenuItem("Timeline", NULL, &Editor.TimelineVisible)) {}
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();

        }

        if(ImGuiFileDialog::Instance()->Display("Load project"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string FilePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                FILE* File = fopen(FilePathName.c_str(), "rb");
                if(File)
                {
                    u8 MagicHeader[4];

                    fread(MagicHeader, sizeof(MagicHeader), 1, File);
                    if(MagicHeader[0] == 'P' &&
                       MagicHeader[1] == 'A' &&
                       MagicHeader[2] == 'F' &&
                       MagicHeader[3] == '0')
                    {
                        ZeroMemory(Editor.BitmapPool.Base, Editor.BitmapPool.Used);
                        Editor.BitmapPool.Used = 0;
                        ZeroMemory(Editor.LayerPool.Base, Editor.LayerPool.Used);
                        Editor.LayerPool.Used = 0;
                        
                        fread(Editor.ProjectName, sizeof(Editor.ProjectName), 1, File);
                        fread(Editor.ProjectName, sizeof(Editor.ProjectName), 1, File);
                        fread(&Editor.BitmapWidth, sizeof(Editor.BitmapWidth), 1, File);
                        fread(&Editor.BitmapHeight, sizeof(Editor.BitmapHeight), 1, File);                

                        Editor.FirstLayer = PushStruct(&Editor.LayerPool, layer);
                        Editor.FirstLayer->FirstFrame = Editor.FirstLayer->LastFrame = Editor.FirstLayer->SelectedFrame = 0;
                        fread(Editor.FirstLayer->Name, sizeof(Editor.FirstLayer->Name), 1, File);
                        while(!feof(File))
                        {
                            animation_frame* Frame = AllocateEmptyAnimationFrame(&Editor.BitmapPool, Editor.BitmapWidth, Editor.BitmapHeight);
                            fread(&Frame->Bitmap.Width, sizeof(Frame->Bitmap.Width), 1, File);
                            fread(&Frame->Bitmap.Height, sizeof(Frame->Bitmap.Height), 1, File);
                            fread(&Frame->Bitmap.Stride, sizeof(Frame->Bitmap.Stride), 1, File);
                            fread(&Frame->Bitmap.Data, Frame->Bitmap.Width * Frame->Bitmap.Height * sizeof(*Frame->Bitmap.Data), 1, File);
                            if(Editor.FirstLayer->LastFrame)
                            {
                                Frame->PrevFrame = Editor.FirstLayer->LastFrame;
                                Editor.FirstLayer->LastFrame->NextFrame = Frame;
                                Editor.FirstLayer->LastFrame = Frame;
                            }
                            else
                            {
                                Editor.FirstLayer->SelectedFrame = Editor.FirstLayer->FirstFrame = Editor.FirstLayer->LastFrame = Frame;
                            }
                                                        
                        }
                    }
                    else
                    {
                        // TODO: proper error handling
                        Assert(0 && "Wrong magic header");
                    }

                    fclose(File);
                }

            }
            ImGuiFileDialog::Instance()->Close();
        }

        if(ImGuiFileDialog::Instance()->Display("Save project"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string FilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                u8 MagicHeader[4] = {'P', 'A', 'F', '0'};

                FILE* File = fopen(FilePathName.c_str(), "wb");
                if(File)
                {
                    // TODO: this is probably very inefficient (maybe not, if fwrite using buffering)
                    fwrite(MagicHeader, sizeof(MagicHeader), 1, File);
                    fwrite(Editor.ProjectName, sizeof(Editor.ProjectName), 1, File);
                    fwrite(&Editor.BitmapWidth, sizeof(Editor.BitmapWidth), 1, File);
                    fwrite(&Editor.BitmapHeight, sizeof(Editor.BitmapHeight), 1, File);

                    // Saving each layer(for now we have only one)
                    // TODO: now we are saving full buffer, even if name length is less than buffer length,
                    // probably should switch to length + name
                    layer* Layer = Editor.FirstLayer;
                    Assert(Layer && "In save project - no layer found");
                    fwrite(Layer->Name, sizeof(Layer->Name), 1, File);
                    animation_frame* Frame = Layer->FirstFrame;
                    while(Frame)
                    {
                        fwrite(&Frame->Bitmap.Width, sizeof(Frame->Bitmap.Width), 1, File);
                        fwrite(&Frame->Bitmap.Height, sizeof(Frame->Bitmap.Height), 1, File);
                        fwrite(&Frame->Bitmap.Stride, sizeof(Frame->Bitmap.Stride), 1, File);
                        fwrite(Frame->Bitmap.Data, Frame->Bitmap.Width * Frame->Bitmap.Height * sizeof(*Frame->Bitmap.Data), 1, File);   
                        
                        Frame = Frame->NextFrame;
                    }

                    fclose(File);
                    printf("File Saved!\n");
                }
                else
                {
                    // TODO: error handling
                    Assert(0 && "Error during saving file");
                }

            }
            ImGuiFileDialog::Instance()->Close();
        }

        if(NewProject)
        {
            ImGui::OpenPopup("New Project");
            ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(Center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        if(ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char NewName[MaxProjectNameLength + 1] = {};
            static s32 NewWidth = 4;
            static s32 NewHeight = 4;
            
            ImGui::InputText("Project Name:", NewName, sizeof(NewName));
            ImGui::InputInt("Bitmap Width", &NewWidth);
            ImGui::InputInt("Bitmap Height", &NewHeight);

            bool DisableOkWidth = (NewWidth < 4 || NewWidth > 256) ? true : false;
            bool DisableOkHeight = (NewHeight < 4 || NewHeight > 256) ? true : false;
            bool DisableOkName = (NewName[0] == '\0') ? true : false;

            // TODO: Explain why we cant create new project
            bool DisableOk = DisableOkWidth || DisableOkHeight || DisableOkName;
            
            if(DisableOk)
                ImGui::BeginDisabled();

            if(ImGui::Button("OK", ImVec2(120, 0))) 
            {
                ZeroMemory(Editor.ProjectName, sizeof(Editor.ProjectName));
                Editor.BitmapWidth = NewWidth;
                Editor.BitmapHeight = NewHeight;
                strcpy(Editor.ProjectName, NewName);

                ZeroMemory(Editor.BitmapPool.Base, Editor.BitmapPool.Used);
                Editor.BitmapPool.Used = 0;
                ZeroMemory(Editor.LayerPool.Base, Editor.LayerPool.Used);
                Editor.LayerPool.Used = 0;

                Editor.FirstLayer = PushStruct(&Editor.LayerPool, layer);
                strcpy(Editor.FirstLayer->Name, "Test Layer");
                animation_frame* Frame = AllocateEmptyAnimationFrame(&Editor.BitmapPool, Editor.BitmapWidth, Editor.BitmapHeight);
                Editor.FirstLayer->SelectedFrame = Editor.FirstLayer->FirstFrame = Editor.FirstLayer->LastFrame = Frame;
                FillBitmap(&Editor.FirstLayer->FirstFrame->Bitmap, 0x00000000);

                NewWidth = 4;
                NewHeight = 4;
                ZeroMemory(NewName, sizeof(NewName));
                ImGui::CloseCurrentPopup(); 
            }

            if(DisableOk)
                ImGui::EndDisabled();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            
            ImGui::EndPopup();
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

                if(ImGui::IsWindowHovered())
                {
                    f32 MouseWheel = ImGui::GetIO().MouseWheel;
                    f32 NewDim = Editor.PixelDim.x + MouseWheel;
                    f32 MinPixelDim = 10;
                    f32 MaxPixelDim = 50;
                    NewDim = Clamp(NewDim, MinPixelDim, MaxPixelDim);
                    
                    Editor.PixelDim = V2(NewDim, NewDim);
                }

                layer* Layer = Editor.FirstLayer;
                if(Layer)
                {
                    bitmap Bitmap = Editor.FirstLayer->SelectedFrame->Bitmap;
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
                            DrawList->AddRectFilled(DrawOffset, DrawOffset + PixelDim, Editor.BitmapBgColor);
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
                        if(ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                        {
                            SetPixelColor(&Bitmap, X, Y, Editor.CurrentColor);
                        }
                    }
                }
                
            }
            ImGui::End();
        }

        if(Editor.ColorPickerVisible)
        {
            if(ImGui::Begin("Color Picker", &Editor.ColorPickerVisible))
            {
                ImGui::ColorPicker4("EditorColor##4", (f32 *)&Editor.FloatColor);
                Editor.CurrentColor = ColorFloatToU32(*(ImVec4* )&Editor.FloatColor);

                ImGui::ColorPicker4("BitmapBgColor##4", (f32 *)&Editor.FloatBitmapBgColor);
                Editor.BitmapBgColor = ColorFloatToU32(*(ImVec4* )&Editor.FloatBitmapBgColor);
            }
            
            ImGui::End();
        }

        if(Editor.TimelineVisible)
        {
            if(ImGui::Begin("Timeline", &Editor.TimelineVisible))
            {
                layer* Layer = Editor.FirstLayer;
                if(Layer)
                {
                    if(Editor.Animating)
                    {
                        if(ImGui::Button("Stop"))
                        {
                            Editor.Animating = !Editor.Animating; 
                        }
                    }
                    else
                    {
                        if(ImGui::Button("Start"))
                        {
                            Editor.Animating = !Editor.Animating;
                            Editor.CurrentFrameTicks = Editor.Ticks; 
                        }
                    }

                    ImGui::Text(Layer->Name);
                    ImGui::SameLine();
                    animation_frame* Frame = Layer->FirstFrame;
                    u32 FrameCount = 1;
                    char FrameCountStr[3];
                    
                    while(Frame)
                    {
                        snprintf(FrameCountStr, sizeof(FrameCountStr), "%d", FrameCount);
                        bool Selected = Layer->SelectedFrame == Frame;
                        // TODO: hard coded sizes
                        if(ImGui::Selectable(FrameCountStr, Selected, 0, ImVec2(10, 20)) && !Editor.Animating)
                        {
                            Layer->SelectedFrame = Frame;
                        }
                        
                        FrameCount++;
                        Frame = Frame->NextFrame;
                        ImGui::SameLine();
                    }

                    if(ImGui::Button("+") && !Editor.Animating)
                    {
                        animation_frame* NewFrame = AllocateEmptyAnimationFrame(&Editor.BitmapPool, Editor.BitmapWidth, Editor.BitmapHeight);
                        NewFrame->PrevFrame = Layer->LastFrame;
                        Layer->LastFrame->NextFrame = NewFrame;
                        Layer->LastFrame = NewFrame;
                    }

                    if(Editor.Animating)
                    {
                        u32 FrameTimeMs = 250;
                        u32 Difference = Editor.Ticks - Editor.CurrentFrameTicks; 
                        if(Difference >= FrameTimeMs)
                        {
                            Editor.CurrentFrameTicks = Editor.Ticks;
                            if(Layer->SelectedFrame->NextFrame)
                            {
                                Layer->SelectedFrame = Layer->SelectedFrame->NextFrame;
                            }
                            else
                            {
                                Layer->SelectedFrame = Layer->FirstFrame;
                            }
                        }
                    }
                }
                   
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

        Editor.Ticks = SDL_GetTicks();
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
