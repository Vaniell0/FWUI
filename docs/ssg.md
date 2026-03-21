# Static Site Generator

## Что это

`fwui-ssg` — CLI-утилита, которая рендерит зарегистрированные страницы в статические HTML-файлы.

## Сборка

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target fwui-ssg
```

## Использование

```bash
./build/fwui-ssg [options]
```

### Флаги

| Флаг | Описание | По умолчанию |
|------|----------|-------------|
| --output, -o <dir> | Директория вывода | dist/ |
| --data, -d <file.json> | Глобальный JSON-контекст | — |
| --templates, -t <dir> | Директория Inja-шаблонов | — |
| --pages <dir> | Директория шаблонных страниц (*.html) | — |
| --data-dir <dir> | Директория JSON-данных для страниц | data/ |
| --minify | Минификация HTML | — |
| --pretty | Красивый вывод | по умолчанию |
| --clean | Удалить dist/ перед сборкой | — |

## Система страниц

### C++ страницы (встроенные)

Регистрируются через `pages::RegisterSSGPages(registry)` в `src/pages/ssg_pages.cpp`. Используют C++ DSL:

```cpp
registry.RegisterPage("/", [](const json& data) {
    return document("Home", {stylesheet("/static/style.css")}, {
        h1("Welcome"), p("Content here")
    });
});
```

### Шаблонные страницы (Inja)

Когда указан `--pages dir`, загружаются `*.html` файлы через TemplatePageLoader:

- `pages/index.html` → роут `/`
- `pages/about.html` → роут `/about`
- `pages/projects/detail.html` → роут `/projects/detail`

Синтаксис — Jinja2:
```html
<h1>{{ title }}</h1>
{% for item in items %}
  <div>{{ item.name }}</div>
{% endfor %}
{% include "header.html" %}
```

## Данные

### Для C++ страниц (RegisterPage)

Данные передаются через `--data site.json` (явный флаг). Автозагрузка из `data/` не происходит.

### Для шаблонных страниц (TemplatePageLoader)

Приоритет слияния (последний побеждает):
1. Глобальные: все `*.json` из `--data-dir` (`site.json` сливается на корневой уровень, остальные — по имени файла: `projects.json` → `data.projects`)
2. Per-page: `data/{stem}.json` — по имени файла (не по роуту). Пример: `pages/blog/post.html` → `data/post.json`
3. Runtime: переданные при вызове `CreatePage(route, data)`

## Роутинг

| Файл | Роут | Выход |
|------|------|-------|
| pages/index.html | / | dist/index.html |
| pages/about.html | /about | dist/about/index.html |
| pages/blog/post.html | /blog/post | dist/blog/post/index.html |

## Static файлы

Директория `static/` копируется в `dist/static/` рекурсивно.

## Пример от нуля

```bash
mkdir -p pages data static
echo '<h1>{{ title }}</h1><p>{{ description }}</p>' > pages/index.html
echo '{"title": "My Site", "description": "Built with FWUI"}' > data/site.json
echo 'body { font-family: sans-serif; }' > static/style.css

cmake --build build --target fwui-ssg
./build/fwui-ssg --pages pages --data-dir data --clean
# dist/index.html ready
```
