# z_order.py
import sys
from pymorton import interleave_latlng

if len(sys.argv) != 3:
    print("Usage: python3 z_order.py MRBs_centers.txt z_values.txt")
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]

with open(input_path, 'r') as infile, open(output_path, 'w') as outfile:
    for line in infile:
        line = line.strip()
        if not line:
            continue
        try:
            x_str, y_str = line.split(',')
            x = float(x_str)
            y = float(y_str)
            z = interleave_latlng(y, x)  # lat = y, lng = x
            outfile.write(f"{z}\n")
        except Exception as e:
            print(f"Error processing line '{line}': {e}")
