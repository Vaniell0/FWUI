#include "pages.hpp"

// Создание заголовка
REGISTER_COMPONENT(header, [](){
    int age = calculateAge(19, 9, 2007);

    auto header = vbox({
        text("👋 Привет, я Vaniello") | Center() | SetClass("header-title text-black"),
        text(fmt::format("Разработчик • {} {} • Где-то на Земле", age, getYearAddition(age))) | 
            Center() | SetClass("header-subtitle text-bolder")
    }) | SetClass("section");
    
    header->SetID("home");
    return header->Render();
});
