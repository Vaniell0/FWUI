#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

#include "node.hpp"
#include <string>
#include <functional>
#include <filesystem>
#include <map>

// ==================== ОБЪЯВЛЕНИЯ КЛАССОВ ====================

// Text класс
class Text : public Node {
public:
    explicit Text(const std::string& content);
    std::string Render() const override;
    Dimensions CalculateDimensions() const override;
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
    explicit HTML(const std::filesystem::path& file_path);
    
    std::string Render() const override;
    Dimensions CalculateDimensions() const override;
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
    std::filesystem::path file_path_;
    
    // Вспомогательный метод для загрузки файла
    std::string loadFromFile() const;
};

class Ul : public Node {
public:
    explicit Ul(const Elements& children = {});
    std::string Render() const override;
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

struct ScreenPixel {
    std::string character;
    std::string color;
    
    ScreenPixel(std::string ch = " ", const std::string& col = "") 
        : character(ch), color(col) {}
};

struct AsciiPixel {
    char character;
    std::string color; // hex цвет, например "#FF0000"
    
    AsciiPixel(char ch = ' ', const std::string& col = "#000000")
        : character(ch), color(col) {}
};

// Screen класс
class Screen : public Node {
public:
    Screen(int width_chars, int height_chars, int font_width_px = 8, int font_height_px = 16);

    double CalculateLetterSpacing() const;

    void SetAsciiArt(const std::vector<std::vector<AsciiPixel>> &ascii_art);
    Element LoadImage(const std::string &path, bool use_color, bool invert);
    static Element FromImage(const std::string &image_path, int target_width_px, int target_height_px, int font_width_px, int font_height_px, bool use_color, bool invert);

    std::string GetStyle() const override {
        auto it = attributes_.find("style");
        return (it != attributes_.end()) ? it->second : "";
    }
    void SetPixel(int x, int y, std::string ch, std::string color);
    ScreenPixel GetPixel(int x, int y) const;
    std::string GetPixelColor(int x, int y) const;
    void Clear(std::string ch = " ");
    void FillWithPattern();
    void CreateBorder();
    
    std::string Render() const override;
    Dimensions CalculateDimensions() const override;
    Dimensions CalculatePixelDimensions(int font_width_px = 8, int font_height_px = 16) const override;
    
    int GetWidthChars() const;
    int GetHeightChars() const;
    int GetFontWidthPx() const;
    int GetFontHeightPx() const;
    Dimensions GetPixelDimensions() const;
    
    Element SetStyle(const std::string& style) override;
    Element SetBackgroundColor(const std::string &color);
    Element SetClass(const std::string &cls) override;
    Element SetID(const std::string& id) override;
    Element SetFontSize(int font_width_px, int font_height_px);
    
private:
    int width_chars_;
    int height_chars_;
    int font_width_px_;
    int font_height_px_;
    std::vector<std::vector<ScreenPixel>> pixels_;
    std::map<std::string, std::string> attributes_;
};

// Декораторы
class BoldDecorator : public Node {
public:
    explicit BoldDecorator(Element child);
    std::string Render() const override;
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
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
    Dimensions CalculateDimensions() const override;
    Element SetStyle(const std::string& style) override;
    Element SetClass(const std::string& cls) override;
    Element SetID(const std::string& id) override;
private:
    std::map<std::string, std::string> attributes_;
};

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

std::pair<int, int> CalculateTerminalSize(int image_width_px, int image_height_px, 
                                         int font_width_px = 8, int font_height_px = 16);
Dimensions CharsToPixels(int cols, int rows, int font_width_px = 8, int font_height_px = 16);
std::pair<int, int> PixelsToChars(int width_px, int height_px, int font_width_px = 8, int font_height_px = 16);

// ==================== СТРУКТУРА ДЕКОРАТОРА ====================

struct Decorator {
    std::function<Element(Element)> apply;
    
    Decorator(std::function<Element(Element)> func) : apply(func) {}
    
    Element operator()(Element elem) const {
        return apply(elem);
    }
};

// ==================== ФАБРИЧНЫЕ ФУНКЦИИ ====================

Element text(const std::string& content);
Element hbox(const Elements& children = {});
Element vbox(const Elements& children = {});
Element ul(const Elements& children = {});
Element ol(const Elements& children = {});

Element separator();

Element screen(int width_chars, int height_chars, int font_width_px = 8, int font_height_px = 16);
Element screen_from_image(const std::string& image_path, int target_width_px, int target_height_px,
                         int font_width_px = 8, int font_height_px = 16,
                         bool use_color = true, bool invert = false);

// Функции для изображений
Element image(const std::string& path, int width_chars, int height_chars,
             bool use_colors = true, bool invert = false);
Element image_auto(const std::string& path, int max_width_chars, int max_height_chars,
                  bool use_colors = true, bool invert = false);

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
Element html_file(const std::filesystem::path& path);
Element load(const std::filesystem::path& path);

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

// ==================== ОПЕРАТОРЫ ====================

Element operator|(Element element, const Decorator& decorator);
Element& operator|=(Element& element, const Decorator& decorator);
Elements operator|(Elements elements, const Decorator& decorator);
Decorator operator|(const Decorator& d1, const Decorator& d2);

#endif // ELEMENTS_HPP