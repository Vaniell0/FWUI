#include "pages.hpp"

#include <iostream>
#include <fstream>

#include <fmt/ranges.h>

using namespace std;

fs::path base_path = fs::current_path().parent_path(); // ../ -> ./build
std::map<std::string, Project> projectsById;
vector<Project> projects = loadProjects();

// В Crow статическая директория определяется здесь
#define CROW_STATIC_DIRECTORY (base_path / "static").string()
#include <crow.h>

int main() {
    crow::SimpleApp app;
    app.add_static_dir();
    
    PageManager& pm = PageManager::instance();

    // Загрузка проектов
    for (const auto& project : projects) {
        projectsById[project.id] = project;
    }

    auto ids = projects | std::views::transform([](const Project& p) { return p.id; });
    fmt::println("projects: {{{}}}", fmt::join(ids, ", "));
    
    // Главная страница
    CROW_ROUTE(app, "/")
    ([&]() {
        try {
            const Img avatar((base_path / "avatar.png").string());
            
            map<string, string> variables;
            variables["title"] = "Vaniello - Портфолио";
            variables["meta_tags"] = R"(
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <meta http-equiv="X-UA-Compatible" content="IE=edge">
                <meta name="description" content="Портфолио Vaniello">
            )";
            
            variables["css_links"] = R"(
                <link rel="stylesheet" href="/static/style.css">
                <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
            )";
            
            variables["navigation"] = pm("navigation");
            variables["header"] = pm("header");
            variables["about"] = pm("about", avatar);
            variables["projects"] = pm("projects");
            variables["contact_form"] = pm("contactForm");
            variables["contacts"] = pm("contacts");
            variables["footer"] = pm("footer");
            
            variables["js_scripts"] = R"(
                <script src="/static/script.js"></script>
            )";
            
            string templatePath = (base_path / "template" / "index.html").string();
            string html = loadTemplate(templatePath, variables);
            
            crow::response res(html);
            res.set_header("Content-Type", "text/html; charset=UTF-8");
            return res;
            
        } catch (const exception& e) {
            fmt::println("Ошибка генерации страницы: {}", e.what());
            crow::response res(fmt::format("<html><body><h1>500 Ошибка сервера</h1><p>{}</p></body></html>", string(e.what())));
            res.code = 500;
            res.set_header("Content-Type", "text/html; charset=UTF-8");
            return res;
        }
    });
    
    // Страницы проектов
    CROW_ROUTE(app, "/project/<string>")
    ([&](const crow::request& req, string projectId) {
        try {
            if (projectsById.find(projectId) == projectsById.end()) {
                return crow::response(404, "Проект не найден");
            }
            
            const Project& project = projectsById[projectId];
            map<string, string> variables;
            variables["title"] = project.name + " - Проект Vaniell0";
            variables["meta_tags"] = fmt::format(R"(
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <meta http-equiv="X-UA-Compatible" content="IE=edge">
                <meta name="description" content="){}(">
            )", project.description);
            
            variables["css_links"] = R"(
                <link rel="stylesheet" href="/static/style.css">
                <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
            )";
            
            variables["navigation"] = pm("navigation");
            variables["project_content"] = pm("projectPage", project);
            variables["footer"] = pm("footer");
            
            variables["js_scripts"] = R"(
                <script src="/static/script.js"></script>
            )";
            
            string templatePath = (base_path / "template" / "project.html").string();
            string html = loadTemplate(templatePath, variables);
            
            crow::response res(html);
            res.set_header("Content-Type", "text/html; charset=UTF-8");
            return res;
            
        } catch (const exception& e) {
            cerr << "Ошибка генерации страницы проекта: " << e.what() << endl;
            return crow::response(500, "Внутренняя ошибка сервера");
        }
    });

    // API для формы обратной связи
    CROW_ROUTE(app, "/api/contact")
    .methods("POST"_method)
    ([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data) {
            return crow::response(400, "Ошибка: неверный JSON");
        }
        
        string name = data["name"].s();
        string email = data["email"].s();
        string message = data["message"].s();
        
        cout << "Новое сообщение от: " << name << " (" << email << ")" << endl;
        cout << "Сообщение: " << message << endl;
        
        crow::json::wvalue response;
        response["success"] = true;
        response["message"] = "Сообщение успешно отправлено!";
        
        return crow::response(response);
    });

    app.port(8080).multithreaded().run();
    
    return 0;
}
