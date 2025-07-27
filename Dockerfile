FROM ubuntu:24.04

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install dependencies
RUN apt-get update && apt-get install -y \
    # Essential build tools
    build-essential \
    git \
    ninja-build \
    pkg-config \
    meson \
    flex \
    bison \
    # Core QEMU dependencies
    libglib2.0-dev \
    libfdt-dev \
    libpixman-1-dev \
    zlib1g-dev \
    # Useful for basic functionality
    libbz2-dev \
    libcap-ng-dev \
    libseccomp-dev \
    liblzo2-dev \
    # Debug tools (optional)
    gdb \
    gdb-multiarch \
    # Python for build scripts
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /qemu

# Copy the source code
COPY . .

# Create build directory and configure
RUN mkdir -p build && \
    cd build && \
    ../configure --enable-plugins

# Build qemu-system-arm and test-plugins
RUN cd build && \
    make qemu-system-arm && \
    make test-plugins

# Set the final working directory to build
WORKDIR /qemu/build

# Default command
CMD ["/bin/bash"]