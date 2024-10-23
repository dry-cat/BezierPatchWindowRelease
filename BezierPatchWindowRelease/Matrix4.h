//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  October, 2024
//
//  ------------------------
//  Matrix4.h
//  ------------------------
//  
//  A minimal class for a homogeneous 4x4 matrix
//  
///////////////////////////////////////////////////

// include guard
#ifndef MATRIX4_H
#define MATRIX4_H

#include <array>
#include <iostream>
#include "Vector3.h"
#include "Homogeneous4.h"

// forward declaration
class Matrix4;

#include "Quaternion.h"

// this allows us to get a matrix in the 
// column-major form preferred by OpenGL
class columnMajorMatrix
    { // class columnMajorMatrix
    public:
    float coordinates[16];
    }; // class columnMajorMatrix
    
// the class itself, stored in row-major form
class Matrix4
    { // Matrix4
    public:
    // the coordinates
    float coordinates[4][4];

    // constructor - default to the zero matrix
    Matrix4();
    // constructor - initialize the matrix with values
    Matrix4(const std::array<std::array<float, 4>, 4> &m);
    // copy constructor
    // Matrix4(const Matrix4 &other);
    
    // equality operator
    bool operator ==(const Matrix4 &other) const;

    // indexing - retrieves the beginning of a line
    // array indexing will then retrieve an element
    float * operator [](const int rowIndex);
    
    // similar routine for const pointers
    const float * operator [](const int rowIndex) const;

    // scalar operations
    // multiplication operator (no division operator)
    Matrix4 operator*(float factor) const
    { // operator *()
        // start with a zero matrix
        Matrix4 returnMatrix;

        // multiply by the factor
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                returnMatrix[row][col] = coordinates[row][col] * factor;

        // and return it
        return returnMatrix;
    } // operator *()

    // vector operations on homogeneous coordinates
    // multiplication is the only operator we use
    Homogeneous4 operator*(const Homogeneous4 &vector) const
    { // operator *()
        // get a zero-initialised vector
        Homogeneous4 productVector;

        // loop adding products
        for (int row = 0; row < 4; row++)
            for (int k = 0; k < 4; k++)
                productVector[row] += coordinates[row][k] * vector[k];

        // return the result
        return productVector;
    } // operator *()

    // and on Cartesian coordinates
    Point3 operator*(const Point3 &vector) const
    { // cartesian multiplication
        // convert to Homogeneous coords and multiply
        Homogeneous4 productVector = (*this) * Homogeneous4(vector);

        // then divide back through
        return productVector.Point();
    } // cartesian multiplication

    // and on vectors
    Vector3 operator*(const Vector3 &vector) const
    { // matrix-vector multiplication
        // convert to Homogeneous coords and multiply
        Homogeneous4 productVector = (*this) * Homogeneous4(vector);

        // drop w (assumed to be 0)
        return productVector.Vector();
    } // matrix-vector multiplication

    // matrix operations
    // addition operator
    Matrix4 operator+(const Matrix4 &other) const
    { // operator +()
        // start with a zero matrix
        Matrix4 sumMatrix;

        // loop adding products
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                sumMatrix[row][col] = coordinates[row][col] + other[row][col];

        // return the result
        return sumMatrix;
    } // operator +()

    // subtraction operator
    Matrix4 operator-(const Matrix4 &other) const
    { // operator -()
        // start with a zero matrix
        Matrix4 differenceMatrix;

        // loop subtracting products
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                differenceMatrix[row][col] = coordinates[row][col] - other[row][col];

        // return the result
        return differenceMatrix;
    } // operator -()

    // multiplication operator
    Matrix4 operator*(const Matrix4 &other) const
    { // operator *()
        // start with a zero matrix
        Matrix4 productMatrix;
        // This function is provided to give the correct result ...
        // ... on the OpenGL render widget in the app UI
        // loop, adding products
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++)
                for (int entry = 0; entry < 4; entry++)
                    productMatrix.coordinates[row][col] += coordinates[row][entry] * other.coordinates[entry][col];

        // return the result
        return productMatrix;
    } // operator *()
    
    // matrix transpose
    Matrix4 transpose() const;
    
    // returns a column-major array of 16 values
    // for use with OpenGL
    columnMajorMatrix columnMajor() const;

    // methods that set to particular matrices
    void SetZero();
    // the identity matrix
    void SetIdentity();
    void SetTranslation(const Vector3 &vector);
    void SetRotation(const Vector3 &axis, float theta);
    void SetScale(float xScale, float yScale, float zScale);
    }; // Matrix4

// scalar operations
// additional scalar multiplication operator
inline Matrix4 operator*(float factor, const Matrix4 &matrix)
{ // operator *()
    // since this is commutative, call the other version
    return matrix * factor;
} // operator *()

// stream input
std::istream & operator >> (std::istream &inStream, Matrix4 &value);

// stream output
std::ostream & operator << (std::ostream &outStream, const Matrix4 &value);
        
#endif
