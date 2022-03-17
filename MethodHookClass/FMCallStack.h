//
//  SMCallStack.h
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FMCallLib.h"

typedef NS_ENUM(NSUInteger, FMCallStackType) {
    FMCallStackTypeAll,     //全部线程
    FMCallStackTypeMain,    //主线程
    FMCallStackTypeCurrent  //当前线程
};



@interface FMCallStack : NSObject

+ (NSString *)callStackWithType:(FMCallStackType)type;

extern NSString *fmStackOfThread(thread_t thread);

@end
