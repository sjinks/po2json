FROM alpine:3.19.1 AS build-alpine

RUN apk add --no-cache cmake make libc-dev clang17 gettext-static gettext-dev libunistring-static libunistring-dev

WORKDIR /
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-static -DENABLE_TESTING=Off -DEXTRA_LIBS="unistring;intl" && \
    cmake --build build && \
    strip --strip-unneeded build/src/po2json


FROM ubuntu:24.04 AS build-ubuntu

RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends cmake make clang libgettextpo-dev libunistring-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-static -DENABLE_TESTING=Off -DEXTRA_LIBS="unistring" && \
    cmake --build build && \
    strip --strip-unneeded build/src/po2json

FROM scratch AS release-alpine
COPY --from=build-alpine /app/build/src/po2json /po2json

FROM scratch AS release-ubuntu
COPY --from=build-ubuntu /app/build/src/po2json /po2json
