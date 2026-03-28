// fwui-ssg — Static site generator CLI.
//
// Usage:
//   fwui-ssg [options]
//     --output, -o <dir>       Output directory (default: dist/)
//     --data, -d <file.json>   Global JSON data context
//     --templates, -t <dir>    Inja template directory
//     --minify                 Minify HTML output
//     --pretty                 Pretty-print HTML (default)
//     --clean                  Remove output dir before build
//     --help, -h               Show usage

#include <fwui/fwui.hpp>
#include "fwui/pages/ssg_pages.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

using namespace fwui;

struct Config {
    std::string output_dir = "dist";
    std::string data_file;
    std::string template_dir;
    std::string pages_dir;
    std::string data_dir;
    bool minify = false;
    bool pretty = true;
    bool clean  = false;
};

static void print_usage() {
    std::cout <<
        "fwui-ssg — FWUI static site generator\n"
        "\n"
        "Usage:\n"
        "  fwui-ssg [options]\n"
        "\n"
        "Options:\n"
        "  --output, -o <dir>       Output directory (default: dist/)\n"
        "  --data, -d <file.json>   Global JSON data context\n"
        "  --templates, -t <dir>    Inja template directory\n"
        "  --pages <dir>            Template pages directory (*.html with Inja syntax)\n"
        "  --data-dir <dir>         JSON data directory for template pages\n"
        "  --minify                 Minify HTML output\n"
        "  --pretty                 Pretty-print HTML (default)\n"
        "  --clean                  Remove output dir before build\n"
        "  --help, -h               Show this message\n";
}

static Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            std::exit(0);
        } else if ((arg == "--output" || arg == "-o") && i + 1 < argc) {
            cfg.output_dir = argv[++i];
        } else if ((arg == "--data" || arg == "-d") && i + 1 < argc) {
            cfg.data_file = argv[++i];
        } else if ((arg == "--templates" || arg == "-t") && i + 1 < argc) {
            cfg.template_dir = argv[++i];
        } else if (arg == "--pages" && i + 1 < argc) {
            cfg.pages_dir = argv[++i];
        } else if (arg == "--data-dir" && i + 1 < argc) {
            cfg.data_dir = argv[++i];
        } else if (arg == "--minify") {
            cfg.minify = true;
            cfg.pretty = false;
        } else if (arg == "--pretty") {
            cfg.pretty = true;
            cfg.minify = false;
        } else if (arg == "--clean") {
            cfg.clean = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage();
            std::exit(1);
        }
    }
    return cfg;
}

static nlohmann::json load_data(const std::string& path) {
    if (path.empty()) return nlohmann::json::object();
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open data file: " + path);
    }
    auto data = nlohmann::json::parse(f, nullptr, false);
    if (data.is_discarded()) {
        throw std::runtime_error("Invalid JSON in " + path);
    }
    return data;
}

static std::string route_to_path(const std::string& route, const std::string& output_dir) {
    if (route == "/") {
        return (fs::path(output_dir) / "index.html").string();
    }
    // "/about" → "dist/about/index.html"
    std::string clean = route;
    if (!clean.empty() && clean.front() == '/') clean.erase(0, 1);
    if (!clean.empty() && clean.back() == '/')  clean.pop_back();
    return (fs::path(output_dir) / clean / "index.html").string();
}

int main(int argc, char* argv[]) {
    auto cfg = parse_args(argc, argv);

    // Load JSON data
    nlohmann::json data;
    try {
        data = load_data(cfg.data_file);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    // Set template directory
    if (!cfg.template_dir.empty()) {
        TemplateEngine::SetTemplateDirectory(cfg.template_dir);
    }

    // Clean output
    if (cfg.clean && fs::exists(cfg.output_dir)) {
        fs::remove_all(cfg.output_dir);
    }
    fs::create_directories(cfg.output_dir);

    // Register pages
    Registry registry;

    // Register C++ pages (built-in defaults)
    pages::RegisterSSGPages(registry);

    // Load template pages if --pages specified (override C++ pages)
    if (!cfg.pages_dir.empty()) {
        TemplatePageLoader loader({
            cfg.pages_dir,
            cfg.data_dir.empty() ? "data" : cfg.data_dir,
            cfg.template_dir.empty() ? "templates" : cfg.template_dir
        });
        auto tpl_routes = loader.LoadPages(registry);
        for (const auto& r : tpl_routes) {
            std::cout << "  [template] " << r << "\n";
        }
    }

    // Configure renderer
    HtmlRenderer::Options opts;
    opts.pretty = cfg.pretty;
    opts.minify = cfg.minify;
    HtmlRenderer renderer(opts);

    // Render each page
    auto routes = registry.PageRoutes();
    int rendered = 0;
    int errors = 0;

    for (const auto& route : routes) {
        auto page = registry.CreatePage(route, data);
        auto html = renderer.Render(page);

        auto file_path = route_to_path(route, cfg.output_dir);
        fs::create_directories(fs::path(file_path).parent_path());

        std::ofstream out(file_path);
        if (!out.is_open()) {
            std::cerr << "Error: cannot write " << file_path << "\n";
            errors++;
            continue;
        }
        out << html;
        if (!out.good()) {
            std::cerr << "Error: write failed for " << file_path << "\n";
            errors++;
            continue;
        }
        rendered++;

        std::cout << "  " << route << " -> " << file_path << "\n";
    }

    // Copy static/ directory
    auto static_dir = fs::current_path() / "static";
    if (fs::exists(static_dir) && fs::is_directory(static_dir)) {
        auto static_dest = fs::path(cfg.output_dir) / "static";
        fs::copy(static_dir, static_dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

        int file_count = 0;
        for (auto& entry : fs::recursive_directory_iterator(static_dir)) {
            if (entry.is_regular_file()) file_count++;
        }
        std::cout << "  static/ -> " << file_count << " files copied\n";
    }

    std::cout << "\nBuild complete: " << rendered << " pages -> " << cfg.output_dir << "/\n";
    return errors > 0 ? 1 : 0;
}
