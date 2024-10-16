//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "Homogeneous4.h"
#include "math.h"
#include <iomanip>

// constructors
Homogeneous4::Homogeneous4() 
    : 
    x(0.0), 
    y(0.0), 
    z(0.0), 
    w(0.0)
    {}

Homogeneous4::Homogeneous4(float X, float Y, float Z, float W)
    : 
    x(X), 
    y(Y), 
    z(Z),
    w(W) 
    {}

Homogeneous4::Homogeneous4(const Point3 &other)
    :
    x(other.x),
    y(other.y),
    z(other.z),
    w(1)
    {}

Homogeneous4::Homogeneous4(const Vector3 &other)
    :
    x(other.x),
    y(other.y),
    z(other.z),
    w(0)
{}

Homogeneous4::Homogeneous4(const Homogeneous4 &other)
    : 
    x(other.x), 
    y(other.y), 
    z(other.z), 
    w(other.w)
    {}

// routine to get a point by perspective division
Point3 Homogeneous4::Point() const
    { // Homogeneous4::Point()
    Point3 returnVal;

    // convert from Homogeneous coordinates to a Point
    returnVal.x = x / w;
    returnVal.y = y / w;
    returnVal.z = z / w;

    return returnVal;
    } // Homogeneous4::Point()

// routine to get a vector by dropping w (assumed to be 0)
Vector3 Homogeneous4::Vector() const
    { // Homogeneous4::Vector()
    Vector3 returnVal;

    // convert from Homogeneous coordinates to a Vector
    returnVal.x = x;
    returnVal.y = y;
    returnVal.z = z;

    return returnVal;
    } // Homogeneous4::Vector()


// addition operator
Homogeneous4 Homogeneous4::operator +(const Homogeneous4 &other) const
    { // Homogeneous4::operator +()
    Homogeneous4 returnVal;

    // add
    // point + point = point
    if (w && other.w) {
        // allow point + point for linear combinations
        returnVal.x = x*other.w + other.x*w;
        returnVal.y = y*other.w + other.y*w;
        returnVal.z = z*other.w + other.z*w;
        returnVal.w = w * other.w;
    // point + vector = point
    } else if (w && !other.w) {
        returnVal.x = x + other.x*w;
        returnVal.y = y + other.y*w;
        returnVal.z = z + other.z*w;
        returnVal.w = w;
    // vector + vector = vector
    } else if (!w && !other.w) {
        returnVal.x = x + other.x;
        returnVal.y = y + other.y;
        returnVal.z = z + other.z;
    // do not allow vector + point
    } else {
        std::cerr << "Error adding: vector + point in Homogeneous4::operator+.";
        std::exit(EXIT_FAILURE);
    }

    return returnVal;
    } // Homogeneous4::operator +()

// subtraction operator
Homogeneous4 Homogeneous4::operator -(const Homogeneous4 &other) const
    { // Homogeneous4::operator -()
    Homogeneous4 returnVal;

    // subtract
    // point - point = vector
    if (w && other.w) {
        returnVal.x = x/w - other.x/other.w;
        returnVal.y = y/w - other.y/other.w;
        returnVal.z = z/w - other.z/other.w;
        returnVal.w = 0;
    // point - vector = point
    } else if (w && !other.w) {
        returnVal.x = x - other.x*w;
        returnVal.y = y - other.y*w;
        returnVal.z = z - other.z*w;
        returnVal.w = w;
    // vector - vector = vector
    } else if (!w && !other.w) {
        returnVal.x = x - other.x;
        returnVal.y = y - other.y;
        returnVal.z = z - other.z;
    // vector + point
    } else {
        std::cerr << "Error subtracting: vector - point in Homogeneous4::operator-.";
        std::exit(EXIT_FAILURE);
    }

    return returnVal;
    } // Homogeneous4::operator -()

// multiplication operator
Homogeneous4 Homogeneous4::operator *(float factor) const
    { // Homogeneous4::operator *()
    Homogeneous4 returnVal;

    // scalar multiplication
    returnVal.x = x * factor;
    returnVal.y = y * factor;
    returnVal.z = z * factor;
    // TODO: check if this is what we want
    // for linear combinations this makes sense but maybe for other things it
    // doesn't
    returnVal.w = w;

    return returnVal;
    } // Homogeneous4::operator *()

// division operator
Homogeneous4 Homogeneous4::operator /(float factor) const
    { // Homogeneous4::operator /()
    Homogeneous4 returnVal;

    // scalar division
    returnVal.x = x / factor;
    returnVal.y = y / factor;
    returnVal.z = z / factor;
    // TODO: check if this is what we want
    // for linear combinations this makes sense but maybe for other things it
    // doesn't
    returnVal.w = w;

    return returnVal;
    } // Homogeneous4::operator /()

// operator that allows us to use array indexing instead of variable names
float &Homogeneous4::operator [] (const int index)
    { // operator []
    // use default to catch out of range indices
    // we could throw an exception, but will just return the 0th element instead
    switch (index)
        { // switch on index
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        // actually the error case
        default:
            return x;       
        } // switch on index
    } // operator []

// operator that allows us to use array indexing instead of variable names
const float &Homogeneous4::operator [] (const int index) const
    { // operator []
    // use default to catch out of range indices
    // we could throw an exception, but will just return the 0th element instead
    switch (index)
        { // switch on index
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        // actually the error case
        default:
            return x;       
        } // switch on index
    } // operator []

// multiplication operator
Homogeneous4 operator *(float factor, const Homogeneous4 &right)
    { // operator *
    // scalar multiplication is commutative, so flip & return
    return right * factor;
    } // operator *

Homogeneous4 Homogeneous4::modulate(Homogeneous4 b)
    { // Homogeneous4::Vector()
        return Homogeneous4(x*b.x,y*b.y,z*b.z,w*b.w);
    } // Homogeneous4::Vector()

// stream input
std::istream & operator >> (std::istream &inStream, Homogeneous4 &value)
    { // stream output
    inStream >> value.x >> value.y >> value.z >> value.w;
    return inStream;
    } // stream output
        
// stream output
std::ostream & operator << (std::ostream &outStream, const Homogeneous4 &value)
    { // stream output
    outStream << std::setprecision(4) << value.x << " " << std::setprecision(4) << value.y << " " << std::setprecision(4) << value.z << " " << std::setprecision(4) << value.w;
    return outStream;
    } // stream output
        
