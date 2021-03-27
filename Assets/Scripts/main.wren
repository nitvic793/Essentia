
import "math.vector" for Vec3
import "math" for Math
import "engine" for Entity, MyBehavior
import "meta" for Meta

var point = Vec3.new(1.0, 0.0, 0.0).normalize()
var point2 = Vec3.new(0.1, 0.5, 0.0).normalize()
var result = point.dot(point2)
var test = Meta.getModuleVariables("main")

// var sphere = Entity.new("SphereEntity")
// var pos = sphere.getPosition()
// System.print(pos.x)

System.print("Init Main")


class GameEngine {
  static xPos=(val){
    __xPos = val
  }

  static xPos {
    return __xPos
  }

  static init() {
    xPos = 0
    __sphere = Entity.new("SphereEntity3")
    __speed = 3.0
  }

  static update(deltaTime, elapsedTime) {
    var pos = __sphere.getPosition()
    
    xPos = 7 + Math.sin(elapsedTime) * __speed
    __sphere.setPosition(xPos, pos.y, 4.0)
    
    //System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}

