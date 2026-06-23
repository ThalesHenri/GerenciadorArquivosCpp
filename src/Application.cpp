#include <iostream>
#include "Application.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <cstdio>

Application::Application(GLFWwindow* win) : window(win), monitor(100) {}

void Application::render_sidebar(int display_w, int display_h) {
    ImGui::BeginChild("Sidebar", ImVec2(300, 0), true);
    
    // --- SEÇÃO HARDWARE ---
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "STATUS DO SISTEMA");
    ImGui::Separator();
    ImGui::Spacing();

    char cpu_label[32];
    sprintf(cpu_label, "CPU: %.1f%%", monitor.get_current_cpu() * 100.0f);
    ImGui::PlotLines("##CPU", monitor.get_cpu_data(), monitor.get_samples_count(), 
                     monitor.get_offset(), cpu_label, 0.0f, 100.0f, ImVec2(0, 60));
    
    ImGui::Spacing();

    char ram_label[32];
    sprintf(ram_label, "RAM: %.1f%%", monitor.get_current_ram() * 100.0f);
    ImGui::PlotLines("##RAM", monitor.get_ram_data(), monitor.get_samples_count(), 
                     monitor.get_offset(), ram_label, 0.0f, 100.0f, ImVec2(0, 60));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- SEÇÃO ARMAZENAMENTO (NOVO!) ---
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "DISCO LOCAL");
    ImGui::Separator();
    

    auto space = file_controller.get_disk_space_info();
    uint64_t used_space = space.capacity - space.free;
    float used_ratio = space.capacity > 0 ? (float)used_space / space.capacity : 0.0f;



    std::string total_str = Application::format_bytes(static_cast<unsigned long>(space.capacity));
    std::string livre_str = Application::format_bytes(static_cast<unsigned long>(space.free));

    ImGui::Text("Total: %s", total_str.c_str());
    ImGui::Text("Livre: %s", livre_str.c_str());

    // Barra de progresso visual do uso do disco
    char progress_label[32];
    sprintf(progress_label, "Uso: %.1f%%", used_ratio * 100.0f);
    ImGui::ProgressBar(used_ratio, ImVec2(-1, 20), progress_label);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- SEÇÃO OTIMIZAÇÃO ---
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "OTIMIZAÇÃO");
    ImGui::Spacing();

    // 1. Botão de Temporários
    if (ImGui::Button("Limpar Cache (Temporários)", ImVec2(-1, 35))) {
        modal_title = "Limpeza de Temporários";
        show_process_modal = true;
        is_optimizing = true; // Ativa o estado de progresso imediatamente
        
        std::thread([this]() {
            this->file_controller.clear_temporary_files(
                this->is_optimizing, 
                this->optimization_progress, 
                this->progress_text
            );
            
            std::lock_guard<std::mutex> lock(this->mtx);

            // Quando a função pesada termina, apenas atualiza estados simples
            this->optimization_status = "Limpeza de temporários concluída!";
            this->is_optimizing = false;
            this->atualizar_diretorio = true; 
        }).detach();
    }
    
    ImGui::Spacing();
    
    // 2. Botão de Lixeira
    if (ImGui::Button("Esvaziar Lixeira", ImVec2(-1, 35))) {
        modal_title = "Esvaziar Lixeira";
        show_process_modal = true;
        is_optimizing = true;
        
        std::thread([this]() {
            this->file_controller.empty_trash(
                this->is_optimizing, 
                this->optimization_progress, 
                this->progress_text
            );

            std::lock_guard<std::mutex> lock(this->mtx);

            
            this->optimization_status = "Lixeira esvaziada com sucesso!";
            this->is_optimizing = false;
            this->atualizar_diretorio = true;
        }).detach();
    }

    ImGui::Spacing();

    // 3. Botão de Snaps
    if (ImGui::Button("Limpar Snaps Antigos", ImVec2(-1, 35))) {
        modal_title = "Limpeza de Snaps";
        show_process_modal = true;
        is_optimizing = true;
        
        std::thread([this]() {
            this->file_controller.clear_old_snaps(
                this->is_optimizing, 
                this->optimization_progress, 
                this->progress_text
            );

            std::lock_guard<std::mutex> lock(this->mtx);
            
            this->optimization_status = "Limpeza de Snaps concluída!";
            this->is_optimizing = false;
            this->atualizar_diretorio = true;
        }).detach();
    }

    ImGui::EndChild();
    if (atualizar_diretorio) {
        file_controller.set_current_path(file_controller.get_current_path_string());
        atualizar_diretorio = false; // Desliga a flag para não atualizar em loop
    }
    if (show_process_modal && !ImGui::IsPopupOpen("Resultado da Operacao")) {
        ImGui::OpenPopup("Resultado da Operacao");
    }
 

    if (ImGui::BeginPopupModal("Resultado da Operacao", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        std::lock_guard<std::mutex> lock(this->mtx);
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.8f, 1.0f), "%s", modal_title.c_str());
        ImGui::Separator();
        ImGui::Spacing();

        if (is_optimizing) {
            // --- SE ESTIVER RODANDO: Mostra o texto atual e a Barra de Progresso ---
            ImGui::Text("%s", progress_text.c_str());
            ImGui::Spacing();
            
            // Desenha a barra de progresso (Largura: 300px, Altura: 25px)
            char percentage[32];
            sprintf(percentage, "%.0f%%", optimization_progress * 100.0f);
            ImGui::ProgressBar(optimization_progress, ImVec2(300, 25), percentage);
        } else {
            // --- SE TERMINOU: Mostra o resultado final e o botão de fechar ---
            ImGui::Text("%s", optimization_status.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                show_process_modal = false;
                optimization_status = "";
            }
        }
        
        ImGui::EndPopup();
    }

}
// Expllorador de arquivos
void Application::render_file_browser() {
    // Abre a área direita ocupando o resto da tela
    ImGui::BeginChild("FileBrowser", ImVec2(0, 0), true);
    
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "EXPLORADOR DE ARQUIVOS");
    ImGui::Separator();
    ImGui::Spacing();

    // --- BARRA DE NAVEGAÇÃO SUPERIOR ---
    if (ImGui::Button(" <- Voltar ", ImVec2(100, 25))) {
        file_controller.navigate_up();
        file_controller.clear_selection(); // Limpa a seleção ao mudar de diretório
    }
    ImGui::SameLine();
    ImGui::Text("Diretório: %s", file_controller.get_current_path_string().c_str());
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- TABELA DE ARQUIVOS ---
    // Criamos 3 colunas: Nome, Tipo, Tamanho.
    // ImGuiTableFlags_ScrollY permite que a tabela tenha barra de rolagem se tiver muitos arquivos
    if (ImGui::BeginTable("FilesTable", 3, ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, -40))) {
        
        ImGui::TableSetupColumn("Nome", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Tamanho", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        // Pega a lista de arquivos reais através do nosso FileController
        const std::vector<FileInfo>& items = file_controller.get_cached_files();
        for (const auto& item : items) {
            ImGui::TableNextRow();
            
            // Verifica se este item específico está selecionado atualmente
            bool is_selected = (file_controller.get_selected_file() == item.name);

            // --- COLUNA 1: NOME COM SELEÇÃO E DUPLO CLIQUE ---
            ImGui::TableSetColumnIndex(0);
            
            // Diferenciação visual de cores para pastas e arquivos
            if (item.is_directory) {
                ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.3f, 1.0f), "[Pasta]");
            } else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[Arq]  ");
            }
            ImGui::SameLine();

            // O Selectable cria uma linha clicável na tabela do ImGui
            // O ID precisa ser único, então passamos o nome do arquivo/pasta
            if (ImGui::Selectable(item.name.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns)) {
                
                // Clique Único: Seleciona o arquivo
                file_controller.select_file(item.name);
                
                // Clique Duplo: Se for diretório, navega para dentro dele
                if (ImGui::IsMouseDoubleClicked(0) && item.is_directory) {
                    std::string separator = "/";
                    // Se estiver no Windows, ajustamos o separador de caminho se necessário, mas o <filesystem> aceita / nativamente
                    std::string next_path = file_controller.get_current_path_string() + separator + item.name;
                    file_controller.set_current_path(next_path);
                    file_controller.clear_selection();
                }
            }

            // --- COLUNA 2: TIPO ---
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", item.is_directory ? "Diretório" : "Arquivo");

            // --- COLUNA 3: TAMANHO ---
            ImGui::TableSetColumnIndex(2);
            if (item.is_directory && item.size == 0) {

                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Calculando..."); 

            } else {
        // Mostra o tamanho real e formatado (seja de um arquivo comum ou de uma pasta já calculada)
                ImGui::Text("%s", Application::format_bytes(static_cast<unsigned long>(item.size)).c_str());
            }
        }
        ImGui::EndTable();
    }

   ImGui::Separator();
    std::string selecionado = file_controller.get_selected_file();
    
    if (!selecionado.empty()) {
        ImGui::Text("Item Selecionado: %s", selecionado.c_str());
        
        ImGui::SameLine();
        // Botão vermelho de deletar
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        
        if (ImGui::Button("Deletar Selecionado", ImVec2(160, 23))) {
            ImGui::OpenPopup("Confirmar Exclusão");
        }
        ImGui::PopStyleColor(3);

        // POPUP DE CONFIRMAÇÃO
        if (ImGui::BeginPopupModal("Confirmar Exclusão", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Tem certeza que deseja deletar permanentemente:\n\"%s\"?", selecionado.c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Esta ação não poderá ser desfeita!");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Sim, Deletar", ImVec2(120, 30))) {
                // Executa a deleção no controller
                file_controller.delete_selected_item();
                // Ativa a flag que você já tem para atualizar a lista de arquivos na tela
                this->atualizar_diretorio = true; 
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancelar", ImVec2(120, 30))) {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    } else {
        ImGui::Text("Nenhum item selecionado.");
    }

    ImGui::EndChild();
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        monitor.update(glfwGetTime());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
                                       ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("Gerenciador Mestre", nullptr, window_flags);

        render_sidebar(display_w, display_h);
        
        ImGui::SameLine();
        render_file_browser();

        ImGui::End();
       

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}


std::string Application::format_bytes(unsigned long bytes) {
    double size = static_cast<double>(bytes);
    std::string units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_idx = 0;
    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit_idx].c_str());
    return std::string(buffer);
}