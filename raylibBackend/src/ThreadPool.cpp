#include "ThreadPool.h"
#include <raylib.h>

ThreadPool::~ThreadPool() {
	shutdown();
	delete[] m_threads;
	delete[] m_tasks;
	delete[] m_returnFlags;
	delete[] m_isFree;
}

ThreadPool::ThreadPool(int numberOfThreads) : m_numberOfThreads(numberOfThreads) {
	m_threads = new std::thread[numberOfThreads];
	m_tasks = new std::function<void()>[numberOfThreads];
	m_returnFlags = new std::atomic<bool>*[numberOfThreads];
	m_isFree = new std::atomic<bool>[numberOfThreads];

	for (int i = 0; i < numberOfThreads; i++) {
		m_isFree[i].store(true);
	}

	initializeThreadRunners(numberOfThreads);
}

void ThreadPool::initializeThreadRunners(int numberOfThreads) {
	for (int i = 0; i < numberOfThreads; i++) {
		std::thread thread(&ThreadPool::ThreadRunner, &m_shutdown, m_tasks + i, m_returnFlags + i, m_isFree + i);
		m_threads[i] = std::move(thread);
	}
}

void ThreadPool::update() {
	while (!m_tasksQueue.empty() && isAnyThreadAvailable()) {
		for (int i = 0; i < m_numberOfThreads; i++) {
			if (m_isFree[i].load()) {
				TraceLog(LOG_DEBUG, "ThreadPool: Using thread %i", i);
				m_tasks[i] = m_tasksQueue.front();
				m_returnFlags[i] = m_flagsQueue.front();
				m_tasksQueue.pop();
				m_flagsQueue.pop();
				m_isFree[i].store(false);
				m_isFree[i].notify_one();
				break;
			}
		}
	}
}

void ThreadPool::shutdown() {
	m_shutdown.store(true);
	for (int i = 0; i < m_numberOfThreads; i++) {
		m_isFree[i].store(false);
		m_isFree[i].notify_one();
		if (m_threads[i].joinable()) m_threads[i].join();
	}
}

bool ThreadPool::isAnyThreadAvailable() {
	for (int i = 0; i < m_numberOfThreads; i++) {
		if (m_isFree[i].load()) return true;
	}
	return false;
}

void ThreadPool::addTask(std::function<void()> task, std::atomic<bool>* flag) {
	m_tasksQueue.push(task);
	m_flagsQueue.push(flag);
}

void ThreadPool::ThreadRunner(const std::atomic<bool>* shutdown, const std::function<void()>* task, std::atomic<bool>** returnFlag, std::atomic<bool>* isFree) {
	while (!shutdown->load()) {
		isFree->wait(true);
		if (shutdown->load()) break;
		TraceLog(LOG_DEBUG, "ThreadRunner: Running task");
		if (task) (*task)();
		if (*returnFlag) (*returnFlag)->store(true);
		isFree->store(true);
	}
}