FROM ubuntu:18.04

# Use aliyun registry
RUN sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
RUN apt clean
RUN apt update && apt install -y sudo git cmake g++ gcc vim tar gdb flexc++ bisonc++ openssh-server rsync python3.8 python3-pip dos2unix clang-format-10
RUN useradd -ms /bin/bash -G sudo stu
RUN passwd -d stu
RUN ln -sf /usr/bin/clang-format-10 /usr/bin/clang-format

USER stu
WORKDIR /home/stu
CMD sudo service ssh restart && sudo usermod --password $(echo 123 | openssl passwd -1 -stdin) stu && /bin/bash

