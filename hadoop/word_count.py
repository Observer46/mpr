from collections import defaultdict
from typing import List, Dict

import os
import sys
import time

Word = str


def read_input(filepath: str) -> List[Word]:
    with open(filepath, "r", encoding='latin-1') as handle:
        for line in handle.readlines():
            yield line.split()


def word_count(path: str):
    files = [os.path.join(path, file) for file in os.listdir(path)]
    word_to_wc = defaultdict(lambda: 0)
    for filepath in files:
        data = read_input(filepath)
        for words in data:
            for word in words:
                word_to_wc[word] += 1
    return word_to_wc


def print_res(word_to_wc: Dict[Word, int]):
    output_file = "output.txt"
    with open(output_file, 'w', encoding='utf-8') as handle:
        for word, count in word_to_wc.items():
            line = f"{word}: {count}\n"
            handle.write(line)


if __name__ == "__main__":
    if len(sys.argv[1]) < 2:
        print("Usage: python word_count.py <path_to_folder_with_txt_files>")
        exit(1)

    path_to_files = sys.argv[1]
    start = time.time()
    wc = word_count(path_to_files)
    print_res(wc)
    end = time.time()
    print("Time:", end-start)
