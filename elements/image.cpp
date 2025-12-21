#include "image.hpp"
#include <fmt/core.h>
#include <algorithm>
#include <cmath>

ImageConverter::ImageConverter() 
    : original_width_(0), original_height_(0),
      ascii_chars_(" .:-=+*#%@") {}

ImageConverter::~ImageConverter() {}

bool ImageConverter::LoadImage(const std::string& path) {
    original_image_ = cv::imread(path);
    if (original_image_.empty()) {
        return false;
    }
    
    original_width_ = original_image_.cols;
    original_height_ = original_image_.rows;
    return true;
}

std::string ImageConverter::ColorToHex(const cv::Vec3b& color) const {
    // OpenCV хранит цвета в формате BGR
    int b = color[0];
    int g = color[1];
    int r = color[2];
    
    return fmt::format("#{:02X}{:02X}{:02X}", r, g, b);
}

char ImageConverter::BrightnessToChar(int brightness, const std::string& charset) const {
    if (charset.empty()) return ' ';
    
    // Нормализуем яркость от 0 до charset.length()-1
    int index = (brightness * (charset.length() - 1)) / 255;
    index = std::clamp(index, 0, static_cast<int>(charset.length() - 1));
    
    return charset[index];
}

cv::Vec3b ImageConverter::GetAverageColor(const cv::Mat& region) const {
    cv::Scalar average = cv::mean(region);
    return cv::Vec3b(
        static_cast<uchar>(average[0]),
        static_cast<uchar>(average[1]),
        static_cast<uchar>(average[2])
    );
}

std::vector<std::vector<AsciiPixel>> ImageConverter::ConvertToAscii(
    int cols, int rows, bool use_color, bool invert) {
    
    if (original_image_.empty()) {
        throw std::runtime_error("Изображение не загружено");
    }
    
    // Изменяем размер изображения под нужное количество символов
    cv::Mat resized;
    cv::resize(original_image_, resized, cv::Size(cols, rows));
    
    // Конвертируем в оттенки серого для расчета яркости
    cv::Mat gray;
    cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);
    
    // Подготавливаем результат
    std::vector<std::vector<AsciiPixel>> result(rows, 
        std::vector<AsciiPixel>(cols));
    
    // Проходим по всем регионам (символам)
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            // Получаем яркость пикселя
            int brightness = gray.at<uchar>(y, x);
            if (invert) {
                brightness = 255 - brightness;
            }
            
            // Преобразуем яркость в символ
            char ch = BrightnessToChar(brightness, ascii_chars_);
            
            // Получаем цвет (если нужно)
            std::string color = "#000000";
            if (use_color) {
                cv::Vec3b pixel_color = resized.at<cv::Vec3b>(y, x);
                color = ColorToHex(pixel_color);
            }
            
            result[y][x] = AsciiPixel(ch, color);
        }
    }
    
    return result;
}

std::string ImageConverter::GenerateHTML(
    const std::vector<std::vector<AsciiPixel>>& ascii_art,
    int font_size_px) const {
    
    if (ascii_art.empty()) return "";
    
    int rows = ascii_art.size();
    int cols = ascii_art[0].size();
    
    fmt::memory_buffer html;
    fmt::format_to(std::back_inserter(html), "<div style=\""
        "font-family: 'Courier New', monospace; "
        "font-size: {}px; "
        "line-height: 1; "
        "white-space: pre; "
        "letter-spacing: {}px; "
        "display: inline-block;"
        "\">", 
        font_size_px, 
        font_size_px * 0.4);
    
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            const AsciiPixel& pixel = ascii_art[y][x];
            
            // Экранируем специальные HTML символы
            std::string escaped_char(1, pixel.character);
            if (pixel.character == '<') escaped_char = "&lt;";
            else if (pixel.character == '>') escaped_char = "&gt;";
            else if (pixel.character == '&') escaped_char = "&amp;";
            else if (pixel.character == '"') escaped_char = "&quot;";
            else if (pixel.character == '\'') escaped_char = "&#39;";
            
            // Если цвет черный или не указан, просто выводим символ
            if (pixel.color == "#000000" || pixel.color.empty()) {
                // Используем fmt::format вместо format_to для строковых литералов
                html.append(escaped_char);
            } else {
                // Используем format_to для форматированной строки
                fmt::format_to(std::back_inserter(html), 
                    "<span style=\"color: {};\">{}</span>",
                    pixel.color, escaped_char);
            }
        }
        if (y < rows - 1) {
            fmt::format_to(std::back_inserter(html), "\n");
        }
    }
    
    fmt::format_to(std::back_inserter(html), "</div>");
    return fmt::to_string(html);
}
