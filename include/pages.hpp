#ifndef PAGES_HPP
#define PAGES_HPP

#include <map>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "elements.hpp"
#include "pageManager.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

// Структуры данных
struct Img {
    fs::path _path;
    Img() = default;
    Img(const fs::path& path) : _path(path) {}
};

struct Project {
    std::string id;
    std::string name;
    std::string description;
    std::string tech;
    std::string details;
    std::string github_link;
};

// Глобальные переменные
extern fs::path base_path;
extern std::map<std::string, Project> projectsById;
extern std::vector<Project> projects;

// Функция загрузки шаблона
std::string loadTemplate(const std::string& filename, 
                        const std::map<std::string, std::string>& variables);

// Вспомогательные функции
std::vector<Project> loadProjects();
std::string getRandomBrightColor();
int calculateAge(int day, int month, int year);
std::string getYearAddition(int count);

#endif