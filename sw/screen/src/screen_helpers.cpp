#include <iostream> // cout
#include <cstring>  // string
#include <chrono>   // time
#include <thread>   // sleep_for

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize2.h"

#include "fonts.h"

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

bool Screen::waitForPowerState(screen::PowerState target, std::chrono::milliseconds timeout) {

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    while (true) {
        if (readPowerState() == target) {
            return true;
        }

        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

std::vector<screen::Color> Screen::importImageAsBitmap(const std::string &path){

    int inputWidth, inputHeight, channels, outputWidth, outputHeight;

    // Load image, force RGB
    unsigned char* img = stbi_load(path.c_str(), &inputWidth, &inputHeight, &channels, 3);
    // Check valid imported image
    if (!img) {
        std::cerr << "Error importing the image: " << path.c_str() << std::endl;
        return {};
    }
    // Check aspect ratio
    if (m_orientation == screen::Orientation::Horizontal_0 || m_orientation == screen::Orientation::Horizontal_180){
        if ((inputWidth * screen::Geometry::Rows) != (inputHeight * screen::Geometry::Columns)) {
            std::cerr << "Wrong aspect ratio of horizontal image " << path.c_str() << std::endl;
            return {};
        }
        outputWidth = screen::Geometry::Columns;
        outputHeight = screen::Geometry::Rows;
    } else {
        if ((inputWidth * screen::Geometry::Columns) != (inputHeight * screen::Geometry::Rows)) {
            std::cerr << "Wrong aspect ratio of vertical image " << path.c_str() << std::endl;
            return {};
        }
        outputWidth = screen::Geometry::Rows;
        outputHeight = screen::Geometry::Columns;
    }


    // Resized buffer
    std::vector<uint8_t> resized(screen::Geometry::Pixels * 3);

    unsigned char* result = stbir_resize_uint8_linear(
        img,            // Input image
        inputWidth,     // Input width
        inputHeight,    // Input height
        0,              // Input stride (0 = tightly packed)
        resized.data(), // Output vector
        outputWidth,    // Output width
        outputHeight,   // Output height
        0,              // Output stride (0 = tightly packed)
        STBIR_RGB       // Pixel format
    );

    if (!result) {
        std::cerr << "Error resizing image to " << outputWidth << " x " << outputHeight << std::endl;
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

    // Reserve bitmap vector
    std::vector<screen::Color> bitmap(m_fontWidth * m_fontHeight);

    // Determine which font array and internal index to use
    const uint8_t *fontArray = nullptr;
    uint8_t symbolIndex = symbol;

    if (symbol < screen::Font::BasicSize) {
        fontArray = m_fontBasic;
        symbolIndex -= screen::Font::BasicOffset;
    } else if (symbol < screen::Font::BasicSize + screen::Font::ControlSize) {
        fontArray = m_fontControl;
        symbolIndex -= screen::Font::ControlOffset;
    } else {
        fontArray = m_fontExtLatin;
        symbolIndex -= screen::Font::ExtLatinOffset;
    }

    // Fill bitmap
    for (size_t i = 0; i < m_fontHeight; i++) {
        for (size_t j = 0; j < m_fontWidth; j++) {
            if (fontArray[symbolIndex * m_fontHeight + i] & (1 << j)) {
                bitmap[i * m_fontWidth + j] = color;
            } else {
                bitmap[i * m_fontWidth + j] = screen::StandardColor::Black;
            }
        }
    }

    return bitmap;
}

uint32_t Screen::utf8_decode(const uint8_t *s, size_t *len) {

    uint32_t codepoint;

    // First byte is 0XXX_XXXX
    if (s[0] < 0x80) {
        *len = 1;
        return s[0];
    }
    // First byte is 110X_XXXX
    if ((s[0] & 0xE0) == 0xC0) {
        *len = 2;
         codepoint = ((s[0] & 0x1F) << 6) |
             (s[1] & 0x3F);
        return codepoint;
    }
    // First byte is 1110_XXXX
    if ((s[0] & 0xF0) == 0xE0) {
        *len = 3;
         codepoint = ((s[0] & 0x0F) << 12) |
             ((s[1] & 0x3F) << 6) |
             (s[2] & 0x3F);
        return codepoint;
    }
    // First byte is 1111_0XXX
    if ((s[0] & 0xF8) == 0xF0) {
        *len = 4;
         codepoint = ((s[0] & 0x07) << 18) |
             ((s[1] & 0x3F) << 12) |
             ((s[2] & 0x3F) << 6) |
             (s[3] & 0x3F);
        return codepoint;
    }

    *len = 1;
    return 0xFFFD; // Replacement char
}