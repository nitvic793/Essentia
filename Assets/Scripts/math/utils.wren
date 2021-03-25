
class BaseTest {

}

class Utils is BaseTest {
    foreign static test(a, b)
}

var result = Utils.test(1, 2)

System.print(result)