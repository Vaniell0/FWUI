#include "pages.hpp"

// Создание навигации
string createNavigation() {
    auto nav = hbox({
        text("Главная") | Hyperlink("/", "_self") | SetClass("nav-link"),
        text("Обо мне") | Hyperlink("#about", "_self") | SetClass("nav-link"),
        text("Проекты") | Hyperlink("#projects", "_self") | SetClass("nav-link"),
        text("Контакты") | Hyperlink("#contact", "_self") | SetClass("nav-link")
    }) | SetClass("navbar");
    
    return nav->Render();
}
