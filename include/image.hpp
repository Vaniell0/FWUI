#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <opencv2/opencv.hpp>
#include "elements.hpp"

class ImageConverter {
public:
    ImageConverter();
    ~ImageConverter();
    
    bool LoadImage(const std::string& path);
    
    std::vector<std::vector<AsciiPixel>> ConvertToAscii(
        int cols,
        int rows,
        bool use_color = true,
        bool invert = false,
        float contrast_factor = 1.5f,    // Фактор контрастности
        float saturation_factor = 1.5f   // Фактор насыщенности
    );
    
    int GetOriginalWidth() const { return original_width_; }
    int GetOriginalHeight() const { return original_height_; }
    
    void SetAsciiChars(const std::string& chars) { ascii_chars_ = chars; }
    
private:
    cv::Mat original_image_;
    int original_width_;
    int original_height_;
    std::string ascii_chars_;
    
    std::string ColorToHex(const cv::Vec3b& color) const;
    char BrightnessToChar(int brightness, const std::string& charset) const;

    std::vector<std::vector<AsciiPixel>> ConvertToAscii(int cols, int rows, bool use_color, bool invert);

    // Функции для улучшения цвета
    cv::Vec3b EnhanceColor(const cv::Vec3b& color, float contrast, float saturation) const;
    cv::Mat EnhanceContrast(const cv::Mat& image, float factor) const;
    cv::Mat EnhanceSaturation(const cv::Mat& image, float factor) const;
    
    // Функция для группировки пикселей по цвету
    std::vector<ColorSegment> GroupByColor(const std::vector<AsciiPixel>& row) const;
};

// Структура для сегмента цвета (группа пикселей одного цвета)
struct ColorSegment;

#endif // IMAGE_HPP