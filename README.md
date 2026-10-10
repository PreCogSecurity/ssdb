# SSDB - A fast NoSQL database for storing big list of data

[![Author](https://img.shields.io/badge/author-@ideawu-blue.svg?style=flat)](http://www.ideawu.net/) [![Platform](https://img.shields.io/badge/platform-Linux,%20BSD,%20OS%20X,%20Windows-green.svg?style=flat)](https://github.com/ideawu/ssdb) [![NoSQL](https://img.shields.io/badge/db-NoSQL-pink.svg?style=flat)](https://github.com/ideawu/ssdb) [![License](https://img.shields.io/badge/license-New%20BSD-yellow.svg?style=flat)](LICENSE) [![Build Status](https://github.com/PreCogSecurity/ssdb/actions/workflows/ci.yml/badge.svg)](https://github.com/PreCogSecurity/ssdb/actions/workflows/ci.yml)

SSDB is a high performance key-value(key-string, key-zset, key-hashmap) NoSQL database, **an alternative to Redis**.

SSDB is stable, production-ready and is widely used by many Internet companies including QIHU 360.

## Features

* LevelDB client-server support, written in C/C++
* Designed to store collection data
* Persistent key-value, key-zset, key-map('hashmap') storage
* Redis clients are supported
* Client API supports including C++, PHP, Python, Cpy, Java, nodejs, Ruby, Go ([see all](http://ssdb.io/docs/clients.html))
* Persistent queue service
* **Replication(master-slave), load balance**
* GUI administration tool ([phpssdbadmin](https://github.com/ssdb/phpssdbadmin))
* Built-in CLI nagios self-checks

## Quick Start

### Using Docker Compose (Recommended)

The fastest way to get SSDB running:

```bash
# Clone the repository
git clone https://github.com/PreCogSecurity/ssdb.git
cd ssdb

# Start SSDB with Docker Compose
docker compose up --build -d

# Verify it's running
docker compose exec ssdb ssdb-cli -p 8888 ping
# Should output: pong

# Test basic operations
docker compose exec ssdb ssdb-cli -p 8888 set mykey "hello world"
docker compose exec ssdb ssdb-cli -p 8888 get mykey

# Stop when done
docker compose down
```

This starts:
- **ssdb** (master) on port 8888
- **ssdb-slave** (replica) on port 8889
- Persistent data volumes for both

### Using Docker (Single Container)

```bash
# Build the image
docker build -t ssdb .

# Run the container
docker run -d \
  --name ssdb \
  -p 8888:8888 \
  -v ssdb_data:/var/lib/ssdb \
  ssdb

# Test
docker exec ssdb ssdb-cli -p 8888 ping
```

### Building from Source

#### Prerequisites

- Linux, macOS, or BSD
- GCC 7+ or Clang 6+
- GNU Make
- autoconf, automake, libtool

On Ubuntu/Debian:
```bash
sudo apt-get update && sudo apt-get install -y build-essential autoconf automake libtool
```

On macOS:
```bash
brew install autoconf automake libtool
```

#### Build Steps

```bash
# Clone and build
git clone https://github.com/PreCogSecurity/ssdb.git
cd ssdb
make

# Run tests
make test

# Optional: Install system-wide
sudo make install
```

#### Start the Server

```bash
# Start in foreground
./ssdb-server ssdb.conf

# Start as daemon
./ssdb-server -d ssdb.conf

# Stop the server
./ssdb-server ssdb.conf -s stop
```

#### Use the CLI

```bash
# Connect to local server
./tools/ssdb-cli -p 8888

# Run a command directly
./tools/ssdb-cli -p 8888 set key value
./tools/ssdb-cli -p 8888 get key
```

## Configuration

SSDB is configured via `ssdb.conf` (see [ssdb.conf](ssdb.conf) for the default configuration).

### Key Configuration Options

| Option | Description | Default |
|--------|-------------|---------|
| `work_dir` | Data directory (relative to config file) | `./var` |
| `server.ip` | Bind IP address | `127.0.0.1` |
| `server.port` | Listen port | `8888` |
| `server.auth` | Authentication password (min 32 chars) | *disabled* |
| `replication.binlog` | Enable binlog for replication | `yes` |
| `logger.level` | Log level (debug/info/warn/error) | `debug` |
| `logger.output` | Log file path | `log.txt` |
| `leveldb.cache_size` | LevelDB cache size in MB | `500` |
| `leveldb.compression` | Enable compression | `yes` |

### Environment Variables

The following environment variables can override configuration:

| Variable | Config Equivalent | Description |
|----------|-------------------|-------------|
| `SSDB_WORK_DIR` | `work_dir` | Data directory |
| `SSDB_IP` | `server.ip` | Bind IP address |
| `SSDB_PORT` | `server.port` | Listen port |
| `SSDB_AUTH` | `server.auth` | Authentication password |
| `SSDB_LOG_LEVEL` | `logger.level` | Log level |
| `SSDB_CACHE_SIZE` | `leveldb.cache_size` | Cache size in MB |

Example:
```bash
SSDB_PORT=9999 SSDB_AUTH="very-strong-password-at-least-32-chars" ./ssdb-server ssdb.conf
```

See [.env.example](.env.example) for a complete reference.

## Client APIs

SSDB supports multiple client languages:

| Language | Library | Documentation |
|----------|---------|---------------|
| C++ | Built-in (`src/client/`) | [API](api/cpp/README.md) |
| Python | `api/python/SSDB.py` | [Docs](http://ssdb.io/docs/python/) |
| PHP | `api/php/SSDB.php` | [Docs](http://ssdb.io/docs/php/) |
| Java | `api/java/` | [Docs](http://ssdb.io/docs/java/) |
| Go | Community | [github.com/ssdb/gossdb](https://github.com/ssdb/gossdb) |
| Node.js | Community | [npm ssdb](https://www.npmjs.com/package/ssdb) |
| Ruby | Community | [ruby-ssdb](https://github.com/ssdb/ruby-ssdb) |

### C++ Client Example

```cpp
#include "SSDB_client.h"

using namespace ssdb;

int main() {
    Client* client = Client::connect("127.0.0.1", 8888);
    
    // Set a key
    Status s = client->set("key", "value");
    if (s.ok()) {
        std::cout << "SET OK" << std::endl;
    }
    
    // Get a key
    std::string val;
    s = client->get("key", &val);
    if (s.ok()) {
        std::cout << "GET: " << val << std::endl;
    }
    
    delete client;
    return 0;
}
```

## Replication

SSDB supports master-slave replication for high availability.

### Configure Master

In `ssdb.conf`:
```conf
replication:
    binlog: yes
    sync_speed: -1  # -1 = no limit
```

### Configure Slave

In `ssdb_slave.conf`:
```conf
replication:
    binlog: yes
    slaveof:
        id: master_1
        host: 127.0.0.1
        port: 8888
        type: sync  # or mirror
```

Start both servers:
```bash
# Terminal 1: Master
./ssdb-server ssdb.conf

# Terminal 2: Slave
./ssdb-server ssdb_slave.conf
```

## Testing

Run the test suite:

```bash
# Build and run all tests
make test
```

The test suite covers:
- Basic KV operations (set, get, del, incr, decr)
- Hash operations (hset, hget, hdel, hscan)
- Sorted set operations (zset, zget, zdel, zrange)
- Queue operations (qpush, qpop, qfront, qback)
- Scan operations (scan, rscan, keys)
- TTL/expiration (setx, ttl, expire)
- Multi-key operations (multi_set, multi_get, multi_del)
- Bit operations (setbit, getbit, bitcount)
- Error handling

## Development

### Code Style

We use clang-format and clang-tidy for code formatting and static analysis.

```bash
# Check formatting
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h

# Run static analysis
clang-tidy src/**/*.cpp -p=. --checks='bugprone-*,performance-*,modernize-*,readability-*'
```

### CI Pipeline

The CI pipeline runs on every push and PR:
1. Build on Ubuntu and macOS
2. Run clang-format check
3. Run clang-tidy static analysis
4. Run unit tests

See [.github/workflows/ci.yml](.github/workflows/ci.yml) for details.

### Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Reporting Issues

- **Bug reports**: Use the GitHub issue tracker with the "bug" label
- **Security issues**: Email security@precogsecurity.com (do not file public issues)
- **Feature requests**: Use the GitHub issue tracker with the "enhancement" label

## Security

### Authentication

SSDB supports password authentication. Set a strong password (minimum 32 characters) in `ssdb.conf`:

```conf
server:
    auth: very-strong-password-at-least-32-characters-long
```

### Network Security

- Bind to `127.0.0.1` for local-only access
- Use `ip_filter` to restrict access:
  ```conf
  server:
      deny: all
      allow: 10.0.0.0/8
      allow: 192.168.0.0/16
  ```
- Run behind a firewall
- Use TLS termination proxy (nginx, haproxy) for encrypted connections

### Known Security Considerations

- Vendored dependencies (LevelDB, Jemalloc, Snappy) may have known CVEs
- See [VERSIONS.md](VERSIONS.md) for current versions and security status
- Automated dependency checks run weekly via GitHub Actions

## Performance Tuning

### LevelDB Options

```conf
leveldb:
    cache_size: 500        # MB, increase for larger datasets
    block_size: 32         # KB
    write_buffer_size: 64  # MB
    compaction_speed: 1000 # MB/s, -1 for no limit
    compression: yes       # snappy compression
```

### System Settings

```bash
# Increase file descriptor limits
ulimit -n 65535

# Disable transparent huge pages
echo never > /sys/kernel/mm/transparent_hugepage/enabled

# Set swappiness low
echo 1 > /proc/sys/vm/swappiness
```

## Monitoring

SSDB provides built-in monitoring:

```bash
# Server info
./tools/ssdb-cli -p 8888 info

# Replication status
./tools/ssdb-cli -p 8888 sync140

# Slow log
./tools/ssdb-cli -p 8888 slowlog get
```

Nagios-compatible health checks:
```bash
./tools/ssdb-cli -p 8888 nagios_check
```

## Documentation

* [Online Documentation](http://ssdb.io/docs/)
* [API Reference](http://ssdb.io/docs/api.html)
* [Architecture](http://ssdb.io/ssdb.png)
* [Compile and Install Wiki](http://ssdb.io/docs/install.html)
* [Changes Made to LevelDB](https://github.com/ideawu/ssdb/wiki/Changes-Made-to-LevelDB)

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for release history.

## License

SSDB is licensed under the [New BSD License](LICENSE).

## Authors

* @ideawu (original author)
* PreCog Security (maintenance and security hardening)

## Thanks

* 刘建辉, liujianhui@gongchang.com
* wendal(陈镇铖), wendal1985@gmail.com, http://wendal.net
* All contributors and users of SSDB