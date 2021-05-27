import "engine" for Controller
import "math.vector" for Vec3
import "math" for Math
import "engine" for Entity
import "meta" for Meta
import "components" for Rotatable

class RotationController is Controller {
    construct new(){}

    init(){
        _speed = 1
        _rotationSpeed = 40.0 // Degrees/second
        _piDiv180 = 3.14/180.0
        System.print("Init RotationController")
    }

    update(deltaTime, totalTime){
        var compCount = Rotatable.count()
        var entities = Rotatable.getEntities()
        var components = Rotatable.getComponents()
        for (i in 0...compCount) {
            var comp = components[i]
            var speed = comp.Speed
            var entity = entities[i]
            var pos = entity.position
            var up = Vec3.new(0,1,0).normalize()
            entity.rotate(_piDiv180 * totalTime * _rotationSpeed , up)

            var radius = 1
            pos.x = 7 + Math.sin(totalTime * speed) * radius
            pos.z = 7 + -Math.cos(totalTime * speed) * radius
            entity.position = pos

            var rotatableComponent = Rotatable[i]
            System.print("Rotatable Comp[%(i)]: %(comp.Rotation), Comp Count = %(compCount)")
        }
    }
}