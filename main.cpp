// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <ctime>
#include <cstdlib>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// =============================================================================
// CONFIGURABLE PARAMETERS — edit these to tweak without touching logic
// =============================================================================

// --- Desktop Parameters ---
static const ImVec4 WALLPAPER_COLOR_TOP    = ImVec4(0.05f, 0.05f, 0.20f, 1.00f); // Dark navy
static const ImVec4 WALLPAPER_COLOR_BOTTOM = ImVec4(0.10f, 0.35f, 0.55f, 1.00f); // Teal blue
static const char*  CLOCK_FORMAT           = "%H:%M:%S";                          // strftime format
static const char*  DATE_FORMAT            = "%A, %B %d, %Y";                     // strftime format
static const float  CLOCK_FONT_SCALE       = 2.0f;                                // Clock text scale
static const float  DATE_FONT_SCALE        = 1.2f;                                // Date text scale
static const float  CLOCK_MARGIN           = 20.0f;                               // Margin from screen edges
static const ImVec4 CLOCK_TEXT_COLOR        = ImVec4(1.0f, 1.0f, 1.0f, 0.95f);
static const ImVec4 DATE_TEXT_COLOR         = ImVec4(0.85f, 0.85f, 0.95f, 0.80f);
static const char*  WINDOW_TITLE           = "CSOPESY Desktop OS";                // GLFW window title
static const int    WINDOW_WIDTH           = 1280;
static const int    WINDOW_HEIGHT          = 800;
static const float  PWR_BUTTON_SIZE        = 50.0f;                               // PWR button diameter
static const ImVec4 PWR_BUTTON_COLOR       = ImVec4(0.80f, 0.15f, 0.15f, 1.00f); // Red
static const ImVec4 PWR_BUTTON_HOVER       = ImVec4(1.00f, 0.20f, 0.20f, 1.00f); // Brighter red on hover
static const ImVec4 PWR_BUTTON_ACTIVE      = ImVec4(0.60f, 0.10f, 0.10f, 1.00f); // Darker red on click

// --- Taskbar Parameters ---
static const float  TASKBAR_HEIGHT         = 48.0f;
static const ImVec4 TASKBAR_COLOR          = ImVec4(0.08f, 0.08f, 0.12f, 0.95f); // Near-black
static const float  TASKBAR_BUTTON_WIDTH   = 100.0f;
static const float  TASKBAR_BUTTON_HEIGHT  = 36.0f;
static const ImVec4 TASKBAR_BTN_COLOR      = ImVec4(0.18f, 0.18f, 0.25f, 1.00f);
static const ImVec4 TASKBAR_BTN_HOVER      = ImVec4(0.28f, 0.28f, 0.40f, 1.00f);
static const ImVec4 TASKBAR_BTN_ACTIVE     = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
static const char*  APP1_NAME              = "Notes";
static const char*  APP2_NAME              = "System Info";
static const char*  APP3_NAME              = "Task Mgr";

// --- Task Manager Parameters ---
struct ProcessInfo {
    const char* name;
    float       cpuPercent;
    float       memoryMB;
    const char* status;
};

static ProcessInfo DUMMY_PROCESSES[] = {
    { "System",              0.1f,   4.2f,   "Running" },
    { "csrss.exe",           0.3f,  12.8f,   "Running" },
    { "explorer.exe",        2.5f,  85.4f,   "Running" },
    { "svchost.exe",         1.2f,  42.1f,   "Running" },
    { "dwm.exe",             3.8f,  64.0f,   "Running" },
    { "chrome.exe",         12.4f, 312.5f,   "Running" },
    { "chrome.exe (GPU)",    5.6f, 180.3f,   "Running" },
    { "discord.exe",         1.8f, 120.7f,   "Running" },
    { "spotify.exe",         0.9f,  95.2f,   "Running" },
    { "taskmgr.exe",         0.4f,  28.6f,   "Running" },
    { "winlogon.exe",        0.0f,   8.4f,   "Running" },
    { "lsass.exe",           0.1f,  16.3f,   "Running" },
    { "RuntimeBroker.exe",   0.2f,  22.1f,   "Running" },
    { "SearchIndexer.exe",   0.7f,  48.9f,   "Suspended" },
    { "OneDrive.exe",        0.5f,  55.0f,   "Running" },
};
static const int NUM_PROCESSES = sizeof(DUMMY_PROCESSES) / sizeof(DUMMY_PROCESSES[0]);

// --- Notes App Placeholder Content ---
static const char* NOTES_PLACEHOLDER =
    "Welcome to CSOPESY Desktop OS!\n\n"
    "This is a placeholder Notes application.\n"
    "You can type here to test the text editor.\n\n"
    "--- Meeting Notes ---\n"
    "1. Discuss project timeline\n"
    "2. Review OS architecture\n"
    "3. Assign module tasks\n";

// --- System Info Placeholder Content ---
static const char* SYSINFO_OS_NAME     = "CSOPESY Desktop OS v1.0";
static const char* SYSINFO_KERNEL      = "ImGui Kernel 1.91.9";
static const char* SYSINFO_CPU         = "Intel Core i7-13700K @ 5.4GHz";
static const char* SYSINFO_GPU         = "OpenGL 3.0 (GLFW Backend)";
static const char* SYSINFO_RAM         = "16384 MB DDR5";
static const char* SYSINFO_RESOLUTION  = "1280 x 800";

// =============================================================================
// END CONFIGURABLE PARAMETERS
// =============================================================================

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + generally GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + generally GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(WINDOW_WIDTH * main_scale), (int)(WINDOW_HEIGHT * main_scale), WINDOW_TITLE, nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // =========================================================================
    // APPLICATION STATE
    // =========================================================================
    bool app_running       = true;   // Controlled by PWR button
    bool show_notes        = false;  // Taskbar App 1
    bool show_sysinfo      = false;  // Taskbar App 2
    bool show_taskmgr      = false;  // Taskbar App 3 (Task Manager)

    // Notes app text buffer
    static char notes_buffer[4096];
    snprintf(notes_buffer, sizeof(notes_buffer), "%s", NOTES_PLACEHOLDER);

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window) && app_running)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get display size for layout calculations
        ImVec2 display_size = io.DisplaySize;

        // =====================================================================
        // --- Desktop --- (Base layer: wallpaper drawn on background draw list)
        // =====================================================================
        {
            // Use the background draw list — always renders behind all ImGui windows
            ImDrawList* bg = ImGui::GetBackgroundDrawList();

            // --- Draw gradient wallpaper ---
            ImU32 col_top    = ImGui::ColorConvertFloat4ToU32(WALLPAPER_COLOR_TOP);
            ImU32 col_bottom = ImGui::ColorConvertFloat4ToU32(WALLPAPER_COLOR_BOTTOM);
            bg->AddRectFilledMultiColor(
                ImVec2(0, 0), display_size,
                col_top, col_top, col_bottom, col_bottom);

            // --- Draw decorative pattern (subtle grid lines) ---
            ImU32 grid_color = IM_COL32(255, 255, 255, 8);
            float grid_spacing = 60.0f;
            for (float x = 0; x < display_size.x; x += grid_spacing)
                bg->AddLine(ImVec2(x, 0), ImVec2(x, display_size.y), grid_color);
            for (float y = 0; y < display_size.y; y += grid_spacing)
                bg->AddLine(ImVec2(0, y), ImVec2(display_size.x, y), grid_color);
        }

        // --- Clock overlay (bottom-right corner, above taskbar) ---
        {
            time_t now = time(nullptr);
            struct tm* local_time = localtime(&now);
            char time_str[64];
            char date_str[128];
            strftime(time_str, sizeof(time_str), CLOCK_FORMAT, local_time);
            strftime(date_str, sizeof(date_str), DATE_FORMAT, local_time);

            // Calculate text sizes using font directly (safe outside Begin/End)
            ImFont* font = ImGui::GetFont();
            float base_font_size = ImGui::GetFontSize();
            ImVec2 time_size = font->CalcTextSizeA(base_font_size * CLOCK_FONT_SCALE, FLT_MAX, 0.0f, time_str);
            ImVec2 date_size = font->CalcTextSizeA(base_font_size * DATE_FONT_SCALE, FLT_MAX, 0.0f, date_str);

            float block_w = (time_size.x > date_size.x) ? time_size.x : date_size.x;
            float block_h = time_size.y + date_size.y + 4.0f;
            float clock_x = display_size.x - block_w - CLOCK_MARGIN;
            float clock_y = display_size.y - TASKBAR_HEIGHT - block_h - CLOCK_MARGIN;

            // Draw text via foreground draw list (always on top, no window needed)
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            ImU32 time_col = ImGui::ColorConvertFloat4ToU32(CLOCK_TEXT_COLOR);
            ImU32 date_col = ImGui::ColorConvertFloat4ToU32(DATE_TEXT_COLOR);

            // Right-align time
            float time_x = display_size.x - CLOCK_MARGIN - time_size.x;
            dl->AddText(font, base_font_size * CLOCK_FONT_SCALE,
                ImVec2(time_x, clock_y), time_col, time_str);

            // Right-align date below time
            float date_x = display_size.x - CLOCK_MARGIN - date_size.x;
            dl->AddText(font, base_font_size * DATE_FONT_SCALE,
                ImVec2(date_x, clock_y + time_size.y + 4.0f), date_col, date_str);
        }

        // --- PWR Button (top-right corner) ---
        {
            float pwr_x = display_size.x - PWR_BUTTON_SIZE - CLOCK_MARGIN;
            float pwr_y = CLOCK_MARGIN;
            ImGui::SetNextWindowPos(ImVec2(pwr_x - 5.0f, pwr_y - 5.0f));
            ImGui::SetNextWindowSize(ImVec2(PWR_BUTTON_SIZE + 10.0f, PWR_BUTTON_SIZE + 10.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, PWR_BUTTON_COLOR);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PWR_BUTTON_HOVER);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, PWR_BUTTON_ACTIVE);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, PWR_BUTTON_SIZE * 0.5f); // Circular button

            ImGui::Begin("##PWR", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::Button("PWR", ImVec2(PWR_BUTTON_SIZE, PWR_BUTTON_SIZE)))
            {
                app_running = false; // Graceful shutdown via PWR button
            }

            ImGui::End();
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(4);
        }

        // =====================================================================
        // --- Taskbar --- (Fixed bottom panel with app launchers)
        // =====================================================================
        {
            float taskbar_y = display_size.y - TASKBAR_HEIGHT;
            ImGui::SetNextWindowPos(ImVec2(0, taskbar_y));
            ImGui::SetNextWindowSize(ImVec2(display_size.x, TASKBAR_HEIGHT));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, TASKBAR_COLOR);
            ImGui::PushStyleColor(ImGuiCol_Button, TASKBAR_BTN_COLOR);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TASKBAR_BTN_HOVER);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, TASKBAR_BTN_ACTIVE);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            ImGui::Begin("##Taskbar", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNav);

            // Button 1: Notes
            if (ImGui::Button(APP1_NAME, ImVec2(TASKBAR_BUTTON_WIDTH, TASKBAR_BUTTON_HEIGHT)))
                show_notes = !show_notes;

            ImGui::SameLine();

            // Button 2: System Info
            if (ImGui::Button(APP2_NAME, ImVec2(TASKBAR_BUTTON_WIDTH, TASKBAR_BUTTON_HEIGHT)))
                show_sysinfo = !show_sysinfo;

            ImGui::SameLine();

            // Button 3: Task Manager
            if (ImGui::Button(APP3_NAME, ImVec2(TASKBAR_BUTTON_WIDTH, TASKBAR_BUTTON_HEIGHT)))
                show_taskmgr = !show_taskmgr;

            // Show clock in taskbar (right side)
            {
                time_t now = time(nullptr);
                struct tm* lt = localtime(&now);
                char tb_time[32];
                strftime(tb_time, sizeof(tb_time), "%H:%M", lt);
                ImVec2 text_size = ImGui::CalcTextSize(tb_time);
                ImGui::SameLine(display_size.x - text_size.x - 20.0f);
                ImGui::SetCursorPosY((TASKBAR_HEIGHT - text_size.y) * 0.5f);
                ImGui::Text("%s", tb_time);
            }

            ImGui::End();
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(4);
        }

        // =====================================================================
        // --- App Windows --- (Opened by taskbar buttons)
        // =====================================================================

        // --- Notes App Window ---
        if (show_notes)
        {
            ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(80, 60), ImGuiCond_FirstUseEver);
            ImGui::Begin("Notes", &show_notes);
            ImGui::Text("Simple Notepad");
            ImGui::Separator();
            ImGui::InputTextMultiline("##NotesEditor", notes_buffer, sizeof(notes_buffer),
                ImVec2(-1.0f, -1.0f));
            ImGui::End();
        }

        // --- System Info Window ---
        if (show_sysinfo)
        {
            ImGui::SetNextWindowSize(ImVec2(420, 280), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_FirstUseEver);
            ImGui::Begin("System Information", &show_sysinfo);
            ImGui::Text("About This Computer");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("OS:         %s", SYSINFO_OS_NAME);
            ImGui::Text("Kernel:     %s", SYSINFO_KERNEL);
            ImGui::Text("CPU:        %s", SYSINFO_CPU);
            ImGui::Text("GPU:        %s", SYSINFO_GPU);
            ImGui::Text("Memory:     %s", SYSINFO_RAM);
            ImGui::Text("Display:    %s", SYSINFO_RESOLUTION);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Application average %.1f FPS", io.Framerate);
            ImGui::End();
        }

        // =====================================================================
        // --- Task Manager ---
        // =====================================================================
        if (show_taskmgr)
        {
            ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(350, 80), ImGuiCond_FirstUseEver);
            ImGui::Begin("Task Manager", &show_taskmgr);

            // Summary bar
            float total_cpu = 0.0f;
            float total_mem = 0.0f;
            for (int i = 0; i < NUM_PROCESSES; i++) {
                total_cpu += DUMMY_PROCESSES[i].cpuPercent;
                total_mem += DUMMY_PROCESSES[i].memoryMB;
            }
            ImGui::Text("Processes: %d    CPU: %.1f%%    Memory: %.0f MB", NUM_PROCESSES, total_cpu, total_mem);
            ImGui::Separator();

            // Process table
            if (ImGui::BeginTable("ProcessTable", 4,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_SizingStretchProp))
            {
                // Header
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Process Name",  ImGuiTableColumnFlags_None, 3.0f);
                ImGui::TableSetupColumn("CPU %",         ImGuiTableColumnFlags_None, 1.0f);
                ImGui::TableSetupColumn("Memory (MB)",   ImGuiTableColumnFlags_None, 1.5f);
                ImGui::TableSetupColumn("Status",        ImGuiTableColumnFlags_None, 1.2f);
                ImGui::TableHeadersRow();

                // Rows
                for (int i = 0; i < NUM_PROCESSES; i++)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", DUMMY_PROCESSES[i].name);

                    ImGui::TableSetColumnIndex(1);
                    // Color-code high CPU usage
                    if (DUMMY_PROCESSES[i].cpuPercent > 5.0f)
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%.1f%%", DUMMY_PROCESSES[i].cpuPercent);
                    else
                        ImGui::Text("%.1f%%", DUMMY_PROCESSES[i].cpuPercent);

                    ImGui::TableSetColumnIndex(2);
                    if (DUMMY_PROCESSES[i].memoryMB > 100.0f)
                        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "%.1f", DUMMY_PROCESSES[i].memoryMB);
                    else
                        ImGui::Text("%.1f", DUMMY_PROCESSES[i].memoryMB);

                    ImGui::TableSetColumnIndex(3);
                    if (strcmp(DUMMY_PROCESSES[i].status, "Suspended") == 0)
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", DUMMY_PROCESSES[i].status);
                    else
                        ImGui::Text("%s", DUMMY_PROCESSES[i].status);
                }
                ImGui::EndTable();
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
