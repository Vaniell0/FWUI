// Minimal example pages for fwui-ssg and fwui-embed.
// Replace with your own page definitions.

#include "fwui/pages/ssg_pages.hpp"
#include <fwui/fwui.hpp>

namespace fwui::pages {

void RegisterSSGPages(Registry& registry) {
    // --- Components ---

    registry.RegisterComponent("navbar", [](const nlohmann::json&) {
        return nav({
            a("Home",    "/",       "_self") | AddClass("nav-link"),
            a("About",   "#about",  "_self") | AddClass("nav-link"),
            a("Contact", "#contact","_self") | AddClass("nav-link"),
        }) | SetClass("navbar");
    });

    registry.RegisterComponent("footer", [](const nlohmann::json&) {
        return footer({
            paragraph("Built with FWUI") | Center() | Color("#999"),
        }) | Padding(24);
    });

    // --- Pages ---

    registry.RegisterPage("/", [&](const nlohmann::json& data) {
        return document("FWUI Example",
            { stylesheet("/static/style.css") },
            {
                registry.CreateComponent("navbar", data),
                div({
                    h1("Hello from FWUI") | Center() | Bold(),
                    paragraph("Declarative HTML generation in C++20") | Center() | Color("#666"),
                    hr() | Margin(24),
                    section({
                        h2("About"),
                        paragraph("FWUI is a C++20 library for building HTML with "
                                  "a pipe-operator DSL, component registry, and template engine."),
                    }) | SetID("about") | Padding(20),
                }) | SetClass("container") | MaxWidth("800px") | SetStyle("margin: 0 auto"),
                registry.CreateComponent("footer", data),
            }
        );
    });
}

} // namespace fwui::pages
