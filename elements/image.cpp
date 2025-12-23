#include "image.hpp"
#include <algorithm>
#include <cmath>
#include <fmt/core.h>

ImageConverter::ImageConverter()
    : original_width_(0), original_height_(0),
      ascii_chars_(" .:-=+*#%@") {}

bool ImageConverter::LoadImage(const std::string &path) {
    original_image_ = cv::imread(path);
    if (original_image_.empty()) {
        fmt::println("Error loading image: {}", path);
        return false;
    }

    original_width_ = original_image_.cols;
    original_height_ = original_image_.rows;
    fmt::println("Loaded image: {}x{}", original_width_, original_height_);
    return true;
}

// Применение контрастности
cv::Mat ImageConverter::ApplyContrast(const cv::Mat &image, float factor) const {
    if (std::abs(factor - 1.0f) < 0.01f)
        return image.clone();

    cv::Mat result;
    image.convertTo(result, -1, factor, 0);

    // Обрезаем значения в диапазон 0-255
    result = cv::min(cv::max(result, 0), 255);
    return result;
}

// Применение насыщенности
cv::Mat ImageConverter::ApplySaturation(const cv::Mat &image, float factor) const {
    if (std::abs(factor - 1.0f) < 0.01f)
        return image.clone();

    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

    // Разделяем каналы
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);

    // Увеличиваем насыщенность (S канал)
    channels[1].convertTo(channels[1], -1, factor, 0);

    // Обрезаем в диапазон 0-255
    channels[1] = cv::min(cv::max(channels[1], 0), 255);

    // Объединяем обратно
    cv::merge(channels, hsv);

    // Конвертируем обратно в BGR
    cv::Mat result;
    cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);

    return result;
}

// Округление цвета для уменьшения количества уникальных цветов
cv::Vec3b ImageConverter::RoundColor(const cv::Vec3b &color, int step) const {
    if (step <= 1)
        return color;

    // Округляем каждый канал до ближайшего step
    uchar r = ((color[2] + step / 2) / step) * step;
    uchar g = ((color[1] + step / 2) / step) * step;
    uchar b = ((color[0] + step / 2) / step) * step;

    // Гарантируем диапазон 0-255
    r = std::min(r, static_cast<uchar>(255));
    g = std::min(g, static_cast<uchar>(255));
    b = std::min(b, static_cast<uchar>(255));

    return cv::Vec3b(b, g, r);
}

// Нормализация яркости для лучшего контраста
cv::Mat ImageConverter::NormalizeBrightness(const cv::Mat &gray) const {
    cv::Mat normalized;

    // Применяем CLAHE (Contrast Limited Adaptive Histogram Equalization)
    // для улучшения локального контраста
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(2.0);
    clahe->apply(gray, normalized);

    return normalized;
}

std::string ImageConverter::ColorToHex(const cv::Vec3b &color) const {
    // OpenCV хранит цвета в формате BGR
    int b = color[0];
    int g = color[1];
    int r = color[2];

    return fmt::format("#{:02X}{:02X}{:02X}", r, g, b);
}

char ImageConverter::BrightnessToChar(int brightness, const std::string &charset) const {
    if (charset.empty())
        return ' ';

    // Нормализуем яркость от 0 до charset.length()-1
    int index = (brightness * (charset.length() - 1)) / 255;
    index = std::clamp(index, 0, static_cast<int>(charset.length() - 1));

    return charset[index];
}

std::vector<std::vector<AsciiPixel>> ImageConverter::ConvertToAscii(
    int cols, int rows, bool use_color, bool invert,
    float contrast_factor, float saturation_factor,
    bool round_colors, int color_round_step)
{
    if (original_image_.empty())
    { throw std::runtime_error("Изображение не загружено"); }

    // Изменяем размер изображения под нужное количество символов
    cv::Mat resized;
    cv::resize(original_image_, resized, cv::Size(cols, rows));

    // Применяем улучшения к цветному изображению
    if (contrast_factor != 1.0f)
    { resized = ApplyContrast(resized, contrast_factor); }

    if (saturation_factor != 1.0f)
    { resized = ApplySaturation(resized, saturation_factor); }

    // Конвертируем в оттенки серого для расчета яркости
    cv::Mat gray;
    cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);

    // Улучшаем яркость (применяем нормализацию)
    gray = NormalizeBrightness(gray);

    // Применяем инвертирование если нужно
    if (invert) gray = 255 - gray;

    // Дополнительно применяем контрастность к яркости
    if (contrast_factor != 1.0f) {
        gray.convertTo(gray, -1, contrast_factor, 0);
        // Обрезаем значения
        gray = cv::min(cv::max(gray, 0), 255);
    }

    // Подготавливаем результат
    std::vector<std::vector<AsciiPixel>> result(rows, std::vector<AsciiPixel>(cols));

    // Проходим по всем регионам (символам)
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            // Получаем яркость пикселя
            int brightness = gray.at<uchar>(y, x);

            // Преобразуем яркость в символ
            char ch = BrightnessToChar(brightness, ascii_chars_);

            // Получаем цвет (если нужно)
            std::string color = "#000000";
            if (use_color) {
                cv::Vec3b pixel_color = resized.at<cv::Vec3b>(y, x);

                // Округляем цвета для лучшего объединения
                if (round_colors)
                { pixel_color = RoundColor(pixel_color, color_round_step); }

                color = ColorToHex(pixel_color);
            }

            result[y][x] = AsciiPixel(ch, color);
        }
    }

    fmt::println("Generated ASCII art: {}x{} (contrast: {}, saturation: {}, round_colors: {})",
                 cols, rows, contrast_factor, saturation_factor, round_colors);
    return result;
}

// Группировка пикселей по цвету
std::vector<ColorSegment> ImageConverter::GroupByColor(const std::vector<AsciiPixel> &row) const
{
    std::vector<ColorSegment> segments;

    if (row.empty()) return segments;

    ColorSegment current;
    current.color = row[0].color;
    current.text = std::string(1, row[0].character);
    current.length = 1;

    for (size_t i = 1; i < row.size(); ++i) {
        if (row[i].color == current.color) {
            current.text += row[i].character;
            current.length++;
        } else {
            segments.push_back(current);
            current.color = row[i].color;
            current.text = std::string(1, row[i].character);
            current.length = 1;
        }
    }

    segments.push_back(current);
    return segments;
}

// Генерация HTML с объединением цветов
std::string ImageConverter::GenerateHTML(
    const std::vector<std::vector<AsciiPixel>> &ascii_art,
    int font_size_px,
    bool merge_colors) const
{
    if (ascii_art.empty()) return "";

    int rows = ascii_art.size();
    int cols = ascii_art[0].size();

    fmt::memory_buffer html;

    // Начинаем основной div
    fmt::format_to(std::back_inserter(html),
                    "<span style=\""
                    "font-family: 'Courier New', monospace; "
                    "font-size: {}px; "
                    "line-height: 1; "
                    "white-space: pre; "
                    "letter-spacing: {}px;"
                    "\">",
                   font_size_px,
                   font_size_px * 0.4);

    if (merge_colors) {
        // Генерация с объединением цветов
        for (int y = 0; y < rows; ++y) {
            auto segments = GroupByColor(ascii_art[y]);

            for (const auto &segment : segments) {
                // Экранируем HTML-спецсимволы
                std::string escaped_text;
                for (char c : segment.text) {
                    if (c == '<')
                        escaped_text += "&lt;";
                    else if (c == '>')
                        escaped_text += "&gt;";
                    else if (c == '&')
                        escaped_text += "&amp;";
                    else if (c == '"')
                        escaped_text += "&quot;";
                    else if (c == '\'')
                        escaped_text += "&#39;";
                    else
                        escaped_text += c;
                }

                // Если цвет черный, не оборачиваем в span
                if (segment.color == "#000000" || segment.color.empty())
                { fmt::format_to(std::back_inserter(html), "{}", escaped_text); }
                else {
                    fmt::format_to(std::back_inserter(html),
                                   "<span style=\"color: {};\">{}</span>",
                                   segment.color, escaped_text);
                }
            }

            if (y < rows - 1) fmt::format_to(std::back_inserter(html), "\n");
        }
    } else {
        // Без объединения цветов
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                const AsciiPixel &pixel = ascii_art[y][x];

                std::string escaped_char(1, pixel.character);
                if (pixel.character == '<')
                    escaped_char = "&lt;";
                else if (pixel.character == '>')
                    escaped_char = "&gt;";
                else if (pixel.character == '&')
                    escaped_char = "&amp;";
                else if (pixel.character == '"')
                    escaped_char = "&quot;";
                else if (pixel.character == '\'')
                    escaped_char = "&#39;";

                if (pixel.color == "#000000" || pixel.color.empty())
                { fmt::format_to(std::back_inserter(html), "{}", escaped_char); }
                else {
                    fmt::format_to(std::back_inserter(html),
                                   "<span style=\"color: {};\">{}</span>",
                                   pixel.color, escaped_char);
                }
            }

            if (y < rows - 1)
            { fmt::format_to(std::back_inserter(html), "\n"); }
        }
    }

    fmt::format_to(std::back_inserter(html), "</span>");
    return fmt::to_string(html);
}

// ----------

AsciiArt::AsciiArt(const std::string &image_path,
                   int max_width_px,
                   int max_height_px,
                   bool use_colors,
                   bool invert,
                   int font_size_px,
                   float contrast_factor,
                   float saturation_factor,
                   bool round_colors,
                   int color_round_step)
    : image_path_(image_path),
      max_width_px_(max_width_px),
      max_height_px_(max_height_px),
      use_colors_(use_colors),
      invert_(invert),
      font_size_px_(font_size_px),
      contrast_factor_(contrast_factor),
      saturation_factor_(saturation_factor),
      round_colors_(round_colors),
      color_round_step_(color_round_step)
{
    if (!fs::exists(image_path_))
        throw std::runtime_error(fmt::format("Image file not found: {}", image_path_));
}

std::string AsciiArt::generateAsciiArt() const {
    try {
        ImageConverter converter;
        if (!converter.LoadImage(image_path_)) {
            return fmt::format("<span style='color: red;'>Error loading image: {}</span>",
                               image_path_);
        }

        // Рассчитываем КОЛИЧЕСТВО СИМВОЛОВ на основе пикселей и размера шрифта
        int cols = max_width_px_ / font_size_px_;
        int rows = max_height_px_ / font_size_px_;

        // Минимальные размеры
        cols = std::max(1, cols);
        rows = std::max(1, rows);

        // Рассчитываем с сохранением пропорций оригинального изображения
        int original_width = converter.GetOriginalWidth();
        int original_height = converter.GetOriginalHeight();
        double aspect_ratio = static_cast<double>(original_width) / original_height;

        // Подгоняем под соотношение сторон
        int actual_cols = cols;
        int actual_rows = static_cast<int>(actual_cols / aspect_ratio);

        if (actual_rows > rows) {
            actual_rows = rows;
            actual_cols = static_cast<int>(actual_rows * aspect_ratio);
        }

        // Еще раз проверяем минимум
        actual_cols = std::max(1, actual_cols);
        actual_rows = std::max(1, actual_rows);

        fmt::println("Generating ASCII: {}x{} chars ({}x{}px, font: {}px, contrast: {}, saturation: {}, round_colors: {})",
                     actual_cols, actual_rows,
                     actual_cols * font_size_px_, actual_rows * font_size_px_,
                     font_size_px_, contrast_factor_, saturation_factor_, round_colors_);

        // Конвертируем в ASCII с улучшенными параметрами
        auto ascii_data = converter.ConvertToAscii(
            actual_cols, actual_rows,
            use_colors_, invert_,
            contrast_factor_, saturation_factor_,
            round_colors_, color_round_step_);

        // Генерируем HTML с объединением цветов
        return converter.GenerateHTML(ascii_data, font_size_px_, true);
    }
    catch (const std::exception &e)
    { return fmt::format("<span style='color: red;'>Error: {}</span>", e.what()); }
}

// Методы установки параметров
Element AsciiArt::SetContrast(float contrast_factor) {
    contrast_factor_ = contrast_factor;
    return shared_from_this();
}

Element AsciiArt::SetSaturation(float saturation_factor) {
    saturation_factor_ = saturation_factor;
    return shared_from_this();
}

Element AsciiArt::SetRoundColors(bool round_colors, int step) {
    round_colors_ = round_colors;
    color_round_step_ = step;
    return shared_from_this();
}

std::string AsciiArt::Render() const {
    std::string ascii_html = generateAsciiArt();
    std::string attrs = Node::FormatAttributes(attributes_);

    if (attrs.empty()) return ascii_html;

    // Оборачиваем в div с атрибутами
    return fmt::format("<div{}>{}</div>", attrs, ascii_html);
}

Element AsciiArt::SetStyle(const std::string &style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element AsciiArt::SetClass(const std::string &cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element AsciiArt::SetID(const std::string &id) {
    attributes_["id"] = id;
    return shared_from_this();
}

Element AsciiArt::UpdateImage(const std::string &new_path) {
    image_path_ = new_path;
    return shared_from_this();
}

Element AsciiArt::SetSize(int max_width_px, int max_height_px) {
    max_width_px_ = max_width_px;
    max_height_px_ = max_height_px;
    return shared_from_this();
}

Element AsciiArt::SetUseColors(bool use_colors) {
    use_colors_ = use_colors;
    return shared_from_this();
}

Element AsciiArt::SetInvert(bool invert) {
    invert_ = invert;
    return shared_from_this();
}

Element AsciiArt::SetFontSize(int font_size_px) {
    font_size_px_ = font_size_px;
    return shared_from_this();
}
