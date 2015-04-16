#include "InfiniteThread.h"

InfiniteThread::InfiniteThread():
	m_isInterrupted(false) {}

InfiniteThread::InterruptException::InterruptException(){

}

void InfiniteThread::InterruptException::InterruptionPoint(){
	InfiniteThread * thread = dynamic_cast<InfiniteThread*> (QThread::currentThread());
	if(thread)
		if(thread->m_isInterrupted)
			throw InterruptException();
}

void InfiniteThread::interrupt(){
	m_isInterrupted = true;
}

void InfiniteThread::run(){
	try {
        while(true){
            InfiniteThread::InterruptException::InterruptionPoint();
            iteration();
        }
    }
    catch(InfiniteThread::InterruptException  const & ){
		exception();
    }
}