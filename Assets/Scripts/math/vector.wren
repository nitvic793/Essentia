
foreign class Vec3 {
  construct new(x, y, z) {}

  // Functions
  foreign normalize()
  foreign length()
  foreign dot( rhs )
  foreign cross( rhs )
  foreign +( rhs )
  foreign -( rhs )

  // Accessors
  foreign x
  foreign x=( rhs )
  foreign y
  foreign y=( rhs )
  foreign z
  foreign z=( rhs )
  foreign set( rhs )
}


foreign class Vec4 {
  construct new(x, y, z, w) {}

  // Accessors
  foreign x
  foreign x=( rhs )
  foreign y
  foreign y=( rhs )
  foreign z
  foreign z=( rhs )
  foreign w
  foreign w=( rhs )
}