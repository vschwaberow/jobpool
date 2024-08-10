#include "dns_resolver.h"
#include <gtest/gtest.h>
#include <future>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

class DnsResolverTest : public ::testing::Test {
protected:
    DnsResolver resolver;
    const std::vector<std::string> valid_hostnames = {
        "www.google.com",
        "www.github.com",
        "www.example.com",
        "www.openai.com"
    };
    const std::vector<std::string> invalid_hostnames = {
        "invalid.hostname.test",
        "nonexistent.domain.com",
        "*.invalidcharacter.com"
    };
};

TEST_F(DnsResolverTest, ResolvesValidHostname) {
    for (const auto& hostname : valid_hostnames) {
        auto future = resolver.resolve(hostname);
        auto result = future.get();
        EXPECT_FALSE(result.empty()) << "Failed for hostname: " << hostname;
        EXPECT_NE(result, "No results") << "Failed for hostname: " << hostname;
        EXPECT_NE(result.substr(0, 6), "Error:") << "Failed for hostname: " << hostname;
    }
}

TEST_F(DnsResolverTest, HandlesInvalidHostname) {
    for (const auto& hostname : invalid_hostnames) {
        auto future = resolver.resolve(hostname);
        auto result = future.get();
        EXPECT_TRUE(result.empty() || result == "No results" || result.substr(0, 6) == "Error:")
            << "Unexpected result for invalid hostname: " << hostname;
    }
}

TEST_F(DnsResolverTest, ResolvesConcurrently) {
    std::vector<std::future<std::string>> futures;
    for (const auto& hostname : valid_hostnames) {
        futures.push_back(resolver.resolve(hostname));
    }

    std::vector<std::string> results;
    for (auto& future : futures) {
        results.push_back(future.get());
    }

    EXPECT_EQ(results.size(), valid_hostnames.size());
    for (const auto& result : results) {
        EXPECT_FALSE(result.empty());
        EXPECT_NE(result, "No results");
        EXPECT_NE(result.substr(0, 6), "Error:");
    }
}

TEST_F(DnsResolverTest, HandlesSlowResolution) {
    // Use a list of hostnames that are likely to be slow or non-existent
    const std::vector<std::string> slow_hosts = {
        "very-slow-dns.example.com",
        "non-existent-domain-123456789.com",
        "this-domain-definitely-does-not-exist-123.org"
    };

    for (const auto& slow_host : slow_hosts) {
        auto start_time = std::chrono::steady_clock::now();
        auto future = resolver.resolve(slow_host);

        // Wait for a reasonable amount of time (e.g., 5 seconds)
        auto status = future.wait_for(std::chrono::seconds(5));

        std::string result;
        if (status == std::future_status::ready) {
            result = future.get();
        } else {
            EXPECT_EQ(status, std::future_status::timeout)
                << "DNS resolution did not complete or timeout as expected for: " << slow_host;
            // Force the future to complete
            result = future.get();
        }

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Check if the result indicates a failure or non-existent domain
        bool is_failure = result.empty() || result == "No results" || result.substr(0, 6) == "Error:";

        if (duration.count() < 100) {  // Less than 100 ms
            EXPECT_TRUE(is_failure)
                << "Fast resolution (" << duration.count()
                << "ms) should indicate a failure for: " << slow_host;
        } else {
            EXPECT_GE(duration.count(), 100)
                << "Resolution completed too quickly for a slow resolution test: "
                << slow_host;
        }

        if (!is_failure) {
            std::cout << "Unexpected successful resolution for " << slow_host
                      << ": " << result << " (took " << duration.count() << "ms)" << std::endl;
        }
    }
}

TEST_F(DnsResolverTest, HandlesNonRoutableIP) {
    const std::string non_routable_ip = "10.255.255.1";

    auto future = resolver.resolve(non_routable_ip);
    auto result = future.get();

    EXPECT_EQ(result, non_routable_ip)
        << "Expected non-routable IP to be returned as-is, got: " << result;
}

TEST_F(DnsResolverTest, ResolvesMultipleTimesConsistently) {
    const int num_attempts = 5;
    std::vector<std::string> results;

    for (int i = 0; i < num_attempts; ++i) {
        auto future = resolver.resolve("www.google.com");
        results.push_back(future.get());
    }

    EXPECT_EQ(results.size(), num_attempts);
    EXPECT_TRUE(std::all_of(results.begin(), results.end(),
        [&](const std::string& result) { return result == results[0]; }))
        << "Inconsistent results across multiple resolutions";
}

TEST_F(DnsResolverTest, HandlesEmptyHostname) {
    auto future = resolver.resolve("");
    auto result = future.get();
    EXPECT_TRUE(result.empty() || result == "No results" || result.substr(0, 6) == "Error:")
        << "Unexpected result for empty hostname: " << result;
}

TEST_F(DnsResolverTest, HandlesLongHostname) {
    std::string long_hostname(300, 'a');
    long_hostname += ".com";

    auto future = resolver.resolve(long_hostname);
    auto result = future.get();
    EXPECT_TRUE(result.empty() || result == "No results" || result.substr(0, 6) == "Error:")
        << "Unexpected result for long hostname: " << result;
}

TEST_F(DnsResolverTest, ResolvesIPAddress) {
    auto future = resolver.resolve("8.8.8.8");
    auto result = future.get();
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result, "No results");
    EXPECT_NE(result.substr(0, 6), "Error:");
    EXPECT_EQ(result, "8.8.8.8") << "IP address should resolve to itself";
}

TEST_F(DnsResolverTest, ResolvesManyNonExistentHostnamesConcurrently) {
    static const int num_hostnames = 100;
    std::vector<std::string> hostnames;
    std::vector<std::future<std::string>> futures;

    for (int i = 0; i < num_hostnames; ++i) {
        hostnames.push_back("nonexistent-test-" + std::to_string(i) + ".example.invalid");
    }

    auto start_time = std::chrono::steady_clock::now();
    for (const auto& hostname : hostnames) {
        futures.push_back(resolver.resolve(hostname));
    }

    std::vector<std::string> results;
    int failed_resolutions = 0;

    for (auto& future : futures) {
        try {
            auto result = future.get();
            results.push_back(result);
            if (result.empty() || result == "No results" || result.substr(0, 6) == "Error:") {
                failed_resolutions++;
            }
        } catch (const std::exception& e) {
            failed_resolutions++;
            std::cerr << "Exception caught: " << e.what() << std::endl;
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Attempted to resolve " << num_hostnames << " non-existent hostnames in " << duration.count() << " ms" << std::endl;
    std::cout << "Failed resolutions (as expected): " << failed_resolutions << std::endl;

    EXPECT_EQ(results.size(), num_hostnames) << "Not all hostnames were processed";
    EXPECT_EQ(failed_resolutions, num_hostnames) << "Expected all resolutions to fail";

    for (const auto& result : results) {
        EXPECT_TRUE(result.empty() || result == "No results" || result.substr(0, 6) == "Error:")
            << "Expected all results to indicate failure";
    }

    const int max_expected_duration_ms = 30000; // 30 seconds
    EXPECT_LE(duration.count(), max_expected_duration_ms)
        << "DNS resolution took longer than expected";
}
