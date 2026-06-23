#include "SystemMonitor.hpp"
#include <fstream>

SystemMonitor::SystemMonitor(size_t samples) 
    : max_samples(samples), offset(0), last_update_time(0.0), current_cpu(0.0f), current_ram(0.0f) {
    cpu_history.resize(max_samples, 0.0f);
    ram_history.resize(max_samples, 0.0f);
}


void SystemMonitor::update(double current_time) {
    // Atualiza a cada 1 segundo
    if(current_time - last_update_time >= 1.0) {
        collect_system_data();
        last_update_time = current_time;

    }
}
unsigned long long last_user = 0, last_nice = 0, last_system = 0, last_idle = 0;

void SystemMonitor::collect_system_data() {
    // Coleta de RAM
    std::ifstream memfile("/proc/meminfo");
    std::string key;
    unsigned long long value;
    unsigned long long mem_total = 0, mem_avail = 0;

    while (memfile >> key >> value) {
        if (key == "MemTotal:") mem_total = value;
        if (key == "MemAvailable:") { mem_avail = value; break; }
        memfile.ignore(256, '\n'); // Pula o resto da linha (como "kB")
    }
    memfile.close();// Fecha o arquivo após a leitura

    if (mem_total > 0) {
        current_ram = 1.0f - ((float)mem_avail / (float)mem_total);
    }

    // COLETA DE CPU 
    std::ifstream cpufile("/proc/stat");
    std::string cpu_cpu;
    unsigned long long user, nice, system, idle;
    cpufile >> cpu_cpu >> user >> nice >> system >> idle;
    cpufile.close();// Fecha o arquivo após a leitura

    unsigned long long total_diff = (user - last_user) + (nice - last_nice) + (system - last_system) + (idle - last_idle);
    unsigned long long idle_diff = idle - last_idle;

    if (total_diff > 0) {
        current_cpu = 1.0f - ((float)idle_diff / (float)total_diff);
    }

    // Salva o estado atual para o próximo segundo
    last_user = user; last_nice = nice; last_system = system; last_idle = idle;
    cpu_history[offset] = current_cpu *100.0f; // Armazena o valor em porcentagem
    ram_history[offset] = current_ram* 100.0f; // Armazena o valor em porcentagem
    offset = (offset + 1) % max_samples;

}

//Getters
const float* SystemMonitor::get_cpu_data(){
    return cpu_history.data();
}
const float* SystemMonitor::get_ram_data(){
    return ram_history.data();
}
size_t SystemMonitor::get_samples_count(){
    return max_samples;
}
size_t SystemMonitor::get_offset(){
    return offset;
}
float SystemMonitor::get_current_cpu(){
    return current_cpu;
}
float SystemMonitor::get_current_ram(){        
    return current_ram;
}