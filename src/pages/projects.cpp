#include "pages.hpp"

// Создание секции проектов
REGISTER_COMPONENT(projects, [](){
    std::vector<Project> projects = loadProjects();
    
    Elements projectCards;
    for (const auto& project : projects) {
        auto card = vbox({
            text(project.name) | Bold() | Center() | Color(getRandomBrightColor()) | SetClass("text-bolder"),
            text(project.description) | Center() | SetClass("project-desc text-bold"),
            text(project.tech) | Center() | SetClass("project-tech"),
            text("Посмотреть →")->SetClass("project-link") | Hyperlink(fmt::format("/project/{}", project.id), "_self") | Center()
        }) | SetClass("project-card");
        
        projectCards.push_back(card);
    }
    
    auto projectsSection = vbox({
        text("Мои проекты") | Bold() | SetClass("section-title text-bolder"),
        hbox(projectCards) | SetClass("projects-grid")
    }) | SetClass("section");

    projectsSection->SetID("projects");
    return projectsSection->Render();
});

// Генерация страницы проекта
REGISTER_COMPONENT(projectPage, [](const Project& project) {
    return vbox({
        vbox({
        text(project.name) | Bold() | SetClass("project-page-title"),
        text(project.tech) | SetClass("project-page-tech")
    }) | Center() | SetClass("section project-header"),
        hbox({
        // Левая колонка: изображение и ссылки
        vbox({            
            text("Ссылки проекта") | Bold() | SetClass("section-title small"),
            text("GitHub") | Hyperlink(project.github_link, "_blank") | SetClass("project-link")
        }) | SetClass("project-sidebar"),
        // Правая колонка: детали
        vbox({
            text("Описание") | Bold() | SetClass("section-title"),
            text(project.details) | SetClass("project-details"),
            
            text("Использованные технологии") | Bold() | SetClass("section-title"),
            text(project.tech) | SetClass("project-tech-list")
        }) | SetClass("project-details-container")
    }) | SetClass("project-content")
    })->Render();
});

#include <fstream>

// Загрузка проектов
std::vector<Project> loadProjects() {
    std::vector<Project> projects;
    fs::path projects_path = base_path / "data" / "projects.json";
    
    if (fs::exists(projects_path)) {
        try {
            std::ifstream file(projects_path);
            json data = json::parse(file);
            
            for (const auto& item : data) {
                Project project;
                project.id = item.value("id", "");
                project.name = item.value("name", "");
                project.description = item.value("description", "");
                project.tech = item.value("tech", "");
                project.details = item.value("details", "");
                project.github_link = item.value("github_link", "");
                projects.push_back(project);
            }
        } catch (const std::exception& e) {
            fmt::println("Error loading projects: {}", e.what());
        }
    }
    
    return projects;
}
