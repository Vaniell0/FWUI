#include "elements.hpp"
#include <fmt/core.h>
#include <algorithm>
#include <image.hpp>

// ==================== КЛАСС SCREEN ====================

Screen::Screen(int width_chars, int height_chars, int font_width_px, int font_height_px)
    : width_chars_(width_chars), height_chars_(height_chars),
      font_width_px_(font_width_px), font_height_px_(font_height_px) {
    
    // Инициализируем экран пробелами
    pixels_.resize(height_chars_);
    for (auto& row : pixels_) {
        row.resize(width_chars_, ScreenPixel(" ", ""));
    }
    
    // Создаем рамку для демонстрации
    // CreateBorder();
}

double Screen::CalculateLetterSpacing() const {
    // Для квадратных символов: ширина символа должна быть равна высоте
    // В моноширинных шрифтах реальная ширина символа обычно 60% от высоты
    // letter-spacing = font_height_px_ * 0.4  (чтобы получить ширину = высоте)
    return font_height_px_ * 0.4;
}

// Метод для заполнения экрана ASCII-артом из ImageConverter
void Screen::SetAsciiArt(const std::vector<std::vector<AsciiPixel>>& ascii_art) {
    int rows = std::min(static_cast<int>(ascii_art.size()), height_chars_);
    int cols = std::min(static_cast<int>(ascii_art[0].size()), width_chars_);
    
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            SetPixel(x, y, std::string(1, ascii_art[y][x].character), ascii_art[y][x].color);
        }
    }
}

// Метод для загрузки и конвертации изображения
Element Screen::LoadImage(const std::string& path, bool use_color, bool invert) {
    ImageConverter converter;
    if (!converter.LoadImage(path)) {
        // Если не удалось загрузить, создаем ошибку
        Clear("?");
        return shared_from_this();
    }
    
    // Автоматически рассчитываем размер на основе оригинального изображения
    int original_width = converter.GetOriginalWidth();
    int original_height = converter.GetOriginalHeight();
    
    // Рассчитываем оптимальное количество символов
    // Сохраняем соотношение сторон
    double aspect_ratio = static_cast<double>(original_width) / original_height;
    double font_aspect = 1.0; // квадратные символы
    
    int cols = width_chars_;
    int rows = static_cast<int>(cols / (aspect_ratio * font_aspect));
    
    // Ограничиваем размеры экрана
    cols = std::min(cols, width_chars_);
    rows = std::min(rows, height_chars_);
    
    // Конвертируем в ASCII
    auto ascii_art = converter.ConvertToAscii(cols, rows, use_color, invert);
    
    // Заполняем экран
    SetAsciiArt(ascii_art);
    
    return shared_from_this();
}

Element Screen::FromImage(const std::string& image_path, int target_width_px, int target_height_px,
                         int font_width_px, int font_height_px, bool use_color, bool invert) {
    ImageConverter converter;
    
    if (!converter.LoadImage(image_path)) {
        // Если не удалось загрузить изображение, создаем пустой экран
        auto screen = std::make_shared<Screen>(1, 1, font_width_px, font_height_px);
        screen->SetPixel(0, 0, "?", "red");
        return screen;
    }
    
    // Рассчитываем размеры в символах
    int cols = target_width_px / font_width_px;
    int rows = target_height_px / font_height_px;
    
    // Создаем экран
    auto screen = std::make_shared<Screen>(cols, rows, font_width_px, font_height_px);
    
    // Конвертируем изображение в ASCII
    auto ascii_art = converter.ConvertToAscii(cols, rows, use_color, invert);
    
    // Устанавливаем ASCII-арт на экран
    screen->SetAsciiArt(ascii_art);
    
    return screen;
}

void Screen::SetPixel(int x, int y, std::string ch, std::string color="") {
    if (x >= 0 && x < width_chars_ && y >= 0 && y < height_chars_) {
        pixels_[y][x] = ScreenPixel(ch, color);
    }
}

ScreenPixel Screen::GetPixel(int x, int y) const {
    if (x >= 0 && x < width_chars_ && y >= 0 && y < height_chars_) {
        return pixels_[y][x];
    }
    return ScreenPixel();
}

std::string Screen::GetPixelColor(int x, int y) const {
    if (x >= 0 && x < width_chars_ && y >= 0 && y < height_chars_) {
        return pixels_[y][x].color;
    }
    return "";
}

void Screen::Clear(std::string ch) {
    for (auto& row : pixels_) {
        std::fill(row.begin(), row.end(), ch);
    }
}

void Screen::FillWithPattern() {
    const std::string pattern = " .:-=+*#%@";
    for (int y = 0; y < height_chars_; ++y) {
        for (int x = 0; x < width_chars_; ++x) {
            int index = (x + y) % pattern.length();
            pixels_[y][x].character = pattern[index];
        }
    }
}

void Screen::CreateBorder() {
    // Верхняя и нижняя границы
    for (int x = 0; x < width_chars_; ++x) {
        SetPixel(x, 0, "─");
        SetPixel(x, height_chars_ - 1, "─");
    }
    
    // Левая и правая границы
    for (int y = 0; y < height_chars_; ++y) {
        SetPixel(0, y, "|");
        SetPixel(width_chars_ - 1, y, "|");
    }
    
    // Углы
    if (width_chars_ > 1 && height_chars_ > 1) {
        SetPixel(0, 0, "╭");
        SetPixel(width_chars_ - 1, 0, "╮");
        SetPixel(0, height_chars_ - 1, "╰");
        SetPixel(width_chars_ - 1, height_chars_ - 1, "╯");
    }
}

std::string Screen::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    
    // Рассчитываем letter-spacing для квадратных символов
    double letter_spacing = CalculateLetterSpacing();
    
    // Собираем содержимое экрана с учетом цветов с оптимизацией
    fmt::memory_buffer html;
    fmt::format_to(std::back_inserter(html),
        "<div{} style=\""
        "font-family: 'Courier New', monospace; "
        "font-size: {}px; "
        "line-height: 1; "
        "white-space: pre; "
        "letter-spacing: {}px; "
        "display: inline-block;"
        "\">",
        attrs, 
        font_height_px_,
        letter_spacing
    );
    
    for (int y = 0; y < height_chars_; ++y) {
        std::string current_color = "";
        bool in_span = false;
        fmt::memory_buffer line_buffer;
        
        for (int x = 0; x < width_chars_; ++x) {
            const ScreenPixel& pixel = pixels_[y][x];
            
            // Экранируем специальные HTML символы
            std::string escaped_char;
            if (pixel.character == "<") escaped_char = "&lt;";
            else if (pixel.character == ">") escaped_char = "&gt;";
            else if (pixel.character == "&") escaped_char = "&amp;";
            else if (pixel.character == "\"") escaped_char = "&quot;";
            else if (pixel.character == "'") escaped_char = "&#39;";
            else escaped_char = pixel.character;
            
            // Проверяем, нужно ли менять цвет
            bool has_color = !pixel.color.empty() && pixel.color != "#000000";
            
            if (has_color) {
                if (current_color != pixel.color) {
                    // Закрываем предыдущий span, если был
                    if (in_span) {
                        fmt::format_to(std::back_inserter(line_buffer), "</span>");
                    }
                    // Начинаем новый span
                    fmt::format_to(std::back_inserter(line_buffer), 
                        "<span style=\"color: {};\">", pixel.color);
                    current_color = pixel.color;
                    in_span = true;
                }
            } else {
                // Если цвет черный, закрываем span, если был открыт
                if (in_span) {
                    fmt::format_to(std::back_inserter(line_buffer), "</span>");
                    in_span = false;
                    current_color = "";
                }
            }
            
            // Добавляем символ
            line_buffer.append(escaped_char);
        }
        
        // Закрываем последний span, если он открыт
        if (in_span) {
            fmt::format_to(std::back_inserter(line_buffer), "</span>");
        }
        
        // Добавляем строку в общий буфер
        html.append(line_buffer);
        if (y < height_chars_ - 1) {
            fmt::format_to(std::back_inserter(html), "\n");
        }
    }
    
    fmt::format_to(std::back_inserter(html), "</div>");
    return fmt::to_string(html);
}

Dimensions Screen::CalculateDimensions() const {
    return {width_chars_, height_chars_};
}

Dimensions Screen::CalculatePixelDimensions(int font_width_px, int font_height_px) const {
    return {width_chars_ * font_width_px_, height_chars_ * font_height_px_};
}

Dimensions Screen::GetPixelDimensions() const {
    return {width_chars_ * font_width_px_, height_chars_ * font_height_px_};
}

Element Screen::SetStyle(const std::string& style) {
    // Комбинируем стили
    auto it = attributes_.find("style");
    if (it != attributes_.end() && !it->second.empty()) {
        attributes_["style"] = it->second + " " + style;
    } else {
        attributes_["style"] = style;
    }
    return shared_from_this();
}

Element Screen::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element Screen::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

Element Screen::SetFontSize(int font_width_px, int font_height_px) {
    font_width_px_ = font_width_px;
    font_height_px_ = font_height_px;
    return shared_from_this();
}