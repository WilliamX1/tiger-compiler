.PHONY: docker-build docker-run docker-run-backend transform build gradelab1 gradelab2 gradelab3 gradelab4 gradelab5 gradelab6 gradeall handin clean register

docker-build:
	docker build -t ipadsse302/tigerlabs_env .

docker-run:
	docker run -it --privileged -p 2222:22 \
		-v /home/parallels/Desktop/compilers-2021:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest

docker-run-backend:
	docker run -dt --privileged -p 2222:22 \
		-v /home/parallels/Desktop/compilers-2021:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest

transform:
	find . \( -name "*.cc" -o -name "*.tig" -o -name "*.h" -o -name "*.sh" -o -name "*.txt" \) | xargs -I % sh -c 'dos2unix -n % tmp; mv -f tmp %;'

build:transform
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make

build-debug:transform
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

gradelab1:
	bash scripts/gradelab1.sh

gradelab2:
	bash scripts/gradelab2.sh

gradelab3:
	bash scripts/gradelab3.sh

gradelab4:
	bash scripts/gradelab4.sh

gradelab5:
	bash scripts/gradelab5\&6.sh

gradelab6:
	bash scripts/gradelab5\&6.sh

gradeall:
	bash scripts/gradelab1.sh
	bash scripts/gradelab2.sh
	bash scripts/gradelab3.sh
	bash scripts/gradelab4.sh
	bash scripts/gradelab5\&6.sh

handin:
	bash scripts/handin.sh

clean:
	bash scripts/clean.sh

register:
	python3 scripts/register.py

