#include "image.hpp"
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
