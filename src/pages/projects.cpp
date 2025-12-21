#include "pages.hpp"

// Создание секции проектов
string createProjectsSection() {
    vector<Project> projects = loadProjects();
    
    Elements projectCards;
    for (const auto& project : projects) {
        auto card = vbox({
            text(project.name) | Bold() | Center() | Color(getRandomBrightColor()) | SetClass("text-bolder"),
            text(project.description) | Center() | SetClass("project-desc text-bold"),
            text(project.tech) | Center() | SetClass("project-tech"),
            text("Посмотреть →")->SetClass("project-link") | Hyperlink("/project/" + project.id, "_self") | Center()
        }) | SetClass("project-card");
        
        projectCards.push_back(card);
    }
    
    auto projectsSection = vbox({
        text("Мои проекты") | Bold() | SetClass("section-title text-bolder"),
        hbox(projectCards) | SetClass("projects-grid")
    }) | SetClass("section");

    projectsSection->SetID("projects");
    return projectsSection->Render();
}

// Генерация страницы проекта
string createProjectPage(const Project& project) {
    // Заголовок проекта
    auto header = vbox({
        text(project.name) | Bold() | SetClass("project-page-title"),
        text(project.tech) | SetClass("project-page-tech")
    }) | Center() | SetClass("section project-header");

    // Основной контент
    auto content = hbox({
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
    }) | SetClass("project-content");
    
    return vbox({
        header,
        content
    })->Render();
}

// Загрузка проектов
vector<Project> loadProjects() {
    vector<Project> projects = {
        {
            "fwui", 
            "FWUI Framework",
            "Фреймворк для терминального UI с поддержкой веб-рендеринга",
            "C++, HTML, CSS, JavaScript",
            "FWUI - это современный фреймворк для создания пользовательских интерфейсов в веб. Он предоставляет декларативный API для создания UI-компонентов и поддерживает различные стили оформления.",
            "https://github.com/Vaniell0/fwui",
        },
        {
            "image2ascii", 
            "Image2ASCII",
            "Конвертер изображений в ASCII-арт с сохранением цветов",
            "C++, OpenCV",
            "Программа преобразует любые изображения в текстовое представление с сохранением цветов и деталей. Поддерживает различные алгоритмы обработки и настройки вывода.",
            "https://github.com/Vaniell0/Image2ASCII",
        },
        {
            "crow-template", 
            "Crow Template",
            "Веб-сервер с шаблонами и интеграцией C++",
            "C++, Crow, HTML, CSS",
            "Шаблон для быстрого старта веб-приложений на C++ с использованием фреймворка Crow. Включает систему шаблонов, статические файлы и примеры REST API.",
            "https://github.com/Vaniell0/crow-template",
        },
        {
            "terminal-game",
            "Консольная игра",
            "Openworld с рендером на C#",
            "C#, Console GUI",
            "Игра с открытым миром, полностью реализованная в консоли с использованием ASCII-графики и процедурной генерации ландшафта.",
            "https://github.com/Vaniell0/terminal-game",
        }
    };
    
    return projects;
}
