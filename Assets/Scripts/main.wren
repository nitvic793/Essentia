
import "math.utils" for Utils
import "meta" for Meta

var test = Meta.getModuleVariables("main")
System.print(test[0].type)

class GameEngine {
  static update(deltaTime, elapsedTime) {
    var result = Utils.test(5, 6)
    System.print("Test: %(result)")
    System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}

