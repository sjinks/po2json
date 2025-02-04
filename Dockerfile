FROM --platform=${BUILDPLATFORM} tonistiigi/xx:master@sha256:d126dfa8de1813964bd641a41c557d055aecabd49d6d3ff299ab895551cf00bf AS xx

FROM --platform=${BUILDPLATFORM} alpine:3.21.2@sha256:56fa17d2a7e7f168a043a2712e63aed1f8543aeafdcee47c58dcffe38ed51099 AS build-alpine
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
    if [ "${ARCHITECTURE}" = "ppc64le" ]; then export XX_CC_PREFER_LINKER=ld; fi && \
    export SYSROOT=$(xx-info sysroot) && \
    export HOSTSPEC=$(xx-info triple) && \
    xx-clang --setup-target-triple && \
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-static -DENABLE_TESTING=Off -DEXTRA_LIBS="unistring;intl" && \
    cmake --build build -j 2 && \
    ${HOSTSPEC}-strip build/src/po2json

FROM --platform=${BUILDPLATFORM} ubuntu:24.04@sha256:72297848456d5d37d1262630108ab308d3e9ec7ed1c3286a32fe09856619a782 AS build-ubuntu
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
    if [ "${ARCHITECTURE}" = "ppc64le" ]; then export XX_CC_PREFER_LINKER=ld; fi && \
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
