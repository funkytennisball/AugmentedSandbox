#ifndef INFINITETHREAD_H
#define INFINITETHREAD_H

#include <Qt\qthread.h>

class InfiniteThread : public QThread
 {
 private:
	 bool m_isInterrupted;
	
 public:
	 InfiniteThread();

	 class InterruptException : public std::exception {
	 public:
		InterruptException();
		static void InterruptionPoint();
	 };

 public slots:
	 void interrupt();

 private:
     void run();
	 virtual void iteration() = 0;
	 virtual void exception() = 0;
 };

#endif