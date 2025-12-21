#include "pages.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

// В Crow статическая директория определяется здесь
#define CROW_STATIC_DIRECTORY (base_path / "static").string()
#include <crow.h>

// Загрузка шаблона с заменой переменных
string loadTemplate(const string& filename, const map<string, string>& variables) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка: шаблон не найден: " << filename << endl;
        return "<html><body>Template not found</body></html>";
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    string template_str = buffer.str();
    
    // Замена переменных
    for (const auto& [key, value] : variables) {
        string placeholder = "{{ " + key + " }}";
        size_t pos = 0;
        while ((pos = template_str.find(placeholder, pos)) != string::npos) {
            template_str.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    
    return template_str;
}

int main() {    
    crow::SimpleApp app;
    app.add_static_dir();
    
    // Загрузка проектов
    vector<Project> projects = loadProjects();
    map<string, Project> projectsById;
    for (const auto& project : projects) {
        projectsById[project.id] = project;
    }

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
    
    // Главная страница
    CROW_ROUTE(app, "/")
    ([&]() {
        try {
            Img avatar((base_path / "avatar.png").string());
            
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
            
            variables["navigation"] = createNavigation();
            variables["header"] = createHeader();
            variables["about"] = createAboutSection(avatar);
            variables["projects"] = createProjectsSection();
            variables["contact_form"] = createContactForm();
            variables["contacts"] = createContactSection();
            variables["footer"] = createFooter();
            
            variables["js_scripts"] = R"(
                <script src="/static/script.js"></script>
            )";
            
            string templatePath = (base_path / "template" / "index.html").string();
            string html = loadTemplate(templatePath, variables);
            
            crow::response res(html);
            res.set_header("Content-Type", "text/html; charset=UTF-8");
            return res;
            
        } catch (const exception& e) {
            cerr << "Ошибка генерации страницы: " << e.what() << endl;
            crow::response res("<html><body><h1>500 Ошибка сервера</h1><p>" + string(e.what()) + "</p></body></html>");
            res.code = 500;
            res.set_header("Content-Type", "text/html");
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
            variables["meta_tags"] = R"(
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <meta http-equiv="X-UA-Compatible" content="IE=edge">
                <meta name="description" content=")" + project.description + R"(">
            )";
            
            variables["css_links"] = R"(
                <link rel="stylesheet" href="/static/style.css">
                <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
            )";
            
            variables["navigation"] = createNavigation();
            variables["project_content"] = createProjectPage(project);
            variables["footer"] = createFooter();
            
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

    app.port(8080).multithreaded().run();
    
    return 0;
}
