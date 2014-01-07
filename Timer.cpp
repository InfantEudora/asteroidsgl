//////////////////////////////////////////////////////////////////////////////
// Timer.cpp
// =========
// High Resolution Timer.
// This timer is able to measure the elapsed time with 1 micro-second accuracy
// in both Windows, Linux and Unix system 
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2003-01-13
// UPDATED: 2006-01-13
//
// Copyright (c) 2003 Song Ho Ahn
//////////////////////////////////////////////////////////////////////////////

#include "Timer.h"
#include <stdlib.h>


////////////////////////////////C-Style counters/////////////////////////////
void counter_s_clear(counter_s* t){
	t->average = 0;
	t->cnt = 0;
	t->sum = 0;
	t->last = 0;
}

//Keep last.
void counter_s_reset(counter_s* t){
	t->last = t->average;
	t->average = 0;
	t->cnt = 0;
	t->sum = 0;
}


void counter_s_add(counter_s* t, float val){
	t->sum += val;	
	t->cnt++;
	t->average = (double)t->sum/(double)t->cnt;
}



///////////////////////////////////////////////////////////////////////////////
// constructor
///////////////////////////////////////////////////////////////////////////////
Timer::Timer()
{
#ifdef WIN32
    QueryPerformanceFrequency(&frequency);
    startCount.QuadPart = 0;
    endCount.QuadPart = 0;
#else
    startCount.tv_sec = startCount.tv_usec = 0;
    endCount.tv_sec = endCount.tv_usec = 0;
#endif

    stopped = 0;
    startTimeInMicroSec = 0;
    endTimeInMicroSec = 0;
}



///////////////////////////////////////////////////////////////////////////////
// distructor
///////////////////////////////////////////////////////////////////////////////
Timer::~Timer()
{
}



///////////////////////////////////////////////////////////////////////////////
// start timer.
// startCount will be set at this point.
///////////////////////////////////////////////////////////////////////////////
void Timer::start()
{
    stopped = 0; // reset stop flag
#ifdef WIN32
    QueryPerformanceCounter(&startCount);
#else
    gettimeofday(&startCount, NULL);
#endif
}



///////////////////////////////////////////////////////////////////////////////
// stop the timer.
// endCount will be set at this point.
///////////////////////////////////////////////////////////////////////////////
void Timer::stop()
{
    stopped = 1; // set timer stopped flag

#ifdef WIN32
    QueryPerformanceCounter(&endCount);
#else
    gettimeofday(&endCount, NULL);
#endif
}



///////////////////////////////////////////////////////////////////////////////
// compute elapsed time in micro-second resolution.
// other getElapsedTime will call this first, then convert to correspond resolution.
///////////////////////////////////////////////////////////////////////////////
double Timer::getElapsedTimeInMicroSec()
{
#ifdef WIN32
    if(!stopped)
        QueryPerformanceCounter(&endCount);

    startTimeInMicroSec = startCount.QuadPart * (1000000.0 / frequency.QuadPart);
    endTimeInMicroSec = endCount.QuadPart * (1000000.0 / frequency.QuadPart);
#else
    if(!stopped)
        gettimeofday(&endCount, NULL);

    startTimeInMicroSec = (startCount.tv_sec * 1000000.0) + startCount.tv_usec;
    endTimeInMicroSec = (endCount.tv_sec * 1000000.0) + endCount.tv_usec;
#endif

    return endTimeInMicroSec - startTimeInMicroSec;
}



///////////////////////////////////////////////////////////////////////////////
// divide elapsedTimeInMicroSec by 1000
///////////////////////////////////////////////////////////////////////////////
double Timer::getElapsedTimeInMilliSec()
{
    return this->getElapsedTimeInMicroSec() * 0.001;
}



///////////////////////////////////////////////////////////////////////////////
// divide elapsedTimeInMicroSec by 1000000
///////////////////////////////////////////////////////////////////////////////
double Timer::getElapsedTimeInSec()
{
    return this->getElapsedTimeInMicroSec() * 0.000001;
}



///////////////////////////////////////////////////////////////////////////////
// same as getElapsedTimeInSec()
///////////////////////////////////////////////////////////////////////////////
double Timer::getElapsedTime()
{
    return this->getElapsedTimeInSec();
}
