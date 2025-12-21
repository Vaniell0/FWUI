#include "elements.hpp"
#include <fmt/core.h>
#include <image.hpp>

// ==================== ФАБРИЧНЫЕ ФУНКЦИИ ====================

Element text(const std::string& content) {
    return std::make_shared<Text>(content);
}

Element hbox(const Elements& children) {
    return std::make_shared<HBox>(children);
}

Element vbox(const Elements& children) {
    return std::make_shared<VBox>(children);
}

Element ul(const Elements& children) {
    return std::make_shared<Ul>(children);
}

Element ol(const Elements& children) {
    return std::make_shared<Ol>(children);
}

Element separator() {
    return std::make_shared<Separator>();
}

Element screen(int width_chars, int height_chars, int font_width_px, int font_height_px) {
    return std::make_shared<Screen>(width_chars, height_chars, font_width_px, font_height_px);
}

Element screen_from_image(const std::string& image_path, int target_width_px, int target_height_px,
                         int font_width_px, int font_height_px,
                         bool use_color, bool invert) {
    return Screen::FromImage(image_path, target_width_px, target_height_px, 
                            font_width_px, font_height_px, use_color, invert);
}

Element image(const std::string& path, int width_chars, int height_chars,
             bool use_colors, bool invert) {
    auto screen = std::make_shared<Screen>(width_chars, height_chars);
    return screen->LoadImage(path, use_colors, invert);
}

Element image_auto(const std::string& path, int max_width_chars, int max_height_chars,
                  bool use_colors, bool invert) {
    ImageConverter converter;
    if (!converter.LoadImage(path)) {
        return text("Error loading image");
    }
    
    // Автоматический расчет размеров с сохранением пропорций
    int original_width = converter.GetOriginalWidth();
    int original_height = converter.GetOriginalHeight();
    double aspect_ratio = static_cast<double>(original_width) / original_height;
    
    int width_chars = max_width_chars;
    int height_chars = static_cast<int>(width_chars / aspect_ratio);
    
    if (height_chars > max_height_chars) {
        height_chars = max_height_chars;
        width_chars = static_cast<int>(height_chars * aspect_ratio);
    }
    
    auto screen = std::make_shared<Screen>(width_chars, height_chars);
    return screen->LoadImage(path, use_colors, invert);
}

// Декораторы как функции
Element bold(Element elem) {
    return std::make_shared<BoldDecorator>(elem);
}

Element dim(Element elem) {
    // Вместо opacity используем более светлый цвет
    std::map<std::string, std::string> attrs = {{"style", 
        "opacity: 0.8; filter: brightness(1.2);"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element italic(Element elem) {
    return std::make_shared<ItalicDecorator>(elem);
}

Element underline(Element elem) {
    return std::make_shared<UnderlineDecorator>(elem);
}

Element strikethrough(Element elem) {
    return std::make_shared<StrikethroughDecorator>(elem);
}

Element color(Element elem, const std::string& color) {
    std::map<std::string, std::string> attrs = {{"style", fmt::format("color: {};", color)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element bgcolor(Element elem, const std::string& color) {
    std::map<std::string, std::string> attrs = {{"style", fmt::format("background-color: {};", color)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element center(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", "display: flex; justify-content: center; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element border(Element elem, int thickness, const std::string& color, const std::string& style) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("border: {}px {} {}; display: inline-block;", thickness, style, color)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element hyperlink(Element elem, const std::string& url, const std::string& target) {
    return std::make_shared<AnchorDecorator>(elem, url, target);
}

// Margin и Padding функции
Element margin(Element elem, int all) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("margin: {}px; display: inline-block;", all)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element margin(Element elem, int vertical, int horizontal) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("margin: {}px {}px; display: inline-block;", vertical, horizontal)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element margin(Element elem, int top, int right, int bottom, int left) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("margin: {}px {}px {}px {}px; display: inline-block;", top, right, bottom, left)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element padding(Element elem, int all) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("padding: {}px; display: inline-block;", all)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element padding(Element elem, int vertical, int horizontal) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("padding: {}px {}px; display: inline-block;", vertical, horizontal)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element padding(Element elem, int top, int right, int bottom, int left) {
    std::map<std::string, std::string> attrs = {{"style", 
        fmt::format("padding: {}px {}px {}px {}px; display: inline-block;", top, right, bottom, left)}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

// Горизонтальное выравнивание
Element align_left(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-start; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_center(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: center; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_right(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-end; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

// Вертикальное выравнивание
Element align_top(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; align-items: flex-start;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_middle(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_bottom(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; align-items: flex-end;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

// Комбинированное выравнивание
Element align_left_top(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-start; align-items: flex-start;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_left_middle(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-start; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_left_bottom(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-start; align-items: flex-end;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_center_top(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: center; align-items: flex-start;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_center_middle(Element elem) {
    return align_center(elem); // уже есть
}

Element align_center_bottom(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: center; align-items: flex-end;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_right_top(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-end; align-items: flex-start;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_right_middle(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-end; align-items: center;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element align_right_bottom(Element elem) {
    std::map<std::string, std::string> attrs = {{"style", 
        "display: flex; justify-content: flex-end; align-items: flex-end;"}};
    return std::make_shared<SpanDecorator>(elem, attrs);
}

Element set_style(Element elem, const std::string& style) {
    return elem->SetStyle(style);
}

Element set_class(Element elem, const std::string& _class) {
    return elem->SetClass(_class);
}

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

// Просто HTML контент
Element html(const std::string& content) {
    return std::make_shared<HTML>(content);
}

// HTML из файла с кэшированием
Element html_file(const std::filesystem::path& path) {
    return std::make_shared<HTML>(path);
}

// Универсальная загрузка (определяет тип по расширению)
Element load(const std::filesystem::path& path) {
    if (!fs::exists(path)) {
        return text("File not found: " + path.string()) | Color("red");
    }
    
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Диспетчеризация по расширению
    static const std::map<std::string, std::function<Element(const fs::path&)>> handlers = {
        {".html", html_file},
        {".htm", html_file},
        {".txt", [](const fs::path& p) { 
            std::ifstream file(p);
            std::stringstream buffer;
            buffer << file.rdbuf();
            return text(buffer.str());
        }},
        {".png", [](const fs::path& p) {
            return image_auto(p.string(), 80, 40);
        }},
        {".jpg", [](const fs::path& p) { return image_auto(p.string(), 80, 40); }},
        {".jpeg", [](const fs::path& p) { return image_auto(p.string(), 80, 40); }}
    };
    
    auto it = handlers.find(ext);
    if (it != handlers.end()) {
        return it->second(path);
    }
    
    // По умолчанию - как текстовый файл
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return text(buffer.str()) | Color("gray") | Italic();
}

// Декораторы для операторов
Decorator SetStyle(const std::string& style) {
    return {[style](Element elem) { return set_style(elem, style); }};
}

Decorator SetClass(const std::string& _class) {
    return {[_class](Element elem) { return set_class(elem, _class); }};
}

Decorator AlignLeft() { return {[](Element elem) { return align_left(elem); }}; }
Decorator AlignCenter() { return {[](Element elem) { return align_center(elem); }}; }
Decorator AlignRight() { return {[](Element elem) { return align_right(elem); }}; }
Decorator AlignTop() { return {[](Element elem) { return align_top(elem); }}; }
Decorator AlignMiddle() { return {[](Element elem) { return align_middle(elem); }}; }
Decorator AlignBottom() { return {[](Element elem) { return align_bottom(elem); }}; }
Decorator AlignLeftTop() { return {[](Element elem) { return align_left_top(elem); }}; }
Decorator AlignLeftMiddle() { return {[](Element elem) { return align_left_middle(elem); }}; }
Decorator AlignLeftBottom() { return {[](Element elem) { return align_left_bottom(elem); }}; }
Decorator AlignCenterTop() { return {[](Element elem) { return align_center_top(elem); }}; }
Decorator AlignCenterMiddle() { return {[](Element elem) { return align_center_middle(elem); }}; }
Decorator AlignCenterBottom() { return {[](Element elem) { return align_center_bottom(elem); }}; }
Decorator AlignRightTop() { return {[](Element elem) { return align_right_top(elem); }}; }
Decorator AlignRightMiddle() { return {[](Element elem) { return align_right_middle(elem); }}; }
Decorator AlignRightBottom() { return {[](Element elem) { return align_right_bottom(elem); }}; }

// Декораторы как объекты для операторов
Decorator Bold() { return {[](Element elem) { return bold(elem); }}; }
Decorator Dim() { return {[](Element elem) { return dim(elem); }}; }
Decorator Italic() { return {[](Element elem) { return italic(elem); }}; }
Decorator Underline() { return {[](Element elem) { return underline(elem); }}; }
Decorator Strikethrough() { return {[](Element elem) { return strikethrough(elem); }}; }
Decorator Color(const std::string& color) { 
    return {[color](Element elem) { return ::color(elem, color); }};
}
Decorator BgColor(const std::string& color) { 
    return {[color](Element elem) { return bgcolor(elem, color); }};
}
Decorator Center() { return {[](Element elem) { return center(elem); }}; }
Decorator Border(int thickness, const std::string& color, const std::string& style) { 
    return {[thickness, color, style](Element elem) { return border(elem, thickness, color, style); }};
}
Decorator Hyperlink(const std::string& url, const std::string& target) { 
    return {[url, target](Element elem) { return hyperlink(elem, url, target); }};
}

// Margin и Padding декораторы
Decorator Margin(int all) { 
    return {[all](Element elem) { return margin(elem, all); }};
}

Decorator Margin(int vertical, int horizontal) { 
    return {[vertical, horizontal](Element elem) { return margin(elem, vertical, horizontal); }};
}

Decorator Margin(int top, int right, int bottom, int left) { 
    return {[top, right, bottom, left](Element elem) { return margin(elem, top, right, bottom, left); }};
}

Decorator Padding(int all) { 
    return {[all](Element elem) { return padding(elem, all); }};
}

Decorator Padding(int vertical, int horizontal) { 
    return {[vertical, horizontal](Element elem) { return padding(elem, vertical, horizontal); }};
}

Decorator Padding(int top, int right, int bottom, int left) { 
    return {[top, right, bottom, left](Element elem) { return padding(elem, top, right, bottom, left); }};
}

// ==================== ОПЕРАТОРЫ ====================

Element operator|(Element element, const Decorator& decorator) {
    return decorator(element);
}

Element& operator|=(Element& element, const Decorator& decorator) {
    element = decorator(element);
    return element;
}

Elements operator|(Elements elements, const Decorator& decorator) {
    Elements result;
    for (auto& elem : elements) {
        result.push_back(decorator(elem));
    }
    return result;
}

Decorator operator|(const Decorator& d1, const Decorator& d2) {
    return {[d1, d2](Element elem) { return d2(d1(elem)); }};
}
