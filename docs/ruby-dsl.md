# Ruby DSL

[← Главная](index.md) · [pipe-operator](pipe-operator.md) · [components](components.md)

## DSL Mixin

Чтобы не писать `FWUI.` перед каждым вызовом (аналог C++ `using namespace fwui`):

```ruby
module Pages::MyPage
  extend FWUI::DSL   # теперь h1(), div(), Bold() доступны напрямую

  def self.build
    h1("Hello") | Bold() | Color("red")
  end
end
```

`FWUI::DSL` делегирует все singleton-методы FWUI через `define_method`. Используется с `extend`.

## Элементы

### Текстовые

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

| Метод | HTML |
|-------|------|
| `h1(text)` — `h6(text)` | `<h1>` — `<h6>` |
| `p(text)` | `<p>` |
| `span_elem(text)` | `<span>` |
| `strong(text)` | `<strong>` |
| `em(text)` | `<em>` |
| `code(text)` | `<code>` |
| `pre(text)` | `<pre>` |

### Контейнеры

```ruby
div([h1("Hello"), p("World")])
```

```html
<div>
  <h1>Hello</h1>
  <p>World</p>
</div>
```

| Метод | HTML | Описание |
|-------|------|----------|
| `div(children)` | `<div>` | Универсальный контейнер |
| `section(children)` | `<section>` | Секция |
| `article(children)` | `<article>` | Статья |
| `nav(children)` | `<nav>` | Навигация |
| `header(children)` | `<header>` | Шапка |
| `footer(children)` | `<footer>` | Подвал |
| `main_elem(children)` | `<main>` | Основной контент |
| `aside(children)` | `<aside>` | Боковая панель |

### Layout

```ruby
hbox([
  div([p("Left")]).flex_grow,
  div([p("Right")]).flex_grow,
]).gap(16)
```

```html
<div style="display: flex; flex-direction: row; gap: 16px">
  <div style="flex-grow: 1"><p>Left</p></div>
  <div style="flex-grow: 1"><p>Right</p></div>
</div>
```

| Метод | Описание |
|-------|----------|
| `vbox(children)` | `<div>` с `display: flex; flex-direction: column` |
| `hbox(children)` | `<div>` с `display: flex; flex-direction: row` |

### Списки

```ruby
ul([
  li("Первый"),
  li("Второй"),
  li("Третий"),
])
```

```html
<ul>
  <li>Первый</li>
  <li>Второй</li>
  <li>Третий</li>
</ul>
```

### Формы

```ruby
form([
  label("Email"),
  input_elem("email", name: "email", placeholder: "you@example.com"),
  button("Send") | HxPost("/api/submit") | HxTarget("#result"),
])
```

```html
<form>
  <label>Email</label>
  <input type="email" name="email" placeholder="you@example.com">
  <button hx-post="/api/submit" hx-target="#result">Send</button>
</form>
```

| Метод | Описание |
|-------|----------|
| `form(children)` | `<form>` |
| `input_elem(type, **attrs)` | `<input>` |
| `textarea(content)` | `<textarea>` |
| `button(label)` | `<button>` |
| `label(content)` | `<label>` |
| `select_elem(options)` | `<select>` |
| `option(label, value:)` | `<option>` |

### Ссылки и медиа

```ruby
a("GitHub", href: "https://github.com", target: "_blank")
```

```html
<a href="https://github.com" target="_blank">GitHub</a>
```

```ruby
img("/photo.png", alt: "Photo")
```

```html
<img src="/photo.png" alt="Photo">
```

### Таблицы

```ruby
table([
  thead([tr([th("Имя"), th("Возраст")])]),
  tbody([
    tr([td("Иван"), td("18")]),
    tr([td("Мария"), td("20")]),
  ])
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

### Прочее

| Метод | Описание |
|-------|----------|
| `br()` | `<br>` |
| `hr()` | `<hr>` |
| `raw(html)` | Сырой HTML без экранирования |
| `text(content)` | Текстовый узел |
| `node(tag, ...)` | Произвольный элемент |

## Chainable методы

Каждый метод возвращает `self`, позволяя цепочки:

```ruby
h1("Title")
  .bold.italic.color("#333")
  .font_size("2rem").center
  .set_class("main-title").set_id("title")
```

```html
<h1 id="title" class="main-title"
    style="font-weight: bold; font-style: italic; color: #333;
           font-size: 2rem; display: flex; justify-content: center;
           align-items: center">
  Title
</h1>
```

Основные категории:
- **Типографика**: `bold`, `italic`, `underline`, `strikethrough`, `font_size`, `font_family`, `letter_spacing`, `line_height`, `text_transform`
- **Цвета**: `color(c)`, `bg_color(c)`
- **Layout**: `center`, `padding(v)`, `margin(v)`, `gap(g)`, `flex_wrap`, `flex_grow`, `justify_content`, `align_items`
- **Размеры**: `width(w)`, `height(h)`, `min_width`, `max_width`, `min_height`, `max_height`
- **Визуальные**: `border(b)`, `border_radius(r)`, `opacity(v)`, `box_shadow(s)`, `transform(t)`, `filter(f)`
- **Позиционирование**: `position(p)`, `z_index(z)`, `top/right/bottom/left(v)`
- **Атрибуты**: `set_attr(k, v)`, `set_style(p, v)`, `add_class(c)`, `set_class(c)`, `set_id(id)`, `data(k, v)`
- **HTMX**: `hx_get`, `hx_post`, `hx_target`, `hx_swap`, `hx_trigger`, `hx_push_url`, `hx_vals`, `hx_confirm`

## Документ

```ruby
document("Page Title",
  head: [
    stylesheet("/static/style.css"),
    google_font("Inter:wght@400;700"),
    style_elem("body { margin: 0; }"),
  ],
  body: [
    h1("Hello"),
    p("World"),
  ],
  body_attrs: { class: "dark" }
)
```

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Page Title</title>
  <link rel="stylesheet" href="/static/style.css">
  <link rel="stylesheet"
        href="https://fonts.googleapis.com/css2?family=Inter:wght@400;700&display=swap">
  <style>body { margin: 0; }</style>
</head>
<body class="dark">
  <h1>Hello</h1>
  <p>World</p>
</body>
</html>
```

Автоматически добавляет `<meta charset="UTF-8">`, `<meta viewport>`, `<title>`.

## Файловые хелперы

| Метод | Описание |
|-------|----------|
| `stylesheet(href)` | `<link rel="stylesheet" href="...">` |
| `script(src)` | `<script src="...">` |
| `script_inline(code)` | `<script>code</script>` |
| `style_elem(css)` | `<style>css</style>` |
| `html_file(path)` | Загрузка HTML из файла как raw |
| `style_file(path)` | Загрузка CSS из файла в `<style>` |
| `script_file(path)` | Загрузка JS из файла в `<script>` |
| `google_font(family)` | Google Fonts `<link>` |

## Layout

Стандартная структура страницы — navbar + container + footer:

```ruby
layout("My Site",
  head: [stylesheet("/static/style.css")],
  navbar: [
    a("Home", href: "/") | AddClass("nav-link"),
    a("About", href: "/about") | AddClass("nav-link"),
  ],
  body: [
    h1("Hello") | Bold(),
    p("World"),
  ],
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
    <a class="nav-link" href="/" target="_self">Home</a>
    <a class="nav-link" href="/about" target="_self">About</a>
  </nav>
  <div class="container">
    <h1 style="font-weight: bold">Hello</h1>
    <p>World</p>
  </div>
  <footer class="footer">
    <p style="display: flex; justify-content: center; align-items: center">
      Built with FWUI
    </p>
  </footer>
</body>
</html>
```

Все параметры опциональны — пустой `navbar:` или `footer:` просто не рендерит соответствующий блок.

## Рендеринг

```ruby
node = h1("Hello").bold

node.to_html   # HTML строка
node.to_json   # JSON AST
```

```html
<h1 style="font-weight: bold">Hello</h1>
```

```json
{
  "tag": "h1",
  "styles": { "font-weight": "bold" },
  "text": "Hello"
}
```
