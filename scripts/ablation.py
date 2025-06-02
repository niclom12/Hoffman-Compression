import os
import subprocess
import time
import csv
from pathlib import Path
from itertools import product

input_base_dir = Path("../test_data2")
output_dir = Path("./compressed_results")
exe_path = "../bin/compression"  
output_dir.mkdir(exist_ok=True)
pipeline_combinations = list(product([0, 1], repeat=3))
results = []
for category_dir in input_base_dir.iterdir():
    if not category_dir.is_dir():
        continue

    for file in category_dir.glob("*.txt"):
        if not file.is_file():
            continue

        original_size = os.path.getsize(file)

        for bwt, mtf, rle in pipeline_combinations:
            output_filename = f"{file.stem}_b{bwt}m{mtf}r{rle}.zip"
            print(output_filename)
            output_path = output_dir / output_filename

            cmd = [exe_path, "c", str(file), str(output_path), str(bwt), str(mtf), str(rle)]

            start = time.time()
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            end = time.time()

            if result.returncode != 0 or not output_path.exists():
                continue 

            compressed_size = os.path.getsize(output_path)

            results.append({
                "category": category_dir.name,
                "file": file.name,
                "bwt": bwt,
                "mtf": mtf,
                "rle": rle,
                "original_size": original_size,
                "compressed_size": compressed_size,
                "compression_ratio": compressed_size / original_size if original_size else 0,
                "time_seconds": end - start
            })

csv_file = "compression_test_results.csv"
with open(csv_file, "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=results[0].keys())
    writer.writeheader()
    writer.writerows(results)
