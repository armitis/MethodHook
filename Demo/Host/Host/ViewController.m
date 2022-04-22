//
//  ViewController.m
//  Host
//
//  Created by zhengzhiwen on 2022/4/22.
//

#import "ViewController.h"
#import <MethodHook/FMTimeProfiler.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [FMTimeProfiler startRecord];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [FMTimeProfiler stopRecord];
        NSLog(@"%@");
    });
    [self do1];

}

- (void)do1
{
    for (int i = 0; i < 50000; i++) {
        NSLog(@"");
    }
}


@end
