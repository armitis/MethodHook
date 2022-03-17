//
//  SMCallStackModel.h
//  GCDFetchFeed
//
//  Created by Armitis on 2022/3/17.
//  Copyright © 2022年 Armtiis. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface FMCallStackModel : NSObject

@property (nonatomic, copy) NSString *stackStr;       //完整堆栈信息
@property (nonatomic) BOOL isStuck;                   //是否被卡住
@property (nonatomic, assign) NSTimeInterval dateString;   //可展示信息

@end
