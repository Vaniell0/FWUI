#ifndef PAGES_HPP
#define PAGES_HPP

#include "elements.hpp"
using namespace std;
#include <string>
#include <fmt/core.h>

namespace fs = filesystem;
inline fs::path base_path = fs::current_path().parent_path();
// Структура для изображения
struct Img {
    Img() = default;
    Img(const fs::path& path) : _path(path) {
        if (!fs::exists(path)) {
            throw invalid_argument("Img: файл не найден: " + path.string());
        }
    }
    fs::path _path;
};

// Структура проекта
struct Project {
    string id;
    string name;
    string description;
    string tech;
    string details;
    string github_link;
};

string getRandomBrightColor();
int calculateAge(int day, int month, int year);
string getYearAddition(int count);

string createNavigation();
string createHeader();
string createAboutSection(const Img& avatar);
string createProjectsSection();
string createContactForm();
string createContactSection();
string createFooter();

string createProjectPage(const Project& project);
vector<Project> loadProjects();

#endif