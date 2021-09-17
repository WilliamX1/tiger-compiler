# Tiger Compiler Labs in C++

## Contents

- [Tiger Compiler Labs in C++](#tiger-compiler-labs-in-c)
  - [Contents](#contents)
  - [Overview](#overview)
  - [Difference Between C Labs and C++ Labs](#difference-between-c-labs-and-c-labs)
  - [Getting Newly Released Labs](#getting-newly-released-labs)
  - [Installing Dependencies](#installing-dependencies)
    - [Ubuntu18.04](#ubuntu1804)
    - [MacOS, Windows Professional, Ubuntu16.04, and other Linux Distributions](#macos-windows-professional-ubuntu1604-and-other-linux-distributions)
  - [Compiling](#compiling)
  - [Debugging](#debugging)
  - [Grading Your Labs](#grading-your-labs)
  - [Submitting Your Labs](#submitting-your-labs)
  - [FAQ](#faq)

## Overview

We rewrote the Tiger Compiler labs using the C++ programming language. This is
because C++ has some features like inheritance and polymorphism, which we think
is more suitable for these labs and less error-prone.

We provide you all the codes of all labs at one time. In each lab, you only
need to code in some of the directories.

The Tiger Compiler Labs are not perfect, but we have tried our best and spent a
lot of time on it. If you find any bugs or have a better design, please inform
us.

## Difference Between C Labs and C++ Labs

1. This new labs use [flexc++](https://fbb-git.gitlab.io/flexcpp/manual/flexc++.html) and [bisonc++](https://fbb-git.gitlab.io/bisoncpp/manual/bisonc++.html) instead of flex and bison because flexc++ and bisonc++ is more flexc++ and bisonc++ are able to generate pure C++ codes instead of C codes wrapped in C++ files.

2. The new labs use namespace for modularization and use inheritance and polymorphism to replace unions used in the old labs.

3. These new labs use CMake instead of Makefile to compile and build the target.

## Installing Dependencies

flexc++ and bisonc++ will be needed in lab2 and later.
Although these libraries are not needed in lab1, you have to install them before you start lab1.

**Notice:**: Now we only support the following version of flexc++ and bisonc++:

```bash
flexc++ - V2.06.02
bisonc++ - V6.01.00
```

### Docker (Recommended)

We provide you a Docker image that has already installed all the dependencies. You can compile your codes directly in this Docker image.

1. Install [Docker](https://docs.docker.com/).

2. Run a docker container and mount the lab directory on it.

```bash
docker run -it --privileged -p 2222:22 -v ${PWD}:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest  # or make docker-run
```

### Ubuntu18.04

```bash
sudo apt install git tar cmake g++ gcc gdb flexc++ bisonc++
```

**Notice:** This series of labs now only support Ubuntu18.04. For those who use **Ubuntu16.04**, you still need to use Docker to build and run your labs.

## Compiling and Debugging

There are five makeable targets in total, including `test_slp`, `test_lex`
, `test_parse`, `test_semant`,  and `tiger-compiler`.

1. Run container environment and attach to it

```bash
# Run container and directly attach to it
docker run -it --privileged -p 2222:22 \
    -v ${PWD}:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest  # or `make docker-run`
# Or run container in the backend and attach to it later
docker run -dt --privileged -p 2222:22 \
    -v ${PWD}:/home/stu/tiger-compiler ipadsse302/tigerlabs_env:latest
docker attach ${YOUR_CONTAINER_ID}
```

2. Build in the container environment

```bash
mkdir build && cd build && cmake .. && make test_xxx  # or `make build`
```

3. Debug using gdb or any IDEs

```bash
gdb test_xxx # e.g. `gdb test_slp`
```

**Note: we will use `-DCMAKE_BUILD_TYPE=Release` to grade your labs, so make
sure your lab passed the released version**

## Testing Your Labs

Use `make`
```bash
make gradelabx
```
or run the script manually
```bash
# cd to the lab root directory, i.e. compilers-2021
./scripts/gradelabx.sh # e.g. `./gradelab1.sh`
```

You can test all the labs by
```bash
make gradeall
```

## Submitting Your Labs

Run `make register` and input your name in English and student ID. You can
check it in the `.info` file generated later.

We are using CI in GitLab to grade your labs automatically. **So please make
sure the `Enable shared runners for this project`
under `Your GitLab repo - Settings - CI/CD` is turned on**.

Push your code to your GitLab repo
```bash
git add somefiles
git commit -m "A message"
git push
```

Wait for a while and check the latest pipeline (`Your GitLab repo - CI/CD -
Pipelines`) passed. Otherwise, you won't get a full score in your lab.

## Formatting Your Codes

We provide an LLVM-style .clang-format file in the project directory. You can use it to format your code.

Use `clang-format` command
```
find . \( -name "*.h" -o -iname "*.cc" \) | xargs clang-format -i -style=file  # or make format
```

or config the clang-format file in your IDE and use the built-in format feature in it.

## Other Commands

Utility commands can be found in the `Makefile`. They can be directly run by `make xxx` in a Unix shell. Windows users cannot use the `make` command, but the contents of `Makefile` can still be used as a reference for the available commands.

## Contributing to Tiger Compiler

You can post questions, issues, feedback, or even MR proposals through (our main GitLab repository)[https://ipads.se.sjtu.edu.cn:2020/compilers-2021/compilers-2021/issues]. We are rapidly refactoring the original C tiger compiler implementation into modern C++ style, so any suggestion to make this lab better is welcomed.
