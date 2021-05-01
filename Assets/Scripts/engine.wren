
foreign class Entity {
    construct get(name) {}
    foreign getPosition()
    foreign setPosition(x, y, z)
    foreign position
    foreign position=(rhs)
    foreign rotate(angles, axis)
}

class Engine {
    // controllers{
    //     return __controllers
    // }

    // static controllers=(controllerList){
    //     __controllers = controllerList
    // }

    // static registerController(controller){
    //     __controllers.add(controller.new())
    // }
}

class Controller {
    construct new(){

    }

    init(){

    }

    update(deltaTime, totalTime) {
        System.print("Base")
        return "Update"
    }
}