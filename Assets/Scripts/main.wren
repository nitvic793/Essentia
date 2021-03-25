
import "math.utils" for Utils

class GameEngine {
  static update(deltaTime, elapsedTime) {
    var result = Utils.test(5, 6)
    System.print("Test: %(result)")
    System.print("Update Call: %(deltaTime), %(elapsedTime)")
  }
}