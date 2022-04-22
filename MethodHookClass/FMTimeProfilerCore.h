//
//  FMCallTraceCore.h
//  DecoupleDemo
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#ifndef FMTimeProfilerCore_h
#define FMTimeProfilerCore_h

#include <stdio.h>
#include <objc/objc.h>

typedef struct {
    __unsafe_unretained Class cls;
    SEL sel;
    SEL clsName;
    uint64_t time; // us （1/1000 ms）
    //增加开始和结束时间戳
    uint64_t start;
    uint64_t end;
    int depth;
    int dirty;
} dtp_call_record;

extern void dtp_hook_begin(void);
extern void dtp_hook_end(void);

extern void dtp_set_min_time(uint64_t us); //default 1000
extern void dtp_set_max_depth(int depth); //deafult 10

extern dtp_call_record *dtp_get_call_records(int *num);
extern int dtp_get_current_index(void);
extern void dtp_clear_call_records(void);

#endif /* FMTimeProfilerCore_h */
