
#include <stdio.h>
#include <memory>
#include <vector>
#include "voice_activity_detector.h"
#define POOLSIZE 10
#define MAIN_SPEAKER_SIZE 50
#define AGC_SCALEDIFF32(A, B, C) \
  ((C) + ((B) >> 16) * (A) + (((0x0000FFFF & (B)) * (A)) >> 16))

namespace AgoraSTT {

class SpeakerJudge {
public:
    typedef struct{
      int32_t pool_100ms[POOLSIZE];
      int32_t frame_mx;
      int32_t capacitorFast;
      int32_t cnt;
    } EnvCal;
    typedef struct{
      float voiceProbThr;
      float rmsThr;
      float jointThr;
      float aggressive;
    } STT_Param;
    typedef struct {
            int index;
            int result_100[100];
    } SmoothPram;
public:
    SpeakerJudge();
    ~SpeakerJudge();
    int Initialize();
    int STT_EnvCal(EnvCal* stt, const int16_t* in_near, size_t num_bands, int32_t FS);
    int STT_NoiseJudge(EnvCal* stt, double rms, double frame_pitch_gain, double voiceProb);
    int VadCal(const int16_t* audio, size_t length, int sample_rate_hz);
    int Smooth(EnvCal* stt, int result, SmoothPram* param);
    int Process(const int16_t* in_near, size_t num_bands, int32_t FS);
    void STT_SetParam(STT_Param param);
    void STT_GetParam(STT_Param *param);
    STT_Param config;
private:
    const int kPoolSize = 200;
    SmoothPram envSmoth;
    SmoothPram noisyJudgeSmooth;
    int result_100[100] = {0};
    float rmsdb = -100;
    webrtc::VoiceActivityDetector vad_;
    EnvCal inst_;
    int index = 0;
};
}
