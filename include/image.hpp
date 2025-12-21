#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

#include "elements.hpp"

// Класс для конвертации изображений в ASCII-арт
class ImageConverter {
public:
    ImageConverter();
    ~ImageConverter();
    
    // Загрузка изображения
    bool LoadImage(const std::string& path);
    
    // Конвертация в ASCII арт
    std::vector<std::vector<AsciiPixel>> ConvertToAscii(
        int cols,                 // Количество колонок в ASCII
        int rows,                 // Количество строк в ASCII
        bool use_color = true,    // Использовать цвет
        bool invert = false       // Инвертировать яркость
    );
    
    // Получение оригинальных размеров
    int GetOriginalWidth() const { return original_width_; }
    int GetOriginalHeight() const { return original_height_; }
    
    // Установка набора символов для конвертации
    void SetAsciiChars(const std::string& chars) { ascii_chars_ = chars; }
    
    // Генерация HTML из ASCII арта
    std::string GenerateHTML(
        const std::vector<std::vector<AsciiPixel>>& ascii_art,
        int font_size_px = 16
    ) const;
    
private:
    cv::Mat original_image_;
    int original_width_;
    int original_height_;
    std::string ascii_chars_;
    
    // Вспомогательные методы
    std::string ColorToHex(const cv::Vec3b& color) const;
    char BrightnessToChar(int brightness, const std::string& charset) const;
    cv::Vec3b GetAverageColor(const cv::Mat& region) const;
};

#endif // IMAGE_HPP