#pragma once

#include <GLFW/glfw3.h>
#include "SystemMonitor.hpp"
#include "FileController.hpp"
#include <string>
#include <mutex>

class Application {
private:
    GLFWwindow* window;
    SystemMonitor monitor;
    FileController file_controller;

    void render_sidebar(int display_w, int display_h);
    void render_file_browser();
    std::string optimization_status;
    std::string modal_title;
    bool show_process_modal = false;
    bool is_optimizing = false;       
    float optimization_progress = 0.0f; 
    std::string progress_text = "";    
    std::string status_buffer;
    std::mutex mtx;
    bool atualizar_diretorio = false;

public:
    Application(GLFWwindow* win);
    static std::string format_bytes(unsigned long bytes);
    void run();
};

