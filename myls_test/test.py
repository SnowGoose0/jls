import sys
import test_utils
import time

OPT = list(test_utils.powerset(["i", "l", "R"]))
DIR = list(test_utils.powerset(["./testdir/", "./testdir/dir1", "./testdir/dir2", "./testdir/dir2/dir2sub1", "./testdir/dir3", "./testdir/dir3/dir3sub1", "./testdir/dir3/dir3sub2/dir3sub2sub1", "./testdir/dir3/dir3sub2/dir3sub2sub1/dir3sub2sub1sub1", "./testdir/dir3/dir3sub3", "./testdir/dir3/dir3sub3/dir3sub3sub1", "./testdir/dir4", "./testdir/dir4/dir4sub1", "./testdir/dir4/dir4sub1/dir4sub1sub1"]))

CMD_MLS = "./myls {opt} {dir} > {file}"
CMD_LLS = "ls -1 {opt} {dir} > {file}"
CMD_MEM = "valgrind --leak-check=full --log-file={file} {command}"

MEM_LOG = "./.hidden/valgrind_log.txt"

OUT_MLS = "./.hidden/myls.out"
OUT_LLS = "./.hidden/ls.out"

RED = '\033[31m'
GREEN = '\033[32m'
BOLD = '\033[1m'
RESET = '\033[0m'

STEP = -1

def pprint(color_code, text):
    print(color_code + BOLD + text + RESET)

def main():
    memCheck = False

    if len(sys.argv) >= 2:
        print(sys.argv[1])
        if sys.argv[1] == "-memcheck":
             memCheck = True

    testCaseCount = len(OPT) * len(DIR)
    testCasePassCount = 0

    startTime = time.time()

    for path in DIR:
        for op in OPT:
            options = test_utils.getOpt(op)
            options_ls = options
            directories = test_utils.getPath(path)

            if "l" in options:
                    options_ls = options + " --time-style=\"+%b %e %Y %H:%M\" "
            
            if memCheck:
                test_utils.run(CMD_MEM.format(file=MEM_LOG, command=CMD_MLS.format(opt=options, dir=directories, file=OUT_MLS)))
            else:
                test_utils.run(CMD_MLS.format(opt=options, dir=directories, file=OUT_MLS))

            test_utils.run(CMD_LLS.format(opt=options_ls, dir=directories, file=OUT_LLS))

            result = test_utils.compareFiles(OUT_MLS, OUT_LLS)

            if result == False:
                pprint(RED, "Error:      test case: {num}".format(num=(testCasePassCount + 1)))
                pprint(RED, CMD_MLS.format(opt=options, dir=directories, file=OUT_MLS))
                pprint(RED, CMD_LLS.format(opt=options_ls, dir=directories, file=OUT_LLS))
                return
            
            else:
                pprint(GREEN, "Success:    test case: {num}".format(num=(testCasePassCount + 1)))
            
            if memCheck:
                if not test_utils.mCheck(MEM_LOG):
                     pprint(RED, "Mem Leak:   test case: {num}".format(num=(testCasePassCount + 1)))
                     pprint(RED, CMD_MLS.format(opt=options, dir=directories, file=OUT_MLS))
                     return
                else:
                     pprint(GREEN, "Mem Ok:     test case: {num}".format(num=(testCasePassCount + 1)))
                     

            print("\n")
            
            testCasePassCount += 1

            if testCasePassCount == STEP:
                    break
           
        if testCasePassCount == STEP:
                break

    print("Tests Passed: " + str(testCasePassCount) + "/" + str(testCaseCount))

    endTime = time.time()
    execTime = endTime - startTime
    print(f"Execution Time: {execTime:.6f} seconds")

main()