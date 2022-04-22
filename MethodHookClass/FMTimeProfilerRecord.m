//
//  SMCallTraceTimeCostModel.m
//  DecoupleDemo
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#import "FMTimeProfilerRecord.h"

@implementation FMTimeProfilerRecord

- (NSString *)descriptionWithDepth {
    NSMutableString *str = [NSMutableString new];
    [str appendFormat:@"%2d| ",(int)_callDepth];
    [str appendFormat:@"%6.2f| ",_timeCost * 1000.0];
    for (NSUInteger i = 0; i < _callDepth; i++) {
        [str appendString:@"  "];
    }
    [str appendFormat:@"%s[%@ %@]",(_isClassMethod ? "+" : "-"),_className,_methodName];
    return str;
}

- (NSString *)description {
    NSMutableString *str = [NSMutableString new];
    [str appendFormat:@"%2d| ",(int)_callDepth];
    [str appendFormat:@"%6.2f|",_timeCost * 1000.0];
    [str appendFormat:@"%s[%@ %@]", (_isClassMethod ? "+" : "-"), _className, _methodName];
    return str;
}

@end
