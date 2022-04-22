//
//  SMCallTraceTimeCostModel.h
//  DecoupleDemo
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#import <Foundation/Foundation.h>


NS_ASSUME_NONNULL_BEGIN

// 一条 OC 函数 调用记录
@interface FMTimeProfilerRecord : NSObject

@property (nonatomic, strong) NSString *className;
@property (nonatomic, strong) NSString *methodName;
@property (nonatomic, assign) BOOL isClassMethod;
@property (nonatomic, assign) NSTimeInterval timeCost;
@property (nonatomic, assign) NSUInteger callDepth;
@property (nonatomic, assign) NSTimeInterval start;
@property (nonatomic, assign) NSTimeInterval end;
@property (nonatomic, strong) NSArray <FMTimeProfilerRecord *>*subRecords;

- (NSString *)descriptionWithDepth ;

@end

NS_ASSUME_NONNULL_END
