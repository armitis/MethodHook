//
//  SMCallTraceTimeCostModel.m
//  DecoupleDemo
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#import "FMCallTraceTimeCostModel.h"

@implementation FMCallTraceTimeCostModel

- (NSString *)des {
    NSMutableString *str = [NSMutableString new];
    [str appendFormat:@"%2d| ",(int)_callDepth];
    [str appendFormat:@"%6.2f|",_timeCost * 1000.0];
    for (NSUInteger i = 0; i < _callDepth; i++) {
        [str appendString:@"  "];
    }
    [str appendFormat:@"%s[%@ %@]", (_isClassMethod ? "+" : "-"), _className, _methodName];
    [str appendString:@"  "];
    [str appendFormat:@"%@", @(self.frequency)];
    return str;
}

- (NSString *)storeKey
{
    return [NSString stringWithFormat:@"%@[%@ %@]", (self.isClassMethod ? @"+" : @"-"), self.className, self.methodName];
}

- (NSMutableDictionary *)storeValue
{
    NSMutableDictionary *value = [[NSMutableDictionary alloc] init];
    value[@"cost"] = @(self.timeCost * 1000.0);
    return value;
}

@end
