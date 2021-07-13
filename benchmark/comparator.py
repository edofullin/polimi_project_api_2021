#!.env/bin python3

import os
import subprocess
import shutil
import json
import argparse
import string
import hashlib
import random
from sys import stderr, stdin
import time
import matplotlib.pyplot as plt

results = []

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('source_filename')
    parser.add_argument('input_dir')
    parser.add_argument('comment', default='')
    parser.add_argument('--compare_old', action='store_true')
    parser.add_argument('--store_output', action='store_true')
    parser.add_argument('--log_scale', action='store_true')
    args = parser.parse_args()
    randstr = ''

    if args.compare_old and os.path.exists('benchmark/results.json'):
        print('\nloading old result', end='', flush=True)
        results = json.loads('benchmark/results.json')
        print('\n  done.', end='', flush=True)

    with open(args.source_filename, 'r') as f:
        file_con = f.read()
        time.sleep(1)   
        randstr = hashlib.sha1(file_con.encode('utf-8')).hexdigest()


    source_filename = args.source_filename
    source_filename_dir = f'benchmark/sources/{source_filename}.{randstr}'
    shutil.copy2(source_filename, source_filename_dir)

    print("compiling... ", end='', flush=True)
    binary_filename = f"benchmark/binaries/{source_filename.replace('.c', '')}.{randstr}"

    #print(f"\nsource file is {source_filename_dir} binary is {binary_filename} cwd is {os.getcwd()}")

    comp_res = subprocess.run(['/usr/bin/gcc', '-DEVAL', '-Wall', '-std=gnu11', '-O2', '-pipe', f'-o', binary_filename, source_filename], capture_output=True, cwd=os.getcwd())
    
    if comp_res.returncode != 0:
        print("compilation failed", end='', flush=True)
        print(comp_res.stderr)
        exit()
    
    print('  done.', end='', flush=True)

    obj = {
            'name': randstr,
            'comment': args.comment,
            'runs': []
    }

    tests = os.listdir('benchmark/input')
    tests.sort(key=lambda x: int(x.split('.')[1]))

    for file in tests:
        print(f'\nrunning input {file} on {binary_filename}', end='', flush=True)
        input = open(os.path.join('benchmark/input', file))
        output = None
        proc_res = None
        start_time = 0

        if args.store_output:
            output = open(os.path.join('benchmark/output', f"out_{randstr}_{file}"), 'w')
            start_time = time.perf_counter()
            proc_res = subprocess.Popen(binary_filename, stdin=input, stdout=output, stderr=output)
        else:
            start_time = time.perf_counter()
            proc_res = subprocess.Popen(binary_filename, stdin=input, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        proc_res.wait()
        stop_time = time.perf_counter()
        
        if args.store_output:
            output.flush()
            output.close()

        input.close()

        print('  done.', end='', flush=True)

        obj['runs'].append(
            {
                 'input': file,
                 'runtime': stop_time-start_time
            }
        )
    
    results.append(obj)

    if args.compare_old:
        print('\ndumping result to file', end='', flush=True)
        with open('benchmark/results.json', 'w', encoding='utf-8') as file:
            json.dump(results, file)
        print('  done.', end='', flush=True)

    legend = []

    for obj in results:
        inputs = list(map(lambda x: int(x['input'].split('.')[1]), obj['runs']))
        runtimes = list(map(lambda x: x['runtime'] * 1000, obj['runs']))
        legend.append(obj['comment'])
        plt.plot(inputs, runtimes, 'ro')

    plt.xlabel('test #')
    plt.ylabel('runtime (ms)')

    plt.legend(legend)

    if(args.log_scale):
        plt.yscale('log')

    plt.show()
        


        
















    


