# The image to use while developing this project

FROM alpine:latest as base

RUN apk update
RUN apk add --no-cache build-base
RUN apk add --no-cache gdb
RUN apk add --no-cache cmake
RUN apk add --no-cache libstdc++
RUN apk add --no-cache git
RUN apk add --no-cache openssh
RUN apk update
RUN apk upgrade