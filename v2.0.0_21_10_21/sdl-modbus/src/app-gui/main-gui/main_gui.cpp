/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/
#include "app-gui/main-gui/main_gui.h"
#include "stb/stb_image.h"
#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int app_main_gui_t::imgui_main_window()
{
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);

  if(!glfwInit()) {
    // Log errors
    fprintf(stderr, "Failed to initialize glfwInit() !\n");
    exit(0);
  }

  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  //
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Vidcentum Smart Data Logger + Advisor Software Unit", NULL, NULL);

  if(window == NULL) {
    // Log errors
    fprintf(stderr, "Failed to initialize glfwCreateWindow() !\n");
    exit(0);
  }

  GLFWimage images[1];
  images[0].pixels = stbi_load("../config/images/logo.png", &images[0].width, &images[0].height, 0, 4); // rgba channels
  glfwSetWindowIcon(window, 1, images);
  stbi_image_free(images[0].pixels);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
  if(gl3wInit()) {
    // Log errors
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    exit(0);
  }

  if(!gl3wIsSupported(4, 6)) {
    fprintf(stderr, "OpenGL 4.6 not supported\n");
    return -1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  io.Fonts->AddFontFromFileTTF("../fonts/Roboto-Medium.ttf", 16.0f);
  
  // merge in icons from Font Awesome
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig icons_config;
  icons_config.MergeMode = true;
  icons_config.PixelSnapH = true;
  icons_config.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
  
  io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, 14.0f, &icons_config, icons_ranges );
  // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

  static const ImWchar md_icons_ranges [] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
  ImFontConfig md_icons_config;
  md_icons_config.MergeMode = true;
  md_icons_config.PixelSnapH = true;
  md_icons_config.GlyphMinAdvanceX = 22.0f; // Use if you want to make the icon monospaced
  
  io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_MD, 22.0f, &md_icons_config, md_icons_ranges );
  

  io.Fonts->Build();

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  
  // Help Menu
  bool show_help_about = false;

  ImGui::CreateContext();

  ImNodesContext* g_logger_canvas_context = ImNodes::CreateContext();

  if(!g_logger_canvas_context) {
    fprintf(stderr, "Failed to initialize Logger help node editor canvas context!\n");
    return 1;
  }

  ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
  float height = ImGui::GetFrameHeight();

  // System Menu State Variables.
  bool show_system_quit = false;
  bool show_system_reload_config = false;

  // Main loop
  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Top level window to navidate to the modbus devices and registers etc.
    if(ImGui::BeginViewportSideBar(
           "Smart Data Logger + Advisor Software Unit", viewport, ImGuiDir_Up, height, window_flags)) {
      // Menu Bar
      if(ImGui::BeginMenuBar()) {
        // Quit Menu
        if(ImGui::BeginMenu(ICON_FA_DRAFTING_COMPASS " System")) {

          ImGui::Separator();
          if(ImGui::Button( ICON_FA_FILE " Backup Dataset (No Truncate)")) {

            ImGui::OpenPopup("Backup Dataset (No Truncate)?");
          }

          // Always center this window when appearing
          ImVec2 center = ImGui::GetMainViewport()->GetCenter();
          ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

          if(ImGui::BeginPopupModal("Backup Dataset (No Truncate)?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Dataset file will be backed-up.\nThe working dataset will not be truncated!\n\n");
            ImGui::Separator();

            if(ImGui::Button("OK", ImVec2(120, 0))) {
              if(mb_h5_) {
                mb_h5_->backup_dataset_file_async();
              }

              ImGui::CloseCurrentPopup();
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if(ImGui::Button("Cancel", ImVec2(120, 0))) {
              ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
          }

          ImGui::Separator();

          ImGui::Separator();
          ImGui::MenuItem("Reload Config", NULL, &show_system_reload_config);
          ImGui::Separator();

          ImGui::Separator();
          ImGui::MenuItem("Quit", NULL, &show_system_quit);
          ImGui::Separator();

          ImGui::EndMenu();
        }

        // Help Menu
        if(ImGui::BeginMenu("Help")) {
          ImGui::MenuItem("About", NULL, &show_help_about);
          ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
      }

      // IoT Device Menu
      mb_gui_->modbus_master_menu();

      ImGui::End();
    }

    // System Menu functions
    {
      if(show_system_reload_config) {

        ImGui::OpenPopup("Reload config?");
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if(ImGui::BeginPopupModal("Reload config?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
          ImGui::Text("Any data in transit will be discarded!\n\n");
          ImGui::Separator();

          if(ImGui::Button("OK", ImVec2(120, 0))) {
            // All is well.
            exit(0);
            ImGui::CloseCurrentPopup();
          }

          ImGui::SetItemDefaultFocus();
          ImGui::SameLine();

          if(ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_system_reload_config = false;
            ImGui::CloseCurrentPopup();
          }

          ImGui::EndPopup();
        }
      }

      if(show_system_quit) {
        // All is well.
        auto sr = std::system("pm2 del sdl-lite-modbus");
        exit(sr);
      }
    }
    // About Window
    {
      if(show_help_about) {
        
        ImGui::ShowDemoWindow();

        /*
        ImGui::OpenPopup("About Smart Data Logger Lite (Modbus)");

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if(ImGui::BeginPopupModal("About Smart Data Logger Lite (Modbus)", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
          ImGui::Text("Vidcentum Smart Data Logger Platform!\n\n");
          ImGui::Text("Version 1.0.8 Build 06-June-2021\n\n");

          ImGui::SetItemDefaultFocus();

          if(ImGui::Button("OK", ImVec2(120, 0))) {
            show_help_about = false;
            ImGui::CloseCurrentPopup();
          }

          ImGui::SameLine();

          if(ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_help_about = false;
            ImGui::CloseCurrentPopup();
          }

          ImGui::EndPopup();
        }
        */
      }
    }

    {
      ImGuiIO& io = ImGui::GetIO();
      // Set the window position
      ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
      // Set the size of the window
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
      // Set the window to be transparent
      ImGui::SetNextWindowBgAlpha(0);
      // Texture ID
      static GLuint bg_tex_id = 0;
      static int my_image_width = 0;
      static int my_image_height = 0;

      if(!bg_tex_id) {
        load_texture_from_file("../config/images/bg.jpg", &bg_tex_id, &my_image_width, &my_image_height);
      }

      // Set the padding of the window to 0 to fill the window with the picture control
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      // Set the window to no border
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
      // Create a window to fix it in a position
      ImGui::Begin("background", NULL,
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
              ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
      ImGui::Image((void*)(intptr_t)bg_tex_id, ImVec2(1920, 1080));
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    // Modbus Nodes ...
    mb_gui_->modbus_master_window();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  exit(0);
}

bool app_main_gui_t::load_texture_from_file(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);

  if(image_data == NULL)
    return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

void app_main_gui_t::set_style_seaborn()
{

  ImPlotStyle& style = ImPlot::GetStyle();

  ImVec4* colors = style.Colors;
  colors[ImPlotCol_Line] = IMPLOT_AUTO_COL;
  colors[ImPlotCol_Fill] = IMPLOT_AUTO_COL;
  colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
  colors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;
  colors[ImPlotCol_ErrorBar] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_PlotBg] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
  colors[ImPlotCol_PlotBorder] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImPlotCol_LegendBg] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
  colors[ImPlotCol_LegendBorder] = ImVec4(0.80f, 0.81f, 0.85f, 1.00f);
  colors[ImPlotCol_LegendText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_TitleText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_InlayText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_XAxis] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_XAxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_YAxis] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_YAxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_YAxis2] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_YAxisGrid2] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_YAxis3] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_YAxisGrid3] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_Selection] = ImVec4(1.00f, 0.65f, 0.00f, 1.00f);
  colors[ImPlotCol_Query] = ImVec4(0.23f, 0.10f, 0.64f, 1.00f);
  colors[ImPlotCol_Crosshairs] = ImVec4(0.23f, 0.10f, 0.64f, 0.50f);

  style.LineWeight = 1.5;
  style.Marker = ImPlotMarker_None;
  style.MarkerSize = 4;
  style.MarkerWeight = 1;
  style.FillAlpha = 1.0f;
  style.ErrorBarSize = 5;
  style.ErrorBarWeight = 1.5f;
  style.DigitalBitHeight = 8;
  style.DigitalBitGap = 4;
  style.PlotBorderSize = 0;
  style.MinorAlpha = 1.0f;
  style.MajorTickLen = ImVec2(0, 0);
  style.MinorTickLen = ImVec2(0, 0);
  style.MajorTickSize = ImVec2(0, 0);
  style.MinorTickSize = ImVec2(0, 0);
  style.MajorGridSize = ImVec2(1.2f, 1.2f);
  style.MinorGridSize = ImVec2(1.2f, 1.2f);
  style.PlotPadding = ImVec2(12, 12);
  style.LabelPadding = ImVec2(5, 5);
  style.LegendPadding = ImVec2(5, 5);
  style.MousePosPadding = ImVec2(5, 5);
  style.PlotMinSize = ImVec2(300, 225);
}