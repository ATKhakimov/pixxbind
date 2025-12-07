
#pragma once
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <pybind11/numpy.h>

namespace pixx {
inline uint8_t sat_i(int v) {
    return static_cast<uint8_t>(std::min(255, std::max(0, v)));
}

inline uint8_t sat_f(float v) {
    return static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, v)));
}

struct ImgShape {
    pybind11::ssize_t h;
    pybind11::ssize_t w;
    pybind11::ssize_t c;
};

// Проверка входного массива: 3D, C=1 или 3, размеры >0.
inline ImgShape validate_image(const pybind11::array &img) {
    if (img.ndim() != 3) {
        throw std::runtime_error("input must be 3-D array (H x W x C)");
    }
    auto h = img.shape(0);
    auto w = img.shape(1);
    auto c = img.shape(2);
    if (h <= 0 || w <= 0) {
        throw std::runtime_error("H and W must be > 0");
    }
    if (!(c == 1 || c == 3)) {
        throw std::runtime_error("channel dimension C must be 1 or 3");
    }
    return {h, w, c};
}

} 
