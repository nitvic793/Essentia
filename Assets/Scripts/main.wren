
import "math.vector" for Vec3
import "engine" for Entity
import "meta" for Meta

var point = Vec3.new(1.0, 0.0, 0.0).normalize()
var point2 = Vec3.new(0.1, 0.5, 0.0).normalize()
var result = point.dot(point2)
var test = Meta.getModuleVariables("main")

var sphere = Entity.new("SphereEntity")
var pos = sphere.getPosition()
//System.print(pos)

System.print(result)

class GameEngine {
  static update(deltaTime, elapsedTime) {
    // System.print("Test: %(result)")
    // System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}

