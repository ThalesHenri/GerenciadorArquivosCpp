#include "FileController.hpp"
#include <iostream>
#include <cstdlib>

FileController::FileController() {
    current_path = std::filesystem::current_path();
    refresh_cache();
}

void FileController::set_current_path(const std::string& path_str) {
    if(std::filesystem::exists(path_str) && std::filesystem::is_directory(path_str)) {
        current_path = path_str;
        refresh_cache();
    }
}

std::string FileController::get_current_path_string() const {
    return current_path.string();
}

std::vector<FileInfo> FileController::get_cached_files() const {
    return cached_files;
}

std::filesystem::space_info FileController::get_disk_space_info() {
    // Retorna uma estrutura com .capacity, .free e .available em bytes
    try {
        return std::filesystem::space(current_path.root_path());
    } catch (...) {
        return std::filesystem::space_info{0, 0, 0};
    }
}

std::string FileController::clear_temporary_files(bool& is_running, float& progress, std::string& current_action) {
    is_running = true;
    progress = 0.0f;
    current_action = "Mapeando arquivos de cache...";

    const char* home = std::getenv("HOME");
    if (!home) {
        is_running = false;
        return "Erro: Diretório HOME não encontrado.";
    }
    
    std::filesystem::path thumbnails_path = std::filesystem::path(home) / ".cache/thumbnails";
    std::vector<std::filesystem::path> paths_to_delete;
    
    // 1. Mapeia todos os itens primeiro para saber o total
    try {
        if (std::filesystem::exists(thumbnails_path)) {
            for (const auto& entry : std::filesystem::directory_iterator(thumbnails_path)) {
                paths_to_delete.push_back(entry.path());
            }
        }
    } catch (...) {}
    
    if (paths_to_delete.empty()) {
        is_running = false;
        progress = 1.0f;
        return "Cache já estava limpo!";
    }

    // 2. Loop de deleção atualizando a interface
    int arquivos_removidos = 0;
    size_t total = paths_to_delete.size();

    for (size_t i = 0; i < total; ++i) {
        current_action = "Apagando: " + paths_to_delete[i].filename().string();
        progress = static_cast<float>(i) / static_cast<float>(total);

        try {
            arquivos_removidos += std::filesystem::remove_all(paths_to_delete[i]);
        } catch (...) {}
    }
    
    progress = 1.0f;
    is_running = false;
    return std::to_string(arquivos_removidos) + " arqs. temporários limpos.";
}

std::string FileController::empty_trash(bool& is_running, float& progress, std::string& current_action) {
    is_running = true;
    progress = 0.0f;
    current_action = "Analisando lixeira...";

    const char* home = std::getenv("HOME");
    if (!home) {
        is_running = false;
        return "Erro: Diretório HOME não encontrado.";
    }
    
    std::filesystem::path trash_files = std::filesystem::path(home) / ".local/share/Trash/files";
    std::filesystem::path trash_info = std::filesystem::path(home) / ".local/share/Trash/info";
    std::vector<std::filesystem::path> paths_to_delete;
    
    // Lambda para coletar os caminhos sem repetir código
    auto collect_paths = [&paths_to_delete](const std::filesystem::path& dir) {
        try {
            if (std::filesystem::exists(dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                    paths_to_delete.push_back(entry.path());
                }
            }
        } catch (...) {}
    };
    
    // 1. Lista o que tem na lixeira (arquivos e metadados)
    collect_paths(trash_files);
    collect_paths(trash_info);
    
    if (paths_to_delete.empty()) {
        is_running = false;
        progress = 1.0f;
        return "A lixeira já está vazia.";
    }
    
    // 2. Loop de limpeza com progresso
    int itens_deletados = 0;
    size_t total = paths_to_delete.size();

    for (size_t i = 0; i < total; ++i) {
        current_action = "Excluindo: " + paths_to_delete[i].filename().string();
        progress = static_cast<float>(i) / static_cast<float>(total);

        try {
            itens_deletados += std::filesystem::remove_all(paths_to_delete[i]);
        } catch (...) {}
    }
    
    progress = 1.0f;
    is_running = false;
    return std::to_string(itens_deletados) + " itens excluídos para sempre.";
}

void FileController::navigate_up(){
    current_path = current_path.parent_path();
    refresh_cache();
}

void FileController::clear_selection() {
    selected_file = "";
}

void FileController::select_file(const std::string& file_name) {
    selected_file = file_name;
}

std::string FileController::get_selected_file() const {
    return selected_file;
}

std::uintmax_t FileController::calculate_directory_size(const std::filesystem::path& dir_path)const{
    std::uintmax_t total_size = 0;
    try {
            if (std::filesystem::exists(dir_path) && std::filesystem::is_directory(dir_path)) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path, std::filesystem::directory_options::skip_permission_denied)) {
                    if (std::filesystem::is_regular_file(entry.status())) {
                        total_size += std::filesystem::file_size(entry);
                    }
                }
            }
        } catch (...) {
            
        }
        
        return total_size;
}

void FileController::refresh_cache() {
    cached_files.clear();
    try{
        for (const auto& entry : std::filesystem::directory_iterator(current_path)){
            FileInfo info;
            info.name = entry.path().filename().string();
            info.is_directory = entry.is_directory();

            if(!info.is_directory){
                info.size = entry.file_size();
            }
            cached_files.push_back(info);
        }
        for (size_t i = 0; i < cached_files.size(); ++i) {
            if (cached_files[i].is_directory) {
                std::filesystem::path folder_path = current_path / cached_files[i].name;

                // Criamos uma thread nativa e descolamos (.detach) ela da main thread
                std::thread([this, i, folder_path]() {
                    std::uintmax_t size = this->calculate_directory_size(folder_path);
                    
                    // Altera os dados no cache guardado em memória
                    if (i < this->cached_files.size()) {
                        this->cached_files[i].size = size;
                        this->cached_files[i].size_ready = true; // Avisa que terminou!
                    }
                }).detach(); 
            }
        }

    }
    catch(const std::exception& e){
        std::cerr << "Erro ao atualizar o cache: " << e.what() << std::endl;
    }
}

std::string FileController::clear_old_snaps(bool& is_running, float& progress, std::string& current_action) {
    is_running = true;
    progress = 0.0f;
    current_action = "Aguardando autenticação...";

    // Criamos um script Bash completo dentro de um ÚNICO pkexec.
    // O uso de R"(...)" (Raw String) facilita lidar com as aspas no C++.
    // Usamos '\'' para injetar aspas simples seguras para o awk dentro do bash -c.
    std::string comando = R"(pkexec bash -c '
        snaps=$(snap list --all | awk '\''/disabled/{print $1 "," $3}'\'')
        if [ -z "$snaps" ]; then
            echo "TOTAL:0"
            exit 0
        fi
        
        total=$(echo "$snaps" | wc -l)
        echo "TOTAL:$total"
        
        atual=0
        echo "$snaps" | while IFS="," read -r name rev; do
            echo "REMOVANDO:$name:$rev"
            snap remove "$name" --revision="$rev" >/dev/null 2>&1
            atual=$((atual + 1))
            echo "PROGRESSO:$atual"
        done
    ')";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(comando.c_str(), "r"), pclose);
    if (!pipe) {
        is_running = false;
        return "Erro ao iniciar o processo de limpeza.";
    }

    std::array<char, 256> buffer;
    int total_snaps = -1;
    int removidos = 0;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string linha(buffer.data());
        
        // Remove quebras de linha residuais
        if (!linha.empty() && linha.back() == '\n') linha.pop_back();

        // 1. Captura o total de snaps encontrados
        if (linha.rfind("TOTAL:", 0) == 0) { 
            total_snaps = std::stoi(linha.substr(6));
            if (total_snaps == 0) {
                is_running = false;
                progress = 1.0f;
                return "Nenhum snap antigo (desativado) encontrado.";
            }
        }
        // 2. Atualiza qual snap está sendo removido no momento
        else if (linha.rfind("REMOVANDO:", 0) == 0) {
            size_t p = linha.find(':', 10);
            std::string nome = linha.substr(10, p - 10);
            std::string rev = linha.substr(p + 1);
            
            current_action = "Removendo: " + nome + " (rev " + rev + ")...";
        }
        // 3. Atualiza a barra de progresso do ImGui
        else if (linha.rfind("PROGRESSO:", 0) == 0) {
            removidos = std::stoi(linha.substr(10));
            if (total_snaps > 0) {
                progress = static_cast<float>(removidos) / static_cast<float>(total_snaps);
            }
        }
    }

    progress = 1.0f;
    is_running = false;
    
    if (total_snaps == -1) {
        return "Operação cancelada ou falha na autenticação.";
    }

    return "Sucesso! " + std::to_string(removidos) + " snap(s) antigo(s) removido(s).";
}

void FileController::delete_selected_item() {
    std::string selecionado = get_selected_file();
    if (selecionado.empty()) return;

    // Constrói o caminho completo: diretório_atual + / + nome_do_arquivo
    std::filesystem::path caminho_completo = std::filesystem::path(get_current_path_string()) / selecionado;

    try {
        if (std::filesystem::exists(caminho_completo)) {
            // remove_all apaga arquivos normais E pastas com tudo dentro recursivamente
            std::filesystem::remove_all(caminho_completo); 
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Caso dê erro de permissão ou algo parecido, printa no terminal de debug
        std::cerr << "Erro ao deletar item: " << e.what() << std::endl;
    }

    // Limpa a seleção para o texto sumir do rodapé
    clear_selection();
}