# FWUI (Functional Web User Interface)

**Декларативный C++ фреймворк для генерации веб-интерфейсов**
*Генерация HTML/CSS через C++ APIs в максимально приближённом стиле к ASCII arts*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/compiler_support)
[![CMake](https://img.shields.io/badge/CMake-3.15+-064f8c.svg)](https://cmake.org)

## Краткое описание

FWUI — это C++ фреймворк для декларативного создания веб-интерфейсов. Основная идея — предоставить средства для генерации HTML/CSS через удобный C++ API, включая продвинутую обработку изображений (ASCII-арт) и систему компонентов.

**Ключевые возможности:**
- **Декларативный C++ API** для описания веб-интерфейсов
- **Автоматическая генерация HTML/CSS** с семантической разметкой
- **Генерация ASCII-арта** из изображений для веб-отображения
- **Система компонентов**: контейнеры, текстовые элементы, декораторы
- **Готовые шаблоны**: портфолио, проекты, контактные формы
- **Интеграция с Crow** для запуска веб-сервера

## Архитектура

Процесс рендера основан на виртуальном DOM, где каждый элемент (`Node`) преобразуется в соответствующую HTML-разметку.

### Основные компоненты
```bash
├── avatar.png // демонстрационные данные
├── CMakeLists.txt // сборка
├── data // демонстрационные данные
│   └── projects.json
├── elements // реализации элементов
│   ├── containers.cpp
│   ├── decorators.cpp
│   ├── factories.cpp
│   ├── html.cpp
│   ├── image.cpp
│   └── text.cpp 
├── flake.nix // система сборки окружения
├── flake.lock // вспомагательные данные
├── include
│   ├── elements.hpp // все элементы рендера
│   ├── image.hpp // конвертация и описание ASCII
│   ├── node.hpp // основа
│   ├── pageManager.hpp // менеджер генераторов элементов
│   └── pages.hpp // основа страниц
├── README.md
├── src
│   ├── main.cpp // регистрация роутеров crow
│   ├── pages // демонстрационные данные
│   └── utils.cpp // утилиты для страниц
├── static // демонстрационные данные
│   ├── contactForm.html
│   ├── script.js
│   └── style.css
└── template // демонстрационные данные
    ├── index.html
    └── project.html
```

сервер собирает страницы из `templates/` и отправляет с ними `static/`

регистрация страниц осуществляется в `*.cpp` файлах через макрос `REGISTER_COMPONENT` с лямбдами на этапе компиляции

## Зависимости

| Библиотека | Назначение в проекте |
| --- | --- |
| [fmt](https://github.com/fmtlib/fmt) | Форматирование строк, генерация HTML-атрибутов и текстового содержимого. |
| [Crow](https://github.com/CrowCpp/Crow) | Веб-фреймворк, HTTP-сервер, роутинг запросов. Основа для сервирования страниц. |
| [Boost.Asio](https://github.com/boostorg/asio) | Зависимость Crow. |
| [OpenCV](https://github.com/opencv/opencv) | Обработка изображений, создание ASCII-артов с контрастом и цветокоррекцией. |
| [nlohmann/json](https://github.com/nlohmann/json) | Чтение конфигурационных файлов (проекты, настройки). |
