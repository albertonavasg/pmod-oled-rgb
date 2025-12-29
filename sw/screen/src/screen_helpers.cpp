#include <iostream> // cout
#include <cstring>  // string

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

#include "font8x8_basic.h"

std::vector<screen::Color> Screen::importImageAsBitmap(const std::string &path){

    int width, height, channels;

    // Load image, force RGB
    unsigned char* img = stbi_load(path.c_str(), &width, &height, &channels, 3);
    // Check valid imported image
    if (!img) {
        std::cerr << "Error importing the image: " << path.c_str() << std::endl;
        return {};  // Empty vector
    }
    // Check aspect ratio
    if ((width * screen::Geometry::Rows) != (height * screen::Geometry::Columns)) {
        std::cerr << "Wrong aspect ratio of image " << path.c_str() << std::endl;
        return {};
    }

    // Resized buffer
    std::vector<uint8_t> resized(screen::Geometry::Pixels * 3);

    unsigned char* result = stbir_resize_uint8_linear(
        img,                         // Input image
        width,                       // Input width
        height,                      // Input height
        0,                           // Input stride (0 = tightly packed)
        resized.data(),              // Output vector
        screen::Geometry::Columns,   // Output width
        screen::Geometry::Rows,      // Output height
        0,                           // Output stride (0 = tightly packed)
        STBIR_RGB                    // Pixel format
    );

    if (!result) {
        std::cerr << "Error resizing image to " << screen::Geometry::Columns << " x " << screen::Geometry::Rows << std::endl;
    }

    stbi_image_free(img);

    // Convert to Color bitmap
    std::vector<screen::Color> bitmap(screen::Geometry::Pixels);

    for (int i = 0; i < screen::Geometry::Pixels; i++) {
        uint8_t r8 = resized[3*i + 0];
        uint8_t g8 = resized[3*i + 1];
        uint8_t b8 = resized[3*i + 2];

        bitmap[i] = {
            static_cast<uint8_t>(r8 >> 3),
            static_cast<uint8_t>(g8 >> 2),
            static_cast<uint8_t>(b8 >> 3)
        };
    }

    return bitmap;
}

std::vector<screen::Color> Screen::importSymbolAsBitmap(const uint8_t symbol, screen::Color color) {

    std::vector<screen::Color> bitmap(screen::FontPixels);

    for (size_t i = 0; i < screen::FontHeight; i++) {
        for (size_t j = 0; j < screen::FontWidth; j++) {
            if (font8x8_basic[symbol][i] >> j & 1) {
                bitmap[screen::FontWidth * i + j] = color;
            } else {
                bitmap[screen::FontWidth * i + j] = {0, 0, 0};
            }
        }
    }

    return bitmap;
}