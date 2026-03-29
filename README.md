# FWUI

Ruby DSL для декларативной генерации HTML. Zero dependencies — только Ruby stdlib. Chainable API, pipe-оператор `|`, HTMX из коробки, Registry компонентов, hot-reload.

```ruby
require 'fwui'

puts FWUI.h1("Hello").bold.color("#4f46e5").to_html
```

```html
<h1 style="font-weight: bold; color: #4f46e5">Hello</h1>
```

## Установка

```sh
gem install fwui                   # чистый Ruby (кроссплатформа)
gem install fwui-native            # опционально: C extension (~3x быстрее)
```

| Gem | Содержимое | Лицензия |
|-----|-----------|----------|
| `fwui` | Чистый Ruby, кроссплатформа | Apache 2.0 |
| `fwui-native` | C extension + Inja (компилируется при установке) | BSL 1.1 |

`fwui` не требует внешних зависимостей — только Ruby stdlib (Socket, JSON).

## Ruby DSL

### Элементы

```ruby
h1("Заголовок")
p("Параграф")
strong("Жирный")
```

```html
<h1>Заголовок</h1>
<p>Параграф</p>
<strong>Жирный</strong>
```

```ruby
div([h1("Hello"), p("World")])
```

```html
<div><h1>Hello</h1><p>World</p></div>
```

### Списки

```ruby
ul([li("Первый"), li("Второй"), li("Третий")])
```

```html
<ul>
  <li>Первый</li>
  <li>Второй</li>
  <li>Третий</li>
</ul>
```

### Таблицы

```ruby
table([
  thead([tr([th("Имя"), th("Возраст")])]),
  tbody([tr([td("Иван"), td("18")]), tr([td("Мария"), td("20")])])
])
```

```html
<table>
  <thead>
    <tr><th>Имя</th><th>Возраст</th></tr>
  </thead>
  <tbody>
    <tr><td>Иван</td><td>18</td></tr>
    <tr><td>Мария</td><td>20</td></tr>
  </tbody>
</table>
```

### Chainable-модификаторы

```ruby
h1("Title").bold.italic.color("#333").font_size("2rem").center.set_class("main-title")
```

```html
<h1 class="main-title"
    style="font-weight: bold; font-style: italic; color: #333;
           font-size: 2rem; display: flex; justify-content: center;
           align-items: center">
  Title
</h1>
```

### Pipe-оператор

```ruby
extend FWUI::DSL

h1("Hello") | Bold() | Color("red") | Class("title")
```

```html
<h1 class="title" style="font-weight: bold; color: red">Hello</h1>
```

```ruby
button("Send") | HxPost("/api/submit") | HxTarget("#result")
```

```html
<button hx-post="/api/submit" hx-target="#result">Send</button>
```

35+ декораторов: типографика, цвета, layout, размеры, рамки, HTMX.

### Layout

Стандартная структура страницы одной строкой:

```ruby
extend FWUI::DSL

layout("My Site",
  head: [stylesheet("/static/style.css")],
  navbar: [a("Home", href: "/"), a("About", href: "/about")],
  body: [h1("Hello") | Bold(), p("World")],
  footer: [p("Built with FWUI") | Center()]
)
```

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>My Site</title>
  <link rel="stylesheet" href="/static/style.css">
</head>
<body>
  <nav class="navbar">
    <a href="/" ...>Home</a>
    <a href="/about" ...>About</a>
  </nav>
  <div class="container">
    <h1 style="font-weight: bold">Hello</h1>
    <p>World</p>
  </div>
  <footer class="footer">
    <p style="...">Built with FWUI</p>
  </footer>
</body>
</html>
```

### Registry

```ruby
registry = FWUI::Registry.new

registry.register_page("/") do |data|
  FWUI.layout("Site",
    head: [FWUI.stylesheet("/static/style.css")],
    navbar: [FWUI.a("Home", href: "/")],
    body: [FWUI.h1("Welcome")]
  )
end

puts registry.create_page("/").to_html
```

### Автозагрузка страниц

```ruby
FWUI::PageLoader.load_pages(registry, "ruby/pages")
```

Каждый файл — модуль `Pages::ИмяФайла` с методом `register(registry)`.

### CLI

```sh
fwui new mysite          # создать проект
fwui new page about      # сгенерировать страницу
fwui build               # отрендерить pages/ → dist/
fwui serve               # dev сервер с hot-reload
```

### Сервер

```sh
ruby examples/ruby/server.rb            # production: http://localhost:10101
ruby examples/ruby/server.rb --dev      # hot-reload (CSS-only + full reload)
```

Stdlib Socket, без гемов. WebSocket RFC 6455, HTMX фрагменты.

---

## fwui-native — C extension (BSL 1.1)

Ускорение рендера в 2-4x (до 600x+ с кешем) + Inja template engine. Подключается одной строкой:

```ruby
require 'fwui/native'  # Node#to_html → C рендерер + Inja
```

### Оптимизации

- **Iterative render** — explicit stack в C вместо рекурсии
- **ROBJECT_IVPTR** — прямой доступ к ivars, O(1)
- **Render cache** — `@_html_cache`, повторный рендер O(1)
- **Subtree cache** — bubble-up invalidation: мутация leaf-ноды сбрасывает только цепочку предков, кеши sibling-поддеревьев сохраняются. Re-render переиспользует кешированный HTML неизменённых поддеревьев
- **Reusable buffer** — один malloc на процесс

### Inja Template Engine

Jinja2-совместимый шаблонизатор — циклы, условия, функции, include:

```ruby
FWUI::Native.render_template("Hello {{ name }}!", { name: "World" })
```

```
Hello World!
```

```ruby
tpl = <<~INJA
  {% for card in cards %}
  <div class="card">
    <h1>{{ card.title }}</h1>
    <p>{{ card.desc }}</p>
  </div>
  {% endfor %}
INJA

FWUI::Native.render_template(tpl, { cards: [
  { title: "Project A", desc: "Description A" },
  { title: "Project B", desc: "Description B" },
]})
```

```html
<div class="card">
  <h1>Project A</h1>
  <p>Description A</p>
</div>
<div class="card">
  <h1>Project B</h1>
  <p>Description B</p>
</div>
```

```ruby
# Шаблоны из файлов с поддержкой {% include %}
FWUI::Native.set_template_directory("templates/")
FWUI::Native.render_template_file("page.html", { title: "Home" })
```

### Baked Templates

Компиляция Node-шаблона в статические чанки + слоты:

```ruby
FWUI::Native.bake(:card, FWUI.div([
  FWUI.h1(FWUI.slot(:title)).bold,
  FWUI.p(FWUI.slot(:desc)),
]).add_class("card"))

FWUI::Native.render_baked(:card, title: "Project X", desc: "A cool project")
```

```html
<div class="card">
  <h1 style="font-weight: bold">Project X</h1>
  <p>A cool project</p>
</div>
```

### PackedBuilder

Opcode stream — рендер без Node-объектов:

```ruby
FWUI::Native.build {
  div {
    h1("Title").bold.color("blue")
    p("Content").padding("10px")
  }.add_class("container")
}
```

```html
<div class="container">
  <h1 style="font-weight: bold; color: blue">Title</h1>
  <p style="padding: 10px">Content</p>
</div>
```

### DOCX / ODT Export

Node tree → DOCX / ODT. Zero dependencies — ZIP + XML в C++:

```ruby
# Базовый экспорт
File.binwrite("report.docx", tree.to_docx)
File.binwrite("report.odt", tree.to_odt)

# С опциями
tree.to_docx({
  "page_size" => "A4",
  "default_font" => "Times New Roman",
  "default_font_size" => "14pt",
  "line_spacing" => "1.5",
  "first_line_indent" => "1.25cm",
  "header" => "Курсовая работа",
  "page_numbers" => true,
})
```

ГОСТ пресет (А4, Times New Roman 14pt, 1.5 интервал, красная строка 1.25cm, поля 30/15/20/20mm):

```ruby
FWUI::Doc.to_docx(tree, preset: :gost, page_numbers: true)
FWUI::Doc.to_odt(tree, preset: :gost)
```

Разрывы страниц:

```ruby
FWUI.page_break                                              # фабрика
FWUI.p("Новая страница").set_style("page-break-before", "always")  # CSS
```

Таблицы, списки, изображения, гиперссылки, колонтитулы, нумерация страниц, colspan/rowspan.

### Performance

500 карточек, 300 итераций, GC off:

| Метод | ms/render | vs Ruby |
|-------|-----------|---------|
| Ruby `to_html` | ~0.60 | 1.0x |
| C ext cold | ~0.19 | 3.2x |
| Registry cache | ~0.003 | 200x |
| Node cache | <0.001 | >600x |
| Baked (1 card) | ~0.001 | 600x |
| PackedBuilder (500) | ~0.45 | 1.3x |
| Partial invalidation (2 mut / 50 widgets) | ~0.014 | 143x |

---

## C++ API (BSL 1.1)

```cpp
#include <fwui/fwui.hpp>
using namespace fwui;

auto page = document("Title", {
    link_elem({{"rel", "stylesheet"}, {"href", "/style.css"}}),
}, {
    h1("Hello") | Bold() | Color("blue"),
    div({ paragraph("Content") }) | SetClass("container"),
});

std::string html = HtmlRenderer::RenderToString(page);
```

Registry, pipe-декораторы, HTMX, JSON рендерер, Inja шаблоны, SSG генератор.

---

## Архитектура

```
FWUI/
├── ruby/                  # Ruby DSL (Apache 2.0)
│   ├── lib/fwui.rb        #   Node, фабрики, pipe, layout, DSL mixin
│   ├── lib/fwui/          #   Registry, PageLoader, WebSocket, CLI
│   └── pages/             #   страницы (автозагрузка)
├── fwui-native/           # C extension + Inja (BSL 1.1)
│   ├── ext/fwui_native/   #   fwui_native.c, fwui_inja.cpp, fwui_docx.cpp
│   ├── lib/fwui/native/   #   packed_builder.rb, docx.rb, doc.rb
│   └── test/              #   тесты + бенчмарки
├── examples/              # Ruby + C++ примеры
├── include/fwui/          # C++ API (BSL 1.1)
├── src/                   # C++ реализация
├── static/                # CSS
├── templates/             # Inja шаблоны
├── docs/                  # документация
└── .github/workflows/     # CI
```

## Сборка из исходников

```sh
# Ruby DSL — работает сразу
ruby examples/ruby/server.rb

# fwui-native (C extension + Inja)
cd fwui-native && rake compile && ruby test/test_native.rb

# C++ (полная сборка)
cmake -B build -G Ninja && cmake --build build
```

## Документация

Подробная документация: [`docs/`](docs/index.md)

- Ruby DSL, layout, pipe-оператор, Registry, сервер, WebSocket, hot-reload
- fwui-native: оптимизации, baked templates, PackedBuilder, Inja, DOCX/ODT, бенчмарки
- C++ API: Node, Renderer, Registry, Inja, PDF, SSG

## Why FWUI

### Что используют сейчас

Админки и дашборды в продакшене — это обычно тяжёлый стек:

| Стек | RSS на процесс | Что внутри |
|------|----------------|------------|
| Next.js SSR | 150-300 MB | Node.js + React + Webpack/Turbopack + SSR runtime |
| Express + EJS | 50-100 MB | Node.js + Express + шаблонизатор + middleware |
| Django + Gunicorn | 100-160 MB / воркер | Python + ORM + template engine |
| Rails + Puma | 150-300 MB / воркер | Ruby + ActiveRecord + ERB/Haml |
| **FWUI (Ruby Native)** | **~18 MB** | Ruby + C extension, zero dependencies |

На роутерах и embedded-устройствах стек другой: OpenWrt LuCI — Lua + uHTTPd (8-32 MB RAM), MikroTik WebFig — vanilla JS + проприетарный протокол, pfSense — PHP + nginx. FWUI C++ API компилируется в нативный бинарник без runtime и работает на таких устройствах напрямую.

### Где FWUI заменяет весь стек

- **Admin dashboards и monitoring panels** — partial invalidation: обновление 2 виджетов из 50 за 0.014 ms, 98% кеша переиспользуется, 14 объектов/тик вместо тысяч
- **Web UI для роутеров и embedded** — C++ API: нативный бинарник без runtime, заменяет Lua/PHP стеки на устройствах с ограниченной памятью. Ruby версия — 18 MB RSS, тоже подходит для не самых тесных устройств
- **Email template generation** — чистый HTML output, без браузера, без Node.js toolchain
- **Генерация документов** — DOCX/ODT/PDF экспорт из одного Node tree, ГОСТ пресет для академических работ
- **HTMX фрагменты** — SPA-like интерактивность без JS фреймворков, WebSocket hot-reload из коробки
- **Static site generation** — CLI: `fwui build`, рендер pages/ → dist/

### Dashboard за N шагов

**FWUI — 4 шага, 1 файл:**

```bash
gem install fwui
fwui new dashboard
```

```ruby

# pages/index.rb — единственный файл
module Pages::Index
  def self.register(r)
    r.register_page("/") do |data|
      widgets = (1..50).map { |i|
        FWUI.div([
          FWUI.h3("Widget #{i}").bold,
          FWUI.span_elem("#{data[:metrics][i] || 0}").font_size("32px").bold,
          FWUI.p("Live").color("#999"),
        ]).add_class("widget").set_style("padding", "16px")
      }
      FWUI.layout("Dashboard",
        body: [FWUI.div(widgets).set_style("display", "grid")
                .set_style("grid-template-columns", "repeat(auto-fill, minmax(280px, 1fr))")],
      )
    end
  end
end
```

```bash

fwui serve  # готово — сервер, hot-reload, WebSocket, HTMX
```

Partial update — `widget.set_text(new_val)`, остальные 49 виджетов отдаются из subtree cache.

**Node.js (Express + EJS + Socket.IO) — минимальный стек для того же:**

| Шаг | Что делать | Файл |
|-----|-----------|------|
| 1 | Установить Node.js runtime | — |
| 2 | `npm init -y` | package.json |
| 3 | `npm install express ejs socket.io` | node_modules/ (70+ пакетов) |
| 4 | Написать HTTP сервер + роуты | server.js |
| 5 | Настроить view engine, static middleware | server.js |
| 6 | Layout шаблон | views/layout.ejs |
| 7 | Dashboard шаблон | views/dashboard.ejs |
| 8 | Widget partial | views/partials/widget.ejs |
| 9 | CSS | public/style.css |
| 10 | Socket.IO сервер (для live update) | server.js |
| 11 | Socket.IO клиент (JS в браузере) | public/dashboard.js |
| 12 | Запустить + nodemon для dev | — |

Partial render невозможен — каждый запрос рендерит все 50 виджетов заново. Для live-обновлений нужен Socket.IO на сервере + клиентский JS для обработки событий и манипуляции DOM.

### Сравнение

| | FWUI | Node.js (Express + EJS + Socket.IO) |
|--|------|-------------------------------------|
| Зависимости | 1 gem (0 transitive) | 3 npm пакета (70+ transitive) |
| Файлы проекта | 1 | 6+ |
| Конфиги | 0 | package.json |
| Клиентский JS | 0 | Socket.IO client + DOM логика |
| RSS процесса | ~18 MB | ~50-100 MB |
| 50-widget cold render | 0.19 ms | 0.93 ms (5x медленнее) |
| Partial update (2/50) | 0.014 ms | 0.93 ms (66x медленнее, full re-render) |
| GC при partial update | 14 объектов/тик | нет partial — full rebuild |
| Live update сервер | Встроен (WebSocket + HTMX) | Socket.IO (+1 зависимость) |
| Live update клиент | 0 строк JS | JS: подписка + DOM update |
| Hot-reload | `fwui serve` (встроен) | nodemon (+1 зависимость) |
| `node_modules/` | — | ~8 MB на диске |
| Embedded / роутер | C++ бинарник без runtime; Ruby 18 MB | Не применимо (V8 runtime) |

### Бенчмарки

Из `bench_dashboard.rb` и `bench_memory.rb` (50 widgets, 500 тиков, 2 мутации/тик):

- **Partial invalidation:** 13.6x быстрее полного re-render, 143x быстрее полного rebuild
- **98% меньше GC давления:** 14 объектов/тик vs 763 (full wipe) vs 9918 (rebuild)
- **0 GC runs** за 100 тиков partial invalidation
- **100% кеш покрытие:** все 1507 нод кешируются
- **C++ render:** <0.001 ms/render cached, >600x vs Ruby

## Лицензия

| Компонент | Лицензия |
|-----------|----------|
| `ruby/`, `static/` | Apache 2.0 |
| `include/`, `src/`, `fwui-native/` | BSL 1.1 |

Ruby DSL работает полностью самостоятельно без C++ или fwui-native. Подробности: [LICENSE](LICENSE.txt).
