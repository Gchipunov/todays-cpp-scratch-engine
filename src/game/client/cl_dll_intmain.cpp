// dllmain.c
#include <windows.h>
#include <stdint.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

// Structure to hold our demo window state
struct DemoState {
    bool show_demo_window;
    float color[4];
    int counter;
};

// Global state
static DemoState g_state = { true, {0.45f, 0.55f, 0.60f, 1.00f}, 0 };

// Exported function to initialize ImGui context
__declspec(dllexport) void InitImGui() {
    // Initialize ImGui context if needed
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup ImGui configuration here if needed
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
}

// Exported function to render the UI
__declspec(dllexport) void RenderUI() {
    // Start the Dear ImGui frame
    ImGui::NewFrame();

    // Simple demo window
    if (g_state.show_demo_window) {
        ImGui::Begin("Demo Window", &g_state.show_demo_window);
        
        ImGui::Text("This is a sample ImGui window from a DLL");
        ImGui::ColorEdit3("Color", g_state.color);
        
        if (ImGui::Button("Click Me"))
            g_state.counter++;
            
        ImGui::Text("Counter: %d", g_state.counter);
        ImGui::End();
    }

    // Render ImGui
    ImGui::Render();
}

// Exported function to cleanup
__declspec(dllexport) void ShutdownImGui() {
    ImGui::DestroyContext();
}

// Exported function to check if demo window is visible
__declspec(dllexport) bool IsDemoWindowVisible() {
    return g_state.show_demo_window;
}
