#ifndef NODE_HPP
#define NODE_HPP

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <functional>
#include <fmt/core.h>

class Node;
using Element = std::shared_ptr<Node>;
using Elements = std::vector<Element>;

struct Dimensions {
    int width = 0;
    int height = 0;
    
    bool operator==(const Dimensions& other) const {
        return width == other.width && height == other.height;
    }
    
    bool operator!=(const Dimensions& other) const {
        return !(*this == other);
    }
};

class Node : public std::enable_shared_from_this<Node> {
public:
    virtual ~Node() = default;
    
    std::weak_ptr<Node> parent;
    Elements children;
    
    virtual std::string Render() const = 0;
    virtual Dimensions CalculateDimensions() const = 0;
    
    void AddChild(Element child) {
        if (child) {
            child->parent = weak_from_this();
            children.push_back(child);
        }
    }
    
    // Новые методы для расчета размеров в разных единицах
    virtual Dimensions CalculatePixelDimensions(int font_width_px = 8, int font_height_px = 16) const {
        auto dims = CalculateDimensions();
        return {dims.width * font_width_px, dims.height * font_height_px};
    }
    
    virtual std::pair<int, int> CalculateOptimalFontSize(int target_width_px, int target_height_px) const {
        auto dims = CalculateDimensions();
        if (dims.width == 0 || dims.height == 0) {
            return {8, 16};
        }
        
        // Рассчитываем оптимальный размер шрифта
        int font_width = target_width_px / dims.width;
        int font_height = target_height_px / dims.height;
        
        // Для сохранения пропорций берем минимум
        int font_size = std::min(font_width * 2, font_height);
        return {font_size / 2, font_size}; // width = height/2
    }
    
    static std::string EscapeHTML(const std::string& text) {
        if (text.empty()) return text;
        
        std::string result = text;
        size_t pos = 0;
        while ((pos = result.find('&', pos)) != std::string::npos) {
            result.replace(pos, 1, "&amp;");
            pos += 5;
        }
        pos = 0;
        while ((pos = result.find('<', pos)) != std::string::npos) {
            result.replace(pos, 1, "&lt;");
            pos += 4;
        }
        pos = 0;
        while ((pos = result.find('>', pos)) != std::string::npos) {
            result.replace(pos, 1, "&gt;");
            pos += 4;
        }
        pos = 0;
        while ((pos = result.find('"', pos)) != std::string::npos) {
            result.replace(pos, 1, "&quot;");
            pos += 6;
        }
        pos = 0;
        while ((pos = result.find('\'', pos)) != std::string::npos) {
            result.replace(pos, 1, "&#39;");
            pos += 5;
        }
        return result;
    }
    
    static std::string FormatAttributes(const std::map<std::string, std::string>& attrs) {
        if (attrs.empty()) return "";
        
        fmt::memory_buffer buff;
        for (const auto& [key, value] : attrs) {
            if (!value.empty()) {
                fmt::format_to(std::back_inserter(buff), " {} =\"{}\"", key, EscapeHTML(value));
            } else {
                fmt::format_to(std::back_inserter(buff), " {}", key);
            }
        }
        return fmt::to_string(buff);
    }
    
    virtual std::string GetStyle() const {
        return ""; // Возвращает пустую строку по умолчанию
    }
    virtual Element SetStyle(const std::string& style) { return shared_from_this(); }
    virtual Element SetClass(const std::string& cls) { return shared_from_this(); }
    virtual Element SetID(const std::string& id) { return shared_from_this(); }
};

#endif // NODE_HPP
