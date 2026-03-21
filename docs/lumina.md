# Lumina

[[data/projects/FWUI/docs/index|← Главная]] · [[server]] · [[websocket]]

## Обзор

Lumina — генератор примитивного арта из изображений. Превращает фотографии в SVG с помощью 6 стилей рендеринга. Встроена в FWUI как CMake-библиотека.

## Стили

| Стиль | Описание |
|-------|----------|
| `ascii` | Символьный рендеринг с Unicode глифами, ориентацией краёв и адаптивной яркостью |
| `stipple` | Пуантилизм — точечный рендеринг с варьирующейся плотностью |
| `hatching` | Штриховка — линейный рендеринг для передачи светотени |
| `contour` | Контурные линии — выделение границ и краёв |
| `mosaic` | Мозаика — блочная абстракция с усреднёнными цветами |
| `voronoi` | Диаграммы Вороного — генеративная геометрия из случайных точек |

## Ruby API

```ruby
require_relative 'lib/lumina'

# Проверка доступности
Lumina.available?  # => true

# Список стилей
Lumina.styles  # => ["ascii", "stipple", "hatching", "contour", "mosaic", "voronoi"]

# Версия
Lumina.version  # => "0.2.0"

# Рендеринг
svg = Lumina.render_svg(
  image_path: "static/avatar.png",
  style: "ascii",
  config: {
    "charset" => "extended",
    "min_block_size" => 6,
    "variance_threshold" => 8.0
  }
)
```

### Конфигурация

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|-------------|----------|
| `charset` | String | `"extended"` | Набор символов для ASCII: extended, basic, blocks, braille |
| `min_block_size` | Integer | `6` | Минимальный размер блока квадтри |
| `variance_threshold` | Float | `8.0` | Порог дисперсии для разделения блоков |

## HTTP API

```
GET /api/lumina/styles
```

Ответ:
```json
{
  "styles": ["ascii", "stipple", "hatching", "contour", "mosaic", "voronoi"],
  "version": "0.2.0"
}
```

```
GET /api/lumina/render?style=ascii&image=avatar.png&block_min=6&variance=8
```

Ответ: SVG (`image/svg+xml`).

## WebSocket API

Через [[websocket]]:

```json
// Запрос
{
  "type": "lumina_preview",
  "style": "ascii",
  "image": "avatar.png",
  "config": {
    "min_block_size": 6,
    "variance_threshold": 8.0
  }
}

// Ответ (успех)
{ "type": "lumina_result", "svg": "<svg>...</svg>" }

// Ответ (ошибка)
{ "type": "lumina_error", "error": "Render failed" }
```

## Live Preview

На портфолио есть интерактивная панель Lumina preview:

- **Выбор стиля** — `<select>` с 6 опциями
- **Размер блока** — `<input type="range" min="2" max="16">`
- **Порог дисперсии** — `<input type="range" min="1" max="30">`
- **Кнопка "Обновить"** — отправляет запрос через WS (fallback HTTP)

SVG результат отображается в контейнере с тёмным фоном.

## Аватар

На странице "Обо мне" Lumina рендерит ASCII-аватар из `static/avatar.png`:

```ruby
ascii_svg_path = File.join(base_dir, 'static', 'avatar_ascii.svg')
if File.exist?(ascii_svg_path)
  div([raw(File.read(ascii_svg_path))]) | Class("lumina-avatar")
else
  img("/static/avatar.png", alt: "Avatar")
end
```

SVG предварительно сгенерирован и сохранён как `avatar_ascii.svg`.

## Архитектура

```
OpenCV (imread, analyze)
       ↓
QuadTree (adaptive block splitting)
       ↓
StyleModule (ascii/stipple/hatching/contour/mosaic/voronoi)
       ↓
SVG output (<rect>, <text>, <circle>, <line>, <polygon>)
```

Каждый стиль — отдельный `StyleModule` с методом `render(block, svg_builder)`.
