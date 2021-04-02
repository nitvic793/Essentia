
foreign class Entity {
    construct get(name) {}
    foreign getPosition()
    foreign setPosition(x, y, z)
    foreign position
    foreign position=(rhs)
}

class Engine {
}

class Behavior {
    update() {
        System.print("Base")
    }
}