
foreign class Entity {
    construct get(name) {}
    foreign getPosition()
    foreign setPosition(x, y, z)
    foreign position
    foreign position=(rhs)
    foreign rotate(angles, axis)
}

class Engine {
}

class Behavior {
    construct new(){

    }

    update() {
        System.print("Base")
        return "Update"
    }
}