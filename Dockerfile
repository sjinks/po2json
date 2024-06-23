FROM --platform=${BUILDPLATFORM} tonistiigi/xx:master@sha256:838660fc0ee1afbc837e4dd32b2a8ccd116072f67ab31a1ca1b33355b879d758 AS xx

FROM --platform=${BUILDPLATFORM} alpine:3.20.1@sha256:b89d9c93e9ed3597455c90a0b88a8bbb5cb7188438f70953fede212a0c4394e0 AS build-alpine
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

FROM --platform=${BUILDPLATFORM} ubuntu:24.04@sha256:2e863c44b718727c860746568e1d54afd13b2fa71b160f5cd9058fc436217b30 AS build-ubuntu
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
