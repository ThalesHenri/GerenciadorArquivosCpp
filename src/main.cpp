#include <iostream>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Application.hpp"
#define WIDTH 1200
#define HEIGHT 600


int main() {
    if(!glfwInit()) return -1;

    // Configurando OpeGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //ponteiro
    GLFWwindow* window = glfwCreateWindow(WIDTH,HEIGHT, "Gerenciador Mestre",nullptr ,nullptr);
    if (!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); //V-Sync
    

    //Inicializando o ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark(); //Tema Escuro

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); //Inicializa de fato o OpenGL


    //Loop de Renderizaçao
    //Chaves sem função, pos em C++ isso funciona como um escopo, e isso é útil para organizar o código, 
    //Tendo impacto mínimo na performance, pois o compilador otimiza isso muito bem.
    {
        Application app(window);
        app.run();

    }
    
    //Limpeza, pois no C++ temos que matar tudo que inicializamos
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
