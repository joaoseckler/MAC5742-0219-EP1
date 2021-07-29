import re
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as plticker
import numpy as np
from scipy.stats import norm

# logs = [f for f in glob2.glob("src/results/**/*.log")]

logs = [f"results/mandelbrot_{f}/triple_spiral.log" for f in
        ('seq', 'pth', 'omp', 'ompi', 'ompi_pth', 'ompi_omp') ]
        # ('seq', 'pth', 'omp', 'ompi') ]

graph_names = [
        'Sequencial',
        'Pthreads',
        'OMP',
        'OMPI',
        'OMPI+Pthreads',
        'OMPI+OMP',
        ]

graph_names = dict(zip(logs, graph_names))

def get_avg_std_time(path, export_csv=True):
    basename = os.path.basename(path).split('.')[0]
    dirname = os.path.dirname(path)

    with open(path, 'r') as file:
        log = file.read()

    # regex to extract interest lines
    times = re.findall("\d.*?seconds", log)
    processes = re.findall("--oversubscribe -n (\d+)", log)
    threads = re.findall("4096 (\d+)' \(15 run", log)

    # create list with information and parse
    avg_time = [float(time.split(' ')[0].replace(',','.')) for time in times]
    std_time = [float(time.split(' ')[2].replace(',','.')) for time in times]

    if not processes:
        processes = [1 for _ in avg_time]
    if not threads:
        threads = [2 for _ in avg_time]


    # create pandas dataframe
    data = {
        'avg_time': avg_time,
        'std_time': std_time,
        'processes': processes,
        'threads': threads,
    }

    df = pd.DataFrame(data)
    df.threads = pd.to_numeric(df.threads)
    df.processes = pd.to_numeric(df.processes)

    # export csv
    if export_csv:
        print('Saving csv in {}'.format(os.path.join(dirname, basename+'.csv')))
        df.to_csv(os.path.join(dirname, basename+'.csv'))

    return df

for path in logs:
    print(path)
    basename = os.path.basename(path).split('.')[0]
    dirname = os.path.dirname(path)
    graph_name = os.path.basename(dirname)

    df = get_avg_std_time(path)

    # set heights of bars
    bars1 = df[df['threads'] == 2].avg_time.to_list()
    bars2 = df[df['threads'] == 4].avg_time.to_list()
    bars3 = df[df['threads'] == 8].avg_time.to_list()
    bars4 = df[df['threads'] == 16].avg_time.to_list()
    bars5 = df[df['threads'] == 32].avg_time.to_list()

    # std*2 => 95% confidence interval
    err1 = np.array(df[df['threads'] == 2].std_time.to_list()) * 2
    err2 = np.array(df[df['threads'] == 4].std_time.to_list()) * 2
    err3 = np.array(df[df['threads'] == 8].std_time.to_list()) * 2
    err4 = np.array(df[df['threads'] == 16].std_time.to_list()) * 2
    err5 = np.array(df[df['threads'] == 32].std_time.to_list()) * 2

    # Set position of bar on X axis

    plt.figure()

    if bars2: # Se houver teste com threads
        barWidth = 0.15

        r1 = np.arange(len(bars1))
        r2 = [x + barWidth for x in r1]
        r3 = [x + barWidth for x in r2]
        r4 = [x + barWidth for x in r3]
        r5 = [x + barWidth for x in r4]

        plt.bar(r1, bars1, width=barWidth, edgecolor='white', label='2 threads',
                log=True, yerr=err1, capsize=3)
        plt.bar(r2, bars2, width=barWidth, edgecolor='white', label='4 threads',
                log=True, yerr=err2, capsize=3)
        plt.bar(r3, bars3, width=barWidth, edgecolor='white', label='8 threads',
                log=True, yerr=err3, capsize=3)
        plt.bar(r4, bars4, width=barWidth, edgecolor='white', label='16 threads',
                log=True, yerr=err4, capsize=3)
        plt.bar(r5, bars5, width=barWidth, edgecolor='white', label='32 threads',
                log=True, yerr=err5, capsize=3)
    else:
        r1 = np.arange(len(bars1))
        plt.bar(r1, bars1, edgecolor='white', label='sequencial',
                log=True, yerr=err1, capsize=3)
        barWidth = 0

    labels = ['1', '8', '16', '32', '64']

    if len(bars1) == 1:
        label = []
        plt.xlabel("1 processo")
    else: # ompi+pth ou #ompi+omp
        label = labels
        plt.xlabel("Númeo de processos")

    plt.xticks([r + barWidth for r in range(len(bars1))], label)

    # Use linear scale
    # plt.yscale('linear')
    # plt.gca().set_ylim(ymin=0)

    # Add some text for labels, title and custom x-axis tick labels, etc.
    plt.ylabel('Tempo Execução (s)')
    plt.title(graph_names[path])

    plt.legend()
    # plt.show()
    print('Saving graph in {}'.format(os.path.join(dirname, basename+'.png')))
    plt.savefig(os.path.join(dirname, basename + '.png'))
    plt.close()

