#pragma once
#ifndef __AGORA_UAP_MODULES_AED_H__
#define __AGORA_UAP_MODULES_AED_H__

#include <stdint.h>

#define AGORA_UAP_AED_VERSION (20230824)

#define AGORA_UAP_AED_MAX_FFT_SZ (1024)  // the max. fft-size supported by VAD module
#define AGORA_UAP_AED_MAX_NBINS ((AGORA_UAP_AED_MAX_FFT_SZ >> 1) + 1)

// #define AGORA_UAP_AED_FRAME_SIZE (160)    // assumed input frame length in sample
#define AGORA_UAP_AED_FS (16000)   // assumed input freq.

#define AGORA_UAP_AED_DEBUG_FLAG (0)

// Configuration Parameters, which impacts dynamic memory occupation, can only be set during
// allocation
typedef struct Aed_StaticCfg_ {
  int enableFlag;  // flag to enable or disable this module
  // 0: disable, o.w.: enable
  size_t fftSz;  // fft-size, only support: 128, 256, 512, 1024
  size_t hopSz;  // fft-Hop Size, will be used to check
  size_t anaWindowSz;  // fft-window Size, will be used to calc rms
  // AgoraRTC::AIVoiceActivityDetectorFactory* aivadFactory;
  // Jim: AI-VoiceActivity is not going to be implemented in plugin module
  int frqInputAvailableFlag;  // whether Aed_InputData will contain external freq. power-sepctra
  int useCVersionAIModule; // whether to use the C version of AI submodules
} Aed_StaticCfg;

// Configuraiton parameters which can be modified/set every frames
typedef struct Aed_DynamCfg_ {
  float extVoiceThr;        // threshold for ai based voice decision [0,1]
  float extMusicThr;        // threshold for ai based music decision [0,1]
  float extEnergyThr;       // threshold for energy based vad decision [0, ---]
  size_t resetFrameNum;     // frame number for aivad reset [1875, 75000]
  float pitchEstVoicedThr;  // threshold for pitch-estimator to output estimated pitch
} Aed_DynamCfg;

// SNsExtraInfo
typedef struct Aed_FrmCtrl_ {
  int trash;
} Aed_FrmCtrl;

// JIM_WORK_HOOK, we should change this interface to spectrum
//typedef struct Aed_InputData_ {
//  int16_t* nearinRawPcm;    // int16_t[inputSz] // externally provided buffer
//  size_t inputSz;           // input pcm length
//  size_t inputSampleRate;   // input pcm sample rate
//} Aed_InputData;

// Spectrum are assumed to be generated with time-domain samples in [-32768, 32767]
// with or without pre-emphasis operation
typedef struct Aed_InputData_ {
  const float* binPower;    // [NBins], power spectrum of 16KHz samples
  int nBins;
  const float* timeSignal;  // [hopSz]   // this frame's input signal, in [-32768, 32767]
  int hopSz;  // should be equal to StaticCfg->hopSz
} Aed_InputData;


// return data from statistical ns module
typedef struct Aed_OutputData_ {
  float frameRms;          // frame energy for input int16 data
  int energyVadRes;        // vad res 0/1 with extEnergyThreshold based on input frame energy
  float voiceProb;         // vad score [0,1]
  int vadRes;              // vad res 0/1 with extVoiceThr based on ai method, t + 16ms res correspond to the t input
  float musicProb;         // music score [0,1]
  int mdRes;               // music detect result 0/1 with extMusicThr based on ai method, t + 16ms res correspond to the t input
  float pitchFreq;         // estimated pitch freq.
} Aed_OutputData;

// counters to be reported to cloud
typedef struct Aed_Counter_ {
  int trash;

} Aed_Counter;

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Agora_UAP_AED_Create(...)
 *
 * This function creats a state handler from nothing, which is NOT ready for
 * processing
 *
 * Input:
 *
 * Output:
 *      - stPtr         : buffer to store the returned state handler
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_create(void** stPtr);

/****************************************************************************
 * Agora_UAP_AED_Destroy(...)
 *
 * destroy VAD instance, and releasing all the dynamically allocated memory
 * ATTENTION!!! this interface will also release ainsFactory, which was
 * created externally and passed to VAD module through memAllocate interface
 *
 * Input:
 *      - stPtr         : buffer of State Handler, after this method, this
 *                        handler won't be usable anymore
 *
 * Output:
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_destroy(void** stPtr);

/****************************************************************************
 * Agora_UAP_AED_MemAllocate(...)
 *
 * This function sets Static Config params and does memory allocation
 * operation, will lose the dynamCfg values
 *
 * Input:
 *      - stPtr         : State Handler which was returned by _create
 *      - pCfg          : static configuration parameters
 *
 * Output:
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_memAllocate(void* stPtr, const Aed_StaticCfg* pCfg);

/****************************************************************************
 * Agora_UAP_AED_init(...)
 *
 * This function resets (initialize) the VAD module and gets it prepared for
 * processing
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate
 *
 * Output:
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_init(void* stPtr);

/****************************************************************************
 * Agora_UAP_AED_setDynamCfg(...)
 *
 * This function set dynamic (per-frame variable) configuration
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate
 *      - pCfg          : configuration content
 *
 * Output:
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_setDynamCfg(void* stPtr, const Aed_DynamCfg* pCfg);

/****************************************************************************
 * Agora_UAP_AED_getStaticCfg(...)
 *
 * This function get static configuration status from VAD module
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate
 *
 * Output:
 *      - pCfg          : configuration content
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_getStaticCfg(const void* stPtr, Aed_StaticCfg* pCfg);


/****************************************************************************
 * Agora_UAP_AED_getDefaultStaticCfg(...)
 *
 * This function get default value of static configuration status from VAD module
 *
 * Input:
 *
 * Output:
 *      - pCfg          : configuration content
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_getDefaultStaticCfg(Aed_StaticCfg* pCfg);



/****************************************************************************
 * Agora_UAP_AED_getDynamCfg(...)
 *
 * This function get dynamic (per-frame variable) configuration status from
 * VAD module
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate
 *
 * Output:
 *      - pCfg          : configuration content
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_getDynamCfg(const void* stPtr, Aed_DynamCfg* pCfg);

/****************************************************************************
 * Agora_UAP_AED_getDefaultDynamCfg(...)
 *
 * This function get default value of dynamic and frame-ctrl configuration
 * from VAD-Stat module
 *
 * Input:
 *
 * Output:
 *      - frmCtrlPtr    : Handler of frame-ctrl structure (can be null)
 *      - pDynamCfg     : Handler of dynamic-config structure (can be null)
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_getDefaultDynamCfg(Aed_FrmCtrl* frmCtrlPtr, Aed_DynamCfg* pDynamCfg);

/****************************************************************************
 * Agora_UAP_AED_getAlgDelay(...)
 *
 * This function get algorithm delay from VAD module
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate
 *
 * Output:
 *      - delayInFrms   : algorithm delay in terms of frames
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_getAlgDelay(const void* stPtr, int* delayInFrms);

/****************************************************************************
 * Agora_UAP_AED_getCounter(...)
 *
 * This function get reporting counters from VAD module
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate and reset
 *
 * Output:
 *      - pCounter      : pointer to externally provided counter buffer
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_getCounter(const void* stPtr, Aed_Counter* pCounter);

/****************************************************************************
 * Agora_UAP_AED_proc(...)
 *
 * process a single frame
 *
 * Input:
 *      - stPtr         : State Handler which has gone through create and
 *                        memAllocate and reset
 *      - pCtrl         : per-frame variable control parameters
 *      - pIn           : input data stream
 *
 * Output:
 *      - pOut          : output data (mask, highband time-domain gain etc.)
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int Agora_UAP_AED_proc(void* stPtr, const Aed_FrmCtrl* pCtrl,
                       const Aed_InputData* pIn, Aed_OutputData* pOut);

#ifdef __cplusplus
}
#endif

#endif


