//
//  RtcManager.swift
//  RTEAIGCServiceDemo
//
//  Created by ZhouRui on 2024/6/19.
//

import AgoraRtcKit
import RTMTokenBuilder

protocol RtcManagerDelegate: NSObjectProtocol {
    func rtcManagerOnCreatedRenderView(view: UIView)
    func rtcManagerOnCaptureAudioFrame(data: Data, isLast: Bool)
    func rtcManagerOnDebug(text: String)
}

class RtcManager: NSObject {
    fileprivate var agoraKit: AgoraRtcEngineKit!
    weak var delegate: RtcManagerDelegate?
    var channelId: String = ""
    var uid: Int = 0
    fileprivate var isRecord = false
    private var soundQueue = Queue<Data>()
    fileprivate let logTag = "RtcManager"
    
    func initEngine() {
        let config = AgoraRtcEngineConfig()
        config.appId = Config.appId
        agoraKit = AgoraRtcEngineKit.sharedEngine(with: config, delegate: self)
        
        agoraKit.setChannelProfile(.liveBroadcasting)
        agoraKit.setClientRole(.broadcaster)
        agoraKit.enableAudioVolumeIndication(50, smooth: 3, reportVad: true)
        agoraKit.setAudioScenario(.gameStreaming)
        agoraKit.disableVideo()
        agoraKit.enableLocalVideo(false)
    }
    
    deinit {
        agoraKit.leaveChannel()
        print("RtcManager deinit")
    }
    
    func joinChannel() {
        channelId = generateDate()
        uid = Int.random(in: 1000...9999)
        let token = TokenBuilder.rtcToken2(Config.appId,
                                           appCertificate: Config.certificate,
                                           uid: Int32(uid),
                                           channelName: channelId)
        let option = AgoraRtcChannelMediaOptions()
        option.clientRoleType = .broadcaster
        option.publishMicrophoneTrack = true
        option.publishCameraTrack = false
        option.autoSubscribeAudio = false
        option.autoSubscribeVideo = false
        agoraKit.setAudioFrameDelegate(self)
        agoraKit.enableAudio()
        let ret = agoraKit.joinChannel(byToken: token,
                                       channelId: channelId,
                                       uid: UInt(uid),
                                       mediaOptions: option)
        if ret != 0 {
            let text = "joinChannel ret \(ret)"
            Log.errorText(text: text, tag: logTag)
        }
    }
    
    func startRecord() {
        isRecord = true
        agoraKit.enableLocalAudio(true)
    }
    
    func stopRecord() {
        agoraKit.enableLocalAudio(false)
        isRecord = false
        delegate?.rtcManagerOnCaptureAudioFrame(data: Data(), isLast: true)
    }
    
    func setPlayData(data: Data) {
        soundQueue.enqueue(data)
    }
    
    func generateDate() -> String {
        let date = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "YYYYMMddHHmmss"
        return formatter.string(from: date) + "\(UInt8.random(in: 0...99))"
    }
}

extension RtcManager: AgoraRtcEngineDelegate {
    func rtcEngine(_ engine: AgoraRtcEngineKit, didOccurError errorCode: AgoraErrorCode) {
        Log.errorText(text: "didOccurError \(errorCode)", tag: logTag)
    }
    
    func rtcEngine(_ engine: AgoraRtcEngineKit, didJoinedOfUid uid: UInt, elapsed: Int) {
        let text = "didJoinedOfUid \(uid)"
        Log.info(text: text, tag: logTag)
    }
    
    func rtcEngine(_ engine: AgoraRtcEngineKit, didJoinChannel channel: String, withUid uid: UInt, elapsed: Int) {
        let text = "didJoinChannel withUid \(uid)"
        Log.info(text: text, tag: logTag)
        engine.muteLocalAudioStream(true)
    }
}

extension RtcManager: AgoraAudioFrameDelegate {
    func onEarMonitoringAudioFrame(_ frame: AgoraAudioFrame) -> Bool {
        true
    }
    
    func getEarMonitoringAudioParams() -> AgoraAudioParams {
        let params = AgoraAudioParams ()
        params.sampleRate = 16000
        params.channel = 1
        params.mode = .readWrite
        params.samplesPerCall = 640
        return params
    }
    
    func getRecordAudioParams() -> AgoraAudioParams {
        let params = AgoraAudioParams()
        params.sampleRate = 16000
        params.channel = 1
        params.mode = .readWrite
        params.samplesPerCall = 640
        return params
    }
    
    func onRecordAudioFrame(_ frame: AgoraAudioFrame, channelId: String) -> Bool {
        if self.isRecord {
            let count = frame.samplesPerChannel * frame.channels * frame.bytesPerSample
            let data = Data(bytes: frame.buffer!, count: count)
            delegate?.rtcManagerOnCaptureAudioFrame(data: data, isLast: false)
        }
        return true
    }
    
    func onRecord(_ frame: AgoraAudioFrame, channelId: String) -> Bool {
        return true
    }
    
    func getPlaybackAudioParams() -> AgoraAudioParams {
        let params = AgoraAudioParams()
        params.sampleRate = 16000
        params.channel = 1
        params.mode = .readWrite
        params.samplesPerCall = 640
        return params
    }
    
    func getObservedAudioFramePosition() -> AgoraAudioFramePosition {
        return [.record, .playback]
    }
    
    func onPlaybackAudioFrame(_ frame: AgoraAudioFrame, channelId: String) -> Bool {
        if let data = soundQueue.dequeue() {
            data.withUnsafeBytes { rawBufferPointer in
                let rawPtr = rawBufferPointer.baseAddress!
                let bufferPtr = UnsafeMutableRawPointer(frame.buffer)
                bufferPtr?.copyMemory(from: rawPtr, byteCount: data.count)
            }
        }
        return true
    }
    
    func onMixedAudioFrame(_ frame: AgoraAudioFrame, channelId: String) -> Bool {
        return true
    }
    
    func getMixedAudioParams() -> AgoraAudioParams {
        let params = AgoraAudioParams()
        params.sampleRate = 16000
        params.channel = 1
        params.mode = .readWrite
        params.samplesPerCall = 640
        return params
    }
    
    func onPlaybackAudioFrame(beforeMixing frame: AgoraAudioFrame, channelId: String, uid: UInt) -> Bool {
        return true
    }
}
