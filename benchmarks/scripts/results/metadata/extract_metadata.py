import csv
import json
from pathlib import Path

DODO_DIR = Path(__file__).resolve().parent / "../../../dodo"
OUTPUT_FILE = Path(__file__).resolve().parent / "alphabet-sizes.csv"


def extract_alphabet_sizes():
    rows = []
    for path in sorted(DODO_DIR.glob("*.json")):
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
        alphabet = data.get("alphabet")
        rows.append((path.stem, len(alphabet) if alphabet is not None else 0))

    with open(OUTPUT_FILE, "w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["name", "alphabet_size"])
        writer.writerows(rows)

    return rows


if __name__ == "__main__":
    extract_alphabet_sizes()
