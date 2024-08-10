#include "job_pool.h"
#include "dns_resolver.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <ranges>
#include <cmath>
#include <string>
#include <sstream>
#include <future>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

uint64_t fibonacci(int n) {
    if (n <= 1) return n;
    uint64_t a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        uint64_t temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

std::vector<uint64_t> prime_factorization(uint64_t n) {
    std::vector<uint64_t> factors;
    for (uint64_t i = 2; i * i <= n; ++i) {
        while (n % i == 0) {
            factors.push_back(i);
            n /= i;
        }
    }
    if (n > 1) factors.push_back(n);
    return factors;
}

int collatz_steps(uint64_t n) {
    int steps = 0;
    while (n != 1) {
        n = (n % 2 == 0) ? (n / 2) : (3 * n + 1);
        ++steps;
    }
    return steps;
}

int main() {
    // Setup logging
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::info);

    constexpr int num_threads = 8;
    thread_utils::JobPool job_pool(num_threads);

    spdlog::info("Starting complex tasks with {} threads", num_threads);

    // Fibonacci calculations
    constexpr int fib_count = 20;
    std::vector<uint64_t> fib_results(fib_count);

    for (int i : std::views::iota(0, fib_count)) {
        job_pool.AddJob([i, &fib_results]() {
            fib_results[i] = fibonacci(i + 30);  // Calculate Fibonacci(30) to Fibonacci(49)
            spdlog::debug("Calculated Fibonacci({}) = {}", i + 30, fib_results[i]);
        });
    }

    // Prime factorization
    constexpr int prime_fact_count = 10;
    std::vector<uint64_t> prime_fact_inputs = {
        1000000007, 999999937, 999999929, 999999893, 999999797,
        999999761, 999999757, 999999751, 999999739, 999999733
    };
    std::vector<std::vector<uint64_t>> prime_fact_results(prime_fact_count);

    for (int i = 0; i < prime_fact_count; ++i) {
        job_pool.AddJob([i, &prime_fact_inputs, &prime_fact_results]() {
            prime_fact_results[i] = prime_factorization(prime_fact_inputs[i]);
            spdlog::debug("Calculated prime factorization of {}", prime_fact_inputs[i]);
        });
    }

    static constexpr int collatz_count = 15;
    std::vector<uint64_t> collatz_inputs = {
        27, 31, 41, 47, 54, 73, 97, 129, 171, 231, 313, 327, 649, 871, 1161
    };
    std::vector<int> collatz_results(collatz_count);

    for (int i = 0; i < collatz_count; ++i) {
        job_pool.AddJob([i, &collatz_inputs, &collatz_results]() {
            collatz_results[i] = collatz_steps(collatz_inputs[i]);
            spdlog::debug("Calculated Collatz steps for {} = {}", collatz_inputs[i], collatz_results[i]);
        });
    }

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

    job_pool.WaitForAllJobs();
    spdlog::info("All jobs completed");

    spdlog::info("Fibonacci Results:");
    for (int i : std::views::iota(0, fib_count)) {
        spdlog::info("Fibonacci({}) = {}", i + 30, fib_results[i]);
    }

    spdlog::info("Prime Factorization Results:");
    for (int i = 0; i < prime_fact_count; ++i) {
        std::ostringstream oss;
        for (const auto& factor : prime_fact_results[i]) {
            oss << factor << " ";
        }
        spdlog::info("Factors of {} = {}", prime_fact_inputs[i], oss.str());
    }

    spdlog::info("Collatz Conjecture Results:");
    for (int i = 0; i < collatz_count; ++i) {
        spdlog::info("Collatz steps for {} = {}", collatz_inputs[i], collatz_results[i]);
    }

    spdlog::info("DNS Resolution Results:");
    for (size_t i = 0; i < hostnames.size(); ++i) {
        spdlog::info("{}: {}", hostnames[i], dns_results[i].get());
    }

    return 0;
}
