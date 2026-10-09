# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- GitHub Actions CI pipeline (`.github/workflows/ci.yml`) with build, lint, and test jobs
- Dependency check workflow (`.github/workflows/dep-check.yml`) for weekly vendor updates
- Project-level test suite in `tests/` directory with comprehensive integration tests
- clang-format configuration (`.clang-format`) for consistent code formatting
- clang-tidy configuration (`.clang-tidy`) for static analysis
- Docker Compose file (`docker-compose.yml`) for self-contained deployment
- Vendored dependency version tracking (`VERSIONS.md`)
- Environment variable reference (`.env.example`)
- Contributing guidelines (`CONTRIBUTING.md`)
- This changelog (`CHANGELOG.md`)
- Comprehensive README update with Quick Start, Docker, configuration, and security sections

### Changed
- Updated `Makefile` with `test` target
- Updated `tests/Makefile` for building and running test suite
- Modernized `Dockerfile` to be self-contained (no git clone at build time)

### Security
- Documented known CVEs in vendored dependencies (Jemalloc 4.1.0, Snappy 1.1.0)
- Added authentication and network security documentation
- Added automated dependency vulnerability scanning

## [1.9.2] - 2016-06-28

### Added
- Original SSDB release by @ideawu
- LevelDB-based key-value storage
- Hash, ZSet, and Queue data structures
- Master-slave replication
- Multi-language client support (C++, PHP, Python, Java, Go, Node.js, Ruby)
- Built-in CLI and admin tools

### Features
- High performance key-value storage
- Persistent collections (hash, zset, queue)
- Redis protocol compatibility
- Replication and load balancing
- TTL/expiration support
- Range queries and scanning

## [1.9.0] - 2015-XX-XX

### Added
- Initial public release
- Core KV engine with LevelDB backend
- Basic replication support

---

## Release Notes Template

### [Version] - YYYY-MM-DD

#### Added
- New features

#### Changed
- Changes in existing functionality

#### Deprecated
- Soon-to-be removed features

#### Removed
- Removed features

#### Fixed
- Bug fixes

#### Security
- Vulnerability fixes

---

## Version History

| Version | Date | Notes |
|---------|------|-------|
| Unreleased | - | Security hardening, CI/CD, testing, documentation |
| 1.9.2 | 2016-06-28 | Last original release by @ideawu |
| 1.9.0 | 2015 | Initial public release |

---

## Upgrade Guide

### From 1.9.x to 2.0.x (Future)

> **Note**: This is a placeholder for future major version upgrades.

When upgrading:
1. Backup your data directory
2. Review breaking changes in release notes
3. Test in staging environment first
4. Run `ssdb-repair` if needed after upgrade
5. Monitor logs for any issues

### Configuration Changes

Check `ssdb.conf` for new options after upgrading. The server will use defaults for missing options.

### Data Compatibility

SSDB maintains forward compatibility within major versions. Data files from 1.9.x should work with 1.9.y (y > x).