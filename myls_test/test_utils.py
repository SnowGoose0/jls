import subprocess
from itertools import chain, combinations

MEM_DELIM = "All heap blocks were freed -- no leaks are possible"

def run(cmd):
    subprocess.run(cmd, shell=True, capture_output=False)

def powerset(iterable):
    "powerset([1,2,3]) --> () (1,) (2,) (3,) (1,2) (1,3) (2,3) (1,2,3)"
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(len(s)+1))

def getOpt(opts):
    if len(opts) == 0:
        return ""
    
    optFormatted = "-"
    
    for op in opts:
        optFormatted += op

    return optFormatted 

def getPath(paths):
    if len(paths) == 0:
        return ""
    
    pathFormatted = ""

    for path in paths:
        pathFormatted += (path + " ")
    
    return pathFormatted

def compareFiles(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        for line1, line2 in zip(f1, f2):
            tokens1 = line1.split()
            tokens2 = line2.split()

            if len(tokens1) > 0 and tokens1[0] == 'total':
                line1 = next(f1, None)
                tokens1 = line1.split()

            if len(tokens2) > 0 and tokens2[0] == 'total':
                line2 = next(f2, None)
                tokens2 = line2.split()

            if tokens1 == tokens2:
                pass
            else:
                return False
            
def mCheck(path):
    try:
        with open(path, 'r') as file:
            for line in file:
                if MEM_DELIM in line:
                    return True
        return False
    except FileNotFoundError:
        print(f"File not found: {path}")
        return False