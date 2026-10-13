#pragma once

#include <cstdio>
#include <chrono>
#include <string>
#include <fstream>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

inline void get_current_time_str(char* buffer, size_t max_size) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    struct tm time_info;
    localtime_r(&time_t_now, &time_info);

    std::snprintf(buffer, max_size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                  time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday,
                  time_info.tm_hour, time_info.tm_min, time_info.tm_sec, static_cast<int>(millis));
}

class LogFileManager {
public:
    static LogFileManager& getInstance() {
        static LogFileManager instance;
        return instance;
    }

    void init() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (initialized_) return;

        char exe_path[1024] = {0};
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len == -1) return;

        std::string full_path(exe_path);
        std::string node_name = full_path.substr(full_path.find_last_of('/') + 1);

        std::string exe_dir = full_path.substr(0, full_path.find_last_of('/'));
        std::string parent_dir = exe_dir.substr(0, exe_dir.find_last_of('/'));
        std::string log_dir = parent_dir + "/log";

        struct stat st = {0};
        if (stat(log_dir.c_str(), &st) == -1) {
            mkdir(log_dir.c_str(), 0777);
        }

        std::string log_file_path = log_dir + "/" + node_name + ".log";
        log_file_.open(log_file_path, std::ios::out | std::ios::app);

        initialized_ = true;
    }

    void writeToFile(const char* level, const char* time_str, const char* content, const char* func, int line) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!initialized_) return;

        if (log_file_.is_open()) {
            log_file_ << "[" << level << "] [" << time_str << "] "
                      << content << " (" << func << ":" << line << ")\n";
            log_file_.flush();
        }
    }

private:
    LogFileManager() : initialized_(false) {}
    ~LogFileManager() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }
    bool initialized_;
    std::ofstream log_file_;
    std::mutex mtx_;
};

template <typename... Args>
inline void _log_base(const char* level, const char* color, const char* func, int line, const char* format, Args... args) {
    LogFileManager::getInstance().init();

    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));

    char content_buf[2048];
    if constexpr(sizeof...(args) == 0) {
        std::snprintf(content_buf, sizeof(content_buf), "%s", format);
    } else {
        std::snprintf(content_buf, sizeof(content_buf), format, args...);
    }

    std::printf("%s[%s] [%s] %s \033[0;37m(%s:%d)\033[0m\n",
                color, level, time_str, content_buf, func, line);
    std::fflush(stdout);

    LogFileManager::getInstance().writeToFile(level, time_str, content_buf, func, line);
}

#define LOGI(format, ...) _log_base("INFO", "\033[1;32m", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOGW(format, ...) _log_base("WARN", "\033[1;33m", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOGE(format, ...) _log_base("ERRO", "\033[1;31m", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOGD(format, ...) _log_base("DEBG", "\033[1;34m", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)