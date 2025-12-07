# pixxbind

Аугментации изображений на C++ с обёрткой в Python (pybind11) и сборкой через scikit-build-core.

## Возможности 

- Контракт: `uint8`, диапазон [0, 255], формат `H × W × C`, `C ∈ {1,2,3}`; каждая функция возвращает новый массив.
- `to_gray(img)`: при `C=3` Y = round(0.299·R + 0.587·G + 0.114·B), clamp; при `C=1` — копия.
- `brightness(img, delta:int)`: x' = clamp(x + delta).
- `contrast(img, alpha:float)`: x' = clamp(round((x − 128)·alpha + 128)).
- `add_noise(img, amp:int, seed:uint64=0)`: равномерный шум в [-amp, amp].
- `random_crop(img, out_h:int, out_w:int, seed:uint64=0)`: случайный кроп указанного размера.
- `resize_pad(img, out_h:int, out_w:int, keep_aspect:bool=true, pad:uint8=0, bilinear:bool=false)`: ресайз с паддингом; nearest или bilinear.

## Быстрый старт (локально)

```powershell
cd "pixxbind"
py -3.11 -m pip install --upgrade pip build scikit-build-core ninja cmake pybind11 numpy
py -3.11 -m build
py -3.11 -m pip install --force-reinstall dist\pixxbind-0.1.0-cp311-cp311-win_amd64.whl
py -3.11 -c "import pixxbind as px; print(px.__all__)"
```

## Сборка под другие версии

- Выберите нужный Python: `py -3.10`, `py -3.11`, `py -3.12` и т.п.
- Выполните ту же последовательность: `py -3.10 -m pip install ...` → `py -3.10 -m build` → поставить собранный wheel из `dist/`.

## Сборка в Docker (Linux, Python 3.10 slim)

```bash
docker build -t pixxbind:py310 .
```

Готовый wheel после сборки лежит в контейнере в `dist/`; можно скопировать через `docker cp` при необходимости.

## Тесты

```powershell
py -3.11 -m pip install -r tests/requirements.txt 
py -3.11 -m pytest tests
```

В ноутбуке `demo_pixxbind.ipynb` есть наглядные сравнения с OpenCV и бенчмарк скорости.

## Файлы проекта

- `src/pixxbind.cpp`, `src/helpers.hpp` — реализация C++.
- `CMakeLists.txt` — сборка расширения `pixxbind_cpp`.
- `pyproject.toml` — конфигурация scikit-build-core/pybind11.
- `Dockerfile`, `Makefile` — сборка/упрощённые команды.
- `tests/test_bindings.py` — автотесты NumPy.
- `demo_pixxbind.ipynb` — демо, сравнения с OpenCV, замеры времени.

(данный ридми сгенерирован)
