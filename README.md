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

## Example

The `complex_tasks_with_dns` application demonstrates the usage of the job pool and DNS resolver.
It calculates the Fibonacci sequence, prime factorization, and Collatz conjecture for a given number.
It also resolves the IP address of a given domain name.

Here is an example with ```spdlog``` how to use the domain name resolver:

```cpp
#include "job_pool.h"
#include "dns_resolver.h"

std::vector<std::string> hostnames = {
    "www.google.com", "www.github.com", "www.stackoverflow.com",
    "www.wikipedia.org", "www.reddit.com"
};
std::vector<std::future<std::string>> dns_results(hostnames.size());
DnsResolver resolver;

for (size_t i = 0; i < hostnames.size(); ++i) {
    job_pool.AddJob([&resolver, &dns_results, &hostnames, i]() {
        spdlog::info("Starting DNS resolution for {}", hostnames[i]);
        dns_results[i] = resolver.resolve(hostnames[i]);
    });
}

job_pool.Wait();

for (size_t i = 0; i < hostnames.size(); ++i) {
    spdlog::info("Resolved IP address for {}: {}", hostnames[i], dns_results[i].get());
}
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
