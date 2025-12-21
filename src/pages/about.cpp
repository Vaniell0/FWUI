#include "pages.hpp"

// Создание секции "Обо мне"
string createAboutSection(const Img& avatar) {
    int experienceYears = calculateAge(1, 1, 2021);
    
    auto about = hbox({
        screen_from_image(avatar._path, 320, 320, 3, 3, false, true) | SetClass("avatar-container text-black"),
        
        vbox({
            text("Навыки:") | Bold() | SetClass("section-title text-bolder"),
            ul({
                text("C++ / Modern C++ / System Design")
            }) | SetClass("skill-item text-bolder"),

            text("Опыт:") | Bold() | SetClass("section-title text-bolder"),
            ul({
                text(fmt::format("{} {} программирования", experienceYears, getYearAddition(experienceYears))),
                text("Множество open-source проектов"),
                text("Кросс-платформенная разработка"),
                text("Веб-разработка (Fullstack)")
            }) | SetClass("experience-item text-bold")
        }) | SetClass("about-info")
    }) | SetClass("section about-section");

    about->SetID("about");
    return about->Render();
}