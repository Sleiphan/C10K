FROM ubuntu:latest

RUN apt update
RUN apt-get -y install gcc
RUN apt-get -y install g++
RUN apt-get -y install gdb
RUN apt-get -y install cmake
RUN apt-get -y install git
RUN apt update
RUN apt -y upgrade

WORKDIR ~/project

COPY * ~/project

# Copy ssh keys to container
# COPY /mnt/c/Users/hfred/.ssh/ /root/.ssh/
# RUN chmod 400 -R /root/.ssh

# The /app directory should act as the main application directory