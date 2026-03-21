#pragma once

#include "registry.hpp"
#include "template_engine.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace fwui {

struct TemplatePageConfig {
    std::string pages_dir     = "pages";
    std::string data_dir      = "data";
    std::string templates_dir = "templates";
};

class TemplatePageLoader {
public:
    explicit TemplatePageLoader(TemplatePageConfig config = {});

    /// Scan pages_dir for *.html files, register each in registry.
    /// Returns list of routes registered.
    std::vector<std::string> LoadPages(Registry& registry);

    /// Unregister old template routes, re-scan and re-register.
    /// Returns list of new/changed routes.
    std::vector<std::string> ReloadPages(Registry& registry);

    /// Load all JSON files from data_dir into a merged object.
    nlohmann::json LoadGlobalData() const;

    const TemplatePageConfig& Config() const { return config_; }

private:
    TemplatePageConfig config_;
    std::vector<std::string> registered_routes_;

    static std::string path_to_route(const std::filesystem::path& rel_path);
};

} // namespace fwui
