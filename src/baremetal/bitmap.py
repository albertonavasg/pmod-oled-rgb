# This script takes the image file and converts it to a 96*64 pixel image and then converts it to RGB565 format.
# The RGB565 format is then converted to C-style array declarations and written to a file called bitmap.h.

from PIL import Image
import numpy as np

# Set the name of the image file
image_name = 'image1.jpg'

# Load the image
image = Image.open(image_name)

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

# Combine the values into a single array of tuples
rgb565_values = list(zip(r_values_565, g_values_565, b_values_565))

# Function to convert an array of structs to a C-style declaration
def array_of_structs_to_c_declaration(name, array):
    c_declaration = f'#include <stdint.h> // For uint8_t\n'
    c_declaration += f'#include "screen.h" // For color definition\n\n'
    c_declaration += f"colorInstance {name}[] = {{\n"
    values_per_line = 4  # Adjust the number of structs per line for readability
    for i in range(0, len(array), values_per_line):
        line_values = array[i:i + values_per_line]
        line_str = ", ".join([f"{{{r}, {g}, {b}}}" for r, g, b in line_values])
        c_declaration += f"    {line_str},\n"
    c_declaration = c_declaration.rstrip(",\n") + "\n};\n"
    return c_declaration

# Convert array to C-style declaration
c_declaration = array_of_structs_to_c_declaration("imageBitmap", rgb565_values)

# Write the C-style declaration to a file
with open("bitmap.h", "w") as file:
    file.write(c_declaration)

print("\n\t" + image_name + " converted to bitmap.h")