#include "pages.hpp"

// Создание секции контактов
string createContactSection() {
    auto contacts = hbox({
            text("Email") | Hyperlink("mailto:ripaivan11@gmail.com", "_self") | SetClass("contact-link text-bold"),
            text("•") | SetClass("separator"),
            text("GitHub") | Hyperlink("https://github.com/Vaniell0", "_blank") | SetClass("contact-link text-bold"),
            text("•") | SetClass("separator"),
            text("Telegram") | Hyperlink("https://t.me/Ivanripa", "_blank") | SetClass("contact-link text-bold")
        }) | Center() | SetClass("contacts-grid") | SetClass("section");
    return contacts->Render();
}

// Создание формы обратной связи (исправлено)
string createContactForm() {
    Element contact_form = load(base_path / "static" / "contactForm.html");
    return contact_form->Render();
}
