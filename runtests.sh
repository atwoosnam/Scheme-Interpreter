 #!/bin/bash

echo "======== RUNNING TESTS ========"
echo "Test01"
./interpreter < interpreter-test.input.01 | diff interpreter-test.output.01 -
echo "Test02"
./interpreter < interpreter-test.input.02 | diff interpreter-test.output.02 -
echo "Test03"
./interpreter < interpreter-test.input.03 | diff interpreter-test.output.03 -
echo "Test04"
./interpreter < interpreter-test.input.04 | diff interpreter-test.output.04 -
echo "Test05"
./interpreter < interpreter-test.input.05 | diff interpreter-test.output.05 -
echo "Test06"
./interpreter < interpreter-test.input.06 | diff interpreter-test.output.06 -
echo "Test07"
./interpreter < interpreter-test.input.07 | diff interpreter-test.output.07 -
echo "Test08"
./interpreter < interpreter-test.input.08 | diff interpreter-test.output.08 -
echo "Test09"
./interpreter < interpreter-test.input.09 | diff interpreter-test.output.09 -
echo "Test10"
./interpreter < interpreter-test.input.10 | diff interpreter-test.output.10 -
echo "Test11"
./interpreter < interpreter-test.input.11 | diff interpreter-test.output.11 -
echo "Test12"
./interpreter < interpreter-test.input.12 | diff interpreter-test.output.12 -
echo "Test13"
./interpreter < interpreter-test.input.13 | diff interpreter-test.output.13 -
echo "Test14"
./interpreter < interpreter-test.input.14 | diff interpreter-test.output.14 -
echo "Test15"
./interpreter < interpreter-test.input.15 | diff interpreter-test.output.15 -
echo "Test16"
./interpreter < interpreter-test.input.16 | diff interpreter-test.output.16 -
echo "Test17"
./interpreter < interpreter-test.input.17 | diff interpreter-test.output.17 -
echo "Test18"
./interpreter < interpreter-test.input.18 | diff interpreter-test.output.18 -
echo "Test19"
./interpreter < interpreter-test.input.19 | diff interpreter-test.output.19 -
echo "Test20"
./interpreter < interpreter-test.input.20 | diff interpreter-test.output.20 -
echo "====== DONE RUNNING TESTS ======"
