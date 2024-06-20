//
//  AIGCService.h
//  GPT
//
//  Created by ZYP on 2023/10/7.
//

#import <Foundation/Foundation.h>
#import "AgoraAIGCObjects.h"

NS_ASSUME_NONNULL_BEGIN

@protocol AgoraAIGCServiceDelegate <NSObject>
/// Callback for the result of SDK events.
/// - Parameters:
///   - event: The event type.
///   - code:  The error code or success code (AIGCServiceCodeSuccess).
///   - message: The error message. nil if code == AIGCServiceCodeSuccess.
- (void)onEventResultWithEvent:(AgoraAIGCServiceEvent)event
                          code:(AgoraAIGCServiceCode)code
                       message:(NSString * _Nullable)message;

/// Callback for the result event of sound to text.
/// - Parameters:
///   - roundId: An unique string value indicate the round.
///   - recognizedSpeech: indicates whether it is a complete sentence
///   - code:  The error code or success code (AIGCServiceCodeSuccess).
///   - return: A AgoraAIGCHandleResult value.
- (AgoraAIGCHandleResult)onSpeech2TextWithRoundId:(NSString *)roundId
                                           result:(NSMutableString *)result
                                 recognizedSpeech:(BOOL)recognizedSpeech
                                             code:(AgoraAIGCServiceCode)code;

/// Callback for the result event of llm.
/// - Parameters:
///   - answer: The result of llm. (part of all result)
///   - roundId: An unique string value indicate the round.
///   - isRoundEnd:  Indicates whether it is at the end of the round.
///   - estimatedResponseTokens: The number of tokens consumed by the round in response data. It is an estimated value and cannot be used for billing.
///   - code:  The error code or success code (AIGCServiceCodeSuccess).
- (AgoraAIGCHandleResult)onLLMResultWithRoundId:(NSString *)roundId
                                         answer:(NSMutableString *)answer
                                     isRoundEnd:(BOOL)isRoundEnd
                        estimatedResponseTokens:(NSUInteger)tokens
                                           code:(AgoraAIGCServiceCode)code;

/// Callback for the result event of text to sound.
/// - Parameters:
///   - roundId: An unique string value indicate the round.
///   - voice: The audio result of text to sound.
///   - sampleRates: The sample rate of the audio (16000).
///   - channels: The number of audio channels (1).
///   - bits: The number of bits per sample (16).
///   - code:  The error code or success code (AIGCServiceCodeSuccess).
- (AgoraAIGCHandleResult)onText2SpeechResultWithRoundId:(NSString *)roundId
                                                  voice:(NSData *)voice
                                            sampleRates:(NSInteger)sampleRates
                                               channels:(NSInteger)channels
                                                   bits:(NSInteger)bits
                                                   code:(AgoraAIGCServiceCode)code;
@optional
- (void)onSpeechStateChange:(AgoraAIGCServiceSpeechState)state;

@end

/// AgoraAIGCService is the main class for the AI service.
__attribute__((visibility("default")))
@interface AgoraAIGCService : NSObject

@property(nonatomic, weak)id<AgoraAIGCServiceDelegate> delegate;
/// Creates a single instance of AgoraAIGCService.
+ (instancetype)create;

/// Destroys the single instance of AgoraAIGCService.
+ (void)destory;

/// Gets the SDK version.
+ (NSString *)getSdkVersion;

/// Initializes
/// - Parameter config: The specified configuration of the AI service.
- (void)initialize:(AgoraAIGCConfigure *)config;

/// Gets a list of roles.
/// - Returns: A list of roles.if nil, no role. if not nil, list of role.
- (NSArray <AgoraAIGCAIRole *>* _Nullable)getRoles;

/// Sets the role with the specified ID.
/// - Parameter roleId: The role ID.
- (void)setRoleWithId:(NSString *)roleId;

/// Gets the current role.
/// - Returns: The current role.
- (AgoraAIGCAIRole * _Nullable)getCurrentRole;

/// Gets the support vendors for STT/LLM/TTS.
/// - Returns: A group object.
- (AgoraAIGCServiceVendorGroup * _Nullable)getServiceVendors;

/// Sets a vendor.
/// - Parameter vendor: The specified vendor.
/// - Returns: The code.
- (AgoraAIGCServiceCode)setServiceVendor:(AgoraAIGCServiceVendor *)vendor;

/// Sets a custom prompt.
/// - Parameter prompt: The custom prompt to be set.
/// - Returns: The code.
- (AgoraAIGCServiceCode)setPrompt:(NSString *)prompt;

/// start the service.
- (void)start;

/// stop the service.
- (void)stop;

/// Sets the STT mode.if not call this method, the default mode is `AgoraAIGCServiceSTTModeFreeStyle`.
- (AgoraAIGCServiceCode)setSTTMode:(AgoraAIGCServiceSTTMode)mode;

/// push the audio data to the STT as input
/// - Parameters:
///   - data: one frame audio data, pcm, sampleRates: 16000, channels:1, bits:16
///   - vad: a value indicated whether it is a vad type. must be `AgoraAIGCVadNone`.
///   - isLastFrame: a value indicated whether it is the last frame.if `sttMode` is `AgoraAIGCSTTModeControlled` and isLastframe is YES, it will be the end of the round.
- (AgoraAIGCServiceCode)pushSpeechDialogueWithData:(NSData *)data
                                               vad:(AgoraAIGCVad)vad
                                       isLastFrame:(BOOL)isLastFrame;

/// push the string data to the LLM as input
/// - Parameters:
///   - text: Text for dialogue.
///   - roundId: An unique string value indicate the round.if set nil, it was created by sdk internal.
- (AgoraAIGCServiceCode)pushTxtDialogue:(NSString *)text roundId:(NSString * _Nullable)roundId;

/// push the string data to the LLM as input
/// - Parameters:
///   - text: Text for tts's input.
///   - roundId: An unique string value indicate the round. if set nil, it was created by sdk internal.
- (AgoraAIGCServiceCode)pushTxtToTTS:(NSString *)text
                             roundId:(NSString * _Nullable)roundId;
/// send a llm request with messagesJsonArray
/// - Parameters:
///   - messagesJsonArray: the message content to request llm. if nil or empty, the current dialogue will be interrupted.
///   - extraInfoJson: extra information for request llm.
///   - roundId: An unique string value indicate the round.if set nil, it was created by sdk internal.
- (AgoraAIGCServiceCode)pushMessagesToLlm:(NSString *)messagesJsonArray
                            extraInfoJson:(NSString * _Nullable)extraInfoJson
                                  roundId:(NSString * _Nullable)roundId;

- (void)interrupt:(AgoraAIGCServiceInterruptMask)mask;

/// unavailable
- (instancetype)init __attribute__((unavailable("Use create instead")));
@end

NS_ASSUME_NONNULL_END
