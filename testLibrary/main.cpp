//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

// system libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Point3.h"
#include "Matrix4.h"


// main routine
int main(int argc, char **argv)
    { // main()

    Point3 p{0.0f, 1.0f, 3.0f};
    Point3 q{1.0f, -1.0f, 2.0f};
    Point3 r{0.0f, 0.0f, 1.0f};

    Vector3 u{-1.0f, 2.0f, 3.0f};
    Vector3 v{1.0f, 2.0f, -1.0f};
    Vector3 w{-1.0f, 2.0f, 4.0f};

    Matrix4 m1{ { {
        { {1.0f, 0.0f, 3.0f, 0.0f} },
        { {1.0f, 9.0f, 2.0f, 0.0f} },
        { {4.0f, 3.0f, 6.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f, 1.0f} }
    } } };

    Matrix4 m2{ { {
        { {0.707f, 0.707f, 0.0f, 0.0f} },
        { {-0.707f, 0.707f, 0.0f, 0.0f} },
        { {0.0f, 0.0f, 1.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f, 1.0f} }
    } } };

    Matrix4 m3{ { {
        { {6.0f, 0.0f, 0.0f, 0.0f} },
        { {-1.0f, 4.0f, 0.0f, 0.0f} },
        { {4.0f, 2.0f, -3.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f, 1.0f} }
    } } };

    std::cout << "p: " << p << '\n';
    std::cout << "q: " << q << '\n';
    std::cout << "r: " << r << '\n';

    std::cout << "u: " << u << '\n';
    std::cout << "v: " << v << '\n';
    std::cout << "w: " << w << '\n';

    std::cout << "m1:\n" << m1 << '\n';
    std::cout << "m2:\n" << m2 << '\n';
    std::cout << "m3:\n" << m3 << '\n';

    auto result1 = p + u;
    std::cout << "p + v = " << result1 << '\n';

    auto result2 = q + m1*v;
    std::cout << "q + m1*v = " << result2 << '\n';

    auto result3 = r + m3*m2*w;
    std::cout << "r + m3*m2*w = " << result3 << '\n';

    auto result4 = m2.transpose();
    std::cout << "m2^-1 = m2.transpose() =\n" << result4 << '\n';

    auto result5 = u.dot(v);
    std::cout << "u1.dot(u2) = " << result5 << '\n';

    auto result6 = v.cross(w);
    std::cout << "v.cross(w) = " << result6 << '\n';

    std::cout << '\n';

    // construct orthornormal basis
    auto b1 = u.unit();
    auto b3 = b1.cross(v).unit();
    auto b2 = b3.cross(b1);
    std::cout << "b1 = " << b1 << '\n';
    std::cout << "b2 = " << b2 << '\n';
    std::cout << "b3 = " << b3 << '\n';
    } // main()
