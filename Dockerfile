# SSDB Dockerfile - Multi-stage build for minimal production image
# Build stage
FROM ubuntu:22.04 AS builder

# Set non-interactive frontend
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    autoconf \
    automake \
    libtool \
    make \
    gcc \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy source code (excludes .git, build artifacts via .dockerignore)
COPY . .

# Build SSDB
RUN ./build.sh && make -j$(nproc)

# Verify build
RUN test -f ssdb-server && test -f tools/ssdb-cli

# Runtime stage
FROM ubuntu:22.04 AS runtime

# Set non-interactive frontend
ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libgcc-s1 \
    libc6 \
    && rm -rf /var/lib/apt/lists/*

# Create ssdb user for security
RUN groupadd -r ssdb && useradd -r -g ssdb -d /var/lib/ssdb -s /bin/false ssdb

# Create directories
RUN mkdir -p /var/lib/ssdb /var/run/ssdb /etc/ssdb && \
    chown -R ssdb:ssdb /var/lib/ssdb /var/run/ssdb /etc/ssdb

# Copy built binaries from builder
COPY --from=builder /build/ssdb-server /usr/bin/ssdb-server
COPY --from=builder /build/tools/ssdb-cli /usr/bin/ssdb-cli
COPY --from=builder /build/tools/ssdb-bench /usr/bin/ssdb-bench
COPY --from=builder /build/tools/ssdb-dump /usr/bin/ssdb-dump
COPY --from=builder /build/tools/ssdb-repair /usr/bin/ssdb-repair
COPY --from=builder /build/tools/ssdb-migrate /usr/bin/ssdb-migrate

# Copy configuration
COPY ssdb.conf /etc/ssdb/ssdb.conf
COPY ssdb_slave.conf /etc/ssdb/ssdb_slave.conf

# Configure for container runtime
RUN sed -i \
    -e 's@^work_dir = .*@work_dir = /var/lib/ssdb@' \
    -e 's@^pidfile = .*@pidfile = /var/run/ssdb/ssdb.pid@' \
    -e 's@^loglevel.*@loglevel info@' \
    -e 's@^level:.*@level: info@' \
    -e 's@^ip:.*@ip: 0.0.0.0@' \
    /etc/ssdb/ssdb.conf

# Set timezone
ENV TZ=Asia/Shanghai

# Switch to non-root user
USER ssdb

# Expose port
EXPOSE 8888

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD ssdb-cli -p 8888 ping || exit 1

# Volume for persistent data
VOLUME /var/lib/ssdb

# Entry point
ENTRYPOINT ["ssdb-server", "/etc/ssdb/ssdb.conf"]