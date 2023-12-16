cdef public struct Vehicle:
    int speed
    float power

cdef api int activate(Vehicle *v) except *:
    if v.speed >= 88 and v.power >= 1.21:
        print("Time travel achieved")
        return 1
    else:
        return 0