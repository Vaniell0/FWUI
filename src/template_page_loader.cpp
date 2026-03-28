#include "fwui/template_page_loader.hpp"
#include "fwui/elements.hpp"

#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace fwui {

TemplatePageLoader::TemplatePageLoader(TemplatePageConfig config)
    : config_(std::move(config)) {}

nlohmann::json TemplatePageLoader::LoadGlobalData() const {
    nlohmann::json data = nlohmann::json::object();

    if (!fs::exists(config_.data_dir)) return data;

    for (auto& entry : fs::directory_iterator(config_.data_dir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        std::ifstream f(entry.path());
        auto parsed = nlohmann::json::parse(f, nullptr, false);
        if (parsed.is_discarded()) {
            std::cerr << "[fwui] Warning: invalid JSON in " << entry.path() << "\n";
            continue;
        }

        auto stem = entry.path().stem().string();
        if (stem == "site") {
            // site.json merges at root level
            if (parsed.is_object()) {
                for (auto& [key, val] : parsed.items()) {
                    data[key] = val;
                }
            }
        } else {
            // other files nested under their name: projects.json → data["projects"]
            data[stem] = std::move(parsed);
        }
    }

    return data;
}

std::string TemplatePageLoader::path_to_route(const fs::path& rel_path) {
    // pages/index.html → "/"
    // pages/about.html → "/about"
    // pages/projects/detail.html → "/projects/detail"
    auto without_ext = rel_path;
    without_ext.replace_extension(); // strip .html

    std::string route = "/" + without_ext.generic_string();

    // /index → /
    if (route == "/index") route = "/";
    // /foo/index → /foo
    const std::string suffix = "/index";
    if (route.size() > suffix.size() &&
        route.compare(route.size() - suffix.size(), suffix.size(), suffix) == 0) {
        route.erase(route.size() - suffix.size());
    }

    return route;
}

std::vector<std::string> TemplatePageLoader::LoadPages(Registry& registry) {
    std::vector<std::string> routes;

    if (!fs::exists(config_.pages_dir)) return routes;

    // Set template directory for {% include %} support
    if (fs::exists(config_.templates_dir)) {
        TemplateEngine::SetTemplateDirectory(
            (fs::path(config_.templates_dir) / "").string());
    }

    auto global_data = LoadGlobalData();
    auto pages_path = fs::path(config_.pages_dir);

    for (auto& entry : fs::recursive_directory_iterator(config_.pages_dir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".html") continue;

        auto rel = fs::relative(entry.path(), pages_path);
        auto route = path_to_route(rel);
        auto abs_path = fs::absolute(entry.path()).string();
        auto data_dir = config_.data_dir;

        // Register page factory that reads template at render time (for hot-reload)
        registry.RegisterPage(route, [abs_path, data_dir, global_data](const nlohmann::json& runtime_data) -> Element {
            // Start with global data
            nlohmann::json merged = global_data;

            // Merge per-page data if exists
            auto page_stem = fs::path(abs_path).stem().string();
            auto page_data_path = fs::path(data_dir) / (page_stem + ".json");
            if (fs::exists(page_data_path)) {
                std::ifstream f(page_data_path);
                auto page_json = nlohmann::json::parse(f, nullptr, false);
                if (!page_json.is_discarded() && page_json.is_object()) {
                    merged.merge_patch(page_json);
                }
            }

            // Merge runtime data (highest priority)
            if (runtime_data.is_object()) {
                merged.merge_patch(runtime_data);
            }

            // Render template
            std::ifstream tpl_file(abs_path);
            std::string tpl_str((std::istreambuf_iterator<char>(tpl_file)),
                                 std::istreambuf_iterator<char>());
            auto html = TemplateEngine::Render(tpl_str, merged);

            return raw(html);
        });

        registered_routes_.push_back(route);
        routes.push_back(route);
    }

    return routes;
}

std::vector<std::string> TemplatePageLoader::ReloadPages(Registry& registry) {
    // Unregister old template routes
    for (const auto& route : registered_routes_) {
        if (registry.HasPage(route)) {
            registry.UnregisterPage(route);
        }
    }
    registered_routes_.clear();

    // Re-scan and register
    return LoadPages(registry);
}

} // namespace fwui
