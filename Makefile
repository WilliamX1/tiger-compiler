.PHONY: docker-build docker-pull docker-run docker-run-backend transform build gradelab1 gradelab2 gradelab3 gradelab4 gradelab5 gradelab6 gradeall clean register format

docker-build:
	docker build -t ipadsse302/tigerlabs_env .

docker-pull:
	docker pull ipadsse302/tigerlabs_env:latest

docker-run:
	docker run -it --privileged -p 2222:22 \
		-v /home/parallels/Desktop/compilers-2021:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest

docker-run-backend:
	docker run -dt --privileged -p 2222:22 \
		-v /home/parallels/Desktop/compilers-2021:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest

transform:
	find src scripts testdata -type f | xargs -I % sh -c 'dos2unix -n % /tmp/tmp; mv -f /tmp/tmp % || true;'

build:transform
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make

build-debug:transform
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

gradelab1:transform
	bash scripts/grade.sh lab1

gradelab2:transform
	bash scripts/grade.sh lab2

gradelab3:transform
	bash scripts/grade.sh lab3

gradelab4:transform
	bash scripts/grade.sh lab4

gradelab5-1:transform
	bash scripts/grade.sh lab5-part1

gradelab5:transform
	bash scripts/grade.sh lab5

gradelab6:transform
	bash scripts/grade.sh lab6

gradeall:transform
	bash scripts/grade.sh all

clean:
	rm -rf build/ src/tiger/lex/scannerbase.h src/tiger/lex/lex.cc \
		src/tiger/parse/parserbase.h src/tiger/parse/parse.cc

register:
	python3 scripts/register.py

format:
	find . \( -name "*.h" -o -iname "*.cc" \) | xargs clang-format -i -style=file

