#include "elements.hpp"

// HBox
HBox::HBox(const Elements& children) {
    for (const auto& child : children) {
        AddChild(child);
    }
}

std::string HBox::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    std::string content;
    
    for (const auto& child : children) {
        if (child) {
            content += child->Render();
        }
    }
    
    return fmt::format("<div{} style=\"display: flex; flex-direction: row;\">{}</div>", 
                      attrs, content);
}

Element HBox::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element HBox::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element HBox::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

// VBox
VBox::VBox(const Elements& children) {
    for (const auto& child : children) {
        AddChild(child);
    }
}

std::string VBox::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    std::string content;
    
    for (const auto& child : children) {
        if (child) {
            content += child->Render();
        }
    }
    
    return fmt::format("<div{} style=\"display: flex; flex-direction: column;\">{}</div>",
                      attrs, content);
}

Element VBox::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element VBox::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element VBox::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

// Separator
Separator::Separator() {
    attributes_["style"] = "border-top: 1px solid #ccc; margin: 10px 0;";
}

std::string Separator::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    return fmt::format("<hr{}>", attrs);
}

Element Separator::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element Separator::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element Separator::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

Ul::Ul(const Elements& children) {
    for (const auto& child : children) {
        AddChild(child);
    }
}

std::string Ul::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    std::string content;
    for (const auto& child : children) {
        if (child) {
            content += fmt::format("<li>{}</li>", child->Render());
        }
    }
    return fmt::format("<ul{}>{}</ul>", attrs, content);
}

Element Ul::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element Ul::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element Ul::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

Ol::Ol(const Elements& children) {
    for (const auto& child : children) {
        AddChild(child);
    }
}

std::string Ol::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    std::string content;
    for (const auto& child : children) {
        if (child) {
            content += fmt::format("<li>{}</li>", child->Render());
        }
    }
    return fmt::format("<ol{}>{}</ol>", attrs, content);
}

Element Ol::SetStyle(const std::string& style) {
    attributes_["style"] = style;
    return shared_from_this();
}

Element Ol::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element Ol::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}