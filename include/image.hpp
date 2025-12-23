#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

#include "elements.hpp"

class ImageConverter {
public:
    ImageConverter();

    bool LoadImage(const std::string &path);

    std::vector<std::vector<AsciiPixel>> ConvertToAscii(
        int cols,
        int rows,
        bool use_color = true,
        bool invert = false,
        float contrast_factor = 2.0f,   // Фактор контрастности
        float saturation_factor = 2.0f, // Фактор насыщенности
        bool round_colors = true,       // Округлять цвета для лучшего объединения
        int color_round_step = 16       // Шаг округления цветов (8, 16, 32)
    );

    int GetOriginalWidth() const { return original_width_; }
    int GetOriginalHeight() const { return original_height_; }

    void SetAsciiChars(const std::string &chars) { ascii_chars_ = chars; }

    std::string GenerateHTML(
        const std::vector<std::vector<AsciiPixel>> &ascii_art,
        int font_size_px = 16,
        bool merge_colors = true) const;

private:
    cv::Mat original_image_;
    int original_width_;
    int original_height_;
    std::string ascii_chars_;

    std::string ColorToHex(const cv::Vec3b &color) const;
    char BrightnessToChar(int brightness, const std::string &charset) const;

    // Функции для улучшения изображения
    cv::Mat ApplyContrast(const cv::Mat &image, float factor) const;
    cv::Mat ApplySaturation(const cv::Mat &image, float factor) const;
    cv::Vec3b RoundColor(const cv::Vec3b &color, int step) const;

    // Группировка по цвету
    std::vector<ColorSegment> GroupByColor(const std::vector<AsciiPixel> &row) const;

    // Нормализация и улучшение яркости
    cv::Mat NormalizeBrightness(const cv::Mat &gray) const;
};

#endif // IMAGE_HPP
