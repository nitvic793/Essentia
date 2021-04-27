
import "math.vector" for Vec3
import "math" for Math
import "engine" for Entity, Behavior
import "meta" for Meta
import "components" for Rotatable

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
    __speed = 1
    __rotationSpeed = 60.0 // Degrees/second
    __piDiv180 = 3.14/180.0
  }

  static update(deltaTime, elapsedTime) {
    var compCount = Rotatable.count()

    for (i in 0...compCount) {
      var sphere = Rotatable.getEntity(i)
      var pos = sphere.position
      var up = Vec3.new(0,1,0).normalize()
      sphere.rotate(__piDiv180 * elapsedTime * __rotationSpeed , up)
      //pos = pos + Vec3.new(1,1,1)
      var radius = 1
      pos.x = 7 + Math.sin(elapsedTime * __speed) * radius
      pos.z = 7 + -Math.cos(elapsedTime * __speed) * radius
      sphere.position = pos
      var rotatableComponent = Rotatable[0]
      System.print("Rotatable Comp[%(i)]: %(rotatableComponent.Rotation), Comp Count = %(compCount)")
    }
    //System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}

