
import "math.vector" for Vec3
import "math" for Math
import "engine" for Entity, Behavior
import "meta" for Meta

var point = Vec3.new(1.0, 0.0, 0.0).normalize()
var point2 = Vec3.new(0.1, 0.5, 0.0).normalize()
var result = point.dot(point2)
var test = Meta.getModuleVariables("main")

// var sphere = Entity.new("SphereEntity")
// var pos = sphere.getPosition()
//System.print(pos.x)

System.print(Behavior.new().update())

class Game {
  static xPos=(val){
    __xPos = val
  }

  static xPos {
    return __xPos
  }

  static init() {
    xPos = 0
    __sphere = Entity.get("SphereEntity3")
    __speed = 1.0
    __rotationSpeed = 20.0 // Degrees/second
    __piDiv180 = 3.14/180.0
  }

  static update(deltaTime, elapsedTime) {
    var pos = __sphere.position
    var up = Vec3.new(0,1,0).normalize()
    __sphere.rotate(__piDiv180 * elapsedTime * __rotationSpeed , up)
    //pos = pos + Vec3.new(1,1,1)
    var radius = 1
    pos.x = 7 + Math.sin(elapsedTime * __speed) * radius
    pos.z = 7 + -Math.cos(elapsedTime * __speed) * radius
    __sphere.position = pos
    
    //System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}

