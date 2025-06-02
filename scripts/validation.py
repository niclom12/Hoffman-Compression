#!/usr/bin/env python3
import os
import subprocess
import time
import random
import string
import csv
DATA_DIR      = "data"
STRUCTURED_DIR= os.path.join(DATA_DIR, "structured")
RANDOM_DIR    = os.path.join(DATA_DIR, "random")
RESULTS_CSV   = "results.csv"
MAX_MB        = 100
VOCAB_SIZE    = 10000
WORD_MIN_LEN  = 5
WORD_MAX_LEN  = 10
SUCCESSORS_PER_WORD = 10
COMPRESS_BIN  = "../bin/compression"

def ensure_dirs():
    for d in (STRUCTURED_DIR, RANDOM_DIR):
        os.makedirs(d, exist_ok=True)

def generate_vocabulary():
    vocab = set()
    while len(vocab) < VOCAB_SIZE:
        length = random.randint(WORD_MIN_LEN, WORD_MAX_LEN)
        vocab.add(''.join(random.choices(string.ascii_lowercase, k=length)))
    return list(vocab)

def build_markov_chain(vocab):
    return { w: random.sample(vocab, SUCCESSORS_PER_WORD) for w in vocab }

def generate_structured_file(path, size_bytes, vocab, chain):
    with open(path, 'w', encoding='ascii') as f:
        word, total, first = random.choice(vocab), 0, True
        while total < size_bytes:
            sep = '' if first else ' '
            chunk = sep + word
            f.write(chunk)
            total += len(chunk)
            first = False
            word = random.choice(chain[word])

def generate_random_file(path, size_bytes):
    chars = [chr(i) for i in range(32, 127)]
    with open(path, 'w', encoding='ascii') as f:
        written = 0
        chunk = 1_048_576 
        while written < size_bytes:
            to_write = ''.join(random.choices(chars, k=min(chunk, size_bytes - written)))
            f.write(to_write)
            written += len(to_write)

def prepare_data():
    print("Generating data files…")
    vocab = generate_vocabulary()
    chain = build_markov_chain(vocab)
    for mb in range(1, MAX_MB + 1):
        size = mb**2 * 1_048_576
        struct_path = os.path.join(STRUCTURED_DIR, f"struct_{mb**2}MB.txt")
        rand_path   = os.path.join(RANDOM_DIR,   f"rand_{mb**2}MB.txt")
        if not os.path.exists(struct_path):
            generate_structured_file(struct_path, size, vocab, chain)
        if not os.path.exists(rand_path):
            generate_random_file(rand_path, size)    
        print("Data generation complete.")

def run_command(cmd, **kwargs):
    start = time.perf_counter()
    subprocess.run(cmd, check=True, **kwargs)
    return time.perf_counter() - start

def benchmark_file(filepath, writer):
    basename   = os.path.basename(filepath)
    size_bytes = os.path.getsize(filepath)
    mb         = size_bytes / 1_048_576

    def record(type_, phase, t, comp_size):
        ratio = comp_size / size_bytes
        writer.writerow({
            "type":      type_,
            "data":      basename,
            "size_MB":   f"{mb:.2f}",
            "phase":     phase,
            "time_s":    f"{t:.4f}",
            "ratio":     f"{ratio:.4f}"
        })

    zipfile = filepath + ".zip"
    t_c = run_command(["zip", "-j", "-qq", zipfile, filepath])
    comp_size = os.path.getsize(zipfile)
    record("zip", "compress", t_c, comp_size)
    t_d = run_command(["unzip", "-p", "-qq", zipfile], stdout=subprocess.DEVNULL)
    record("zip", "decompress", t_d, comp_size)
    os.remove(zipfile)

    experiments = {
        "huffman": ("0","0","0"),
        "bwt":     ("1","0","0"),
        "mtf":     ("0","1","0"),
        "rle":     ("0","0","1"),
    }
    for name, flags in experiments.items():
        comp_path   = f"{filepath}.{name}"
        decomp_path = f"{comp_path}.out"

        t_c = run_command([COMPRESS_BIN, "c", filepath, comp_path, *flags])
        comp_size = os.path.getsize(comp_path)
        record(name, "compress", t_c, comp_size)

        t_d = run_command([COMPRESS_BIN, "d", comp_path, decomp_path, *flags])
        record(name, "decompress", t_d, comp_size)

        os.remove(comp_path)
        os.remove(decomp_path)

def main():
    ensure_dirs()
    file_exists = os.path.exists(RESULTS_CSV)
    with open(RESULTS_CSV, "a", newline='') as csvfile:
        fieldnames = ["type", "data", "size_MB", "phase", "time_s", "ratio"]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        if not file_exists:
            writer.writeheader()
        for dirpath in (STRUCTURED_DIR, RANDOM_DIR):
            for fname in sorted(os.listdir(dirpath), key=lambda x: int(x.split('_')[1].rstrip("MB.txt"))):
                path = os.path.join(dirpath, fname)
                benchmark_file(path, writer)
if __name__ == "__main__":
    main()
