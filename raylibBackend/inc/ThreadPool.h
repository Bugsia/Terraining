#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <atomic>
#include "Updatable.h"

class ThreadPool : public Updatable {
public:
	~ThreadPool();
	ThreadPool(int numberOfThreads);

	void update(int targetFPS);
	void shutdown();

	void addTask(std::function<void()> task, std::atomic<bool>* flag);

private:
	const int m_numberOfThreads;
	std::thread* m_threads;
	std::function<void()>* m_tasks;
	std::atomic<bool>** m_returnFlags;
	std::atomic<bool>* m_isFree;
	std::queue<std::function<void()>> m_tasksQueue;
	std::queue<std::atomic<bool>*> m_flagsQueue;
	std::atomic<bool> m_shutdown{ false };

	static void ThreadRunner(const std::atomic<bool>* shutdown, const std::function<void()>* task, std::atomic<bool>** returnFlag, std::atomic<bool>* isFree);

	void initializeThreadRunners(int numberOfThreads);
	bool isAnyThreadAvailable();
};