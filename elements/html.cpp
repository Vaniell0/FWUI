#include "elements.hpp"
#include <fstream>
#include <filesystem>

HTML::HTML(const std::string& html_content) 
    : html_content_(html_content), is_file_path_(false) {}

HTML::HTML(const std::filesystem::path& file_path) 
    : file_path_(file_path), is_file_path_(true) {
    // Проверяем существование файла при создании
    if (!std::filesystem::exists(file_path_)) {
        throw std::runtime_error(fmt::format("HTML file not found: {}", file_path_.string()));
    }
}

std::string HTML::loadFromFile() const {
    if (!is_file_path_) return html_content_;
    
    std::ifstream file(file_path_);
    if (!file.is_open()) {
        return fmt::format("<div style='color: red;'>Error loading HTML file: {}</div>", file_path_.string());
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string HTML::Render() const {
    std::string content = is_file_path_ ? loadFromFile() : html_content_;
    std::string attrs = Node::FormatAttributes(attributes_);
    
    if (!attrs.empty()) {
        // Оборачиваем контент в div с атрибутами
        return fmt::format("<div{}>{}</div>", attrs, content);
    }
    
    return content;
}

Element HTML::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element HTML::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element HTML::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

Element HTML::SetHTML(const std::string& html_content) {
    html_content_ = html_content;
    is_file_path_ = false;
    return shared_from_this();
}