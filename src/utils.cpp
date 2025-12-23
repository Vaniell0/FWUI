#include "pages.hpp"

#include <chrono>
#include <random>
#include <fstream>
#include <filesystem>

// Генерация случайного яркого цвета
std::string getRandomBrightColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);
    
    int r = dis(gen) * 0.4 + 76;
    int g = dis(gen) * 0.4 + 76;
    int b = dis(gen) * 0.4 + 76;
    
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
    return std::string(hex);
}

// Расчет возраста
int calculateAge(int day, int month, int year) {
    auto now = std::chrono::system_clock::now();
    time_t now_c = std::chrono::system_clock::to_time_t(now);
    tm* now_tm = localtime(&now_c);

    int currentYear = now_tm->tm_year + 1900;
    int currentMonth = now_tm->tm_mon + 1;
    int currentDay = now_tm->tm_mday;

    int age = currentYear - year;

    if (currentMonth < month || (currentMonth == month && currentDay < day)) {
        age--;
    }

    return age;
}

// Склонение лет
std::string getYearAddition(int count) {
    int lastTwoDigits = count % 100;
    int lastDigit = count % 10;

    if (lastTwoDigits >= 11 && lastTwoDigits <= 14) {
        return "лет";
    }
    if (lastDigit == 1) {
        return "год";
    }
    if (lastDigit >= 2 && lastDigit <= 4) {
        return "года";
    }
    return "лет";
}

// Загрузка шаблона с заменой переменных
std::string loadTemplate(const std::string& filename, const std::map<std::string, std::string>& variables) {
    try {
        fs::path file_path(filename);
        
        // Проверка существования и типа файла
        if (!fs::exists(file_path)) {
            fmt::println("Ошибка: файл не найден: {}", filename);
            return "<html><body>Template not found</body></html>";
        }
        
        if (!fs::is_regular_file(file_path)) {
            fmt::println("Ошибка: {} не является обычным файлом", filename);
            return "<html><body>Template not found</body></html>";
        }
        
        // Чтение файла
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            fmt::println("Ошибка: не удалось открыть файл: {}", filename);
            return "<html><body>Template not found</body></html>";
        }
        
        // Определение размера файла
        auto file_size = fs::file_size(file_path);
        std::string template_str(file_size, '\0');
        
        // Чтение всего файла
        file.read(template_str.data(), file_size);
        file.close();
        
        // Замена переменных
        for (const auto& [key, value] : variables) {
            std::string placeholder = "{{ " + key + " }}";
            size_t pos = 0;
            while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
                template_str.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }
        
        return template_str;
        
    } catch (const fs::filesystem_error& e) {
        fmt::println("Ошибка filesystem: {}", e.what());
        return "<html><body>Filesystem error</body></html>";
    }
}