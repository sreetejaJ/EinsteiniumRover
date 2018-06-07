//
//  ViewController.swift
//  EERover
//
//  Created by Sreeteja Jonnada on 12/02/2018.
//  Copyright © 2018 Sreeteja Jonnada. All rights reserved.
//

import UIKit
import SpriteKit
import SwiftSocket
import AVFoundation

class ViewController: UIViewController {

    @IBOutlet weak var Mod151: UILabel!
    @IBOutlet weak var Mod239: UILabel!
    @IBOutlet weak var IR: UILabel!
    @IBOutlet weak var Magnet: UILabel!
    @IBOutlet weak var Audio: UILabel!
    
    
    
    @IBOutlet weak var PredictedElement: UILabel!
    @IBOutlet weak var ConnectionStatus: UILabel!
    
    @IBOutlet weak var joyView: SKView!
    
    let transmit = UDPClient(address: "192.168.0.12", port: 2390)
    let receive =  UDPServer(address: "192.168.0.229", port: 2525)
    
    let sythn = AVSpeechSynthesizer()
    var myUtterance = AVSpeechUtterance(string: "")
    
    var confidenceVal = 70
    var lastElement = " "
    
    var scene: MySKScene!
    var newData = true
    
    let detectColour = UIColor(red: 0.847, green: 0.631, blue: 0.525, alpha: 1)
    let undetectColour = UIColor(red: 0.525, green: 0.733, blue: 0.847, alpha: 0.4)
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        scene = MySKScene(size: joyView.bounds.size)
        scene.scaleMode = .aspectFill
        joyView.presentScene(scene)
        PredictedElement.text = "Unknown"
        ConnectionStatus.text = "Not connected"
        _ = Timer.scheduledTimer(timeInterval: 0.2, target: self, selector: #selector(ViewController.ReceiveMove), userInfo: nil, repeats: true)
        
        _ = Timer.scheduledTimer(timeInterval: 0.1, target: self, selector: #selector(ViewController.SendMove), userInfo: nil, repeats: true)
    }
    
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @objc func SendMove(){
        let message = String(Int(scene.joystick.data.velocity.x)) + ":" + String(Int(scene.joystick.data.velocity.y))
        switch transmit.send(string: message) {
            case .success:
                if(newData){}
            default:
                ConnectionStatus.text = "Could Not Transmit"
        }
    }
    
    @objc func ReceiveMove(){
        DispatchQueue.global(qos: .userInitiated).async {
            let mess = self.receive.recv(512)
            if(mess.0 != nil){
                
                let string = String(bytes: mess.0!, encoding: .utf8)
                let array = string?.components(separatedBy: ":")
                DispatchQueue.main.async {
                    if(array != nil){
                        self.ConnectionStatus.text = "Connected"
                        self.SensorDisplay(array: array!)
                    }
                }
            }
        }
    }
    
    
    func SensorDisplay(array: [String]){
        //Radio Sensor
        var boolArray = [false, false, false, false, false, false]
        if(array[0] == "151"){
            self.Mod151.textColor = detectColour
            self.Mod239.textColor = undetectColour
            boolArray[0] = true
            boolArray[1] = false
        } else if(array[0] == "239"){
            self.Mod151.textColor = undetectColour
            self.Mod239.textColor = detectColour
            boolArray[0] = false
            boolArray[1] = true
        } else {
            self.Mod151.textColor = undetectColour
            self.Mod239.textColor = undetectColour
            boolArray[0] = false
            boolArray[1] = false
        }
        //IR Sensor
        if(array[1] == "1"){
            self.IR.textColor = detectColour
            boolArray[2] = true
        } else {
            self.IR.textColor = undetectColour
            boolArray[2] = false
        }
        //Audio Sensor
        if(array[2] == "1"){
            self.Audio.textColor = detectColour
            boolArray[3] = true
        } else {
            self.Audio.textColor = undetectColour
            boolArray[3] = false
        }
        //Magnetic Sensor
        if(array[3] == "1"){
            self.Magnet.textColor = detectColour
            boolArray[4] = true
        } else {
            self.Magnet.textColor = undetectColour
            boolArray[4] = false
        }
        
        self.identifyElement(array: boolArray)
    }
    // 0: 151Hz     1: 239Hz        2: IR       3: Audio        4: Magnet
    func identifyElement(array: [Bool?]){
        var element = lastElement
        if(array[2]!){
            if(array[0]! && array[3]!){
                element = "Gaborite"
                PredictedElement.text = element
            }
            else if(array[1]! && array[4]!){
                element = "Nucinkisite"
                PredictedElement.text = element
            }
            else if(array[0]! && array[4]!){
                element = "Durranium"
                PredictedElement.text = element
            }
            else if(array[1]! && !array[4]!){
                element = "Brookesite"
                PredictedElement.text = element
            }
            else if(array[2]! && array[3]!){
                element = "Yeatmanine"
                PredictedElement.text = element
            }
            else {
                element = "Cheungtium"
                PredictedElement.text = element
            }
        }
        if(element != lastElement){
            myUtterance = AVSpeechUtterance(string: element)
            sythn.speak(myUtterance)
        }
        lastElement = element
    }

}

