import "engine" for Component

#!component=true
foreign class Rotatable is Component {
    foreign Speed=(speed)
    foreign Speed
    foreign Rotation=(rotation)
    foreign Rotation
    foreign static [index]
    foreign static count()
    foreign static getEntity(index) 
    foreign static getEntities()
    foreign static getComponents()
    construct get(entity) {}
}