#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include "helpers.hpp"

namespace py = pybind11;
using pixx::sat_i;
using pixx::sat_f;
using pixx::validate_image;

namespace pixx {
/**
 * Функция: to_gray
 * Что делает: Конвертирует изображение в градации серого, применяется
 *           Y = round(0.299*R + 0.587*G + 0.114*B), затем насыщение [0,255].
 * Аргументы: img — uint8.
 * Возврат: uint8.
 * Эксепшенс: std::runtime_error при некорректном формате входа.
 */
py::array_t<uint8_t> to_gray(
    py::array_t<uint8_t, py::array::c_style | py::array::forcecast> img
) {
    auto shape = validate_image(img);
    const auto H = shape.h, W = shape.w, C = shape.c;
    std::vector<py::ssize_t> out_shape{H, W, 1};
    py::array_t<uint8_t> out(out_shape);

    auto src = img.unchecked<3>();
    auto dst = out.mutable_unchecked<3>();

    if (C == 1) {
        for (py::ssize_t y = 0; y < H; ++y) {
            for (py::ssize_t x = 0; x < W; ++x) {
                dst(y, x, 0) = src(y, x, 0);
            }
        }
        return out;
    }
    for (py::ssize_t y = 0; y < H; ++y) {
        for (py::ssize_t x = 0; x < W; ++x) {
            int R = src(y, x, 0);
            int G = src(y, x, 1);
            int B = src(y, x, 2);
            int Y = static_cast<int>(std::round(0.299 * R + 0.587 * G + 0.114 * B));
            dst(y, x, 0) = sat_i(Y);
        }
    }
    return out;
}

/**
 * Функция: brightness
 * Что делает: Регулировка яркости для каждого канала.
 * Аргументы: img — uint8, delta — смещение.
 * Возврат: uint8.
 * Эксепшенс: std::runtime_error при некорректном формате входа.
 */
py::array_t<uint8_t> brightness(
    py::array_t<uint8_t, py::array::c_style | py::array::forcecast> img,
    int delta
) {
    auto shape = validate_image(img);
    const auto H = shape.h, W = shape.w, C = shape.c;
    std::vector<py::ssize_t> out_shape{H, W, C};
    py::array_t<uint8_t> out(out_shape);

    auto src = img.unchecked<3>();
    auto dst = out.mutable_unchecked<3>();

    for (py::ssize_t y = 0; y < H; ++y) {
        for (py::ssize_t x = 0; x < W; ++x) {
            for (py::ssize_t c = 0; c < C; ++c) {
                dst(y, x, c) = sat_i(static_cast<int>(src(y, x, c)) + delta);
            }
        }
    }
    return out;
}

/**
 * Функция: contrast
 * Что делает: Регулировка контраста.
 * Аргументы: img — uint8, alpha — коэффициент контраста.
 * Возврат: uint8.
 * Эксепшенс: std::runtime_error при некорректном формате входа.
 */
py::array_t<uint8_t> contrast(
    py::array_t<uint8_t, py::array::c_style | py::array::forcecast> img,
    float alpha
) {
    auto shape = validate_image(img);
    const auto H = shape.h, W = shape.w, C = shape.c;
    std::vector<py::ssize_t> out_shape{H, W, C};
    py::array_t<uint8_t> out(out_shape);

    auto src = img.unchecked<3>();
    auto dst = out.mutable_unchecked<3>();

    for (py::ssize_t y = 0; y < H; ++y) {
        for (py::ssize_t x = 0; x < W; ++x) {
            for (py::ssize_t c = 0; c < C; ++c) {
                float v = (static_cast<int>(src(y, x, c)) - 128) * alpha + 128.0f;
                int vi = static_cast<int>(std::round(v));
                dst(y, x, c) = sat_i(vi);
            }
        }
    }
    return out;
}

/**
 * Функция: add_noise
 * Что делает: Добавляет равномерный целочисленный шум ко всем пикселям.
 * Аргументы: img — uint8, amp — амплитуда шума.
 * Возврат: uint8.
 * Эксепшенс: std::runtime_error при amp<0 или некорректном формате входа.
 */
py::array_t<uint8_t> add_noise(
    py::array_t<uint8_t, py::array::c_style | py::array::forcecast> img,
    int amp,
    std::uint64_t seed = 0
) {
    if (amp < 0) {
        throw std::runtime_error("amp must be >= 0");
    }
    auto shape = validate_image(img);
    const auto H = shape.h, W = shape.w, C = shape.c;
    std::vector<py::ssize_t> out_shape{H, W, C};
    py::array_t<uint8_t> out(out_shape);

    auto src = img.unchecked<3>();
    auto dst = out.mutable_unchecked<3>();

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(-amp, amp);

    for (py::ssize_t y = 0; y < H; ++y) {
        for (py::ssize_t x = 0; x < W; ++x) {
            for (py::ssize_t c = 0; c < C; ++c) {
                int noise = dist(rng);
                dst(y, x, c) = sat_i(static_cast<int>(src(y, x, c)) + noise);
            }
        }
    }
    return out;
}

/**
 * Функция: random_crop
 * Что делает: Случайное вырезание куска (out_h, out_w) из входного изображения.
 * Аргументы: img — uint8, out_h, out_w — размеры окна.
 * Возврат: uint8.
 * Эксепшенс: std::runtime_error при некорректных размерах или формате входа.
 */
py::array_t<uint8_t> random_crop(
    py::array_t<uint8_t, py::array::c_style | py::array::forcecast> img,
    int out_h,
    int out_w,
    std::uint64_t seed = 0
) {
    auto shape = validate_image(img);
    const auto H = static_cast<int>(shape.h);
    const auto W = static_cast<int>(shape.w);
    const auto C = shape.c;

    if (out_h <= 0 || out_w <= 0) {
        throw std::runtime_error("out_h and out_w must be > 0");
    }
    if (out_h > H || out_w > W) {
        throw std::runtime_error("crop size must be <= input size");
    }

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist_y(0, H - out_h);
    std::uniform_int_distribution<int> dist_x(0, W - out_w);
    int y0 = dist_y(rng);
    int x0 = dist_x(rng);

    std::vector<py::ssize_t> out_shape{out_h, out_w, C};
    py::array_t<uint8_t> out(out_shape);

    auto src = img.unchecked<3>();
    auto dst = out.mutable_unchecked<3>();

    for (int y = 0; y < out_h; ++y) {
        for (int x = 0; x < out_w; ++x) {
            for (py::ssize_t c = 0; c < C; ++c) {
                dst(y, x, c) = src(y0 + y, x0 + x, c);
            }
        }
    }
    return out;
}

/**
 * Функция: resize_pad
 * Что делает: Масштабирует изображение с сохранением пропорций и помещает
 *           результат на холст размера (out_h, out_w), заполняя фон значением pad.
 *           Интерполяция — ближайший сосед. 
 * Аргументы: img — uint8, out_h, out_w — размеры выхода, keep_aspect — сохранить пропорции(попытка)
 *            pad — цвет подложки
 * Возврат: uint8.
 * Эксепшенс: std::runtime_error при некорректных размерах или формате входа
 */
py::array_t<uint8_t> resize_pad(
    py::array_t<uint8_t, py::array::c_style | py::array::forcecast> img,
    int out_h,
    int out_w,
    bool keep_aspect = true,
    uint8_t pad = 0,
    bool bilinear = false
) {
    if (bilinear) {
        throw std::runtime_error("bilinear=true is not implemented yet");
    }

    auto shape = validate_image(img);
    const auto H = static_cast<int>(shape.h);
    const auto W = static_cast<int>(shape.w);
    const auto C = shape.c;

    if (out_h <= 0 || out_w <= 0) {
        throw std::runtime_error("out_h and out_w must be > 0");
    }

    std::vector<py::ssize_t> out_shape{out_h, out_w, C};
    py::array_t<uint8_t> out(out_shape);
    {
        auto dst = out.mutable_unchecked<3>();
        for (int y = 0; y < out_h; ++y) {
            for (int x = 0; x < out_w; ++x) {
                for (py::ssize_t c = 0; c < C; ++c) {
                    dst(y, x, c) = pad;
                }
            }
        }
    }
    int target_h = out_h;
    int target_w = out_w;
    int offy = 0, offx = 0;

    if (keep_aspect) {
        double s = std::min(out_h / static_cast<double>(H),
                            out_w / static_cast<double>(W));
        target_h = std::max(1, static_cast<int>(std::round(H * s)));
        target_w = std::max(1, static_cast<int>(std::round(W * s)));
        offy = (out_h - target_h) / 2;
        offx = (out_w - target_w) / 2;
    }
    auto src = img.unchecked<3>();
    auto dst = out.mutable_unchecked<3>();

    auto map_nn = [](int q, int tq, int pmax) -> int {
        double src_pos = (q + 0.5) * (static_cast<double>(pmax) / static_cast<double>(tq)) - 0.5;
        int p = static_cast<int>(std::llround(src_pos));
        if (p < 0) p = 0;
        if (p >= pmax) p = pmax - 1;
        return p;
    };

    for (int y = 0; y < target_h; ++y) {
        int sy = map_nn(y, target_h, H);
        for (int x = 0; x < target_w; ++x) {
            int sx = map_nn(x, target_w, W);
            for (py::ssize_t c = 0; c < C; ++c) {
                dst(offy + y, offx + x, c) = src(sy, sx, c);
            }
        }
    }

    return out;
}

} // namespace pixx

PYBIND11_MODULE(pixxbind_cpp, m) {
    m.doc() = "Image augmentations implemented in C++";
    m.def("to_gray", &pixx::to_gray, "Convert RGB/BGR image to grayscale");
    m.def("brightness", &pixx::brightness,
          py::arg("img"), py::arg("delta"),
          "Shift brightness by the given delta");
    m.def("contrast", &pixx::contrast,
          py::arg("img"), py::arg("alpha"),
          "Adjust contrast with given alpha");
    m.def("add_noise", &pixx::add_noise,
          py::arg("img"), py::arg("amp"), py::arg("seed") = 0,
          "Add uniform integer noise in range [-amp, amp]");
    m.def("random_crop", &pixx::random_crop,
          py::arg("img"), py::arg("out_h"), py::arg("out_w"), py::arg("seed") = 0,
          "Random crop of size out_h x out_w");
    m.def("resize_pad", &pixx::resize_pad,
          py::arg("img"), py::arg("out_h"), py::arg("out_w"),
          py::arg("keep_aspect") = true,
          py::arg("pad") = 0,
          py::arg("bilinear") = false,
          "Resize with optional aspect keeping and padding");
}

