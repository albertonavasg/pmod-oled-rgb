# This script takes the image file and converts it to a 96*64 pixel image and then converts it to RGB565 format.
# The RGB565 format is then stored in a.txt file namd as the image file name.

import numpy as np
from PIL import Image
import argparse

def process_image(image_path):
    # Load the image
    image = Image.open(image_path)

    # Ensure the image is in RGB mode
    image = image.convert('RGB')

    # Resize the image to 96x64
    image = image.resize((96, 64), Image.LANCZOS)

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

    # Write the RGB565 values to a text file in three columns
    output_filename = f"{image_path.split('.')[0]}.txt"
    with open(output_filename, "w") as file:
        file.write(image_path + "\nR\tG\tB\n")  # Write header
        for r, g, b in rgb565_values:
            file.write(f"{r}\t{g}\t{b}\n")

    print("\t" + image_path + " saved as " + output_filename)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process an image and output RGB565 values.")
    parser.add_argument("image_path", type=str, help="Path to the image file")
    args = parser.parse_args()
    
    process_image(args.image_path)
