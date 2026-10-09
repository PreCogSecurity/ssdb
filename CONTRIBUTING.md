# Contributing to SSDB

Thank you for your interest in contributing to SSDB! This document provides guidelines for contributing to the project.

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Workflow](#development-workflow)
4. [Code Style](#code-style)
5. [Testing](#testing)
6. [Submitting Changes](#submitting-changes)
7. [Security Issues](#security-issues)
8. [License](#license)

## Code of Conduct

By participating in this project, you agree to abide by our Code of Conduct:
- Be respectful and inclusive
- Welcome newcomers and help them learn
- Focus on constructive criticism
- No harassment, discrimination, or offensive behavior

## Getting Started

### Prerequisites

- Linux, macOS, or BSD
- GCC 7+ or Clang 6+
- GNU Make
- autoconf, automake, libtool
- Git

### Setting Up Your Development Environment

```bash
# Fork the repository on GitHub
# Clone your fork
git clone https://github.com/YOUR_USERNAME/ssdb.git
cd ssdb

# Add upstream remote
git remote add upstream https://github.com/PreCogSecurity/ssdb.git

# Build the project
make

# Run tests to verify everything works
make test
```

## Development Workflow

### 1. Create a Feature Branch

```bash
# Sync with upstream
git fetch upstream
git checkout main
git merge upstream/main

# Create a new branch for your feature/fix
git checkout -b feature/your-feature-name
# or
git checkout -b fix/your-bug-fix
```

### 2. Make Your Changes

- Write clean, well-documented code
- Follow the existing code style (see [Code Style](#code-style))
- Add tests for new functionality
- Update documentation as needed

### 3. Test Your Changes

```bash
# Run the full test suite
make test

# Run clang-format check
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h

# Run clang-tidy static analysis
clang-tidy src/**/*.cpp -p=. --checks='bugprone-*,performance-*,modernize-*,readability-*'
```

### 4. Commit Your Changes

Write clear, atomic commits:

```bash
# Stage your changes
git add -p

# Commit with a descriptive message
git commit -m "feat: add support for XYZ command

- Implement XYZ command handler in proc_xyz.cpp
- Add protocol documentation
- Add test cases in tests/test_xyz.cpp"
```

**Commit Message Format:**
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `style:` - Code style changes (formatting, etc.)
- `refactor:` - Code refactoring
- `test:` - Test additions/changes
- `chore:` - Maintenance tasks

### 5. Push and Create a Pull Request

```bash
# Push to your fork
git push origin feature/your-feature-name

# Create a Pull Request on GitHub
# Target: PreCogSecurity/ssdb main branch
```

## Code Style

### C/C++ Formatting

We use **clang-format** with the configuration in [.clang-format](.clang-format).

Key style points:
- 4-space indentation (no tabs)
- Allman brace style
- 120 column line limit
- Pointer alignment: left (`int* ptr`)
- Space after control statement keywords (`if (`, `for (`)

```bash
# Check formatting
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h

# Auto-format (run before committing)
clang-format -i src/**/*.cpp src/**/*.h
```

### Static Analysis

We use **clang-tidy** with the configuration in [.clang-tidy](.clang-tidy).

```bash
# Run static analysis
clang-tidy src/**/*.cpp -p=. --checks='bugprone-*,performance-*,modernize-*,readability-*'
```

### C++ Standard

- Target: C++11
- Use modern C++ features where appropriate
- Prefer `std::string` over `char*`
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) for ownership
- Avoid raw `new`/`delete`

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `SSDBServer` |
| Functions/Methods | snake_case | `proc_get` |
| Variables | snake_case | `work_dir` |
| Constants | UPPER_SNAKE_CASE | `MAX_PACKET_SIZE` |
| Macros | UPPER_SNAKE_CASE | `CHECK_NUM_PARAMS` |
| Namespaces | lowercase | `ssdb` |

## Testing

### Test Structure

Tests are located in the `tests/` directory:
- `test_harness.h` - Test framework
- `test_ssdb.cpp` - Integration tests against running server

### Running Tests

```bash
# Build and run all tests
make test

# Run specific test (if supported by harness)
./tests/test_ssdb --filter=KV_BasicSetGet
```

### Writing Tests

1. Add test cases to `tests/test_ssdb.cpp`
2. Use the `TEST(name)` macro
3. Follow the existing patterns for client/server interaction
4. Clean up test data directories in test teardown

### Test Guidelines

- Tests should be independent and runnable in any order
- Use unique ports per test to allow parallel execution
- Clean up temporary data directories
- Test both success and error paths
- Aim for meaningful assertions, not just coverage

## Submitting Changes

### Pull Request Checklist

Before submitting a PR, ensure:

- [ ] Code compiles without warnings (`make`)
- [ ] All tests pass (`make test`)
- [ ] Code passes clang-format check
- [ ] Code passes clang-tidy analysis
- [ ] New functionality has tests
- [ ] Documentation is updated (README, comments, etc.)
- [ ] Commit messages follow the convention
- [ ] PR description explains the change and rationale

### PR Review Process

1. Automated CI checks must pass
2. At least one maintainer review required
3. Address review feedback promptly
4. Squash commits if requested
5. Merge after approval

### Breaking Changes

If your change breaks backward compatibility:
- Mark the PR with `[BREAKING]` in the title
- Document the migration path in the PR description
- Update version number appropriately

## Security Issues

**Do not file public issues for security vulnerabilities.**

Instead, email: **security@precogsecurity.com**

Include:
- Description of the vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if any)

We will acknowledge receipt within 48 hours and work with you on a fix and disclosure timeline.

## License

By contributing to SSDB, you agree that your contributions will be licensed under the [New BSD License](LICENSE).

---

## Additional Resources

- [README.md](README.md) - Project overview and quick start
- [VERSIONS.md](VERSIONS.md) - Vendored dependency versions
- [CHANGELOG.md](CHANGELOG.md) - Release history
- [.github/workflows/ci.yml](.github/workflows/ci.yml) - CI pipeline configuration
- [ssdb.conf](ssdb.conf) - Default configuration reference