#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Plot the CSV files in the benchmarks directory and export them as SVG files.
"""

from os import listdir, path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

INDIR = './benchmarks'
OUTDIR = './figures/'
BASENAME = 'benchmark'


def main():
    files = [f for f in listdir(INDIR)]

    # Lord forgive me for this sin
    for file in files:
        if file.startswith("ss_start"):
            plot(file, title="an early")
        elif file.startswith("ss_end"):
            plot(file, title="a late")
        else:
            plot(file, title="an average")


def plot(file, title):
    df = pd.read_csv(path.join(INDIR, file), index_col=0, header=0)
    df.plot(legend=True, figsize=(7.5, 4), grid=True, marker='o')

    ax = plt.gca()
    ax.set_xticks(df.index.values)
    ax.set_xticklabels([str(el) for el in df.index.values])

    plt.title(f"Execution time by thread count for {title} match")
    plt.xlabel("Thread count [N]")
    plt.ylabel('Time [ms]')
    plt.legend(loc='best', title='CPU')

    plt.savefig(OUTDIR + file.replace('.csv', '.svg'), dpi=300, bbox_inches='tight')
    plt.show()


if __name__ == "__main__":
    main()
