//a simple parallel task system where a task is its own worker thread

#ifndef _PARALLEL_TASK_H_
#define _PARALLEL_TASK_H_

//#include <SDL/SDL.h>
//#include <SDL/SDL_thread.h>
//#include <cassert>
//#include <iostream>

//#define VERBOSE

namespace PARALLEL_TASK
{

int Dispatch(void * data);

class TASK
{
	private:
		bool quit;
		
		SDL_sem * sem_frame_start;
		SDL_sem * sem_frame_end;
		
		bool executing;
		
		SDL_Thread * thread;
		
	public:
		TASK() : quit(false), sem_frame_start(NULL), sem_frame_end(NULL), executing(false), thread(NULL)
		{}
		
		void PARALLEL_TASK_INIT()
		{
			//don't allow double init
			assert (!sem_frame_start);
			assert (!sem_frame_end);
			assert (!thread);
			
			quit = false;
			sem_frame_start = SDL_CreateSemaphore(0);
			sem_frame_end = SDL_CreateSemaphore(0);
			thread = SDL_CreateThread(Dispatch, this);
		}
		
		~TASK()
		{
			PARALLEL_TASK_DEINIT();
		}
		
		void PARALLEL_TASK_DEINIT()
		{
			//don't allow double deinit, or deinit with no init
			if (!(sem_frame_start && sem_frame_end && thread))
				return;
			
			quit = true;
			//at this point our thread is waiting for the frame to start, so have it do one last iteration so it can see the quit flag
#ifdef VERBOSE
			std::cout << "telling child to exit" << std::endl;
#endif
			SDL_SemPost(sem_frame_start);
#ifdef VERBOSE
			std::cout << "waiting for thread to exit" << std::endl;
#endif
			//wait for the thread to exit
			SDL_WaitThread(thread, NULL);
#ifdef VERBOSE
			std::cout << "thread has exited" << std::endl;
#endif
			thread = NULL;
			
			SDL_DestroySemaphore(sem_frame_start);
			SDL_DestroySemaphore(sem_frame_end);
#ifdef VERBOSE
			std::cout << "destroyed semaphores" << std::endl;
#endif
			
			sem_frame_start = NULL;
			sem_frame_end = NULL;
		}
		
		void PARALLEL_TASK_RUN()
		{
#ifdef VERBOSE
			std::cout << "child thread run" << std::endl;
#endif
			this->PARALLEL_TASK_SETUP();
			
#ifdef VERBOSE
			std::cout << "posting end semaphore to allow the main loop to run through the first iteration" << std::endl;
#endif
			SDL_SemPost(sem_frame_end);
			
			while (!quit)
			{
#ifdef VERBOSE
				std::cout << "waiting for start semaphore" << std::endl;
#endif
				SDL_SemWait(sem_frame_start);
#ifdef VERBOSE
				std::cout << "done waiting for start semaphore" << std::endl;
#endif

				executing = true;
				if (!quit) //avoid doing the last tick if we don't have to
					this->PARALLEL_TASK_EXECUTE();
				executing = false;
				
#ifdef VERBOSE
				std::cout << "posting end semaphore" << std::endl;
#endif
				SDL_SemPost(sem_frame_end);
			}
#ifdef VERBOSE
			std::cout << "child thread exit" << std::endl;
#endif
		}
		
		void PARALLEL_TASK_START()
		{
#ifdef VERBOSE
			std::cout << "posting start semaphore" << std::endl;
#endif
			SDL_SemPost(sem_frame_start);
		}
		
		void PARALLEL_TASK_END()
		{
#ifdef VERBOSE
			std::cout << "waiting for end semaphore" << std::endl;
#endif
			SDL_SemWait(sem_frame_end);
#ifdef VERBOSE
			std::cout << "done waiting for end semaphore" << std::endl;
#endif
		}
		
		virtual void PARALLEL_TASK_EXECUTE() = 0;
		virtual void PARALLEL_TASK_SETUP() {}

		bool GetExecuting() const
		{
			return executing;
		}
};

}

#endif
