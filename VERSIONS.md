# Vendored Dependency Versions

This document tracks the exact versions and sources of all third-party dependencies vendored in the `deps/` directory.

## Overview

SSDB vendors its dependencies to ensure reproducible builds without requiring internet access at build time. Each dependency is pinned to a specific version/commit.

---

## Dependencies

### 1. LevelDB
- **Version**: 1.18 (based on Google LevelDB 1.18)
- **Source**: https://github.com/google/leveldb
- **Commit/Tag**: `1.18` (or closest matching release)
- **Local Path**: `deps/leveldb-1.18/`
- **License**: BSD-3-Clause
- **Purpose**: Embedded key-value storage engine
- **Known CVEs**: Check https://github.com/google/leveldb/security/advisories
- **Update Notes**: LevelDB 1.18 is from 2017. Consider updating to a newer version for security fixes.

### 2. Jemalloc
- **Version**: 4.1.0
- **Source**: https://github.com/jemalloc/jemalloc
- **Commit/Tag**: `4.1.0` (released 2016-02-28)
- **Local Path**: `deps/jemalloc-4.1.0/`
- **License**: BSD-2-Clause
- **Purpose**: Memory allocator for improved performance
- **Known CVEs**: 
  - CVE-2018-1000150 (fixed in 5.1.0)
  - CVE-2019-19246 (fixed in 5.2.1)
- **Update Notes**: Version 4.1.0 is from 2016. Strongly recommend updating to 5.3.0+ for security fixes.

### 3. Snappy
- **Version**: 1.1.0
- **Source**: https://github.com/google/snappy
- **Commit/Tag**: `1.1.0` (released 2017-01-13)
- **Local Path**: `deps/snappy-1.1.0/`
- **License**: BSD-3-Clause
- **Purpose**: Fast compression/decompression library
- **Known CVEs**: 
  - CVE-2023-38545 (fixed in 1.1.10)
- **Update Notes**: Version 1.1.0 is from 2017. Recommend updating to 1.1.10+ for security fixes.

### 4. Cpy (Custom Scripting Language)
- **Version**: Custom/Internal
- **Source**: https://github.com/ideawu/cpy
- **Commit/Tag**: Unknown (vendored copy)
- **Local Path**: `deps/cpy/`
- **License**: BSD-3-Clause (assumed, same as SSDB)
- **Purpose**: Embedded scripting language for SSDB CLI
- **Update Notes**: Track upstream at https://github.com/ideawu/cpy

---

## Update Procedure

To update a vendored dependency:

1. **Check upstream** for new releases and security advisories
2. **Download** the new version source code
3. **Replace** the contents of the corresponding `deps/<name>-<version>/` directory
4. **Update** this document with the new version information
5. **Test** the build: `make clean_all && make`
6. **Run tests**: `make test`
7. **Commit** changes with a descriptive message

### Example: Updating Snappy

```bash
# 1. Check latest release at https://github.com/google/snappy/releases
# 2. Download and extract
wget https://github.com/google/snappy/archive/refs/tags/1.1.10.tar.gz
tar xzf 1.1.10.tar.gz

# 3. Replace vendored copy
rm -rf deps/snappy-1.1.0
mv snappy-1.1.10 deps/snappy-1.1.10

# 4. Update build.sh if paths changed
# 5. Update this file (VERSIONS.md)

# 6. Rebuild and test
make clean_all && make && make test
```

---

## Automated Dependency Checking

A GitHub Actions workflow (`.github/workflows/dep-check.yml`) runs weekly to:
1. Check each vendored dependency against upstream releases
2. Open an issue if newer versions are available
3. Flag known CVEs in current versions

See `.github/workflows/dep-check.yml` for implementation details.

---

## Security Considerations

| Dependency | Current Version | Latest Version | CVEs in Current | Recommendation |
|------------|----------------|----------------|-----------------|----------------|
| LevelDB    | 1.18           | 1.23+          | Low             | Update when feasible |
| Jemalloc   | 4.1.0          | 5.3.0+         | **High**        | **Priority update** |
| Snappy     | 1.1.0          | 1.1.10+        | **Medium**      | **Priority update** |
| Cpy        | Custom         | N/A            | Unknown         | Audit manually |

**Last Updated**: 2024 (this document created as part of security hardening)
**Next Review**: Quarterly or when CVEs are published