#ifndef PAGE_MANAGER_HPP
#define PAGE_MANAGER_HPP

#include <map>
#include <any>
#include <string>
#include <stdexcept>
#include <functional>

class PageManager {
public:
    PageManager(const PageManager&) = delete;
    PageManager& operator=(const PageManager&) = delete;
    
    static PageManager& instance() {
        static PageManager instance;
        return instance;
    }
    
    // Регистрация компонента с произвольной сигнатурой
    template<typename Func>
    void registerComponent(const std::string& name, Func&& func) {
        components_[name] = std::function{std::forward<Func>(func)};
    }
    
    // Вызов компонента с аргументами
    template<typename... Args>
    std::string operator()(const std::string& name, Args&&... args) {
        auto it = components_.find(name);
        if (it == components_.end()) {
            throw std::runtime_error(fmt::format("Component not found: {}", name));
        }
        
        try {
            // Пытаемся получить функцию с нужной сигнатурой
            auto& func = std::any_cast<std::function<std::string(Args...)>&>(it->second);
            return func(std::forward<Args>(args)...);
        } catch (const std::bad_any_cast&) {
            // Если не удалось, пробуем как функцию без параметров
            try {
                auto& func = std::any_cast<std::function<std::string()>&>(it->second);
                return func();
            } catch (const std::bad_any_cast&) {
                throw std::runtime_error(fmt::format("Component '{}' called with wrong signature", name));
            }
        }
    }
    
    // Проверка существования компонента
    bool hasComponent(const std::string& name) const {
        return components_.find(name) != components_.end();
    }
    
    // Получение списка имен компонентов
    std::vector<std::string> getComponentNames() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : components_) {
            names.push_back(name);
        }
        return names;
    }

private:
    // Храним компоненты как any, чтобы поддерживать разные сигнатуры
    std::map<std::string, std::any> components_;

    PageManager() = default;
};

// Макрос для автоматической регистрации компонентов
#define REGISTER_COMPONENT(name, func) \
    namespace { \
        struct ComponentRegistrar_##name { \
            ComponentRegistrar_##name() { \
                PageManager::instance().registerComponent(#name, func); \
            } \
        }; \
        static ComponentRegistrar_##name component_registrar_##name; \
    }

#endif
