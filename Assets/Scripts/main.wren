
import "math.vector" for Vec3
import "math.utils" for Utils
import "meta" for Meta

var point = Vec3.new(5.0, 6.0, 15.0)
var test = Meta.getModuleVariables("main")
System.print(point.x + point.z)

class GameEngine {
  static update(deltaTime, elapsedTime) {
    var result = Utils.test(5, 6)
    // System.print("Test: %(result)")
    // System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}

