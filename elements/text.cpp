#include "elements.hpp"

Text::Text(const std::string& content) : content_(content) {}

std::string Text::Render() const {
    std::string escapedContent = Node::EscapeHTML(content_);
    if (attributes_.empty()) {
        return escapedContent;
    }
    std::string tag = "span";
    std::string attrs = Node::FormatAttributes(attributes_);
    return fmt::format("<{}{}>{}</{}>", tag, attrs, escapedContent, tag);
}

Element Text::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element Text::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element Text::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}