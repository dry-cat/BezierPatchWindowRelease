//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  October, 2024
//
//  ------------------------
//  Homogeneous4.cpp
//  ------------------------
//
//  A minimal class for a 3D point in homogeneous coordinates
//
///////////////////////////////////////////////////

#ifndef HOMOGENEOUS4_H
#define HOMOGENEOUS4_H

#include <iostream>
#include "Point3.h"
#include "Vector3.h"

// the class - we will rely on POD for sending to GPU
class Homogeneous4
{ // Homogeneous4
public:
    // the coordinates
    float x, y, z, w;

    // constructors
    Homogeneous4();
    Homogeneous4(float X, float Y, float Z, float W = 1.0);
    Homogeneous4(const Point3 &other);
    Homogeneous4(const Vector3 &other);
    // Homogeneous4(const Homogeneous4 &other);

    // routine to get a point by perspective division
    Point3 Point() const
    { // Homogeneous4::Point()
        Point3 returnVal;

        // convert from Homogeneous coordinates to a Point
        returnVal.x = x / w;
        returnVal.y = y / w;
        returnVal.z = z / w;

        return returnVal;
    } // Homogeneous4::Point()

    // routine to get a vector by dropping w (assumed to be 0)
    Vector3 Vector() const
    { // Homogeneous4::Vector()
        Vector3 returnVal;

        // convert from Homogeneous coordinates to a Vector
        returnVal.x = x;
        returnVal.y = y;
        returnVal.z = z;

        return returnVal;
    } // Homogeneous4::Vector()

    Homogeneous4 modulate(Homogeneous4 b);

    // addition operator
    Homogeneous4 operator+(const Homogeneous4 &other) const
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
    inline Homogeneous4 operator-(const Homogeneous4 &other) const
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
    inline Homogeneous4 operator*(float factor) const
    { // Homogeneous4::operator *()
        Homogeneous4 returnVal;

        // scalar multiplication
        returnVal.x = x * factor;
        returnVal.y = y * factor;
        returnVal.z = z * factor;
        returnVal.w = w;

        return returnVal;
    } // Homogeneous4::operator *()


    // division operator
    Homogeneous4 operator/(float factor) const
    { // Homogeneous4::operator /()
        Homogeneous4 returnVal;

        // scalar division
        returnVal.x = x / factor;
        returnVal.y = y / factor;
        returnVal.z = z / factor;
        returnVal.w = w;

        return returnVal;
    } // Homogeneous4::operator /()

    // operator that allows us to use array indexing instead of variable names
    float &operator [] (const int index);
    const float &operator [] (const int index) const;

}; // Homogeneous4

// multiplication operator
inline Homogeneous4 operator*(float factor, const Homogeneous4 &right)
{ // operator *
    // scalar multiplication is commutative, so flip & return
    return right * factor;
} // operator *

// stream input
std::istream & operator >> (std::istream &inStream, Homogeneous4 &value);

// stream output
std::ostream & operator << (std::ostream &outStream, const Homogeneous4 &value);

#endif
