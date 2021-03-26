
foreign class Vec3 {
  construct new(x, y, z) {}

  // Functions
  foreign normalize()
  foreign length()
  foreign dot(rhs)
  foreign cross(rhs)

  // Accessors
  foreign x
  foreign x=( rhs )
  foreign y
  foreign y=( rhs )
  foreign z
  foreign z=( rhs )
}