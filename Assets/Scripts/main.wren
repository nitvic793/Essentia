import "math.vector" for Vec3
import "math" for Math
import "engine" for Entity
import "meta" for Meta
import "components" for Rotatable
//import "rotationController" for RotationController

var point = Vec3.new(1.0, 0.0, 0.0).normalize()
var point2 = Vec3.new(0.1, 0.5, 0.0).normalize()
var result = point.dot(point2)

var test = Meta.getModuleVariables("engine")
System.print(test)

class Game {
  static init() {
    System.print("Init Main")
    __sphere = Entity.get("SphereEntity3")
    __speed = 1
    __rotationSpeed = 60.0 // Degrees/second
    __piDiv180 = 3.14/180.0
    __controllers = List.new()
  }

  static registerController(controllerMeta){
    var controller = controllerMeta.new()
    controller.init()
    __controllers.add(controller)
  }

  static update(deltaTime, elapsedTime) {
    for(controller in __controllers){
      controller.update(deltaTime, elapsedTime)
    }
  }
}

