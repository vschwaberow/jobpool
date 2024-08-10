#include "job_pool.h"

#include <stdexcept>

namespace thread_utils {

JobPool::JobPool(size_t num_threads) : threads_(num_threads) {
  for (auto& thread : threads_) {
    thread = std::thread(&JobPool::WorkerThread, this);
  }
}

JobPool::~JobPool() {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    stop_flag_.store(true);
  }
  condition_.notify_all();
  for (auto& thread : threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void JobPool::WaitForAllJobs() {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  condition_.wait(lock, [this] {
    return jobs_.empty() && active_jobs_.load() == 0;
  });
  if (last_exception_) {
    std::rethrow_exception(last_exception_);
  }
}

size_t JobPool::GetQueueSize() const {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  return jobs_.size();
}

void JobPool::Pause() {
  paused_.store(true);
}

void JobPool::Resume() {
  paused_.store(false);
  condition_.notify_all();
}

void JobPool::WorkerThread() {
  while (!stop_flag_.load()) {
    std::function<void()> job;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      condition_.wait(lock, [this] {
        return stop_flag_.load() || !jobs_.empty() || !paused_.load();
      });
      if (stop_flag_.load()) return;
      if (paused_.load()) continue;
      if (jobs_.empty()) continue;
      job = std::move(jobs_.front());
      jobs_.pop();
    }
    active_jobs_++;
    try {
      job();
    } catch (...) {
      last_exception_ = std::current_exception();
      spdlog::error("Exception caught in worker thread");
    }
    active_jobs_--;
    condition_.notify_all();
  }
}

}
