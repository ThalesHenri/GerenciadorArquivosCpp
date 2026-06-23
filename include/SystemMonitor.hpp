#pragma once
#include <vector>
#include <cstdlib>
class SystemMonitor {
    public:
        SystemMonitor(size_t samples = 100);
        void update(double current_time); 
        const float* get_cpu_data();
        const float* get_ram_data();
        size_t get_samples_count();
        size_t get_offset();
        float get_current_cpu();
        float get_current_ram();

    private:
        std::vector<float>cpu_history;
        std::vector<float>ram_history;
        size_t max_samples;
        size_t offset;
        double last_update_time;

        float current_cpu;
        float current_ram;

        void collect_system_data();

};