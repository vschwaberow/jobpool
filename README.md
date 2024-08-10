# Jobpool C++ Library

This project demonstrates a multi-threaded job pool implementation that can handle various complex tasks,
including DNS resolution. It showcases the use of modern C++ features, asynchronous programming,
and efficient thread management.

I wrote it for educational purposes to help others learn about multi-threading, asynchronous programming, but
also to provide a useful library that can be used in other projects and in my own projects.

## Features

- **Job Pool**: A thread-safe, multi-threaded job queue system
- **DNS Resolution**: Asynchronous DNS resolution using Boost.Asio
- **Examples**: Implementation of Fibonacci sequence, prime factorization, and Collatz conjecture calculations
- **Logging**: Comprehensive logging using spdlog

## Components

- `job_pool.h` and `job_pool.cc`: Implementation of the thread pool
- `dns_resolver.h`: Asynchronous DNS resolver using Boost.Asio
- `complex_tasks_with_dns.cc`: Main application demonstrating the usage of the job pool and DNS resolver

## Requirements

- C++20 compatible compiler
- Boost libraries
- spdlog

## Building the Project

This project uses CMake as its build system. To build the project:

```bash
cmake -S . -B build
cmake --build build --config Release
```

After building, run the executable:

```bash
./build/complex_tasks_with_dns
```

## Tests

This project uses Google Test for unit testing. To run the tests:

```bash
./build/unit_tests
```

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.

## License
This project is open source and available under the MIT License.
