#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <thread>



struct FileInfo {
    std::string name;
    bool is_directory;
    std::uintmax_t size;
    bool size_ready = false;
};

class FileController{
    public:
        FileController();
        std::string clear_temporary_files(bool& is_running, float& progress, std::string& current_action);
        std::string clear_old_snaps(bool& is_running, float& progress, std::string& current_action);
        std::string empty_trash(bool& is_running, float& progress, std::string& current_action);
        std::string get_selected_file() const;
        std::string get_current_path_string() const;
        std::vector<FileInfo> get_cached_files() const;
        std::filesystem::space_info get_disk_space_info();
        std::uintmax_t calculate_directory_size(const std::filesystem::path& dir_path) const;
        void set_current_path(const std::string& path_str);
        void navigate_up(); //cd
        void select_file(const std::string& file_name);
        void clear_selection();
        void delete_selected_item();
    private:
        void refresh_cache();
        std::string selected_file;
        std::filesystem::path current_path;
        std::vector<FileInfo> cached_files;

};