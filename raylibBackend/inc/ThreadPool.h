#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <atomic>
#include "Updatable.h"

class ThreadPool : public Updatable {
public:
	ThreadPool(int numberOfThreads);

	void update();
	void shutdown();

	void addTask(std::function<void()> task, std::atomic<bool>* flag);

private:
	std::vector<std::thread> m_threads;
	std::vector<std::function<void()>> m_tasks;
	std::vector<std::atomic<bool>*> m_returnFlags;
	std::queue<std::function<void()>> m_tasksQueue;
	std::queue<std::atomic<bool>*> m_flagsQueue;
	std::vector<std::shared_ptr<std::atomic<bool>>> m_isFree;
	std::atomic<bool> m_shutdown{ false };

	static void ThreadRunner(const std::atomic<bool>* shutdown, const std::function<void()>* task, std::atomic<bool>** returnFlag, std::atomic<bool>* isFree);

	void initializeThreadRunners(int numberOfThreads);
	bool isAnyThreadAvailable();
};