/*
 * Copyright (C) 2019 Marijn Suijten
 *               2019-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

namespace SynchronizedWorkerThread {

class WorkerThread;

enum class AsyncState {
    Invalid,
    Idle,
    Pause,
    Authenticate,
    Enroll,
    Stop,
};

struct WorkHandler {
    virtual WorkerThread& getWorker() = 0;

    virtual void AuthenticateAsync() = 0;
    virtual void EnrollAsync() = 0;

    virtual void IdleAsync();

    inline virtual ~WorkHandler() {}
};

class WorkerThread {
    AsyncState currentState = AsyncState::Invalid, desiredState = AsyncState::Invalid;
    int event_fd;
    std::condition_variable mThreadStateChanged;
    /**
     * Two mutexes are required to syncronize between multiple invocations of
     * moveToState/waitForState, and between waitForState and consumeState (called in the worker
     * thread).
     */
    std::mutex mEventWriterMutex, mThreadMutex;
    std::thread thread;
    WorkHandler* mHandler;

    static void* ThreadStart(void*);
    void RunThread();

  public:
    WorkerThread(WorkHandler* handler);
    ~WorkerThread();

    int getEventFd() const;

    void Start();
    void Stop();
    bool Pause();
    bool Resume();

    AsyncState consumeState();
    bool isEventAvailable(int timeout = /* Do not block at all: */ 0) const;
    bool moveToState(AsyncState);
    bool waitForState(AsyncState);

  private:
    // Unsafe functions, both locks need to lock private mEventWriterMutex
    bool moveToState(AsyncState, std::unique_lock<std::mutex>& writerLock,
                     std::unique_lock<std::mutex>& threadLock);
    bool waitForState(AsyncState, std::unique_lock<std::mutex>& writerLock,
                      std::unique_lock<std::mutex>& threadLock);
};

}  // namespace SynchronizedWorkerThread
