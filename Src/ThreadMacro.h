#pragma once

// for MSVC, Win32
#ifdef _WIN32

#include <windows.h>

#define CREATE_THREAD CreateThread
#define THREAD_HANDLE HANDLE
#define THREAD_ID DWORD
#define CRETAE_MUTEX CreateMutex
#define CREATE_EVENT CreateEvent
#define THREAD_EVENT_HANDLE HANDLE
#define CLOSE_THREAD_HANDLE CloseHandle
#define CLOSE_EVENT_HANDLE CloseHandle
#define WAKE_UP_THREAD ResumeThread
#define SUSPEND_THREAD SuspendThread
#define WAIT_FOR_SINGLE_OBJ WaitForSingleObject
#define WAIT_FOR_MULTI_OBJ WaitForMultipleObjects
#define RESET_THREAD_EVENT ResetEvent
#define SET_THREAD_EVENT SetEvent
#define GET_CURRENT_THREAD_ID GetCurrentThreadId
#define INFI_TIME INFINITE
#define RELEASE_THREAD_MUTEX ReleaseMutex

// for GCC, Linux
#elif linux

#define CREATE_THREAD  
#define THREAD_HANDLE  
#define THREAD_ID  
#define CRETAE_MUTEX  
#define CREATE_EVENT  
#define THREAD_EVENT_HANDLE  
#define CLOSE_THREAD_HANDLE  
#define CLOSE_EVENT_HANDLE  
#define WAKE_UP_THREAD  
#define SUSPEND_THREAD  
#define WAIT_FOR_SINGLE_OBJ  
#define WAIT_FOR_MULTI_OBJ  
#define RESET_THREAD_EVENT  
#define SET_THREAD_EVENT  
#define GET_CURRENT_THREAD_ID   
#define INFI_TIME 0xffffffff
#define RELEASE_THREAD_MUTEX  

#endif