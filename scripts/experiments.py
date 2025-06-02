import os
import subprocess
import time
from pathlib import Path
import matplotlib.pyplot as plt
import shutil

BASE_DIR = Path("../test_data")
RUN_SCRIPT_DIR = Path(__file__).resolve().parent.parent
RUN_SCRIPT = RUN_SCRIPT_DIR / "run.sh"
OUT_DIR = RUN_SCRIPT_DIR / "test_outputs"
OUT_DIR.mkdir(exist_ok=True)
RESULTS = []

def run_command(command, cwd=None):
    start = time.time()
    subprocess.run(command, shell=True, check=True, cwd=cwd or RUN_SCRIPT_DIR)
    return time.time() - start

def evaluate_file(file_path: Path):
    rel_type = file_path.parent.name
    base_name = file_path.stem

    # ours
    ours_compressed = OUT_DIR / f"{base_name}.comp"
    ours_decompressed = OUT_DIR / f"{base_name}.txt"
    compress_time = run_command(f"{RUN_SCRIPT} c {file_path.resolve()} {ours_compressed.resolve()}")
    compressed_size = os.path.getsize(ours_compressed)
    decompress_time = run_command(f"{RUN_SCRIPT} d {ours_compressed.resolve()} {ours_decompressed.resolve()}")
    original_size = os.path.getsize(file_path)

    with open(file_path, "rb") as f1, open(ours_decompressed, "rb") as f2:
        same = f1.read() == f2.read()
    if not same:
        print(f"File mismatch after decompression: {file_path.name}")
        return

    #bzip
    bzip2_input = OUT_DIR / f"{base_name}.bzip2src.txt"
    shutil.copy(file_path, bzip2_input)
    bzip2_compress_time = run_command(f"bzip2 -kf {bzip2_input}")
    bzip2_compressed = OUT_DIR / f"{bzip2_input.name}.bz2"
    bzip2_compressed_size = os.path.getsize(bzip2_compressed)

    bzip2_decompress_time = run_command(f"bzip2 -d -kf {bzip2_compressed}")
    bzip2_decompressed = OUT_DIR / f"{bzip2_input.name}"
    with open(file_path, "rb") as f1, open(bzip2_decompressed, "rb") as f2:
        bzip2_same = f1.read() == f2.read()
    if not bzip2_same:
        print(f"[bzip2] File mismatch after decompression: {file_path.name}")
        return

    RESULTS.append({
        "test_type": rel_type,
        "file": file_path.name,
        "original_size": original_size,
        "ours_compressed_size": compressed_size,
        "bzip2_compressed_size": bzip2_compressed_size,
        "ours_ratio": compressed_size / original_size,
        "bzip2_ratio": bzip2_compressed_size / original_size,
        "ours_compress_time": compress_time,
        "ours_decompress_time": decompress_time,
        "bzip2_compress_time": bzip2_compress_time,
        "bzip2_decompress_time": bzip2_decompress_time,
    })

    print(f"Checked: {file_path.name} - {rel_type}")

def scan_and_evaluate():
    for category in BASE_DIR.iterdir():
        if not category.is_dir():
            continue
        for file in category.glob("*.txt"):
            try:
                evaluate_file(file)
            except Exception as e:
                print(f"Error processing {file.name}: {e}")

def extract_numeric_suffix(filename):
    import re
    match = re.search(r"(\d+)(?=\.txt$)", filename)
    return int(match.group(1)) if match else -1

def plot_metrics():
    import pandas as pd
    df = pd.DataFrame(RESULTS)
    df["x"] = df["file"].apply(extract_numeric_suffix)

    for name, group in df.groupby("test_type"):
        group = group.sort_values("x")
        plt.figure(figsize=(8, 5))
        plt.plot(group["x"], group["ours_ratio"], label="Ours", marker='o')
        plt.plot(group["x"], group["bzip2_ratio"], label="bzip2", marker='x')
        plt.title(f"{name} - Compression Ratio vs. Input Size")
        plt.xlabel(f"{name} (Increasing)")
        plt.ylabel("Compression Ratio")
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f"{name}_compression_ratio.png")
        plt.close()

    for name, group in df.groupby("test_type"):
        group = group.sort_values("x")

        plt.figure(figsize=(10, 5))
        plt.plot(group["x"], group["ours_compress_time"], label="Ours (Compress)", marker='o')
        plt.plot(group["x"], group["ours_decompress_time"], label="Ours (Decompress)", marker='x')
        plt.plot(group["x"], group["bzip2_compress_time"], label="bzip2 (Compress)", marker='o', linestyle='--')
        plt.plot(group["x"], group["bzip2_decompress_time"], label="bzip2 (Decompress)", marker='x', linestyle='--')

        plt.title(f"{name} - Time vs. Input Size")
        plt.xlabel(f"{name} (Increasing)")
        plt.ylabel("Time (s)")
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f"{name}_time_comparison.png")
        plt.close()

if __name__ == "__main__":
    scan_and_evaluate()
    plot_metrics()
