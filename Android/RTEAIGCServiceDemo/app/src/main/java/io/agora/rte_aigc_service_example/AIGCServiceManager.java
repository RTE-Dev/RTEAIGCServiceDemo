package io.agora.rte_aigc_service_example;

import android.content.Context;
import android.util.Log;

import io.agora.aigc.sdk.AIGCService;
import io.agora.aigc.sdk.AIGCServiceCallback;

public class AIGCServiceManager {
    private final static String TAG = "AIGCService-" + AIGCServiceManager.class.getSimpleName();
    private volatile static AIGCServiceManager mInstance = null;
    private AIGCService mAIGCService;

    private AIGCServiceManager() {

    }

    public static AIGCServiceManager getInstance() {
        if (mInstance == null) {
            synchronized (AIGCServiceManager.class) {
                if (mInstance == null) {
                    mInstance = new AIGCServiceManager();
                }
            }
        }
        return mInstance;
    }


    /**
     * 初始化AIGCService
     *
     * @param serviceCallback 回调
     * @param onContext       上下文
     */
    public void initAIGCService(AIGCServiceCallback serviceCallback, Context onContext) {
        Log.d(TAG, "initAIGCService");
        if (null == mAIGCService) {
            mAIGCService = AIGCService.create();
            Log.d(TAG, "create AIGCService version:" + AIGCService.getVersion());
        }
        // 初始化AIGCService

    }

    public AIGCService getAIGCService() {
        return mAIGCService;
    }


    public void destroy() {

        mAIGCService = null;
    }
}
