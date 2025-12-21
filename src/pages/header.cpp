#include "pages.hpp"

// Создание заголовка
string createHeader() {
    int age = calculateAge(19, 9, 2007);
    string ageStr = to_string(age) + " " + getYearAddition(age);
    
    auto header = vbox({
        text("👋 Привет, я Vaniello") | Center() | SetClass("header-title text-black"),
        text("Разработчик • " + ageStr + " • Где-то на Земле") | 
            Center() | SetClass("header-subtitle text-bolder")
    }) | SetClass("section");
    
    header->SetID("home");
    return header->Render();
}