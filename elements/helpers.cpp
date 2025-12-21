#include "elements.hpp"
#include <cmath>

// Рассчитывает оптимальное количество колонок и строк для изображения
std::pair<int, int> CalculateTerminalSize(int image_width_px, int image_height_px, 
                                         int font_width_px, int font_height_px) {
    // Учитываем соотношение сторон символов консоли (1:2)
    int cols = image_width_px / font_width_px;
    int rows = image_height_px / font_height_px;
    
    // Для сохранения пропорций
    double aspect_ratio = static_cast<double>(font_width_px) / font_height_px;
    double image_aspect = static_cast<double>(image_width_px) / image_height_px;
    
    if (image_aspect > aspect_ratio) {
        // Шире чем стандартное соотношение
        rows = static_cast<int>(cols * aspect_ratio / image_aspect);
    } else {
        // Выше чем стандартное соотношение
        cols = static_cast<int>(rows * image_aspect / aspect_ratio);
    }
    
    return {std::max(1, cols), std::max(1, rows)};
}

// Преобразует размер в символах в размер в пикселях
Dimensions CharsToPixels(int cols, int rows, int font_width_px, int font_height_px) {
    return {cols * font_width_px, rows * font_height_px};
}

// Преобразует размер в пикселях в размер в символах
std::pair<int, int> PixelsToChars(int width_px, int height_px, int font_width_px, int font_height_px) {
    return {width_px / font_width_px, height_px / font_height_px};
}