//
//  SMCallTrace.m
//  HomePageTest
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armitis. All rights reserved.
//

#import "FMCallTrace.h"
#import "FMCallLib.h"
#import "FMCallTraceTimeCostModel.h"


static NSMutableDictionary *dic;

@implementation FMCallTrace

#pragma mark - Trace
#pragma mark - OC Interface
+ (void)start {
    if (dic == nil) {
        dic = [[NSMutableDictionary alloc] init];
    }
    fmCallTraceStart();
}
+ (void)startWithMaxDepth:(int)depth {
    fmCallConfigMaxDepth(depth);
    [FMCallTrace start];
}
+ (void)startWithMinCost:(double)ms {
    fmCallConfigMinTime(ms * 1000);
    [FMCallTrace start];
}
+ (void)startWithMaxDepth:(int)depth minCost:(double)ms {
    fmCallConfigMaxDepth(depth);
    fmCallConfigMinTime(ms * 1000);
    [FMCallTrace start];
}
+ (void)stop {
    fmCallTraceStop();
}
+ (void)save {
    NSMutableString *mStr = [NSMutableString new];
    NSArray<FMCallTraceTimeCostModel *> *arr = [self loadRecords];
    for (FMCallTraceTimeCostModel *model in arr) {
        //记录方法路径
        model.path = [NSString stringWithFormat:@"[%@ %@]",model.className,model.methodName];
        [self appendRecord:model to:mStr];
    }
//    NSLog(@"%@",mStr);
//    NSLog(@"%@", dic);
}
+ (void)stopSaveAndClean {
    [FMCallTrace stop];
    [FMCallTrace save];
    fmClearCallRecords();
}
+ (void)appendRecord:(FMCallTraceTimeCostModel *)cost to:(NSMutableString *)mStr {
    
    if (dic[cost.storeKey] == nil) {
        dic[cost.storeKey] = cost.storeValue;
    } else {
        //保持耗时最大的
        NSMutableDictionary *valueDict = dic[cost.storeKey];
        NSTimeInterval t = [valueDict[@"cost"] doubleValue];
        if (cost.timeCost * 1000.0 > t) {
            valueDict[@"cost"] = @(cost.timeCost);
        }
    }

//    [mStr appendFormat:@"%@\n path%@\n",[cost des],cost.path];
    if (cost.subCosts.count < 1) {
        cost.lastCall = YES;
        //记录到数据库中
    } else {
        for (FMCallTraceTimeCostModel *model in cost.subCosts) {
            if ([model.className isEqualToString:@"FMCallTrace"]) {
                break;
            }
            //记录方法的子方法的路径
            model.path = [NSString stringWithFormat:@"%@ - [%@ %@]",cost.path,model.className,model.methodName];
            [self appendRecord:model to:mStr];
        }
    }
    
}

+ (NSArray *)mainCallStack {
    NSMutableArray *array = [[NSMutableArray alloc] init];
    fmCallNode_t *node = fmGetCurrentCall();
    if (!node) {
        return @[];
    }
    for (int i = 0; i < 1000; i++) {
        if (node && node->depth == 1) {
            NSString *clsName = NSStringFromClass(node->cls);
            if ([clsName hasPrefix:@"prefix"]) {
                [array addObject:[NSString stringWithFormat:@"[%@ %@]", NSStringFromClass(node->cls), NSStringFromSelector(node->sel)]];
            }
            node = node->previouse;
        }
    }
    
    return [array copy];
}

+ (NSArray<FMCallTraceTimeCostModel *>*)loadRecords {
    NSMutableArray<FMCallTraceTimeCostModel *> *arr = [NSMutableArray new];
    int num = 0;
    fmCallRecord *records = fmGetCallRecords(&num);
    for (int i = 0; i < num; i++) {
        fmCallRecord *rd = &records[i];
        FMCallTraceTimeCostModel *model = [FMCallTraceTimeCostModel new];
        model.className = NSStringFromClass(rd->cls);
        model.methodName = NSStringFromSelector(rd->sel);
        model.isClassMethod = class_isMetaClass(rd->cls);
        model.timeCost = (double)rd->time / 1000000.0;
        model.callDepth = rd->depth;
        [arr addObject:model];
    }
    NSUInteger count = arr.count;
    for (NSUInteger i = 0; i < count; i++) {
        FMCallTraceTimeCostModel *model = arr[i];
        if (model.callDepth > 0) {
            [arr removeObjectAtIndex:i];
            //Todo:不需要循环，直接设置下一个，然后判断好边界就行
            for (NSUInteger j = i; j < count - 1; j++) {
                //下一个深度小的话就开始将后面的递归的往 sub array 里添加
                if (arr[j].callDepth + 1 == model.callDepth) {
                    NSMutableArray *sub = (NSMutableArray *)arr[j].subCosts;
                    if (!sub) {
                        sub = [NSMutableArray new];
                        arr[j].subCosts = sub;
                    }
                    [sub insertObject:model atIndex:0];
                }
            }
            i--;
            count--;
        }
    }
    return arr;
}


+ (NSString *)storeString
{
    return [NSString stringWithFormat:@"%@", dic];
}

@end
