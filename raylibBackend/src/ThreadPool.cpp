#include "ThreadPool.h"
#include <raylib.h>

ThreadPool::ThreadPool(int numberOfThreads) : m_isFree(std::vector<std::shared_ptr<std::atomic<bool>>>(numberOfThreads, std::make_shared<std::atomic<bool>>(true))), m_returnFlags(std::vector<std::atomic<bool>*>(numberOfThreads, nullptr)), m_tasks(std::vector<std::function<void()>>(numberOfThreads, nullptr)) {
	initializeThreadRunners(numberOfThreads);
}

void ThreadPool::initializeThreadRunners(int numberOfThreads) {
	for (int i = 0; i < numberOfThreads; i++) {
		std::thread thread(&ThreadPool::ThreadRunner, &m_shutdown, &m_tasks[i], &m_returnFlags[i], m_isFree[i].get());
		thread.detach();
		m_threads.push_back(std::move(thread));
	}
}

void ThreadPool::update() {
	while (!m_tasksQueue.empty() && isAnyThreadAvailable()) {
		for (int i = 0; i < m_tasks.size(); i++) {
			if (m_isFree[i]->load()) {
				TraceLog(LOG_INFO, "Using thread %i", i);
				m_tasks[i] = m_tasksQueue.front();
				m_returnFlags[i] = m_flagsQueue.front();
				m_tasksQueue.pop();
				m_flagsQueue.pop();
				m_isFree[i]->store(false);
				m_isFree[i]->notify_one();
				break;
			}
		}
	}
}

void ThreadPool::shutdown() {
	m_shutdown.store(true);
	for (std::thread& thread : m_threads) {
		thread.join();
	}
}

bool ThreadPool::isAnyThreadAvailable() {
	for (std::shared_ptr<std::atomic<bool>>& isFree : m_isFree) {
		if (isFree->load()) return true;
	}
	return false;
}

void ThreadPool::addTask(std::function<void()> task, std::atomic<bool>* flag) {
	m_tasksQueue.push(task);
	m_flagsQueue.push(flag);
}

void ThreadPool::ThreadRunner(const std::atomic<bool>* shutdown, const std::function<void()>* task, std::atomic<bool>** returnFlag, std::atomic<bool>* isFree) {
	TraceLog(LOG_INFO, "ThreadRunner started");
	while (!shutdown->load()) {
		isFree->wait(true);
		TraceLog(LOG_INFO, "Running task");
		if (task) (*task)();
		TraceLog(LOG_INFO, "Task finished");
		(*returnFlag)->store(true);
		isFree->store(true);
	}
	TraceLog(LOG_INFO, "ThreadRunner shutdown");
}