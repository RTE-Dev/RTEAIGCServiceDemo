package io.agora.rte_aigc_service_example;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.recyclerview.widget.LinearLayoutManager;

import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import io.agora.aigc.sdk.AIGCServiceCallback;
import io.agora.aigc.sdk.constants.Constants;
import io.agora.aigc.sdk.constants.HandleResult;
import io.agora.aigc.sdk.constants.STTMode;
import io.agora.aigc.sdk.constants.ServiceCode;
import io.agora.aigc.sdk.constants.ServiceEvent;
import io.agora.aigc.sdk.constants.SpeechState;
import io.agora.aigc.sdk.constants.Vad;
import io.agora.aigc.sdk.model.Data;
import io.agora.aigc.sdk.model.LLMVendor;
import io.agora.aigc.sdk.model.ServiceVendor;
import io.agora.aigc.sdk.utils.RingBuffer;
import io.agora.aigc.sdk.utils.Utils;
import io.agora.rtc2.ChannelMediaOptions;
import io.agora.rtc2.IAudioFrameObserver;
import io.agora.rtc2.IRtcEngineEventHandler;
import io.agora.rtc2.RtcEngine;
import io.agora.rtc2.RtcEngineConfig;
import io.agora.rtc2.audio.AudioParams;
import io.agora.rte_aigc_service_example.databinding.AiRobotActivityBinding;


public class AIRobotActivity extends Activity implements AIGCServiceCallback, IAudioFrameObserver {
    private final String TAG = "AIGCService-" + AIRobotActivity.class.getSimpleName();
    private AiRobotActivityBinding binding;

    private boolean mIsSpeaking = false;
    private ExecutorService mExecutorService;
    private RtcEngine mRtcEngine;
    private RingBuffer mSpeechRingBuffer;
    private boolean mRingBufferReady;
    private String mPreTtsRoundId;
    private HistoryListAdapter mAiHistoryListAdapter;
    private List<HistoryModel> mHistoryDataList;
    private SimpleDateFormat mSdf;
    private boolean mAIGCServiceStarted;
    private boolean mJoinChannelSuccess;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = AiRobotActivityBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        enableUi(false);
        initData();
    }


    private void initData() {
        mExecutorService = new ThreadPoolExecutor(1, 1,
                0, TimeUnit.SECONDS,
                new LinkedBlockingDeque<>(), Executors.defaultThreadFactory(), new ThreadPoolExecutor.AbortPolicy());

        if (null == mSpeechRingBuffer) {
            mSpeechRingBuffer = new RingBuffer(1024 * 1024 * 5);
        } else {
            mSpeechRingBuffer.clear();
        }
        mPreTtsRoundId = "";
        mSdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault());
        if (null == mHistoryDataList) {
            mHistoryDataList = new ArrayList<>();
        } else {
            mHistoryDataList.clear();
        }

        mAIGCServiceStarted = false;
        mJoinChannelSuccess = false;
    }

    public void initRtc(Context context) {
        if (mRtcEngine == null) {
            try {
                RtcEngineConfig rtcEngineConfig = new RtcEngineConfig();
                rtcEngineConfig.mContext = context;
                rtcEngineConfig.mAppId = KeyCenter.APP_ID;
                rtcEngineConfig.mChannelProfile = io.agora.rtc2.Constants.CHANNEL_PROFILE_LIVE_BROADCASTING;
                rtcEngineConfig.mEventHandler = new IRtcEngineEventHandler() {
                    @Override
                    public void onJoinChannelSuccess(String channel, int uid, int elapsed) {
                        Log.i(TAG, "onJoinChannelSuccess channel:" + channel + " uid:" + uid + " elapsed:" + elapsed);
                        mJoinChannelSuccess = true;
                        mRtcEngine.registerAudioFrameObserver(AIRobotActivity.this);
                        mRtcEngine.muteLocalAudioStream(true);
                    }

                    @Override
                    public void onLeaveChannel(RtcStats stats) {
                        Log.i(TAG, "onLeaveChannel stats:" + stats);
                        mJoinChannelSuccess = false;
                        AIGCServiceManager.getInstance().destroy();
                    }

                    @Override
                    public void onUserOffline(int uid, int reason) {
                        Log.i(TAG, "onUserOffline uid:" + uid + " reason:" + reason);
                    }

                    @Override
                    public void onUserJoined(int uid, int elapsed) {
                        Log.i(TAG, "onUserJoined uid:" + uid + " elapsed:" + elapsed);
                    }

                };
                rtcEngineConfig.mAudioScenario = io.agora.rtc2.Constants.AUDIO_SCENARIO_GAME_STREAMING;
                mRtcEngine = RtcEngine.create(rtcEngineConfig);

                mRtcEngine.setParameters("{\"rtc.enable_debug_log\":true}");

                mRtcEngine.setParameters("{\n" +
                        "\n" +
                        "\"che.audio.enable.nsng\":true,\n" +
                        "\"che.audio.ains_mode\":2,\n" +
                        "\"che.audio.ns.mode\":2,\n" +
                        "\"che.audio.nsng.lowerBound\":80,\n" +
                        "\"che.audio.nsng.lowerMask\":50,\n" +
                        "\"che.audio.nsng.statisticalbound\":5,\n" +
                        "\"che.audio.nsng.finallowermask\":30\n" +
                        "}");

                mRtcEngine.setParameters("{\"che.audio.adm_android_mode\":9}");


                mRtcEngine.setAudioProfile(
                        io.agora.rtc2.Constants.AUDIO_PROFILE_DEFAULT, io.agora.rtc2.Constants.AUDIO_SCENARIO_GAME_STREAMING
                );



                String channelId = Utils.getCurrentDateStr("yyyyMMddHHmmss") + Utils.getRandomString(2);
                //加入rtc频道

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void initAIGCService() {
        AIGCServiceManager.getInstance().initAIGCService(this, getApplicationContext());
    }


    @Override
    protected void onResume() {
        super.onResume();
        initView();
        handlePermission();
        checkKeys();

    }

    private void handlePermission() {

        // 需要动态申请的权限
        String permission = Manifest.permission.RECORD_AUDIO;

        //查看是否已有权限
        int checkSelfPermission = ActivityCompat.checkSelfPermission(getApplicationContext(), permission);

        if (checkSelfPermission == PackageManager.PERMISSION_GRANTED) {
            //已经获取到权限  获取用户媒体资源

        } else {

            //没有拿到权限  是否需要在第二次请求权限的情况下
            // 先自定义弹框说明 同意后在请求系统权限(就是是否需要自定义DialogActivity)
            if (ActivityCompat.shouldShowRequestPermissionRationale(this, permission)) {

            } else {
                appRequestPermission();
            }
        }

    }

    private void appRequestPermission() {
        String[] permissions = new String[]{
                Manifest.permission.RECORD_AUDIO,
        };
        requestPermissions(permissions, 1);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {

        }
    }

    private void checkKeys() {
        if (!KeyCenter.validKeys()) {
            Toast.makeText(AIRobotActivity.this, "请输入正确账号信息！", Toast.LENGTH_LONG).show();
            binding.btnSpeak.setEnabled(false);
            enableUi(false);
        } else {
            initRtc(getApplicationContext());
            initAIGCService();
        }
    }

    private void enableUi(boolean enable) {
        binding.btnSpeak.setEnabled(enable);
        binding.btnSendTextLlm.setEnabled(enable);
        binding.btnSendMessagesLlm.setEnabled(enable);
    }

    private void initView() {
        if (mAiHistoryListAdapter == null) {
            mAiHistoryListAdapter = new HistoryListAdapter(getApplicationContext(), mHistoryDataList);
            binding.aiHistoryList.setAdapter(mAiHistoryListAdapter);
            binding.aiHistoryList.setLayoutManager(new LinearLayoutManager(getApplicationContext()));
            binding.aiHistoryList.addItemDecoration(new HistoryListAdapter.SpacesItemDecoration(10));
        } else {
            mHistoryDataList.clear();
            mAiHistoryListAdapter.notifyDataSetChanged();
        }

        binding.btnSpeak.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mIsSpeaking) {
                    mIsSpeaking = false;
                    binding.btnSpeak.setText(AIRobotActivity.this.getResources().getString(R.string.start_speak));
                    binding.btnExit.setEnabled(true);
                    //关闭语聊

                } else {
                    mIsSpeaking = true;
                    mSpeechRingBuffer.clear();
                    mRingBufferReady = false;
                    binding.btnSpeak.setText(AIRobotActivity.this.getResources().getString(R.string.end_speak));
                    binding.btnExit.setEnabled(false);
                    //开启语聊

                }
            }
        });


        binding.btnExit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                enableUi(false);
                exit();
            }
        });

        binding.btnSendTextLlm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mAIGCServiceStarted) {
                    Toast.makeText(AIRobotActivity.this, "请先开启语聊", Toast.LENGTH_LONG).show();
                    return;
                }
                if (TextUtils.isEmpty(binding.etTextLlm.getText().toString())) {
                    Toast.makeText(AIRobotActivity.this, "请输入文本", Toast.LENGTH_LONG).show();
                    return;
                }
                //中断当前播放，请求大模型处理

                String roundId = Utils.getSessionId();
                updateHistoryList(roundId, "用户发言：", binding.etTextLlm.getText().toString(), false);
            }
        });

        binding.btnSendMessagesLlm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mAIGCServiceStarted) {
                    Toast.makeText(AIRobotActivity.this, "请先开启语聊", Toast.LENGTH_LONG).show();
                    return;
                }
                //中断当前播放，请求大模型处理

            }
        });
    }

    public boolean updateRoleSpeak(boolean isSpeak) {
        int ret = io.agora.rtc2.Constants.ERR_OK;
        ret += mRtcEngine.updateChannelMediaOptions(new ChannelMediaOptions() {{
            publishMicrophoneTrack = isSpeak;
        }});
        return ret == io.agora.rtc2.Constants.ERR_OK;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

    }

    private void exit() {
        if (mJoinChannelSuccess) {
            mRtcEngine.leaveChannel();
            RtcEngine.destroy();
        } else if (mAIGCServiceStarted) {
            RtcEngine.destroy();
            AIGCServiceManager.getInstance().destroy();
        } else {
            finish();
        }

    }

    private byte[] getSpeechVoiceData(int length) {
        byte[] bytes = null;
        if (mRingBufferReady) {
            bytes = getSpeechBuffer(length);
        } else {
            //wait length*WAIT_AUDIO_FRAME_COUNT data
            if (mSpeechRingBuffer.size() >= length * 3) {
                mRingBufferReady = true;
                bytes = getSpeechBuffer(length);
            }
        }
        return bytes;
    }

    private byte[] getSpeechBuffer(int length) {
        if (mSpeechRingBuffer.size() < length) {
            return null;
        }
        Object o;
        byte[] bytes = new byte[length];
        for (int i = 0; i < length; i++) {
            o = mSpeechRingBuffer.take();
            if (null == o) {
                return null;
            }
            bytes[i] = (byte) o;
        }
        return bytes;
    }

    @Override
    public void onEventResult(@NonNull ServiceEvent event, @NonNull ServiceCode code, @Nullable String msg) {
        Log.i(TAG, "onEventResult event:" + event + " code:" + code + " msg:" + msg);
        if (event == ServiceEvent.INITIALIZE && code == ServiceCode.SUCCESS) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    enableUi(true);
                }
            });
            //设置AIGCService角色和vendor信息

        } else if (event == ServiceEvent.START && code == ServiceCode.SUCCESS) {
            mAIGCServiceStarted = true;
        } else if (event == ServiceEvent.STOP && code == ServiceCode.SUCCESS) {
            mAIGCServiceStarted = false;
        } else if (event == ServiceEvent.DESTROY && code == ServiceCode.SUCCESS) {
            finish();
        }
    }

    @Override
    public HandleResult onSpeech2TextResult(String roundId, Data<String> result, boolean isRecognizedSpeech, ServiceCode code) {
        Log.i(TAG, "onSpeech2TextResult roundId:" + roundId + " result:" + result + " isRecognizedSpeech:" + isRecognizedSpeech + " code:" + code);
        updateHistoryList(roundId + "stt", "用户发言：", result.getData(), false);
        return HandleResult.CONTINUE;
    }


    @Override
    public HandleResult onLLMResult(String roundId, Data<String> answer, boolean isRoundEnd, int estimatedResponseTokens, ServiceCode code) {
        Log.i(TAG, "onLLMResult roundId:" + roundId + " answer:" + answer + " isRoundEnd:" + isRoundEnd + " estimatedResponseTokens:" + estimatedResponseTokens + " code:" + code);
        if (ServiceCode.SUCCESS == code) {
            updateHistoryList(roundId + "llm", "AI说：", answer.getData(), true);
            return HandleResult.CONTINUE;
        } else {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(AIRobotActivity.this, code.getMessage(), Toast.LENGTH_LONG).show();
                }
            });
            return HandleResult.DISCARD;
        }
    }


    @Override
    public HandleResult onText2SpeechResult(String roundId, Data<byte[]> voice, int sampleRates, int channels, int bits, ServiceCode code) {
        Log.i(TAG, "onText2SpeechResult roundId:" + roundId + " voice:" + voice + " sampleRates:" + sampleRates + " channels:" + channels + " bits:" + bits + " code:" + code);
        if (!mPreTtsRoundId.equalsIgnoreCase(roundId)) {
            mSpeechRingBuffer.clear();
            mRingBufferReady = false;
        }
        mPreTtsRoundId = roundId;

        //播放TTS音频


        return HandleResult.CONTINUE;
    }

    @Override
    public void onSpeechStateChange(SpeechState state) {
        Log.i(TAG, "onSpeechStateChange state:" + state);
        //根据说话人状态，判断是否中断当前播放

    }

    @Override
    public boolean onRecordAudioFrame(String channelId, int type, int samplesPerChannel,
                                      int bytesPerSample, int channels, int samplesPerSec, ByteBuffer buffer,
                                      long renderTimeMs, int avsync_type) {
        if (mIsSpeaking && mAIGCServiceStarted) {
            //从rtc获取原始音频pcm内容，推送到AIGCService

        }
        return false;
    }

    @Override
    public boolean onPlaybackAudioFrame(String channelId, int type, int samplesPerChannel,
                                        int bytesPerSample, int channels, int samplesPerSec, ByteBuffer buffer,
                                        long renderTimeMs, int avsync_type) {
        if (mIsSpeaking) {
            //从AIGCService获取TTS音频pcm内容，推送到rtc

        }
        return true;
    }

    @Override
    public boolean onMixedAudioFrame(String channelId, int type, int samplesPerChannel, int bytesPerSample, int channels, int samplesPerSec, ByteBuffer buffer, long renderTimeMs, int avsync_type) {
        return false;
    }

    @Override
    public boolean onEarMonitoringAudioFrame(int type, int samplesPerChannel, int bytesPerSample, int channels, int samplesPerSec, ByteBuffer buffer, long renderTimeMs, int avsync_type) {
        return false;
    }

    @Override
    public boolean onPlaybackAudioFrameBeforeMixing(String channelId, int userId, int type, int samplesPerChannel, int bytesPerSample, int channels, int samplesPerSec, ByteBuffer buffer, long renderTimeMs, int avsync_type, int rtpTimestamp) {
        return false;
    }

    @Override
    public int getObservedAudioFramePosition() {
        return 0;
    }

    @Override
    public AudioParams getRecordAudioParams() {
        return null;
    }

    @Override
    public AudioParams getPlaybackAudioParams() {
        return null;
    }

    @Override
    public AudioParams getMixedAudioParams() {
        return null;
    }

    @Override
    public AudioParams getEarMonitoringAudioParams() {
        return null;
    }

    private void updateHistoryList(String sid, String title, String message, boolean isAppend) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                String date = mSdf.format(System.currentTimeMillis());

                boolean isNewLineMessage = true;
                int updateIndex = -1;
                if (!TextUtils.isEmpty(sid)) {
                    for (HistoryModel historyModel : mHistoryDataList) {
                        updateIndex++;
                        if (sid.equals(historyModel.getSid())) {
                            if (isAppend) {
                                historyModel.setMessage(historyModel.getMessage() + message);
                            } else {
                                historyModel.setMessage(message);
                            }
                            historyModel.setTitle(title);
                            isNewLineMessage = false;
                            break;
                        }
                    }
                }

                if (isNewLineMessage) {
                    HistoryModel aiHistoryModel = new HistoryModel();
                    aiHistoryModel.setDate(date);
                    aiHistoryModel.setTitle(title);
                    aiHistoryModel.setSid(sid);
                    aiHistoryModel.setMessage(message);
                    mHistoryDataList.add(aiHistoryModel);
                    if (null != mAiHistoryListAdapter) {
                        mAiHistoryListAdapter.notifyItemInserted(mHistoryDataList.size() - 1);
                        binding.aiHistoryList.scrollToPosition(mAiHistoryListAdapter.getDataList().size() - 1);
                    }
                } else {
                    if (null != mAiHistoryListAdapter) {
                        mAiHistoryListAdapter.notifyItemChanged(updateIndex);
                        binding.aiHistoryList.scrollToPosition(mAiHistoryListAdapter.getDataList().size() - 1);
                    }
                }
            }
        });
    }
}
