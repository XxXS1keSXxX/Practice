classDiagram
    class Point {
        -double x_
        -double y_
        +getX() double
        +getY() double
        +distanceTo(Point) double
    }

    class IIntersectable {
        <<interface>>
        +intersects(Shape) bool
    }

    class Shape {
        <<abstract>>
        -int id_
        #string color_
        +typeName() string*
        +area() double*
        +perimeter() double*
        +print() void*
        +toSVG() string*
        +clone() unique_ptr*
    }

    class OpenShape
    class ClosedShape
    class Line
    class Circle
    class Rectangle
    class Triangle
    class Registry
    class Factory

    Shape <|-- OpenShape
    Shape <|-- ClosedShape
    OpenShape <|-- Line
    ClosedShape <|-- Circle
    ClosedShape <|-- Rectangle
    ClosedShape <|-- Triangle

    IIntersectable <|-- Line
    IIntersectable <|-- ClosedShape

    Registry *-- Shape

    Line *-- Point
    Circle *-- Point
    Rectangle *-- Point
    Triangle *-- Point

    Registry o-- Shape
    Factory ..> Shape : creates