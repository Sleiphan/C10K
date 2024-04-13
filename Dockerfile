FROM alpine:latest as base

FROM base as build

RUN apk update
RUN apk add --no-cache build-base
RUN apk add --no-cache cmake
RUN apk update
RUN apk upgrade

WORKDIR /app
COPY . /app

RUN cmake -DCMAKE_BUILD_TYPE=Release -S. -B./build
RUN cmake --build ./build --parallel 8



# Run tests
FROM build as test
# WORKDIR /app
RUN ctest --test-dir ./build



# # Create deploy image
FROM base as deploy

RUN apk update
RUN apk add --no-cache libstdc++
RUN apk update
RUN apk upgrade

WORKDIR /app

COPY --from=build /app/build/apps/Run_unix_srv ./Run

ENTRYPOINT [ "/app/Run" ]