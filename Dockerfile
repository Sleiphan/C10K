FROM alpine:latest as base



FROM base as build

RUN apk update
RUN apk add gcc
RUN apk add g++
RUN apk add make
RUN apk add cmake

# The /app directory should act as the main application directory
WORKDIR /app

COPY ./apps /app/apps
COPY ./src /app/src
COPY ./test /app/test
COPY ./CMakeLists.txt /app

# RUN cd /app
# RUN cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ -S/app -B/app/build
# RUN cmake --build /app/build --config Debug --target all

RUN cmake -B./build
RUN cmake --build /app/build --config Debug --target all


# FROM base as runtime

# COPY --from=build /app/build/Run_unix_srv /Run

# ENTRYPOINT [ "/Run" ]
ENTRYPOINT [ "/bin/sh" ]
