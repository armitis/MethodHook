//
//  FMCallTraceCore.h
//  DecoupleDemo
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#ifndef FMCallTraceCore_h
#define FMCallTraceCore_h

#include <stdio.h>
#include <objc/objc.h>

typedef struct {
    __unsafe_unretained Class cls;
    SEL sel;
    uint64_t time; // us (1/1000 ms)
    uint64_t start;
    uint64_t end;
    int depth;
} fmCallRecord;

typedef struct fmCallNode{
    __unsafe_unretained Class cls;
    SEL sel;
    int depth;
    struct fmCallNode *next;
    struct fmCallNode *previouse;
} fmCallNode_t;

extern void fmCallTraceStart();
extern void fmCallTraceStop();

extern void fmCallConfigMinTime(uint64_t us); //default 1000
extern void fmCallConfigMaxDepth(int depth);  //default 3

extern fmCallRecord *fmGetCallRecords(int *num);
extern fmCallNode_t *fmGetCurrentCall();
extern void fmClearCallRecords();



#endif /* FMCallTraceCore_h */
