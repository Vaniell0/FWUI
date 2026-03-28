#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace fwui::embedded {

struct PageData {
    std::string_view route;
    const uint8_t* data;
    size_t size;
    bool compressed;

    std::string_view Content() const {
        return {reinterpret_cast<const char*>(data), size};
    }
};

/// Find an embedded page by route. Returns nullptr if not found.
/// Defined in generated code (embedded_pages.cpp).
const PageData* FindPage(std::string_view route);

/// Number of embedded pages.
size_t PageCount();

/// All embedded pages.
std::span<const PageData> AllPages();

} // namespace fwui::embedded
