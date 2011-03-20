#include "stdafx.h"

#include "parallel_task.h"

int PARALLEL_TASK::Dispatch(void * data)
{
	((TASK *) data)->PARALLEL_TASK_RUN();
	return 0;
}
