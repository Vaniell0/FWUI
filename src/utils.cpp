#include "pages.hpp"

#include <random>
#include <filesystem>

// Генерация случайного яркого цвета
string getRandomBrightColor() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(0, 255);
    
    int r = dis(gen) * 0.4 + 76;
    int g = dis(gen) * 0.4 + 76;
    int b = dis(gen) * 0.4 + 76;
    
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
    return string(hex);
}

// Расчет возраста
int calculateAge(int day, int month, int year) {
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
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
string getYearAddition(int count) {
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
