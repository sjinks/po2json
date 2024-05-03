FROM --platform=${BUILDPLATFORM} tonistiigi/xx:latest@sha256:0cd3f05c72d6c9b038eb135f91376ee1169ef3a330d34e418e65e2a5c2e9c0d4 AS xx

FROM --platform=${BUILDPLATFORM} alpine:3.19.1@sha256:c5b1261d6d3e43071626931fc004f70149baeba2c8ec672bd4f27761f8e1ad6b AS build-alpine
ARG TARGETPLATFORM
COPY --from=xx / /

RUN \
    apk add --no-cache clang lld llvm make cmake && \
    xx-apk add --no-cache gcc musl-dev libstdc++-dev gettext-static gettext-dev libunistring-static libunistring-dev

WORKDIR /
COPY . /app
WORKDIR /app

RUN \
    set -x && \
    export ARCHITECTURE=$(xx-info march) && \
    if [ "$(xx-info alpine-arch)" = "ppc64le" ]; then export XX_CC_PREFER_LINKER=ld; fi && \
    export SYSROOT=$(xx-info sysroot) && \
    export HOSTSPEC=$(xx-info triple) && \
    xx-clang --setup-target-triple && \
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-static -DENABLE_TESTING=Off -DEXTRA_LIBS="unistring;intl" && \
    cmake --build build -j 2 && \
    ${HOSTSPEC}-strip build/src/po2json

FROM --platform=${BUILDPLATFORM} ubuntu:mantic-20240427@sha256:565d62d2283a7cc4b3d759d9a97a5bfcebeb341166f9076a4df504f8f106cd54 AS build-ubuntu
ARG TARGETPLATFORM
COPY --from=xx / /

RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends clang lld llvm make cmake && \
    xx-apt-get install -y --no-install-recommends libc6-dev libstdc++-13-dev libgettextpo-dev libunistring-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /
COPY . /app
WORKDIR /app

RUN \
    set -x && \
    export ARCHITECTURE=$(xx-info march) && \
    export SYSROOT=$(xx-info sysroot) && \
    export HOSTSPEC=$(xx-info triple) && \
    xx-clang --setup-target-triple && \
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-static -DENABLE_TESTING=Off -DEXTRA_LIBS="unistring" && \
    cmake --build build -j 2 && \
    ${HOSTSPEC}-strip build/src/po2json

FROM scratch AS release-alpine
COPY --from=build-alpine /app/build/src/po2json /po2json

FROM scratch AS release-ubuntu
COPY --from=build-ubuntu /app/build/src/po2json /po2json
