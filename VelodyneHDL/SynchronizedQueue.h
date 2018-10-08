#ifndef SYNCHRONIZEDQUEUE_H
#define SYNCHRONIZEDQUEUE_H

#include <queue>
#include <boost/thread.hpp>

/**
 * @brief The SynchronizedQueue class is a FIFO structure whith some mutex to allow acces
 * from multiple thread.
 */
template<typename T>
class SynchronizedQueue
{
public:
  SynchronizedQueue()
    : queue_()
    , mutex_()
    , cond_()
    , request_to_end_(false)
    , enqueue_data_(true)
  {
  }

  void enqueue(const T &data)
  {
    boost::unique_lock<boost::mutex> lock(mutex_);

    if (enqueue_data_)
    {
      queue_.push(data);
      cond_.notify_one();
    }
  }

  bool dequeue(T &result)
  {
    boost::unique_lock<boost::mutex> lock(mutex_);

    while (queue_.empty() && (!request_to_end_))
    {
      cond_.wait(lock);
    }

    if (request_to_end_)
    {
      doEndActions();
      return false;
    }

    result = queue_.front();
    queue_.pop();

    return true;
  }

  void stopQueue()
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    request_to_end_ = true;
    cond_.notify_one();
  }

  unsigned int size()
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    return static_cast<unsigned int>(queue_.size());
  }

  bool isEmpty() const
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    return (queue_.empty());
  }

private:
  void doEndActions()
  {
    enqueue_data_ = false;

    while (!queue_.empty())
    {
      queue_.pop();
    }
  }

  std::queue<T> queue_;            // Use STL queue to store data
  mutable boost::mutex mutex_;     // The mutex to synchronise on
  boost::condition_variable cond_; // The condition to wait for

  bool request_to_end_;
  bool enqueue_data_;
};

#endif // SYNCHRONIZEDQUEUE_H