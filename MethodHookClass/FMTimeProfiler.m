//
//  SMCallTrace.m
//  HomePageTest
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armitis. All rights reserved.
//

#import "FMTimeProfiler.h"
#import "FMTimeProfilerCore.h"
#import "FMTimeProfilerRecord.h"
#include <objc/runtime.h>


static NSTimeInterval startTime;
static NSTimeInterval stopTime;
static NSMutableDictionary *callCount;
static NSMutableArray *traceArray;
static NSString *result;

@implementation FMTimeProfiler

+ (void)startRecord {
    startTime = [NSDate new].timeIntervalSince1970;
    dtp_hook_begin();
}

+ (void)stopRecord {
    dtp_hook_end();
    stopTime = [NSDate new].timeIntervalSince1970;
    [self cacualteCount];
    [self generateTracing];
    [self printRecords];
    [self clearRecords];
}

+ (void)clearRecords {
    dtp_clear_call_records();
}

+ (NSString *)recordsResult
{
    return result;
}

/// 打印调用记录
+ (void)printRecords {
    result = [self getRecordsResult];
    NSLog(@"%@",result);
}

+ (void)cacualteCount {
    if (callCount == nil) {
        callCount = [NSMutableDictionary dictionary];
    } else {
        [callCount removeAllObjects];
    }
    NSArray<FMTimeProfilerRecord *>*arr = [self getRecords];
    for (FMTimeProfilerRecord *record in arr) {
        NSString *key = [NSString stringWithFormat:@"%@ [%@ %@]", record.isClassMethod ? @"+" : @"-", record.className, record.methodName];
        NSNumber *value = callCount[key];
        if (value) {
            callCount[key] = @(value.intValue + 1);
        } else {
            callCount[key] = @(1);
        }
    }
}

//方法耗时转换成chrome://tracing/格式
+ (void)generateTracing
{
    //方法耗时转换成chrome://tracing/格式
    if (traceArray == nil) {
        traceArray = [NSMutableArray array];
    } else {
        [traceArray removeAllObjects];
    }
    
    NSArray<FMTimeProfilerRecord *>*arr = [self getRecords];
    for (int i = 0; i < arr.count; i++) {
        FMTimeProfilerRecord *model = arr[i];
        NSDictionary *traceStart = @{
            @"name": [NSString stringWithFormat:@"%@: %@", model.className, model.methodName],
            @"ph": @"B",
            @"pid": @"APP",
            @"tid": @"Main",
            @"ts": @(model.start)
        };
        NSDictionary *traceEnd = @{
            @"name": [NSString stringWithFormat:@"%@: %@", model.className, model.methodName],
            @"ph": @"E",
            @"pid": @"APP",
            @"tid": @"Main",
            @"ts": @(model.end)
        };
        [traceArray addObject:traceStart];
        [traceArray addObject:traceEnd];
    }
}

+ (NSString *)costTrace
{
    NSError *error = nil;
    NSData *data = [NSJSONSerialization dataWithJSONObject:traceArray options:0 error:&error];
    if (error) {
        return @"";
    }
    return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
}

+ (NSString *)costCount
{
    NSError *error = nil;
    NSData *data = [NSJSONSerialization dataWithJSONObject:callCount options:0 error:&error];
    if (error) {
        return @"";
    }
    return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
}

+ (NSString *)getRecordsResult {
    NSMutableString *str = [NSMutableString new];
    [str appendFormat:@"\n\ntime profile result : \n"];
    NSTimeInterval totalRecord = 0;
    NSArray<FMTimeProfilerRecord *>*arr = [self getRecords];
    for (FMTimeProfilerRecord *r in arr) {
        [self appendRecord:r to:str];
        totalRecord += r.timeCost;
    }
    
    [str appendFormat:@"\n"];
    [str appendFormat:@"totalTimeInRecord:%.2f\n",totalRecord * 1000];
    
    [str appendFormat:@"\ntime profile end\n"];
    
    return str;
}


+ (void)appendRecord:(FMTimeProfilerRecord *)record to:(NSMutableString *)str {
    [str appendFormat:@"%@\n", [record descriptionWithDepth]];
    for (FMTimeProfilerRecord *r in record.subRecords) {
        [self appendRecord:r to:str];
    }
}

+ (NSArray<FMTimeProfilerRecord *>*)getRecords {
    NSMutableArray<FMTimeProfilerRecord *> *arr = [NSMutableArray new];
    int record_num = 0;
    dtp_call_record *records = dtp_get_call_records(&record_num);
    
    
    //获取记录
    int currentIndex = dtp_get_current_index();
    
    if (currentIndex == -1) {
        return @[];
    }
    
    //先找起始位置的索引
    int count = 0;
    //currentIndex 可能是空节点，向前移一步
    int startIndex = currentIndex - 1;
    if (startIndex < 0) {
        startIndex = record_num - 1;
    }
    while (count < record_num) {
        dtp_call_record *rec = &records[startIndex];
        if (rec->dirty == 0) {
            //当前的是空节点，起始位置往后移动一个到非空的节点
            startIndex += 1;
            if (startIndex > record_num - 1) {
                startIndex = 0;
            }
            break;
        }
        
        count += 1;
        
        if (count < record_num) {
            startIndex -= 1;
            if (startIndex < 0) {
                startIndex = record_num - 1;
            }
        }
    }
    
    count = 0;
    int index = startIndex;
    while (count < record_num) {
        dtp_call_record *rec = &records[index];
        if (rec->dirty == 0) {
            break;
        }
        FMTimeProfilerRecord *r = [FMTimeProfilerRecord new];
        //有时候会崩溃,rec->cls有时候返回的野指针，NSStringFromClass(rec->cls)会崩溃，
        //eg:fir_2F72734D-E194-454E-8996-0E376DA03FB5_FBSDKGraphRequestConnection
        //将cls信息存储到sel中，然后从sel获取
        NSString *className = NSStringFromSelector(rec->clsName);
        Class cls = NSClassFromString(className);
        r.className = NSStringFromClass(cls);
        r.methodName = NSStringFromSelector(rec->sel);
        r.isClassMethod = class_isMetaClass(cls);
        r.timeCost = (double)rec->time / 1000000.0;
        r.callDepth = rec->depth;
        r.start = rec->start;
        r.end = rec->end;
        [arr addObject:r];
        
        count += 1;
        if (count < record_num) {
            index += 1;
            if (index > record_num - 1) {
                index = 0;
            }
        }
    }
    
    for (int i = 0, max = (int)arr.count; i < max; i++) {
        FMTimeProfilerRecord *r = arr[i];
        if (r.callDepth > 0) {
            [arr removeObjectAtIndex:i];
            for (int j = i; j < max - 1; j++) {
                if (arr[j].callDepth + 1 == r.callDepth) {
                    NSMutableArray *sub = (NSMutableArray *)arr[j].subRecords;
                    if (!sub) {
                        sub = [NSMutableArray new];
                        arr[j].subRecords = sub;
                    }
                    [sub insertObject:r atIndex:0];
                    break;
                }
            }
            i--; max--;
        }
    }
    return arr;
}

+ (void)setMinCallCost:(double)ms {
    dtp_set_min_time(ms * 1000);
}

+ (void)setMaxCallDepth:(int)depth {
    if (depth < 0) depth = 0;
    dtp_set_max_depth(depth);
}

@end
