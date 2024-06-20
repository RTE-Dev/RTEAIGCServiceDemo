//
//  MainViewController.swift
//  RTEAIGCServiceDemo
//
//  Created by ZhouRui on 2024/6/19.
//

import UIKit
import AgoraAIGCService
import RTMTokenBuilder
import AgoraRtcKit

class MainViewController: UIViewController, AgoraAIGCServiceDelegate, RtcManagerDelegate, MainViewDelegate {
    let mainView = MainView()
    var service: AgoraAIGCService!
    private let rtcManager = RtcManager()
    private let config: Configurate
    
    init(config: Configurate) {
        self.config = config
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        title = config.sttProviderName + "+" + config.llmProviderName + "+" + config.ttsProviderName
        view.backgroundColor = .white
        view.addSubview(mainView)
        mainView.frame = view.bounds
        mainView.delegate = self
        
        let tapGesture = UITapGestureRecognizer(target: self, action: #selector(dismissKeyboard))
        view.addGestureRecognizer(tapGesture)
        
//        mainView.showTextField(show: !config.enableSTT)
        
        initRtc()
        initAIGC()
    }
    
    deinit {
//        service.stop()
        AgoraAIGCService.destory()
    }
    
    @objc func dismissKeyboard() {
        view.endEditing(true)
    }
    
    private func initRtc() {
        rtcManager.delegate = self
        rtcManager.initEngine()
        rtcManager.joinChannel()
    }
    
    private func initAIGC() {
        service = AgoraAIGCService.create()
        let uid = "123"
        let appId = Config.appId
        let cer = Config.certificate
        let token = TokenBuilder.buildRtmToken(appId,
                                               appCertificate: cer,
                                               userUuid: uid)
        
        // TODO： 初始化AIGCService
        
    }
    
    func setupRoleAndVendor() {
        let targetRoleId = config.roleId
        guard let roles = service.getRoles() else {
            fatalError("roles is nil")
        }
        guard roles.contains(where: { $0.roleId == targetRoleId }) else {
            fatalError("no target role")
        }
        
        // TODO： 设置role
        
        // TODO： 设置vendor
    }
    
    func findSpecificVendorGroup() -> AgoraAIGCServiceVendor {
        guard let vendors = service.getVendors() else {
            fatalError("getVendors ret nil")
        }
        
        var stt: AgoraAIGCSTTVendor?
        var llm: AgoraAIGCLLMVendor?
        var tts: AgoraAIGCTTSVendor?
        
        for vendor in vendors.stt {
            if vendor.id == config.sttProviderName {
                stt = vendor
                break
            }
        }
        
        for vendor in vendors.llm {
            if vendor.id == config.llmProviderName {
                llm = vendor
                break
            }
        }
        
        for vendor in vendors.tts {
            if vendor.vendorName == config.ttsProviderName {
                tts = vendor
                break
            }
        }
        let customJosnLLM = "{\"vendorName\":\"lingYi\",\"key\":\"\(Config.lingYi_key)\",\"reserved\":\"https://api.lingyiwanwu.com/v1/chat/completions\",\"secret\":\"\(Config.lingYi_model)\"}"
        let llmC = AgoraAIGCLLMVendor(accountInJson: customJosnLLM, id: "lingYi-yi-large")
        return AgoraAIGCServiceVendor(stt: stt!,
                                      llm: llmC,
                                      tts: tts!)
        
    }
    
    func pushMessageToAIGCService() {
        let messageArray: [[String: Any]] = [
            [
                "role": "user",
                "content": "你想去郊游吗？"
            ],
            [
                "role": "assistant",
                "content": "好啊！我们什么时候去？"
            ],
            [
                "role": "user",
                "content": "周日？"
            ]
        ]
        // TODO： 通过pushMessage接口发送历史聊天消息
        
    }
    
    // MARK: - AIGCServiceDelegate
    
    func onEventResult(with event: AgoraAIGCServiceEvent, code: AgoraAIGCServiceCode, message: String?) {
        DispatchQueue.main.async { [weak self] in
            guard let `self` = self else {
                return
            }
            
            if event == .initialize, code == .initializeFail {
                print("====initializeFail")
                return
            }
            
            if event == .initialize, code == .success {
                print("====initialize success")
                // TODO： 初始化成功后就可以获取role和vendor，并选好的设置给SDK
                
            }
            
        }
    }
    
    func onSpeech2Text(withRoundId roundId: String,
                       result: NSMutableString,
                       recognizedSpeech: Bool,
                       code: AgoraAIGCServiceCode) -> AgoraAIGCHandleResult {
        DispatchQueue.main.async { [weak self] in
            guard let `self` = self else {
                return
            }
            print("====onSpeech2Text:\(result) recognizedSpeech:\(recognizedSpeech)")
            let info = MainView.Info(uuid: "You" + roundId, content: result.copy() as! String)
            mainView.addOrUpdateInfo(info: info)
        }
        return config.enableSTT ? .continue : .discard
    }
    
    func onLLMResult(withRoundId roundId: String,
                     answer: NSMutableString,
                     isRoundEnd: Bool,
                     estimatedResponseTokens tokens: UInt,
                     code: AgoraAIGCServiceCode) -> AgoraAIGCHandleResult {
        DispatchQueue.main.async { [weak self] in
            guard let `self` = self else {
                return
            }
            let info = MainView.Info(uuid: "llm" + roundId, content: answer.copy() as! String)
            mainView.addOrUpdateInfo(info: info)
        }
        return config.enableTTS ? .continue : .discard
    }
    
    func onText2SpeechResult(withRoundId roundId: String,
                             voice: Data,
                             sampleRates: Int,
                             channels: Int,
                             bits: Int,
                             code: AgoraAIGCServiceCode) -> AgoraAIGCHandleResult {
        DispatchQueue.main.async { [weak self] in
            guard let `self` = self else {
                return
            }
            // TODO： 语音合成成功后，传给rtc就可以播放语音了
            
        }
        return .continue
    }
    
    func onSpeechStateChange(_ state: AgoraAIGCServiceSpeechState) {
        if state == .stateStart {
            service.interrupt(.TTS)
        }
    }
    
    // MARK: - MainViewDelegate
    func mainViewDidShouldSendText(text: String) {
        let info = MainView.Info(uuid: "\(UInt8.random(in: 0...200))", content: text)
        mainView.addOrUpdateInfo(info: info)
        service.pushTxtDialogue(text, roundId: nil)
        service.pushMessages(toLlm: "", extraInfoJson: nil, roundId: nil)
    }
    
    func mainViewDidTapAction(action: MainView.Action) {
        if action == .interrupt {
            service.interrupt(.TTS)
        } else if action == .start {
            service.start()
            rtcManager.startRecord()
        } else if action == .stop {
            service.stop()
        } else if action == .sendMsg {
            pushMessageToAIGCService()
        }
    }
    
    // MARK: - RtcManagerDelegate
    func rtcManagerOnCaptureAudioFrame(data: Data, isLast: Bool) {
        guard config.enableSTT else {
            return
        }
        DispatchQueue.main.async {
            self.service.pushSpeechDialogue(with: data, vad: .nonMute, isLastFrame: false)
        }
    }
    func rtcManagerOnVadUpdate(isSpeaking: Bool) {}
    func rtcManagerOnDebug(text: String) {}
    func rtcManagerOnCreatedRenderView(view: UIView) {}
}

extension MainViewController {
    struct Configurate {
        let sttProviderName: String
        let llmProviderName: String
        let ttsProviderName: String
        let roleId: String
        let inputLang:AgoraAIGCLanguage
        let outputLang:AgoraAIGCLanguage
        let userName: String
        let customPrompt: String?
        let enableMultiTurnShortTermMemory: Bool
        let speechRecognitionFiltersLength: UInt
        let enableSTT: Bool
        let enableTTS: Bool
        let noiseEnv: AgoraNoiseEnvironment
        let speechRecCompLevel: AgoraSpeechRecognitionCompletenessLevel
    }
}
