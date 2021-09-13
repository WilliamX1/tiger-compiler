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

We rewrote the Tiger Compiler labs using C++ programming language. This is because C++ has some features like inheritance and polymorphism, which we think is more suitable for these labs and less error-prone.

We provide you all the codes of all labs in one time. In each lab, you only need to code in some of the directories.

The Tiger Compiler Labs are not perfect, but we have tried our best and spent a lot of time on it. If you find any bugs or have a better design, please inform us.

## Difference Between C Labs and C++ Labs

1. This new labs use [flexc++](https://fbb-git.gitlab.io/flexcpp/manual/flexc++.html) and [bisonc++](https://fbb-git.gitlab.io/bisoncpp/manual/bisonc++.html) instead of flex and bison because flexc++ and bisonc++ is more flexc++ and bisonc++ are able to generate pure C++ codes instead of C codes wrapped in C++ files.

2. The new labs use namespace for modularization and use inheritance and polymorphism to replace unions used in the old labs.

3. This new labs use CMake instead of Makefile to compile and build the target.

## Getting Newly Released Labs

1. The first thing you have to do is to clone the current lab repository by issuing the following commands on the command line:

   ```bash
   git clone https://ipads.se.sjtu.edu.cn:1312/lab/tiger-compiler-2019-fall.git
   ```

2. Once a lab is released, pull in the changes from your simpledb directory:

   ```bash
   git pull origin https://ipads.se.sjtu.edu.cn:1312/lab/tiger-compiler-2019-fall.git master
   ```

   And then check the content of `VERSION` file and make sure that the lab environment in your local computer is new enough.

**Notice:** You may need to do some code merging work.

## Installing Dependencies

flexc++ and bisonc++ will be needed in lab2 and later.
Although these libraries are not needed in lab1, you have to install them before you start lab1.

**Notice:**: Now we only support the following version of flexc++ and bisonc++:

```bash
flexc++ - V2.06.02
bisonc++ - V6.01.00
```

### Ubuntu18.04

```bash
sudo apt install git tar cmake g++ gcc gdb flexc++ bisonc++
```

**Notice:** This series of labs now only support Ubuntu18.04. For those who use **Ubuntu16.04**, you still need to use Docker to build and run your labs.

### MacOS, Windows Professional, Ubuntu16.04, and other Linux Distributions

We provide you a Docker image which has already installed all the dependencies. You can compile your codes directly in this Docker image.

1. Install [Docker](https://docs.docker.com/).

2. Build the docker image using the Dockerfile that we provide.

    ```bash
    cd tiger-compiler-2019-fall
    docker build -t se302/tigerlabs_env .
    ```

3. Run a docker container and mount the lab directory on it. You should replace `/path/to/tiger-compiler-2019-fall` to the path where you clone the lab repository on your computer.

    ```bash
    docker run -it --privileged -v /path/to/tiger-compiler-2019-fall:/home/stu/tiger-compiler-2019-fall se302/tigerlabs_env:latest /bin/bash
    cd tiger-compiler-2019-fall
    ```

**Notice:** Please do not use Docker Toolbox. For those who Windows Home, you still use virtual machine instead and install a Ubuntu18.04 image.

## Compiling

There are six makeable targets in total, including `test_slp`, `test_lex`, `test_parse`, `test_semant`, `test_translate`, and `tiger-compiler`.

```bash
mkdir build
cd build
cmake ..
make test_xxx  # e.g. `make test_slp`
```

## Debugging

```bash
# in build direcotry
cmake -DCMAKE_BUILD_TYPE=Debug ..
make test_xxx # e.g. `make test_slp`
gdb test_xxx # e.g. `gdb test_slp`
```

## Grading Your Labs

```bash
# cd to the lab root directory, i.e. tiger-compiler-2019-fall
./gradelabx.sh # e.g. `./gradelab1.sh`
```

## Submitting Your Labs

1. First commit your codes.

   ```bash
   git add somefiles
   git commit -m "A message!"
   ```

2. Pack your codes, rename the packed file, and upload it!

   ```bash
   ./handin.sh
   ```

## FAQ

1. Some students who use Windows OS may get the following output when they run the grading scripts in their docker containers. This is because Git For Windows automatically converts the LF (Unix newline)to CRLF(Windows newline) for all the files that you clone or commit and Linux bash won't execute the scripts that use CRLF newlines.

    ```bash
    bash: ./gradelab1.sh: /bin/bash^M: bad interpreter: No such file or directory
    ```

    ***Solution:*** Add some configs to the `.gitconfig` file in your user directory as below. For example, if your user name is foo, your user directory should be `C:\Users\foo`. For more information, you can refer to this [website](https://github.com/cssmagic/blog/issues/22).

    ```txt
    [core]
        autocrlf = false
        safecrlf = true
    ```

2. How can I get the root privilege in the docker container?

   ***Solution:*** Modify docker file and rebuild the docker image.

   ```dockerfile
   FROM ubuntu:18.04

   # Use aliyun registry
   RUN  sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
   RUN  apt-get clean

   RUN apt-get update && apt-get install -y git cmake g++ gcc vim tar gdb flexc++ bisonc++

   WORKDIR /root
   ```

   And run the command:

   ```bash
   docker run -it --privileged -v /path/to/tiger-compiler-2019-fall:/root/tiger-compiler-2019-fall se302/tigerlabs_env:latest /bin/bash
   cd tiger-compiler-2019-fall
   ```
