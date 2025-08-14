# syntax=docker/dockerfile:1
FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Install cross compilers, QEMU, ZeroMQ, SQLite, protoc, Python, Rust, and development tools
RUN dpkg --add-architecture arm64 && dpkg --add-architecture armhf && \
    apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
    qemu-user-static \
    libzmq3-dev \
    sqlite3 libsqlite3-dev \
    protobuf-compiler libprotobuf-dev \
    libprotobuf-dev:arm64 libprotobuf-dev:armhf \
    python3 python3-pip \
    pkg-config \
    git curl wget ccache \
    && rm -rf /var/lib/apt/lists/*

# Install Rust using rustup
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain stable \
    && /root/.cargo/bin/rustup target add aarch64-unknown-linux-gnu armv7-unknown-linux-gnueabihf
ENV PATH="/root/.cargo/bin:${PATH}"

# Default workdir
WORKDIR /workspace
