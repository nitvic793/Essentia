
foreign class Entity {
    construct new(name) {}
    foreign getPosition()
    foreign setPosition(x, y, z)
}

class Engine {
}

class Behavior {
    update() {
        System.print("Base")
    }
}

class MyBehavior is Behavior {
    construct new(){}

}