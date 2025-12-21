// elements/html.cpp (реализация)
#include "elements.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

HTML::HTML(const std::string& html_content) 
    : html_content_(html_content), is_file_path_(false) {}

HTML::HTML(const std::filesystem::path& file_path) 
    : file_path_(file_path), is_file_path_(true) {
    // Проверяем существование файла при создании
    if (!std::filesystem::exists(file_path_)) {
        throw std::runtime_error("HTML file not found: " + file_path_.string());
    }
}

std::string HTML::loadFromFile() const {
    if (!is_file_path_) return html_content_;
    
    std::ifstream file(file_path_);
    if (!file.is_open()) {
        return "<div style='color: red;'>Error loading HTML file: " + 
               file_path_.string() + "</div>";
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

Dimensions HTML::CalculateDimensions() const {
    // Для HTML сложно рассчитать размеры в символах
    // Можно приблизительно оценить или вернуть дефолтные значения
    return {100, 10}; // Примерные значения
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