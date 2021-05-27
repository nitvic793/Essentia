
class Component {
    construct get(entity){}
}

class Controller {
    construct new(){}

    init(){}

    update(deltaTime, totalTime) {
        System.print("Base")
        return "Update"
    }
}

class EntityController {
    construct new(){}
    init(){}
    update(deltaTime, totalTime, entity){}
}

foreign class Entity {
    construct get(name) {}
    foreign getPosition()
    foreign setPosition(x, y, z)
    foreign position
    foreign position=(rhs)
    foreign rotate(angles, axis)
    getComponent(componentMeta){
        return componentMeta.get(this)
    }
}

class Engine {
}

