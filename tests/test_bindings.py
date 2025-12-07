import numpy as np
import pixxbind as px


def clamp_uint8(arr):
    return np.clip(arr, 0, 255).astype(np.uint8)


def ref_to_gray(img: np.ndarray) -> np.ndarray:
    if img.shape[2] == 1:
        return img.copy()
    r, g, b = img[..., 0], img[..., 1], img[..., 2]
    y = np.round(0.299 * r + 0.587 * g + 0.114 * b)
    y = clamp_uint8(y.astype(np.int32))
    return y[..., None]


def ref_brightness(img: np.ndarray, delta: int) -> np.ndarray:
    return clamp_uint8(img.astype(np.int32) + delta)


def ref_contrast(img: np.ndarray, alpha: float) -> np.ndarray:
    v = (img.astype(np.int32) - 128) * alpha + 128.0
    v = np.round(v)
    return clamp_uint8(v)


def ref_add_noise_zero_amp(img: np.ndarray) -> np.ndarray:
    return img.copy()


def ref_random_crop_full(img: np.ndarray) -> np.ndarray:
    return img.copy()


def map_nn(q: int, tq: int, pmax: int) -> int:
    src_pos = (q + 0.5) * (pmax / float(tq)) - 0.5
    p = int(np.round(src_pos))
    p = 0 if p < 0 else p
    p = pmax - 1 if p >= pmax else p
    return p


def ref_resize_pad(img: np.ndarray, out_h: int, out_w: int, keep_aspect: bool, pad: int) -> np.ndarray:
    h, w, c = img.shape
    out = np.full((out_h, out_w, c), pad, dtype=np.uint8)
    target_h, target_w = out_h, out_w
    offy = offx = 0
    if keep_aspect:
        s = min(out_h / float(h), out_w / float(w))
        target_h = max(1, int(np.round(h * s)))
        target_w = max(1, int(np.round(w * s)))
        offy = (out_h - target_h) // 2
        offx = (out_w - target_w) // 2
    for y in range(target_h):
        sy = map_nn(y, target_h, h)
        for x in range(target_w):
            sx = map_nn(x, target_w, w)
            out[offy + y, offx + x, :] = img[sy, sx, :]
    return out


def test_to_gray_matches_reference():
    img = np.array([
        [[10, 20, 30], [40, 50, 60]],
        [[70, 80, 90], [100, 110, 120]],
    ], dtype=np.uint8)
    expected = ref_to_gray(img)
    result = px.to_gray(img)
    np.testing.assert_array_equal(result, expected)


def test_brightness_and_contrast():
    img = np.arange(27, dtype=np.uint8).reshape(3, 3, 3)
    bright = px.brightness(img, 20)
    expected_bright = ref_brightness(img, 20)
    np.testing.assert_array_equal(bright, expected_bright)

    contrast = px.contrast(img, 1.5)
    expected_contrast = ref_contrast(img, 1.5)
    np.testing.assert_array_equal(contrast, expected_contrast)


def test_add_noise_zero_amp_is_noop():
    img = np.arange(12, dtype=np.uint8).reshape(2, 2, 3)
    noisy = px.add_noise(img, 0, seed=123)
    expected = ref_add_noise_zero_amp(img)
    np.testing.assert_array_equal(noisy, expected)


def test_random_crop_full_size_is_identity():
    img = np.arange(27, dtype=np.uint8).reshape(3, 3, 3)
    cropped = px.random_crop(img, 3, 3, seed=999)
    expected = ref_random_crop_full(img)
    np.testing.assert_array_equal(cropped, expected)


def test_resize_pad_matches_reference():
    img = np.array([
        [[10, 20, 30]],
        [[40, 50, 60]],
    ], dtype=np.uint8)
    result = px.resize_pad(img, 3, 5, keep_aspect=True, pad=7, bilinear=False)
    expected = ref_resize_pad(img, 3, 5, keep_aspect=True, pad=7)
    np.testing.assert_array_equal(result, expected)
