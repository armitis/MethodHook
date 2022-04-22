//
//  SMCallTrace.h
//  HomePageTest
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armitis. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FMTimeProfilerRecord.h"

NS_ASSUME_NONNULL_BEGIN

@interface FMTimeProfiler : NSObject

/// 开始记录
+ (void)startRecord;

/// 停止记录
+ (void)stopRecord;

/// 清空已有记录
+ (void)clearRecords;

/// 打印调用记录
+ (void)printRecords;

///函数调用记录
+ (NSString *)recordsResult;

///函数耗时trace文件
+ (NSString *)costTrace;

///函数调用次数信息
+ (NSString *)costCount;

#pragma mark - 配置项
/// 设置过滤的最小函数调用时间（毫秒），小于该时间的函数调用将被忽略。 默认值:1.0
+ (void)setMinCallCost:(double)ms;

/// 设置过滤的最深函数调用，调用深度超过该值的函数将被忽略。 默认值:10
+ (void)setMaxCallDepth:(int)depth;

@end

NS_ASSUME_NONNULL_END
