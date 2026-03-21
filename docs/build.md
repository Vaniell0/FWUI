# Система сборки

## Зависимости

| Библиотека | Версия | Обязательна | Назначение |
|------------|--------|-------------|-----------|
| fmt | 11.1.4 | Да | Форматирование строк |
| nlohmann_json | 3.11.3 | Да | JSON |
| inja | 3.4.0 | Да | Шаблонизатор (header-only) |
| Catch2 | 3.7.1 | Нет (BUILD_TESTS) | Тесты |

Все зависимости ищутся через `find_package()`. Если не найдены — скачиваются через CMake FetchContent.

Inja подключается через `FetchContent_Populate` (не `MakeAvailable`), потому что её CMakeLists.txt конфликтует с nlohmann_json.

## Сборка

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### С тестами

```bash
cmake -B build -DFWUI_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Таргеты

| Таргет | Тип | Назначение |
|--------|-----|-----------|
| fwui | static lib | Ядро (Node, Renderer, Registry, Template) |
| fwui-demo | executable | Демонстрация C++ API |
| fwui-bench | executable | Бенчмарки рендеринга |
| fwui-bench-mem | executable | Бенчмарки памяти |
| fwui-ssg | executable | Генератор статических сайтов |
| fwui-embed | executable | Генератор embedded pages (constexpr) |
| fwui-tests | executable | Catch2 unit-тесты (BUILD_TESTS) |

### Отдельный таргет

```bash
cmake --build build --target fwui-ssg
```

## Install

```bash
cmake --install build --prefix /usr/local
```

Устанавливает:
- `lib/libfwui.a` — статическая библиотека
- `bin/fwui-demo`, `bin/fwui-ssg`, `bin/fwui-embed` — утилиты
- `include/fwui/` — заголовки

## Nix

Проект включает `flake.nix` со всеми зависимостями:

```bash
nix develop          # войти в dev shell
cmake -B build && cmake --build build
```

## CI (GitHub Actions)

Сборка проходит на трёх платформах:

| ОС | Зависимости | Способ |
|----|------------|--------|
| Ubuntu | `apt install cmake libfmt-dev nlohmann-json3-dev` | Пакетный менеджер |
| macOS | `brew install fmt nlohmann-json` | Homebrew |
| Windows | — | CMake FetchContent (скачивает всё) |

Ruby C extension (fwui-native) дополнительно скачивает vendored headers (json.hpp, inja.hpp) через curl в CI.
