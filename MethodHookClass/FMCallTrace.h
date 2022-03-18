//
//  SMCallTrace.h
//  HomePageTest
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armitis. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FMCallTraceCore.h"


@interface FMCallTrace : NSObject
+ (void)start; //开始记录
+ (void)startWithMaxDepth:(int)depth;
+ (void)startWithMinCost:(double)ms;
+ (void)startWithMaxDepth:(int)depth minCost:(double)ms;
+ (void)stop; //停止记录
+ (void)save; //保存和打印记录，如果不是短时间 stop 的话使用 saveAndClean
+ (void)stopSaveAndClean; //停止保存打印并进行内存清理
+ (NSString *)storeString;
+ (NSArray *)mainCallLog;
//通过特定的方法前缀过滤需要记录的方法
+ (void)setPrefix:(NSString *)prefix;
//获得trace数据
+ (NSString *)traceJsonString;

@end
