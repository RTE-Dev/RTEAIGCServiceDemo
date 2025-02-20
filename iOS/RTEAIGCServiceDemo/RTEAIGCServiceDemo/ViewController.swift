//
//  ViewController.swift
//  RTEAIGCServiceDemo
//
//  Created by ZhouRui on 2024/6/19.
//

import UIKit
import AgoraAIGCService

class ViewController: UIViewController {
    
    struct Section {
        let title: String
        let rows: [Row]
    }
    
    struct Row {
        let title: String
    }

    let tableview = UITableView(frame: .zero, style: .grouped)
    var list = [Section]()

    override func viewDidLoad() {
        super.viewDidLoad()
        createData()
        setupUI()
        commonInit()
    }

    func setupUI() {
        view.addSubview(tableview)
        tableview.translatesAutoresizingMaskIntoConstraints = false

        tableview.leftAnchor.constraint(equalTo: view.leftAnchor).isActive = true
        tableview.topAnchor.constraint(equalTo: view.topAnchor).isActive = true
        tableview.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
        tableview.rightAnchor.constraint(equalTo: view.rightAnchor).isActive = true
    }

    func commonInit() {
        tableview.register(UITableViewCell.self, forCellReuseIdentifier: "cell")
        tableview.dataSource = self
        tableview.delegate = self
        tableview.reloadData()
    }

    func createData() {
        list = [Section(title: "AIGC", rows: [.init(title: "测试demo")]),
                /*Section(title: "KTV", rows: [.init(title: "")]),*/
        ]
    }
}

extension ViewController: UITableViewDelegate, UITableViewDataSource {
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        list[section].title
    }

    func numberOfSections(in tableView: UITableView) -> Int {
        list.count
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return list[section].rows.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableview.dequeueReusableCell(withIdentifier: "cell", for: indexPath)
        let item = list[indexPath.section].rows[indexPath.row]
        cell.textLabel?.text = item.title
        cell.accessoryType = .disclosureIndicator
        return cell
    }

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        
        if indexPath.section == 0 {
            if indexPath.row == 0 {
                let config = MainViewController.Configurate(sttProviderName: "xunfei",
                                                            llmProviderName: "lingYi-yi-large",
                                                            ttsProviderName: "microsoft",
                                                            roleId: "ai_lingyi_yunibobo",
                                                            inputLang: .ZH_CN,
                                                            outputLang: .ZH_CN,
                                                            userName: "小李",
                                                            customPrompt: nil,
                                                            enableMultiTurnShortTermMemory: true,
                                                            speechRecognitionFiltersLength:3,
                                                            enableSTT: true,
                                                            enableTTS: true,
                                                            noiseEnv: .noise,
                                                            speechRecCompLevel: .normal)
                let vc = MainViewController(config: config)
                navigationController?.pushViewController(vc, animated: true)
                return
            }
        }
        
        if indexPath.section == 1 {
            
        }
        
    }
}
