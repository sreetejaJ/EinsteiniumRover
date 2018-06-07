//
//  SKScene.swift
//  EERover
//
//  Created by Sreeteja Jonnada on 13/02/2018.
//  Copyright © 2018 Sreeteja Jonnada. All rights reserved.
//

import SpriteKit

class MySKScene: SKScene{
    
    let joystick = AnalogJoystick(diameter: 100, colors: (UIColor(red:0.2, green:0.4, blue:0.6, alpha:1), UIColor(red:0.651, green:0.813, blue:0.835, alpha:1)))
    
    override func didMove(to view: SKView) {
//        self.backgroundColor = UIColor(red:0, green:0.285, blue:0.361, alpha:1.0)
//        joystick.position = CGPoint(x: self.frame.maxX / 2, y: self.frame.maxY / 2)
//        addChild(joystick)
    }
    override func sceneDidLoad() {
        self.backgroundColor = UIColor(red:0, green:0.285, blue:0.361, alpha:1.0)
        joystick.position = CGPoint(x: self.frame.maxX / 2, y: self.frame.maxY / 2)
        addChild(joystick)
    }
}
