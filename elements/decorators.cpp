#include "elements.hpp"

// BoldDecorator
BoldDecorator::BoldDecorator(Element child) : decorated_child_(child) {
    if (child) AddChild(child);
}

std::string BoldDecorator::Render() const {
    if (decorated_child_) {
        return fmt::format("<b>{}</b>", decorated_child_->Render());
    }
    return "<b></b>";
}

Element BoldDecorator::SetStyle(const std::string& style) {
    if (decorated_child_) {
        decorated_child_->SetStyle(style);
    }
    return shared_from_this();
}

Element BoldDecorator::SetClass(const std::string& cls) {
    if (decorated_child_) {
        decorated_child_->SetClass(cls);
    }
    return shared_from_this();
}

Element BoldDecorator::SetID(const std::string& id) {
    if (decorated_child_) {
        decorated_child_->SetID(id);
    }
    return shared_from_this();
}

// ItalicDecorator
ItalicDecorator::ItalicDecorator(Element child) : decorated_child_(child) {
    if (child) AddChild(child);
}

std::string ItalicDecorator::Render() const {
    if (decorated_child_) {
        return fmt::format("<i>{}</i>", decorated_child_->Render());
    }
    return "<i></i>";
}

Element ItalicDecorator::SetStyle(const std::string& style) {
    if (decorated_child_) {
        decorated_child_->SetStyle(style);
    }
    return shared_from_this();
}

Element ItalicDecorator::SetClass(const std::string& cls) {
    if (decorated_child_) {
        decorated_child_->SetClass(cls);
    }
    return shared_from_this();
}

Element ItalicDecorator::SetID(const std::string& id) {
    if (decorated_child_) {
        decorated_child_->SetID(id);
    }
    return shared_from_this();
}

// UnderlineDecorator
UnderlineDecorator::UnderlineDecorator(Element child) : decorated_child_(child) {
    if (child) AddChild(child);
}

std::string UnderlineDecorator::Render() const {
    if (decorated_child_) {
        return fmt::format("<u>{}</u>", decorated_child_->Render());
    }
    return "<u></u>";
}

Element UnderlineDecorator::SetStyle(const std::string& style) {
    if (decorated_child_) {
        decorated_child_->SetStyle(style);
    }
    return shared_from_this();
}

Element UnderlineDecorator::SetClass(const std::string& cls) {
    if (decorated_child_) {
        decorated_child_->SetClass(cls);
    }
    return shared_from_this();
}

Element UnderlineDecorator::SetID(const std::string& id) {
    if (decorated_child_) {
        decorated_child_->SetID(id);
    }
    return shared_from_this();
}

// StrikethroughDecorator
StrikethroughDecorator::StrikethroughDecorator(Element child) : decorated_child_(child) {
    if (child) AddChild(child);
}

std::string StrikethroughDecorator::Render() const {
    if (decorated_child_) {
        return fmt::format("<s>{}</s>", decorated_child_->Render());
    }
    return "<s></s>";
}

Element StrikethroughDecorator::SetStyle(const std::string& style) {
    if (decorated_child_) {
        decorated_child_->SetStyle(style);
    }
    return shared_from_this();
}

Element StrikethroughDecorator::SetClass(const std::string& cls) {
    if (decorated_child_) {
        decorated_child_->SetClass(cls);
    }
    return shared_from_this();
}

Element StrikethroughDecorator::SetID(const std::string& id) {
    if (decorated_child_) {
        decorated_child_->SetID(id);
    }
    return shared_from_this();
}

// SpanDecorator
SpanDecorator::SpanDecorator(Element child, const std::map<std::string, std::string>& attrs) 
    : decorated_child_(child), attributes_(attrs) {
    if (child) AddChild(child);
}

std::string SpanDecorator::Render() const {
    std::string attrs = Node::FormatAttributes(attributes_);
    if (decorated_child_) {
        return fmt::format("<span{}>{}</span>", attrs, decorated_child_->Render());
    }
    return fmt::format("<span{}></span>", attrs);
}

Element SpanDecorator::SetStyle(const std::string& style) {
    // Комбинируем стили вместо замены
    auto it = attributes_.find("style");
    if (it != attributes_.end() && !it->second.empty()) {
        // Добавляем новый стиль после существующего
        it->second = fmt::format("{} {}", it->second, style);
    } else {
        attributes_["style"] = style;
    }
    return shared_from_this();
}

Element SpanDecorator::SetClass(const std::string& cls) {
    attributes_["class"] = cls;
    return shared_from_this();
}

Element SpanDecorator::SetID(const std::string& id) {
    attributes_["id"] = id;
    return shared_from_this();
}

// AnchorDecorator
AnchorDecorator::AnchorDecorator(Element child, const std::string& href, const std::string& target) 
    : decorated_child_(child), href_(href), target_(target) {
    if (child) AddChild(child);
}

std::string AnchorDecorator::Render() const {
    std::map<std::string, std::string> attrs = {
        {"href", href_},
        {"target", target_},
        {"style", "text-decoration: none; color: inherit;"}
    };
    std::string attrs_str = Node::FormatAttributes(attrs);
    
    if (decorated_child_) {
        return fmt::format("<a{}>{}</a>", attrs_str, decorated_child_->Render());
    }
    return fmt::format("<a{}></a>", attrs_str);
}

Element AnchorDecorator::SetStyle(const std::string& style) {
    if (decorated_child_) {
        decorated_child_->SetStyle(style);
    }
    return shared_from_this();
}

Element AnchorDecorator::SetClass(const std::string& cls) {
    if (decorated_child_) {
        decorated_child_->SetClass(cls);
    }
    return shared_from_this();
}

Element AnchorDecorator::SetID(const std::string& id) {
    if (decorated_child_) {
        decorated_child_->SetID(id);
    }
    return shared_from_this();
}