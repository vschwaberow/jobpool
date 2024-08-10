#pragma once

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

namespace thread_utils {

class JobPool {
 public:
  explicit JobPool(size_t num_threads);
  ~JobPool();

  JobPool(const JobPool&) = delete;
  JobPool& operator=(const JobPool&) = delete;
  JobPool(JobPool&&) = delete;
  JobPool& operator=(JobPool&&) = delete;

  template<typename F>
  requires std::invocable<F>
  void AddJob(F&& job) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      jobs_.emplace(std::forward<F>(job));
    }
    condition_.notify_one();
    spdlog::debug("Job added to the pool. Queue size: {}", jobs_.size());
  }

  template<typename F>
  requires std::invocable<F>
  void AddJobs(std::span<F> jobs) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      for (auto&& job : jobs) {
        jobs_.emplace(std::forward<F>(job));
      }
    }
    condition_.notify_all();
    spdlog::debug("{} jobs added to the pool. Queue size: {}", jobs.size(), jobs_.size());
  }

  void WaitForAllJobs();
  size_t GetQueueSize() const;
  size_t GetThreadCount() const { return threads_.size(); }
  size_t GetActiveJobCount() const { return active_jobs_.load(); }
  void Pause();
  void Resume();

 private:
  void WorkerThread();

  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> jobs_;
  mutable std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> stop_flag_{false};
  std::atomic<size_t> active_jobs_{0};
  std::atomic<bool> paused_{false};
  std::exception_ptr last_exception_;
};

}
