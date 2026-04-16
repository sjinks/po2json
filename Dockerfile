FROM --platform=${BUILDPLATFORM} tonistiigi/xx:master@sha256:7e8c9d88e47741bb0d35981494920eee2857cd75a40193567fe6c1a16adf785d AS xx

FROM --platform=${BUILDPLATFORM} alpine:3.23.4@sha256:5b10f432ef3da1b8d4c7eb6c487f2f5a8f096bc91145e68878dd4a5019afde11 AS build-alpine
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

FROM --platform=${BUILDPLATFORM} ubuntu:24.04@sha256:d1e2e92c075e5ca139d51a140fff46f84315c0fdce203eab2807c7e495eff4f9 AS build-ubuntu
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
