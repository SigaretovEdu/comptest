import argparse
import sys
import os
import subprocess
import re
import time
import signal


compile_options = {
    "cpp": {
        "compiler": "g++",
        "options": ["-Wall", "-Wextra", "-Wpedantic", "-o"]
    },
    "c": {
        "compiler": "gcc",
        "options": ["-Wall", "-Wextra", "-Wpedantic", "-o"]
    },
}

run_options = {
    "py": "python"
}

# TL in seconds
TL = 1


def compile(in_path: str, out_path: str):
    type = gettype(in_path)
    if type not in compile_options:
        return in_path

    compiler = compile_options[type]["compiler"]
    opt = " ".join(compile_options[type]["options"])
    cmd = f"{compiler} {opt} {out_path} {in_path}"
    res_code = os.system(cmd)
    if int(res_code) != 0:
        sys.exit()
    return out_path


def gettype(path: str):
    return os.path.splitext(path)[1].replace('.', '')


def read_tests(testpath: str):
    with open(testpath, 'r') as tf:
        tests = dict()
        lines = tf.readlines()
        num = 0
        in_test = False
        for l in lines:
            # start reading test
            if l[0:2] == ">>":
                l = re.sub(r'\s*$', '', l)
            if not in_test and l[0:2] == ">>":
                in_test = True
                cleared_l = re.sub(r"\s{2,}", " ", l).split(' ')
                if len(cleared_l) > 1:
                    num = int(cleared_l[1])
                else:
                    if len(tests) > 0:
                        num = max(tests) + 1
                    else:
                        num = 1
                continue
            if l[0:2] == "<<":
                in_test = False
                continue
            if in_test:
                # l = l.replace('\n', '')
                if num not in tests:
                    tests[num] = l
                else:
                    tests[num] = tests[num] + l
    for t in tests:
        ll = tests[t].split('>>')
        ll.append(None)
        # print(ll)
        tests[t] = ll[0:2]
    return tests


def run_test(test, exe_file: str):
    cmd = ""
    type = gettype(exe_file)
    if type in run_options:
        cmd = run_options[type] + " "
    cmd = cmd + exe_file

    result = 'OK'
    duration = 0
    output = []

    proc = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding='utf8')
    try:
        outs = proc.communicate(input=test, timeout=TL)
        if proc.returncode == 0:
            result = 'OK'
        else:
            result = 'RE'
    except subprocess.TimeoutExpired:
        proc.kill()
        outs = proc.communicate()
        result = 'TL'

    return [result, outs[0]]


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'


def print_diff(out, ver):
    f1 = open('build/left.txt', 'w')
    f1.write(out)
    f1.close()
    f2 = open('build/right.txt', 'w')
    f2.write(ver)
    f2.close()

    with subprocess.Popen(['icdiff', '--no-headers', '--color-map=change:red_bold', 'build/left.txt', 'build/right.txt'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) as proc:
        for line in proc.stdout:
            print(line, end='')


def check_exe_test(num: int, inp: str, ver: str, exe_file: str):
    code, out = run_test(inp, exe_file)
    if code != 'OK':
        print(f"{bcolors.FAIL}{code}{bcolors.ENDC}")
        print(inp)
        print(out)
        return 1
    if ver is not None:
        if out == ver:
            return 0
        else:
            print(f"{bcolors.FAIL}<<<<<<{bcolors.ENDC} ", end='')
            print(f"{bcolors.FAIL}TEST_{num} WA{bcolors.ENDC}")
            print(f"{bcolors.FAIL}{inp}{bcolors.ENDC}", end='')
            print(f"{bcolors.FAIL}<<<<<<{bcolors.ENDC}")
            print_diff(out, ver)
            print(f"{bcolors.FAIL}<<<<<<{bcolors.ENDC}\n")

            return 1
    else:
        print(f"{bcolors.OKBLUE}<<<<<<{bcolors.ENDC} ", end='')
        print(f"{bcolors.OKBLUE}TEST_{num} UNKNOWN{bcolors.ENDC}")
        print(f"{bcolors.OKBLUE}{inp}{bcolors.ENDC}", end='')
        print(f"{bcolors.OKBLUE}<<<<<<{bcolors.ENDC}")
        print(f"{out}", end='')
        print(f"{bcolors.OKBLUE}<<<<<<{bcolors.ENDC}\n")


def check_stress(num: int, inp: str, exe1: str, exe2: str):
    vercode, verout = run_test(inp, exe2)
    if vercode != 'OK':
        print(f"Stress solution failed at test_{num}")
        return 2
    return check_exe_test(num, inp, verout, exe1)


def clean():
    os.system('rm -r build')

# START


parser = argparse.ArgumentParser(description='comptest: Compile and test')

parser.add_argument('source', type=str, help='source code')
parser.add_argument('test_file', type=str, help='file with test')
parser.add_argument('-c', '--checker', type=str, help='file with right code')
parser.add_argument('-t', '--test', type=int, help='test num')

args = parser.parse_args()

if len(sys.argv) < 3:
    parser.print_help()
    sys.exit()

source_file = args.source
test_file = args.test_file
checker_file = args.checker
test_num = args.test

run_source = None
run_check = None

if not os.path.isdir('build'):
    os.system('mkdir build')

if not os.path.isfile(test_file):
    print(f"{test_file} file doesn't exist")
    sys.exit()

if not os.path.isfile(source_file):
    print(f"{source_file} file doesn't exist")
    sys.exit()
else:
    run_source = compile(source_file, "build/sol")

if checker_file != None:
    if not os.path.isfile(checker_file):
        print(f"{checker_file} file doesn't exist")
        sys.exit()
    else:
        run_check = compile(checker_file, "build/check")

# print(run_source)
# print(run_check)
# print(test_file)
# print(test_num)

alltests = read_tests(test_file)
# print(alltests)

if run_check is None:
    ok, failed, unknown = 0, 0, 0
    summary = len(alltests.keys())
    if test_num is not None:
        if test_num not in alltests.keys():
            print(f"There is no test with num {test_num}")
            sys.exit()
        keys = [test_num]
    else:
        keys = alltests.keys()
    for k in keys:
        result = check_exe_test(
            k, alltests[k][0], alltests[k][1], run_source)
        if result == 0:
            ok += 1
        elif result == 1:
            failed += 1
        else:
            unknown += 1

    if failed > 0 or unknown > 0:
        print()
    print(f"{bcolors.OKGREEN}OK({ok}){bcolors.ENDC}")
    if failed > 0:
        print(f"{bcolors.FAIL}FAILED({failed}){bcolors.ENDC}")
    if unknown > 0:
        print(f"{bcolors.OKBLUE}UNKNOWN({unknown}){bcolors.ENDC}")
else:
    ok, failed, unknown = 0, 0, 0
    summary = len(alltests.keys())
    if test_num is not None:
        if test_num not in alltests.keys():
            print(f"There is no test with num {test_num}")
            sys.exit()
        keys = [test_num]
    else:
        keys = alltests.keys()
    for k in keys:
        result = check_stress(
            k, alltests[k][0], run_source, run_check)
        if result == 0:
            ok += 1
        elif result == 1:
            failed += 1
        else:
            unknown += 1

    if failed > 0 or unknown > 0:
        print()
    print(f"{bcolors.OKGREEN}OK({ok}){bcolors.ENDC}")
    if failed > 0:
        print(f"{bcolors.FAIL}FAILED({failed}){bcolors.ENDC}")
    if unknown > 0:
        print(f"{bcolors.OKBLUE}STRESS_SOL_FAILED({unknown}){bcolors.ENDC}")

clean()
