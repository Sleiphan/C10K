FROM alpine:latest as base

RUN apk update
RUN apk upgrade

COPY ./build/bin/Run_unix_srv /app/App

ENTRYPOINT [ "/app/App" ]