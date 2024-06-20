//
//  AgoraAIGCService+Valid.h
//  AIGCService
//
//  Created by ZYP on 2023/10/24.
//

#import <AgoraAIGCService/AgoraAIGCService.h>

NS_ASSUME_NONNULL_BEGIN

@interface AgoraAIGCService (Valid)
- (BOOL)isValidConfig:(AgoraAIGCConfigure *)config;
@end

NS_ASSUME_NONNULL_END
