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
    zeromq libzmq3-dev \
    sqlite3 libsqlite3-dev \
    protobuf-compiler libprotobuf-dev \
    libprotobuf-dev:arm64 libprotobuf-dev:armhf \
    python3 python3-pip \
    pkg-config \
    git curl wget ccache \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages required for workers
RUN pip3 install --no-cache-dir pyzmq protobuf pytest

# Install Rust using rustup
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain stable \
    && /root/.cargo/bin/rustup target add aarch64-unknown-linux-gnu armv7-unknown-linux-gnueabihf \
    && /root/.cargo/bin/cargo install --locked cargo-local-registry
ENV PATH="/root/.cargo/bin:${PATH}"

# Default workdir
WORKDIR /workspace
