# Hot-Reload

[[data/projects/FWUI/docs/index|← Главная]] · [[server]] · [[websocket]]

## Обзор

Hot-reload автоматически обновляет браузер при изменении файлов во время разработки. Активируется через `--dev` или `FWUI_DEV=1`.

Два типа обновлений:
- **Full reload** — при изменении `.rb` файлов (перезагрузка страницы)
- **CSS-only reload** — при изменении `.css` файлов (без перезагрузки)

## FileWatcher

`FWUI::FileWatcher` отслеживает директории через polling `File.mtime`:

```ruby
watcher = FWUI::FileWatcher.new(
  [PAGES_DIR, STATIC_DIR],
  extensions: ['.rb', '.css', '.js', '.html'],
  interval: 0.5  # 500ms
)

watcher.on_change do |changed_files|
  ruby_files = changed_files.select { |f| f.end_with?('.rb') }
  css_files  = changed_files.select { |f| f.end_with?('.css') }

  # Перезагрузка Ruby модулей
  ruby_files.each do |file|
    load file
    mod_name = File.basename(file, '.rb').split('_').map(&:capitalize).join
    Pages.const_get(mod_name).register(registry)
  end

  # Broadcast через WebSocket
  if css_files.any? && ruby_files.empty?
    broadcast({ type: 'css_reload' })
  else
    broadcast({ type: 'reload', file: names })
  end
end

watcher.start  # запускает фоновый поток
watcher.stop   # останавливает
```

Фичи:
- Детектит новые и удалённые файлы
- Отслеживает только указанные расширения
- Thread-safe (работает в фоновом потоке)

## JS клиент (Hot-Reload)

Константа `FWUI::HOT_RELOAD_JS` содержит JavaScript, который инжектится перед `</body>` в dev mode:

```ruby
resp[:body] = resp[:body].sub('</body>',
  "<script>#{FWUI::HOT_RELOAD_JS}</script></body>")
```

Что делает JS:
1. Подключается к `ws://host/__fwui_ws`
2. При `{ type: 'reload' }` → `location.reload()`
3. При `{ type: 'css_reload' }` → перезагружает `<link>` теги без reload страницы
4. Авто-реконнект с exponential backoff при обрыве

CSS-only reload работает через добавление query-параметра к href:

```javascript
link.href = link.href.split('?')[0] + '?_=' + Date.now();
```

## Схема работы

```
Сохранение файла
      ↓
FileWatcher (polling 500ms)
      ↓
on_change callback
      ↓
  ┌───┴───┐
  │       │
.rb     .css
  │       │
load    broadcast
  +       css_reload
register    │
  +       ↓
broadcast  JS: reload
reload    <link> tags
  │
  ↓
JS: location.reload()
```

## Настройка

FileWatcher конфигурируется при создании:

| Параметр | По умолчанию | Описание |
|----------|-------------|----------|
| `dirs` | — | Массив директорий для отслеживания |
| `extensions` | `['.rb', '.css', '.js', '.html']` | Расширения файлов |
| `interval` | `0.5` | Интервал опроса в секундах |
