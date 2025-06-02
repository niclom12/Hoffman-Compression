import os
import random
import numpy as np
from pathlib import Path
import json

BASE_DIR = Path("../test_data")
BASE_DIR.mkdir(exist_ok=True)

PRINTABLE_ASCII = [chr(i) for i in range(32, 127)]  # printable ASCII characters
FIXED_SEED = 69
CHARS_PER_FILE = 1_000_000

def generate_repetitive_files():
    dir_path = BASE_DIR / "repetitiveness"
    dir_path.mkdir(exist_ok=True)
    random.seed(FIXED_SEED)
    np.random.seed(FIXED_SEED)

    skew_levels = [
        [1/95]*95,                                       # Uniform
        [0.30] + [0.0074]*94,                            # Slight skew
        [0.50] + [0.0053]*94,                            # Moderate skew
        [0.70] + [0.0032]*94,                            # High skew
        [0.90] + [0.0011]*94,                            # Very high skew
        [0.98] + [0.0002]*94                             # Extreme skew
    ]

    for i, probs in enumerate(skew_levels):
        probs = np.array(probs)
        probs /= probs.sum()
        characters = np.random.choice(PRINTABLE_ASCII, size=CHARS_PER_FILE, p=probs)
        content = ''.join(characters)
        file_path = dir_path / f"repetitiveness_entropy_{i}.txt"
        with open(file_path, "w", encoding="ascii") as f:
            f.write(content)
        entropy = -sum(p * np.log2(p) for p in probs if p > 0)
        print(f"Generated {file_path} with entropy ~ {entropy:.2f} bits/char")

def generate_structure_randomness_files():
    dir_path = BASE_DIR / "structure_randomness"
    dir_path.mkdir(exist_ok=True)
    random.seed(FIXED_SEED)

    base_block = (
        "ME, A WIZARD? UNTIL A WEEK AGO, I WAS AN ASTRONOMER, CONTENTEDLY DESIGNING "
        "telescope optics. Looking back on it, I'd lived in an academic dreamland. All "
        "these years, never planning for the future, right up to the day my grant money ran out. "
        "Lucky for me that my laboratory recycled used astronomers. Instead of standing in the "
        "unemployment line, I found myself transferred from the Keck Observatory at the Lawrence "
        "Berkeley Lab, down to the computer center in the basement of the same building. Well, hell, "
        "I could fake enough computing to impress astronomers, and maybe pick it up fast enough that "
        "my co-workers wouldn't catch on. Still, a computer wizard? Not me-I'm an astronomer. Now what? "
        "As I apathetically stared at my computer terminal, I still thought of planetary orbits and "
        "astrophysics. As new kid on the block, I had my choice of a cubicle with a window facing the "
        "Golden Gate Bridge, or an unventilated office with a wall of bookshelves. Swallowing my "
        "claustrophobia, I picked the office, hoping that nobody would notice when I slept under the desk. "
        "On either side were offices of two systems people, Wayne Graves and Dave Cleveland, the old hands "
        "of the system. I soon got to know my neighbors through their bickering. Viewing everyone as "
        "incompetent or lazy, Wayne was crossthreaded with the rest of the staff. Yet he knew the system "
        "thoroughly, from the disk driver software up to the microwave antennas. Wayne was weaned on Digital "
        "Equipment Corporation's Vax computers and would tolerate nothing less: not IBM, not Unix, not "
        "Macintoshes. Dave Cleveland, our serene Unix buddha, patiently listened to Wayne's running stream "
        "of computer comparisons. A rare meeting didn't have Wayne's pitch, \"Vaxes are the choice of "
        "scientists everywhere and help build strong programs twelve ways.\" Dave retorted, \"Look, you keep "
        "your Vax addicts happy and I'll handle the rest of the world.\" Dave never gave him the satisfaction "
        "of getting riled, and Wayne's complaints eventually trailed off to a mutter. Great. First day on the "
        "job, sandwiched between two characters who were already ruining my daydreams with their periodic "
        "disputes. At least nobody could complain about my appearance. I wore the standard Berkeley corporate "
        "uniform: grubby shirt, faded jeans, long hair, and cheap sneakers. Managers occasionally wore ties, "
        "but productivity went down on the days they did. Together, Wayne, Dave, and I were to run the computers "
        "as a lab-wide utility. We managed a dozen mainframe computers-giant workhorses for solving physics problems, "
        "together worth around six million dollars. The scientists using the computers were supposed to see a simple, "
        "powerful computing system, as reliable as the electric company. This meant keeping the machines running full time, "
        "around the clock. And just like the electric company, we charged for every cycle of computing that was used. Of four "
        "thousand laboratory employees, perhaps a quarter used the main computers. Each of these one thousand accounts was "
        "tallied daily, and ledgers kept inside the computer. With an hour of computing costing three hundred dollars, our "
        "bookkeeping had to be accurate, so we kept track of every page printed, every block of disk space, and every minute of "
        "processor time. A separate computer gathered these statistics and sent monthly bills to laboratory departments. And so "
        "it happened that on my second day at work, Dave wandered into my office, mumbling about a hiccup in the Unix accounting ")

    base_data = (base_block * (CHARS_PER_FILE // len(base_block) + 1))[:CHARS_PER_FILE]

    for i in range(6):
        data = list(base_data)
        swap_fraction = i / 5.0
        num_swaps = int(len(data) * swap_fraction)

        for _ in range(num_swaps):
            a, b = random.randint(0, len(data) - 1), random.randint(0, len(data) - 1)
            data[a], data[b] = data[b], data[a]

        file_path = dir_path / f"structure_randomness_{i}.txt"
        with open(file_path, "w", encoding="ascii") as f:
            f.write(''.join(data))
        print(f"Generated {file_path} with randomness level {i}")

# Simulates log files (i wanted some sort of structured data with randomness)
def generate_structured_size_files():
    dir_path = BASE_DIR / "structured_size"
    dir_path.mkdir(exist_ok=True)

    for i in range(1, 6):
        num_entries = 10 ** i
        entries = []

        for j in range(num_entries):
            timestamp = f"2024-01-01 00:{j // 60:02d}:{j % 60:02d}"
            user_id = j
            action = random.choice(["LOGIN", "LOGOUT", "DOWNLOAD", "UPLOAD", "ERROR", "UPDATE"])
            status = random.choice(["SUCCESS", "FAIL", "TIMEOUT", "PENDING"])

            log_line = f"[{timestamp}] USER:{user_id:06d} ACTION:{action:<8} STATUS:{status}"
            entries.append(log_line)

        content = '\n'.join(entries)
        file_path = dir_path / f"structured_size_{i}.txt"
        with open(file_path, "w", encoding="ascii", errors="ignore") as f:
            f.write(content)

        size_kb = len(content) / 1024
        print(f"Generated {file_path} with {num_entries} entries (~{size_kb:.1f} KB)")

if __name__ == "__main__":
    generate_repetitive_files()
    generate_structure_randomness_files()
    generate_structured_size_files()
