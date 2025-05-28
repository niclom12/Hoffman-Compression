import os
import subprocess
import time
from pathlib import Path
import matplotlib.pyplot as plt

BASE_DIR = Path("test_data")
OUT_DIR = Path("test_outputs")
OUT_DIR.mkdir(exist_ok=True)

RUN_SCRIPT = "../run.sh"
RESULTS = []

def run_command(command):
    """Run a shell command and time it."""
    start = time.time()
    subprocess.run(command, shell=True, check=True)
    return time.time() - start

def evaluate_file(file_path: Path):
    rel_type = file_path.parent.name
    base_name = file_path.stem

    compressed = OUT_DIR / f"{base_name}.comp"
    decompressed = OUT_DIR / f"{base_name}.decomp"

    # Compression
    compress_time = run_command(f"{RUN_SCRIPT} c {file_path} {compressed}")
    compressed_size = os.path.getsize(compressed)

    # Decompression
    decompress_time = run_command(f"{RUN_SCRIPT} d {compressed} {decompressed}")
    original_size = os.path.getsize(file_path)

    with open(file_path, "rb") as f1, open(decompressed, "rb") as f2:
        same = f1.read() == f2.read()

    if not same:
        print(f"File mismatch after decompression: {file_path.name}")
        return

    RESULTS.append({
        "test_type": rel_type,
        "file": file_path.name,
        "original_size": original_size,
        "compressed_size": compressed_size,
        "compression_ratio": compressed_size / original_size,
        "compression_time": compress_time,
        "decompression_time": decompress_time
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

def plot_metrics():
    import pandas as pd
    df = pd.DataFrame(RESULTS)

    # Plot Compression Ratio
    plt.figure(figsize=(12, 6))
    for test_type, group in df.groupby("test_type"):
        plt.plot(group["file"], group["compression_ratio"], label=test_type, marker='o')
    plt.title("Compression Ratio by Test Case")
    plt.xlabel("File")
    plt.ylabel("Compression Ratio (compressed/original)")
    plt.xticks(rotation=90)
    plt.legend()
    plt.tight_layout()
    plt.savefig("compression_ratio.png")
    plt.show()

    # Plot Compression & Decompression Times
    plt.figure(figsize=(12, 6))
    for test_type, group in df.groupby("test_type"):
        plt.plot(group["file"], group["compression_time"], label=f"{test_type} (compress)", linestyle='-', marker='o')
        plt.plot(group["file"], group["decompression_time"], label=f"{test_type} (decompress)", linestyle='--', marker='x')
    plt.title("Compression and Decompression Time")
    plt.xlabel("File")
    plt.ylabel("Time (s)")
    plt.xticks(rotation=90)
    plt.legend()
    plt.tight_layout()
    plt.savefig("compression_times.png")
    plt.show()

if __name__ == "__main__":
    scan_and_evaluate()
    plot_metrics()
