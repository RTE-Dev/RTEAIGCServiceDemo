//
//  AIGCObjects.h
//  GPT-Demo
//
//  Created by ZYP on 2023/10/7.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, AgoraAIGCServiceEvent) {
    AgoraAIGCServiceEventIdle,
    AgoraAIGCServiceEventInitialize,
    AgoraAIGCServiceEventStart,
    AgoraAIGCServiceEventStop,
    AgoraAIGCServiceEventDestory
};

typedef NS_ENUM(NSInteger, AgoraAIGCServiceCode) {
    AgoraAIGCServiceCodeSuccess = 0,
    AgoraAIGCServiceCodeErrorGeneral = -1,
    AgoraAIGCServiceCodeInitializeFail = 1,
    /** start (inernal send prompt fail) */
    AgoraAIGCServiceCodeStartFail = 2,
    /** LicenseExpire **/
    AgoraAIGCServiceCodeLicenseExpire = 5,
    /** Timeout */
    AgoraAIGCServiceCodeTimeout = 9,
    /** content filter */
    AgoraAIGCServiceCodeContentManagementPolicy = 10
};
typedef NS_ENUM(NSUInteger, AgoraAIGCHandleResult) {
    AgoraAIGCHandleResultContinue,
    AgoraAIGCHandleResultDiscard,
};

typedef NS_ENUM(NSUInteger, AgoraAIGCLanguage) {
    AgoraAIGCLanguageZH_CN,
    AgoraAIGCLanguageEN_US,
};

typedef NS_ENUM(NSInteger, AgoraAIGCVad) {
    AgoraAIGCVadUnknow = -1,
    AgoraAIGCVadMute = 0,
    AgoraAIGCVadNonMute = 1,
};

typedef NS_ENUM(NSUInteger, AgoraNoiseEnvironment) {
    /// relatively quiet
    AgoraNoiseEnvironmentQuiet = 0,
    /// normal
    AgoraNoiseEnvironmentNormal,
    /// noise
    AgoraNoiseEnvironmentNoise,
};

typedef NS_ENUM(NSUInteger, AgoraSpeechRecognitionCompletenessLevel) {
    /// low completeness
    AgoraSpeechRecognitionCompletenessLevelLow = 0,
    /// normal completeness
    AgoraSpeechRecognitionCompletenessLevelNormal,
    /// high completeness
    AgoraSpeechRecognitionCompletenessLevelHigh,
};

typedef NS_ENUM(UInt8, AgoraAIGCServiceSTTMode) {
    /** FreeStyle mode. it will automatically reconigize the end of speech by vad processor. it can be used for free speech.
     */
    AgoraAIGCServiceSTTModeFreeStyle,
    /** Controlled mode. it will not automatically reconigize the end of speech by vad processor. it can be used for controlled speech.
     */
    AgoraAIGCServiceSTTModeControlled
};

typedef NS_ENUM(UInt8, AgoraAIGCServiceSpeechState) {
    AgoraAIGCServiceSpeechStateStateStart,
    AgoraAIGCServiceSpeechStateStateEnd
};

typedef NS_OPTIONS(NSUInteger, AgoraAIGCServiceInterruptMask) {
    AgoraAIGCServiceInterruptMaskSTT = 1 << 0,
    AgoraAIGCServiceInterruptMaskLLM = 1 << 1,
    AgoraAIGCServiceInterruptMaskTTS = 1 << 2,
};

__attribute__((visibility("default")))
@interface AgoraAIGCSceneMode : NSObject
@property (nonatomic, assign, readonly)AgoraAIGCLanguage language;
@property (nonatomic, assign, readonly)NSUInteger speechFrameBits;
@property (nonatomic, assign, readonly)NSUInteger speechFrameSampleRates;
@property (nonatomic, assign, readonly)NSUInteger speechFrameChannels;
- (instancetype)initWithLanguage:(AgoraAIGCLanguage)language
                 speechFrameBits:(NSUInteger)speechFrameBits
          speechFrameSampleRates:(NSUInteger)speechFrameSampleRates
             speechFrameChannels:(NSUInteger)speechFrameChannels;
- (instancetype)init __attribute__((unavailable("Use initWithLanguage instead")));
@end

__attribute__((visibility("default")))
@interface AgoraAIGCConfigure : NSObject
@property (nonatomic, copy, readonly)NSString *appId;
@property (nonatomic, copy, readonly)NSString *rtmToken;
@property (nonatomic, copy, readonly)NSString *userId;
/// 是否开启多轮短时记忆 推荐值：YES
@property (nonatomic, assign, readonly)BOOL enableMultiTurnShortTermMemory;
/// 过滤语音识别结果字数长度 推荐值：3
@property (nonatomic, assign, readonly)NSUInteger speechRecognitionFiltersLength;
@property (nonatomic, strong, readonly)AgoraAIGCSceneMode *input;
@property (nonatomic, strong, readonly)AgoraAIGCSceneMode *output;
/// indicated wheather enable the console log
@property (nonatomic, assign, readonly)BOOL enableConsoleLog;
@property (nonatomic, assign, readonly)BOOL enableSaveLogToFile;
/// custom file path , sdk will use internal default path if seeting nil or empty string
@property (nonatomic, copy, readonly, nullable)NSString *logFilePath;
/** user name for aigc */
@property (nonatomic, copy, readonly)NSString *userName;
/** enable Idle tip, will invoke `onLLMResultWithRoundId` after 30s no input (`pushSpeechDialogueWithData`) */
@property (nonatomic, assign, readonly)BOOL enableChatIdleTip;
/// set the noise environment by your own environment
@property (nonatomic, assign, readonly) AgoraNoiseEnvironment noiseEnvironment;
/// completeness level of speech recognition sentences
@property (nonatomic, assign, readonly) AgoraSpeechRecognitionCompletenessLevel speechRecognitionCompletenessLevel;
- (instancetype)initWithAppId:(NSString *)appId
                     rtmToken:(NSString *)rtmToken
                       userId:(NSString *)userId
enableMultiTurnShortTermMemory:(BOOL)enableMultiTurnShortTermMemory
speechRecognitionFiltersLength:(NSUInteger)speechRecognitionFiltersLength
                        input:(AgoraAIGCSceneMode *)input
                       output:(AgoraAIGCSceneMode *)output
             enableConsoleLog:(BOOL)enableConsoleLog
          enableSaveLogToFile:(BOOL)enableSaveLogToFile
                     userName:(NSString *)userName
            enableChatIdleTip:(BOOL)enableChatIdleTip
                  logFilePath:(NSString * _Nullable)logFilePath
             noiseEnvironment:(AgoraNoiseEnvironment)noiseEnvironment
speechRecognitionCompletenessLevel:(AgoraSpeechRecognitionCompletenessLevel)speechRecognitionCompletenessLevel;
- (instancetype)initWithAppId:(NSString *)appId
                     rtmToken:(NSString *)rtmToken
                       userId:(NSString *)userId
enableMultiTurnShortTermMemory:(BOOL)enableMultiTurnShortTermMemory
speechRecognitionFiltersLength:(NSUInteger)speechRecognitionFiltersLength
                        input:(AgoraAIGCSceneMode *)input
                       output:(AgoraAIGCSceneMode *)output
                     userName:(NSString *)userName
                  logFilePath:(NSString * _Nullable)logFilePath
             noiseEnvironment:(AgoraNoiseEnvironment)noiseEnvironment
speechRecognitionCompletenessLevel:(AgoraSpeechRecognitionCompletenessLevel)speechRecognitionCompletenessLevel;
- (instancetype)init __attribute__((unavailable("Use initWithAppId instead")));
- (NSString *)description;
@end

__attribute__((visibility("default")))
@interface AgoraAIGCAIRole : NSObject
@property (nonatomic, copy, readonly)NSString *roleId;
@property (nonatomic, copy, readonly)NSString *roleName;
/// means description
@property (nonatomic, copy, readonly)NSString *desc;
@property (nonatomic, copy, readonly)NSString *profession;
/// ["male","female"]
@property (nonatomic, copy, readonly)NSString *gender;

- (instancetype)initWithRoleId:(NSString *)roleId
                      roleName:(NSString *)roleName
                          desc:(NSString *)desc
                    profession:(NSString *)profession
                        gender:(NSString *)gender;
@end

__attribute__((visibility("default")))
@interface AgoraAIGCBaseVendor : NSObject
@property (nonatomic, copy, nullable, readonly)NSString *accountInJson;
- (instancetype)initWithAccountInJson:(NSString * _Nullable)accountInJson;
- (NSDictionary * _Nullable)getAccountJsonDict;
@end

__attribute__((visibility("default")))
@interface AgoraAIGCSTTVendor : AgoraAIGCBaseVendor
@property (nonatomic, copy, readonly)NSString *Id;
@property (nonatomic, copy, readonly)NSString *vendorName;
- (instancetype)initWithId:(NSString *)Id vendorName:(NSString *)vendorName;
- (instancetype)init __attribute__((unavailable("Use initWithAccountInJson:Id:vendorName instead")));
- (instancetype)initWithAccountInJson:(NSString * _Nullable)accountInJson Id:(NSString *)Id;
@end

__attribute__((visibility("default")))
@interface AgoraAIGCLLMVendor : AgoraAIGCBaseVendor
@property (nonatomic, copy, readonly)NSString *Id;
@property (nonatomic, copy, readonly)NSString *vendorName;
@property (nonatomic, copy, readonly)NSString *model;
- (instancetype)initWithId:(NSString *)Id
                vendorName:(NSString *)vendorName
                     model:(NSString *)model;
- (instancetype)init __attribute__((unavailable("Use initWithAccountInJson:Id:vendorName:model instead")));
- (instancetype)initWithAccountInJson:(NSString * _Nullable)accountInJson
                                    Id:(NSString *)Id;
@end

__attribute__((visibility("default")))
@interface AgoraAIGCTTSVendor : AgoraAIGCBaseVendor
@property (nonatomic, copy, readonly)NSString *Id;
@property (nonatomic, copy, readonly)NSString *language;
@property (nonatomic, copy, readonly)NSString *voiceName;
@property (nonatomic, copy, readonly)NSString *voiceNameValue;
@property (nonatomic, copy, readonly)NSString *vendorName;
@property (nonatomic, copy, readonly)NSString *voiceStyle;
- (instancetype)initWithId:(NSString *)Id
                  language:(NSString *)language
                 voiceName:(NSString *)voiceName
            voiceNameValue:(NSString *)voiceNameValue
                vendorName:(NSString *)vendorName
                voiceStyle:(NSString *)voiceStyle;
- (instancetype)init __attribute__((unavailable("Use initWithAccountInJson instead")));
- (instancetype)initWithAccountInJson:(NSString * _Nullable)accountInJson
                                    Id:(NSString *)Id
                        voiceNameValue:(NSString *)voiceNameValue
                            voiceStyle:(NSString *)voiceStyle;
@end

__attribute__((visibility("default")))
@interface AgoraAIGCServiceVendorGroup : NSObject
@property (nonatomic, copy, readonly)NSArray <AgoraAIGCSTTVendor *>*stt;
@property (nonatomic, copy, readonly)NSArray <AgoraAIGCLLMVendor *>*llm;
@property (nonatomic, copy, readonly)NSArray <AgoraAIGCTTSVendor *>*tts;

- (instancetype)initWithStt:(NSArray <AgoraAIGCSTTVendor *>*)stt
                        llm:(NSArray <AgoraAIGCLLMVendor *>*)llm
                        tts:(NSArray <AgoraAIGCTTSVendor *>*)tts;
- (instancetype)init __attribute__((unavailable("Use initWithStt:llm:tts instead")));
@end

__attribute__((visibility("default")))
@interface AgoraAIGCServiceVendor : NSObject
@property (nonatomic, strong, readonly)AgoraAIGCSTTVendor *stt;
@property (nonatomic, strong, readonly)AgoraAIGCTTSVendor *tts;
@property (nonatomic, strong, readonly)AgoraAIGCLLMVendor *llm;
- (instancetype)initWithStt:(AgoraAIGCSTTVendor *)stt
                        llm:(AgoraAIGCLLMVendor *)llm
                        tts:(AgoraAIGCTTSVendor *)tts;
- (instancetype)init __attribute__((unavailable("Use initWithStt:llm:tts instead")));
@end

NS_ASSUME_NONNULL_END
