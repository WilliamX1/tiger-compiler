#!/bin/bash

function doReap() {
	rm -f testcases refs _tmp.txt .tmp.txt
}

mkdir -p build
cd build
doReap
ln -s ../testdata/lab1/testcases testcases
if [[ $? != 0 ]]; then
	echo "[-_-]$ite: Link Error"
	echo "LAB1 SCORE: 0"
	exit 123
fi

ln -s ../testdata/lab1/refs refs
if [[ $? != 0 ]]; then
	echo "[-_-]$ite: Link Error"
	echo "LAB1 SCORE: 0"
	exit 123
fi

cmake .. >&/dev/null
make test_slp -j >/dev/null
if [[ $? != 0 ]]; then
	echo "[-_-]: Compile Error"
	echo "LAB1 SCORE: 0"
	exit 1
fi

test_num=$((($RANDOM % 2)))

if [[ $test_num == 0 ]]; then
	REFOUTPUT=./refs/ref-0.txt
else
	REFOUTPUT=./refs/ref-1.txt
fi

./test_slp $test_num >&_tmp.txt
diff -w _tmp.txt $REFOUTPUT >&.tmp.txt
if [ -s .tmp.txt ]; then
	echo "[*_*]: Output Mismatch"
	cat .tmp.txt
	doReap
	echo "LAB1 SCORE: 0"
	exit 1
fi

doReap
echo "[^_^]: Pass"
echo "LAB1 SCORE: 100"
