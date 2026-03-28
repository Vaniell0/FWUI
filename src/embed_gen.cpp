// fwui-embed — Generate C++ source with embedded page data.
//
// Usage:
//   fwui-embed [options]
//     --output, -o <dir>   Output directory for generated files (default: .)
//     --pages <dir>        Template pages directory (default: pages/)
//     --data <dir>         JSON data directory (default: data/)
//     --templates <dir>    Shared templates directory (default: templates/)
//     --minify             Minify HTML (default: true)
//     --no-minify          Pretty-print HTML
//     --help, -h           Show usage

#include <fwui/fwui.hpp>
#include "fwui/pages/ssg_pages.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using namespace fwui;

struct Config {
    std::string output_dir   = ".";
    std::string pages_dir    = "pages";
    std::string data_dir     = "data";
    std::string templates_dir = "templates";
    bool minify = true;
};

static void print_usage() {
    std::cout <<
        "fwui-embed — Generate C++ source with embedded pages\n"
        "\n"
        "Usage:\n"
        "  fwui-embed [options]\n"
        "\n"
        "Options:\n"
        "  --output, -o <dir>   Output directory (default: .)\n"
        "  --pages <dir>        Template pages directory (default: pages/)\n"
        "  --data <dir>         JSON data directory (default: data/)\n"
        "  --templates <dir>    Shared templates directory (default: templates/)\n"
        "  --minify             Minify HTML (default)\n"
        "  --no-minify          Pretty-print HTML\n"
        "  --help, -h           Show this message\n";
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
        } else if (arg == "--pages" && i + 1 < argc) {
            cfg.pages_dir = argv[++i];
        } else if (arg == "--data" && i + 1 < argc) {
            cfg.data_dir = argv[++i];
        } else if (arg == "--templates" && i + 1 < argc) {
            cfg.templates_dir = argv[++i];
        } else if (arg == "--minify") {
            cfg.minify = true;
        } else if (arg == "--no-minify") {
            cfg.minify = false;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage();
            std::exit(1);
        }
    }
    return cfg;
}

static std::string sanitize_identifier(const std::string& route) {
    // "/" → "index", "/about" → "about", "/projects/detail" → "projects_detail"
    if (route == "/") return "index";
    std::string result;
    for (char c : route) {
        if (c == '/') {
            if (!result.empty()) result += '_';
        } else if (std::isalnum(c) || c == '_') {
            result += c;
        } else {
            result += '_';
        }
    }
    return result;
}

static void write_byte_array(std::ostream& out, const std::string& name, const std::string& data) {
    out << "static constexpr uint8_t " << name << "[] = {\n    ";
    for (size_t i = 0; i < data.size(); i++) {
        out << "0x" << std::hex << std::setfill('0') << std::setw(2)
            << (static_cast<unsigned>(data[i]) & 0xFF);
        if (i + 1 < data.size()) out << ",";
        if ((i + 1) % 16 == 0 && i + 1 < data.size()) out << "\n    ";
    }
    out << std::dec << "\n};\n\n";
}

int main(int argc, char* argv[]) {
    auto cfg = parse_args(argc, argv);

    // Load pages
    Registry registry;

    // Template pages
    TemplatePageLoader loader({cfg.pages_dir, cfg.data_dir, cfg.templates_dir});
    auto tpl_routes = loader.LoadPages(registry);

    // C++ pages (if no template pages found, use built-in SSG pages)
    if (tpl_routes.empty()) {
        pages::RegisterSSGPages(registry);
    }

    // Configure renderer
    HtmlRenderer::Options opts;
    opts.minify = cfg.minify;
    opts.pretty = !cfg.minify;
    HtmlRenderer renderer(opts);

    // Render all pages
    struct RenderedPage {
        std::string route;
        std::string html;
        std::string var_name;
    };
    std::vector<RenderedPage> pages;

    auto routes = registry.PageRoutes();
    for (const auto& route : routes) {
        auto page = registry.CreatePage(route, nlohmann::json::object());
        auto html = renderer.Render(page);
        auto var_name = "page_" + sanitize_identifier(route);

        pages.push_back({route, std::move(html), std::move(var_name)});
    }

    if (pages.empty()) {
        std::cerr << "Error: no pages to embed\n";
        return 1;
    }

    // Generate output files
    fs::create_directories(cfg.output_dir);

    // Header
    auto hpp_path = fs::path(cfg.output_dir) / "embedded_pages.hpp";
    {
        std::ofstream f(hpp_path);
        if (!f.is_open()) {
            std::cerr << "Error: cannot create " << hpp_path << "\n";
            return 1;
        }
        f << "// Auto-generated by fwui-embed. Do not edit.\n"
          << "#pragma once\n"
          << "#include <fwui/embedded.hpp>\n";
    }

    // Source
    auto cpp_path = fs::path(cfg.output_dir) / "embedded_pages.cpp";
    {
        std::ofstream f(cpp_path);
        if (!f.is_open()) {
            std::cerr << "Error: cannot create " << cpp_path << "\n";
            return 1;
        }
        f << "// Auto-generated by fwui-embed. Do not edit.\n"
          << "#include \"embedded_pages.hpp\"\n\n"
          << "namespace fwui::embedded {\n\n";

        // Byte arrays
        for (const auto& p : pages) {
            write_byte_array(f, p.var_name, p.html);
        }

        // Page table
        f << "static constexpr PageData all_pages[] = {\n";
        for (const auto& p : pages) {
            f << "    { \"" << p.route << "\", " << p.var_name
              << ", sizeof(" << p.var_name << "), false },\n";
        }
        f << "};\n\n";

        // Lookup functions
        f << "const PageData* FindPage(std::string_view route) {\n"
          << "    for (const auto& p : all_pages) {\n"
          << "        if (p.route == route) return &p;\n"
          << "    }\n"
          << "    return nullptr;\n"
          << "}\n\n";

        f << "size_t PageCount() {\n"
          << "    return sizeof(all_pages) / sizeof(all_pages[0]);\n"
          << "}\n\n";

        f << "std::span<const PageData> AllPages() {\n"
          << "    return all_pages;\n"
          << "}\n\n";

        f << "} // namespace fwui::embedded\n";
    }

    // Summary
    size_t total_bytes = 0;
    for (const auto& p : pages) total_bytes += p.html.size();

    std::cout << "[fwui-embed] Generated:\n";
    std::cout << "  " << hpp_path.string() << "\n";
    std::cout << "  " << cpp_path.string() << "\n";
    for (const auto& p : pages) {
        std::cout << "  " << p.route << " (" << p.html.size() << " bytes)\n";
    }
    std::cout << "\nTotal: " << pages.size() << " pages, "
              << total_bytes << " bytes\n";

    return 0;
}
