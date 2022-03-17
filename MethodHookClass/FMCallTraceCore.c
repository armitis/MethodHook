//
//  FMCallTraceCore.c
//  DecoupleDemo
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#include "FMCallTraceCore.h"


#ifdef __aarch64__
#include "fishhook.h"

#pragma mark - Record

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <dispatch/dispatch.h>
#include <pthread.h>

static bool _call_record_enabled = true;
static uint64_t _min_time_cost = 1000; //us
static int _max_call_depth = 3;
static pthread_key_t _thread_key;
__unused static id (*orig_objc_msgSend)(id, SEL, ...);

static fmCallRecord *_fmCallRecords;
static fmCallNode_t *_fmFirstNode;
static fmCallNode_t *_fmCallNode;
//static int otp_record_num;
//static int otp_record_alloc;
static int _fmRecordNum;
static int _fmRecordAlloc;

static int _fmStartCount;

typedef struct {
    id self; //通过 object_getClass 能够得到 Class 再通过 NSStringFromClass 能够得到类名
    Class cls;
    SEL cmd; //通过 NSStringFromSelector 方法能够得到方法名
    uint64_t time; //us
    uintptr_t lr; // link register
} thread_call_record;

typedef struct {
    thread_call_record *stack;
    int allocated_length;
    int index;
    bool is_main_thread;
} thread_call_stack;

static inline thread_call_stack * get_thread_call_stack() {
    thread_call_stack *cs = (thread_call_stack *)pthread_getspecific(_thread_key);
    if (cs == NULL) {
        cs = (thread_call_stack *)malloc(sizeof(thread_call_stack));
        cs->stack = (thread_call_record *)calloc(128, sizeof(thread_call_record));
        cs->allocated_length = 64;
        cs->index = -1;
        cs->is_main_thread = pthread_main_np();
        pthread_setspecific(_thread_key, cs);
    }
    return cs;
}

static void release_thread_call_stack(void *ptr) {
    thread_call_stack *cs = (thread_call_stack *)ptr;
    if (!cs) return;
    if (cs->stack) free(cs->stack);
    free(cs);
}

static inline void push_call_record(id _self, Class _cls, SEL _cmd, uintptr_t lr) {
    thread_call_stack *cs = get_thread_call_stack();
    if (cs) {
        int nextIndex = (++cs->index);
        if (nextIndex >= cs->allocated_length) {
            cs->allocated_length += 64;
            cs->stack = (thread_call_record *)realloc(cs->stack, cs->allocated_length * sizeof(thread_call_record));
        }
        thread_call_record *newRecord = &cs->stack[nextIndex];
        newRecord->self = _self;
        newRecord->cls = _cls;
        newRecord->cmd = _cmd;
        newRecord->lr = lr;
        if (cs->is_main_thread && _call_record_enabled) {
            struct timeval now;
            gettimeofday(&now, NULL);
            newRecord->time = (now.tv_sec % 100) * 1000000 + now.tv_usec;
            _fmStartCount++;
            if (!_fmFirstNode) {
                _fmFirstNode = malloc(sizeof(fmCallNode_t) * 1);
                _fmFirstNode->depth = 1;
                _fmFirstNode->cls = _cls;
                _fmFirstNode->sel = _cmd;
                _fmFirstNode->previouse = NULL;
                _fmFirstNode->next = NULL;
                _fmCallNode = _fmFirstNode;
                _fmStartCount = 1;
                return;
            }
            if (_fmStartCount < 1000) {
                fmCallNode_t *new = malloc(sizeof(fmCallNode_t) * 1);
                new->depth = 1;
                new->cls = _cls;
                new->sel = _cmd;
                new->previouse = _fmCallNode;
                new->next = NULL;
                _fmCallNode->next = new;
                _fmCallNode = new;
                _fmStartCount++;
                return;
            }
            if (_fmCallNode->next == NULL) {
                _fmCallNode->next = _fmFirstNode;
                _fmFirstNode->previouse = _fmCallNode;
                _fmCallNode = _fmFirstNode;
                _fmCallNode->depth = 1;
                _fmCallNode->cls = _cls;
                _fmCallNode->sel = _cmd;
                return;
            }
            fmCallNode_t *next = _fmCallNode->next;
            next->depth = 1;
            next->cls = _cls;
            next->sel = _cmd;
            _fmCallNode = next;
        }
    }
}

static inline uintptr_t pop_call_record() {
    thread_call_stack *cs = get_thread_call_stack();
    int curIndex = cs->index;
    int nextIndex = cs->index--;
    thread_call_record *pRecord = &cs->stack[nextIndex];
    
    if (cs->is_main_thread && _call_record_enabled) {
        struct timeval now;
        gettimeofday(&now, NULL);
        uint64_t time = (now.tv_sec % 100) * 1000000 + now.tv_usec;
        if (time < pRecord->time) {
            time += 100 * 1000000;
        }
        uint64_t cost = time - pRecord->time;
        if (cost > _min_time_cost && cs->index < _max_call_depth) {
            if (!_fmCallRecords) {
                _fmRecordAlloc = 1024;
                _fmCallRecords = malloc(sizeof(fmCallRecord) * _fmRecordAlloc);
            }
            _fmRecordNum++;
            if (_fmRecordNum >= _fmRecordAlloc) {
                _fmRecordAlloc += 1024;
                _fmCallRecords = realloc(_fmCallRecords, sizeof(fmCallRecord) * _fmRecordAlloc);
            }
            fmCallRecord *log = &_fmCallRecords[_fmRecordNum - 1];
            log->cls = pRecord->cls;
            log->depth = curIndex;
            log->sel = pRecord->cmd;
            log->time = cost;
        }
    }
    return pRecord->lr;
}

void before_objc_msgSend(id self, SEL _cmd, uintptr_t lr) {
    push_call_record(self, object_getClass(self), _cmd, lr);
}

uintptr_t after_objc_msgSend() {
    return pop_call_record();
}


//replacement objc_msgSend (arm64)
// https://blog.nelhage.com/2010/10/amd64-and-va_arg/
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
// https://developer.apple.com/library/ios/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html
#define call(b, value) \
__asm volatile ("stp x8, x9, [sp, #-16]!\n"); \
__asm volatile ("mov x12, %0\n" :: "r"(value)); \
__asm volatile ("ldp x8, x9, [sp], #16\n"); \
__asm volatile (#b " x12\n");

#define save() \
__asm volatile ( \
"stp x8, x9, [sp, #-16]!\n" \
"stp x6, x7, [sp, #-16]!\n" \
"stp x4, x5, [sp, #-16]!\n" \
"stp x2, x3, [sp, #-16]!\n" \
"stp x0, x1, [sp, #-16]!\n");

#define load() \
__asm volatile ( \
"ldp x0, x1, [sp], #16\n" \
"ldp x2, x3, [sp], #16\n" \
"ldp x4, x5, [sp], #16\n" \
"ldp x6, x7, [sp], #16\n" \
"ldp x8, x9, [sp], #16\n" );

#define link(b, value) \
__asm volatile ("stp x8, lr, [sp, #-16]!\n"); \
__asm volatile ("sub sp, sp, #16\n"); \
call(b, value); \
__asm volatile ("add sp, sp, #16\n"); \
__asm volatile ("ldp x8, lr, [sp], #16\n");

#define ret() __asm volatile ("ret\n");

__attribute__((__naked__))
static void hook_Objc_msgSend() {
    // Save parameters.
    save()
    
    __asm volatile ("mov x2, lr\n");
    __asm volatile ("mov x3, x4\n");
    
    // Call our before_objc_msgSend.
    call(blr, &before_objc_msgSend)
    
    // Load parameters.
    load()
    
    // Call through to the original objc_msgSend.
    call(blr, orig_objc_msgSend)
    
    // Save original objc_msgSend return value.
    save()
    
    // Call our after_objc_msgSend.
    call(blr, &after_objc_msgSend)
    
    // restore lr
    __asm volatile ("mov lr, x0\n");
    
    // Load original objc_msgSend return value.
    load()
    
    // return
    ret()
}


#pragma mark public

void fmCallTraceStart() {
    _call_record_enabled = true;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pthread_key_create(&_thread_key, &release_thread_call_stack);
        rebind_symbols((struct rebinding[6]){
            {"objc_msgSend", (void *)hook_Objc_msgSend, (void **)&orig_objc_msgSend},
        }, 1);
    });
}

void fmCallTraceStop() {
    _call_record_enabled = false;
}

void fmCallConfigMinTime(uint64_t us) {
    _min_time_cost = us;
}
void fmCallConfigMaxDepth(int depth) {
    _max_call_depth = depth;
}

fmCallRecord *fmGetCallRecords(int *num) {
    if (num) {
        *num = _fmRecordNum;
    }
    return _fmCallRecords;
}

fmCallNode_t *fmGetCurrentCall() {
    if (_call_record_enabled) {
        return _fmCallNode;
    }
    return NULL;
}

void fmClearCallRecords() {
    if (_fmCallRecords) {
        free(_fmCallRecords);
        _fmCallRecords = NULL;
    }
    _fmRecordNum = 0;
}

#else

void fmCallTraceStart() {}
void fmCallTraceStop() {}
void fmCallConfigMinTime(uint64_t us) {
}
void fmCallConfigMaxDepth(int depth) {
}
fmCallRecord *fmGetCallRecords(int *num) {
    if (num) {
        *num = 0;
    }
    return NULL;
}
void fmClearCallRecords() {}

#endif
