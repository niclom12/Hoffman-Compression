import os
import random
import numpy as np
from pathlib import Path
import json
from collections import Counter
BASE_DIR = Path("../test_data2")
BASE_DIR.mkdir(exist_ok=True)
PRINTABLE_ASCII = [chr(i) for i in range(32, 127)]
FIXED_SEED = 69
CHARS_PER_FILE = 1_000_000
MAX_STRUCTURED_SIZE_MB = 100
def entropy(s):
    p, l = Counter(s), len(s)
    return -sum(count / l * np.log2(count / l) for count in p.values() if count > 0)

def save_file(category, name, content): 
    dir_path = BASE_DIR / category
    dir_path.mkdir(exist_ok=True)
    file_path = dir_path / name
    with open(file_path, "w", encoding="ascii", errors="ignore") as f:
        f.write(content)
    print(f"Generated {file_path} | Size: {len(content)/1024:.1f} KB | Entropy: {entropy(content):.2f}")

def generate_repetitive_files():
    dir_path = BASE_DIR / "repetitiveness"
    dir_path.mkdir(exist_ok=True)
    random.seed(FIXED_SEED)
    np.random.seed(FIXED_SEED)

    for i in range(30):
        skew = min(0.98, 0.02 + (i / 29) * 0.96)
        probs = [skew] + [(1 - skew) / 94] * 94
        probs = np.array(probs)
        probs /= probs.sum()
        characters = np.random.choice(PRINTABLE_ASCII, size=CHARS_PER_FILE, p=probs)
        content = ''.join(characters)
        entropy_val = -sum(p * np.log2(p) for p in probs if p > 0)
        file_path = dir_path / f"repetitiveness_entropy_{i}.txt"
        with open(file_path, "w", encoding="ascii") as f:
            f.write(content)
        print(f"Generated {file_path} with entropy ~ {entropy_val:.2f} bits/char")

def generate_structure_randomness_files():
    dir_path = BASE_DIR / "structure_randomness"
    dir_path.mkdir(exist_ok=True)
    random.seed(FIXED_SEED)

    base_block = ("ME, A WIZARD? UNTIL A WEEK AGO, I WAS AN ASTRONOMER, CONTENTEDLY DESIGNING " * 20)
    base_data = (base_block * (CHARS_PER_FILE // len(base_block) + 1))[:CHARS_PER_FILE]

    for i in range(30):
        data = list(base_data)
        swap_fraction = i / 29.0
        num_swaps = int(len(data) * swap_fraction)

        for _ in range(num_swaps):
            a, b = random.randint(0, len(data) - 1), random.randint(0, len(data) - 1)
            data[a], data[b] = data[b], data[a]

        save_file("structure_randomness", f"structured_randomness_{i}.txt", ''.join(data))

def generate_structured_size_files():
    dir_path = BASE_DIR / "structured_size"
    dir_path.mkdir(exist_ok=True)

    sizes = [10 ** i for i in range(1, 8)]  
    for i, num_entries in enumerate(sizes):
        entries = []
        for j in range(num_entries):
            timestamp = f"2024-01-01 00:{j // 60 % 60:02d}:{j % 60:02d}"
            user_id = j
            action = random.choice(["LOGIN", "LOGOUT", "DOWNLOAD", "UPLOAD", "ERROR", "UPDATE"])
            status = random.choice(["SUCCESS", "FAIL", "TIMEOUT", "PENDING"])
            log_line = f"[{timestamp}] USER:{user_id:06d} ACTION:{action:<8} STATUS:{status}"
            entries.append(log_line)
        content = '\n'.join(entries)
        if len(content) > MAX_STRUCTURED_SIZE_MB * 1024 * 1024:
            break
        save_file("structured_size", f"structured_size_{i}.txt", content)

def generate_mixed_content():
    for i in range(30):
        content = ''.join(random.choices(PRINTABLE_ASCII, k=CHARS_PER_FILE))
        save_file("mixed_content", f"mixed_{i}.txt", content)

def generate_binary_like():
    for i in range(30):
        content = ''.join(random.choices(['0', '1'], k=CHARS_PER_FILE))
        save_file("binary_like", f"binary_{i}.txt", content)

def generate_base64_noise():
    base64_chars = list("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
    for i in range(30):
        content = ''.join(random.choices(base64_chars, k=CHARS_PER_FILE))
        save_file("base64_noise", f"base64_{i}.txt", content)

def generate_json_xml_structured():
    for i in range(30):
        entries_json = [
            json.dumps({"user": f"user_{j}", "id": j, "event": random.choice(["click", "scroll", "type"]), "value": random.randint(0, 100)})
            for j in range(10000)
        ]
        entries_xml = [
            f"<event><user>user_{j}</user><id>{j}</id><action>{random.choice(['open','close','edit'])}</action></event>"
            for j in range(10000)
        ]
        save_file("json_structured", f"json_{i}.txt", '\n'.join(entries_json))
        save_file("xml_structured", f"xml_{i}.txt", '\n'.join(entries_xml))

generate_repetitive_files()
generate_structure_randomness_files()
generate_structured_size_files()
generate_mixed_content()
generate_binary_like()
generate_base64_noise()
generate_json_xml_structured()
