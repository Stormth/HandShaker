//
// Created by Storm on 2025/4/3.
//
// Logger.h
#pragma once
#include <iostream>
#include <mutex>
#include <sstream>

class Logger {
public:
    Logger() = default;
    ~Logger() {
        std::lock_guard<std::mutex> lock(getMutex());
        std::cout << stream.str() << std::endl;
    }

    template<typename T>
    Logger& operator<<(const T& data) {
        stream << data;
        return *this;
    }

private:
    std::ostringstream stream;

    static std::mutex& getMutex() {
        static std::mutex mtx;
        return mtx;
    }
};
