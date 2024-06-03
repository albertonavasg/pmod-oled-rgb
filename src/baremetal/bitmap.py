# This script takes the image file and converts it to a 96*64 pixel image and then converts it to RGB565 format.
# The RGB565 format is then converted to C-style array declarations and written to a file called bitmap.h.

from PIL import Image
import numpy as np

# Load the image
image = Image.open('CIMG1877.JPG')

# Resize the image to 96x64 pixels
image = image.resize((96, 64), Image.Resampling.LANCZOS)

# Save the resized image (optional)
# image.save('resized_image.jpg')

# Display the image (optional)
# image.show()

# Ensure the image is in RGB mode
image = image.convert('RGB')

# Convert the image to a NumPy array
pixel_values = np.array(image)

# Extract the R, G, and B channels
r_values = pixel_values[:, :, 0]
g_values = pixel_values[:, :, 1]
b_values = pixel_values[:, :, 2]

# Convert to RGB565 format
r_values_565 = np.floor((r_values / 255.0) * 31).astype(int).flatten()
g_values_565 = np.floor((g_values / 255.0) * 63).astype(int).flatten()
b_values_565 = np.floor((b_values / 255.0) * 31).astype(int).flatten()

# Print the vectors (optional)
# print("R values (RGB565):", r_values_565)
# print("G values (RGB565):", g_values_565)
# print("B values (RGB565):", b_values_565)

# Function to convert a NumPy array to a C-style array string
def array_to_c_declaration(name, array):
    c_declaration = f"uint8_t {name}[] = {{\n"
    values_per_line = 16  # Adjust the number of values per line for readability
    for i in range(0, len(array), values_per_line):
        line_values = array[i:i + values_per_line]
        line_str = ", ".join(map(str, line_values))
        c_declaration += f"    {line_str},\n"
    c_declaration = c_declaration.rstrip(",\n") + "\n};\n"
    return c_declaration

# Convert arrays to C-style declarations
bitmap_r = array_to_c_declaration("bitmapR", r_values_565)
bitmap_g = array_to_c_declaration("bitmapG", g_values_565)
bitmap_b = array_to_c_declaration("bitmapB", b_values_565)

# Combine all declarations into one string
c_code = "#include <stdint.h> // For uint8_t\n\n" + bitmap_r + "\n" + bitmap_g + "\n" + bitmap_b

# Write the C-style declarations to a file
with open("bitmap.h", "w") as file:
    file.write(c_code)

print("C-style declarations written to bitmap.h")