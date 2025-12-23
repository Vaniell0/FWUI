#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

#include "node.hpp"
#include <filesystem>

namespace fs = std::filesystem;

// Text класс
class Text : public Node {
public:
    explicit Text(const std::string& content);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::string content_;
    std::map<std::string, std::string> attributes_;
};

class HTML : public Node {
public:
    // Конструкторы для разных способов создания
    explicit HTML(const std::string& html_content);
    explicit HTML(const fs::path& file_path);
    
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;

    // Метод для изменения HTML контента
    Element SetHTML(const std::string& html_content);
    
    // Устаревший метод (лучше использовать конструктор)
    Element SetText(const std::string& text) { return SetHTML(text); }
    
private:
    std::string html_content_;
    std::map<std::string, std::string> attributes_;
    bool is_file_path_ = false;
    fs::path file_path_;
    
    // Вспомогательный метод для загрузки файла
    std::string loadFromFile() const;
};

class Ul : public Node {
public:
    explicit Ul(const Elements& children = {});
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

// Ol класс
class Ol : public Node {
public:
    explicit Ol(const Elements& children = {});
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

struct AsciiPixel {
    char character;
    std::string color;
    
    AsciiPixel() : character(' '), color("#000000") {}
    AsciiPixel(char ch, const std::string& col) : character(ch), color(col) {}
};

struct ColorSegment {
    std::string color;
    std::string text;
    int length;
    
    ColorSegment() : color("#000000"), text(""), length(0) {}
    ColorSegment(const std::string& col, const std::string& txt, int len) 
        : color(col), text(txt), length(len) {}
};

// ASCII Art элемент
class AsciiArt : public Node {
public:
    AsciiArt(const std::string& image_path, 
             int max_width_chars, 
             int max_height_chars,
             bool use_colors = true,
             bool invert = false,
             int font_size_px = 3);
    
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
    
    // Методы для обновления свойств
    Element UpdateImage(const fs::path& new_path);
    Element UpdateImage(const std::string& img);
    Element SetSize(int max_width_chars, int max_height_chars);
    Element SetUseColors(bool use_colors);
    Element SetInvert(bool invert);
    Element SetFontSize(int font_size_px);
    
private:
    std::string image_path_;
    int max_width_chars_;
    int max_height_chars_;
    bool use_colors_;
    bool invert_;
    int font_size_px_;
    std::map<std::string, std::string> attributes_;
    
    std::string generateAsciiArt() const;
};

// Декораторы
class BoldDecorator : public Node {
public:
    explicit BoldDecorator(Element child);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    Element decorated_child_;
};

class ItalicDecorator : public Node {
public:
    explicit ItalicDecorator(Element child);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    Element decorated_child_;
};

class UnderlineDecorator : public Node {
public:
    explicit UnderlineDecorator(Element child);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    Element decorated_child_;
};

class StrikethroughDecorator : public Node {
public:
    explicit StrikethroughDecorator(Element child);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    Element decorated_child_;
};

class SpanDecorator : public Node {
public:
    SpanDecorator(Element child, const std::map<std::string, std::string>& attrs = {});
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    Element decorated_child_;
    std::map<std::string, std::string> attributes_;
};

class AnchorDecorator : public Node {
public:
    AnchorDecorator(Element child, const std::string& href, const std::string& target = "_blank");
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    Element decorated_child_;
    std::string href_;
    std::string target_;
};

// Контейнеры
class HBox : public Node {
public:
    HBox() = default;
    explicit HBox(const Elements& children);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

class VBox : public Node {
public:
    VBox() = default;
    explicit VBox(const Elements& children);
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

class Separator : public Node {
public:
    Separator();
    std::string Render() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

struct Decorator {
    std::function<Element(Element)> apply;
    
    Decorator(std::function<Element(Element)> func) : apply(func) {}
    
    Element operator()(Element elem) const {
        return apply(elem);
    }
};

Element text(const std::string& content);
Element hbox(const Elements& children = {});
Element vbox(const Elements& children = {});
Element ul(const Elements& children = {});
Element ol(const Elements& children = {});

Element separator();

Element image_auto(const fs::path& path, int max_width_chars, int max_height_chars,
                  bool use_colors, bool invert);

// Декораторы как функции
Element bold(Element elem);
Element dim(Element elem);
Element italic(Element elem);
Element underline(Element elem);
Element strikethrough(Element elem);
Element color(Element elem, const std::string& color);
Element bgcolor(Element elem, const std::string& color);
Element center(Element elem);
Element border(Element elem, int thickness = 1, const std::string& color = "black", 
               const std::string& style = "solid");
Element hyperlink(Element elem, const std::string& url, const std::string& target = "_blank");

// Margin и Padding функции
Element margin(Element elem, int all);
Element margin(Element elem, int vertical, int horizontal);
Element margin(Element elem, int top, int right, int bottom, int left);
Element padding(Element elem, int all);
Element padding(Element elem, int vertical, int horizontal);
Element padding(Element elem, int top, int right, int bottom, int left);

// Выравнивание
Element align_left(Element elem);
Element align_center(Element elem);
Element align_right(Element elem);
Element align_top(Element elem);
Element align_middle(Element elem);
Element align_bottom(Element elem);

// Комбинированное выравнивание
Element align_left_top(Element elem);
Element align_left_middle(Element elem);
Element align_left_bottom(Element elem);
Element align_center_top(Element elem);
Element align_center_middle(Element elem);
Element align_center_bottom(Element elem);
Element align_right_top(Element elem);
Element align_right_middle(Element elem);
Element align_right_bottom(Element elem);

Element set_style(Element elem, const std::string& style);
Element set_class(Element elem, const std::string& _class);

Element html(const std::string& content);
Element html_file(const fs::path& path);
Element load(const fs::path& path);

// Декораторы
Decorator AlignLeft();
Decorator AlignCenter();
Decorator AlignRight();
Decorator AlignTop();
Decorator AlignMiddle();
Decorator AlignBottom();
Decorator AlignLeftTop();
Decorator AlignLeftMiddle();
Decorator AlignLeftBottom();
Decorator AlignCenterTop();
Decorator AlignCenterMiddle();
Decorator AlignCenterBottom();
Decorator AlignRightTop();
Decorator AlignRightMiddle();
Decorator AlignRightBottom();

// Декораторы как объекты для операторов
Decorator Bold();
Decorator Dim();
Decorator Italic();
Decorator Underline();
Decorator Strikethrough();
Decorator Color(const std::string& color);
Decorator BgColor(const std::string& color);
Decorator Center();
Decorator Border(int thickness = 1, const std::string& color = "black", 
                 const std::string& style = "solid");
Decorator Hyperlink(const std::string& url, const std::string& target = "_blank");

// Margin и Padding декораторы
Decorator Margin(int all);
Decorator Margin(int vertical, int horizontal);
Decorator Margin(int top, int right, int bottom, int left);
Decorator Padding(int all);
Decorator Padding(int vertical, int horizontal);
Decorator Padding(int top, int right, int bottom, int left);

Decorator SetStyle(const std::string& style);
Decorator SetClass(const std::string& _class);

Element operator|(Element element, const Decorator& decorator);
Element& operator|=(Element& element, const Decorator& decorator);
Elements operator|(Elements elements, const Decorator& decorator);
Decorator operator|(const Decorator& d1, const Decorator& d2);

#endif // ELEMENTS_HPP
