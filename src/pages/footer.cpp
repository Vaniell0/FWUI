#include "pages.hpp"

// Создание подвала
string createFooter() {
    auto footer = vbox({
        text("© 2025 Vaniello Portfolio") | Center() | SetClass("footer-text"),
        text("Сделано с FWUI") | Center() | SetClass("footer-subtext")
    }) | SetClass("footer");
    
    return footer->Render();
}
