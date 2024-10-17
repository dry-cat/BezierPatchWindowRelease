//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////


#include <math.h>
#include <thread>
#include <random>
#include <QTimer>
// include open mp for the advanced tasks
#include <omp.h>
#include <algorithm>

// include the header file
#include "BezierPatchRenderWidget.h"

#include <QElapsedTimer>
#include <GL/glut.h>
#include <GL/gl.h>

#include <chrono>
#include <thread>

//	Ken Shoemake's ArcBall
#include "ArcBall.h"

#define N_THREADS 16

// constructor
BezierPatchRenderWidget::BezierPatchRenderWidget
        (   
        // the Bezier patch control points to show
        ControlPoints       *newPatchControlPoints,
        // the render parameters to use
        RenderParameters    *newRenderParameters,
        // parent widget in visual hierarchy
        QWidget             *parent
        )
    // the : indicates a member initialiser list ...
    // ... it is good practice to use it where possible
    : 
    // start by calling inherited constructor with parent widget's pointer
    QOpenGLWidget(parent),
    // then store the pointers that were passed in
    patchControlPoints(newPatchControlPoints),
    renderParameters(newRenderParameters)
    { // constructor
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &BezierPatchRenderWidget::forceRepaint);
        timer->start(30);
    // leaves nothing to put into the constructor body
    } // constructor

void BezierPatchRenderWidget::forceRepaint(){
    update();
}
// destructor
BezierPatchRenderWidget::~BezierPatchRenderWidget()
    { // destructor
    // empty (for now)
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
    // and OpenGL cleanup is taken care of by Qt
    } // destructor                                                                 

// called when OpenGL context is set up
void BezierPatchRenderWidget::initializeGL()
    { // BezierPatchRenderWidget::initializeGL()
    } // BezierPatchRenderWidget::initializeGL()

// called every time the widget is resized
void BezierPatchRenderWidget::resizeGL(int w, int h)
    { // BezierPatchRenderWidget::resizeGL()
    // resize the render image
    frameBuffer.Resize(w, h);
    } // BezierPatchRenderWidget::resizeGL()


// TODO: This will cause a crash if the window is resized small enough
void BezierPatchRenderWidget::SetPixel(Homogeneous4 coords, const RGBAValue &color) {
        // std::cout << "coords start: " << coords << '\n';

        // std::cout << "modelviewMatrix: " << renderParameters->modelviewMatrix;
        renderParameters->modelviewMatrix.SetIdentity();

        renderParameters->modelviewMatrix = renderParameters->rotationMatrix * renderParameters->modelviewMatrix;
        renderParameters->modelviewMatrix[0][3] = renderParameters->xTranslate;
        renderParameters->modelviewMatrix[1][3] = renderParameters->yTranslate;
        renderParameters->modelviewMatrix[2][3] = renderParameters->zTranslate;

        // convert from model space to view space
        coords = renderParameters->modelviewMatrix * coords;

        // convert from view space to clipping space - projection matrix
        Matrix4 projMatrix;
        projMatrix.SetIdentity();

        // compute the aspect ratio of the widget
        float aspectRatio = static_cast<float>(renderParameters->windowWidth) /
                                static_cast<float>(renderParameters->windowHeight);

        // TODO: store this properly and only change it on renderParameters->triggerResize
        // TODO: explain
        auto SetProjMatrix = [&](float r, float t, float n, float f){
                projMatrix[0][0] = 1.0f / r;
                projMatrix[1][1] = 1.0f / t;
                projMatrix[2][2] = -2.0f / (f - n);
                projMatrix[2][3] = -(f + n)/(f - n);
        };

        float nearPlane = 0.01f;
        float farPlane = 200.0f;
        float scale = 10.0f / renderParameters->zTranslate;
        float adjustedScale = aspectRatio * 10.0f / renderParameters->zTranslate;
        if (renderParameters->orthoProjection) {
            if (aspectRatio > 1.0f) {
                SetProjMatrix(adjustedScale, scale, nearPlane, farPlane);
            } else {
                SetProjMatrix(scale, adjustedScale, nearPlane, farPlane);
            }
        } else {
            std::cerr << "Error perspective projection not implemented yet.";
            std::exit(EXIT_FAILURE);
        }

        coords = projMatrix * coords;
        // std::cout << "clip space coords: " << coords << '\n';

        // TODO: why doesn't z-clipping work?
        // perform per pixel clipping
        if (coords.x < -coords.w || coords.x > coords.w ||
            coords.y < -coords.w || coords.y > coords.w) {
            return;
        }

        // std::cout << "coords: " << coords << '\n';

        // convert from clipping space to NDCS - perspective division
        Point3 P = coords.Point();

        // std::cout << "P: " << P << '\n';

        auto width = static_cast<float>(frameBuffer.width);
        auto height = static_cast<float>(frameBuffer.height);
        // convert from NDCS to DCS - viewport transformation
        float N = 0.0f;
        float F = 1.0f;
        float X = 0.0f;
        float Y = 0.0f;
        Matrix4 viewportTransform{ { {
            { {width / 2.0f, 0.0f,          0.0f,           width / 2.0f} },
            { {0.0f,         height / 2.0f, 0.0f,           height / 2.0f} },
            { {0.0f,         0.0f,          (F - N) / 2.0f, (F + N) / 2.0f} },
            { {0.0f,         0.0f,          0.0f,           1.0f} }
        } } };

        coords = viewportTransform * Homogeneous4(P);

        auto x = static_cast<long>(coords.x);
        auto y = static_cast<long>(coords.y);

        frameBuffer[y][x] = color;
}

void BezierPatchRenderWidget::DrawLine(const Homogeneous4 &A, const Homogeneous4 &B, const RGBAValue &color) {
    for (float alpha = 0.0f; alpha <= 1.0f; alpha += 0.001f) {
        float beta = 1.0f - alpha;

        Homogeneous4 P = (alpha*A + beta*B);

        SetPixel(P, color);
    }
}



// called every time the widget needs painting
void BezierPatchRenderWidget::paintGL()
{ // BezierPatchRenderWidget::paintGL()

    // TODO:
    // To match the OpenGL widget, refer to "RenderWidget.{h/cpp}.
    // There you shall find the precise colours, positions, etc.
    // No additional OpenGL calls should be added here for drawing.
    // Make use of the renderParameters matrices for converting
    //  points from local coordinates to screen space.
    // The final screen space coordinates (row/column) should then be used
    //  to set the 2D frameBuffer array with the final colour.
    // (glDrawPixels then puts the frameBuffer on the screen
    //   to display the final image at the end of paintGL)

    // clear the (non-OpenGL) buffer where we will set pixels to:
    frameBuffer.clear(renderParameters->theClearColor);

    // now clear the OpenGL buffer:
    glClearColor(0.8, 0.8, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    Matrix4 identity_matrix;
    identity_matrix.SetIdentity();

    if(renderParameters->verticesEnabled)
    {// UI control for showing vertices
        for(int i = 0 ; i < static_cast<int>(patchControlPoints->vertices.size()); i++)
        {
            // draw each vertex as a point
            // (paint the active vertex in red, ...
            //  ... keep the others in white)
            RGBAValue color{};
            if (i == renderParameters->activeVertex) {
                color = RGBAValue{255.0f, 0.0f, 0.0f, 1.0f};
            } else {
                float intensity = 0.75f * 255.0f;
                color = RGBAValue{intensity, intensity, intensity, 255.0f};
            }

            // auto vertex = (*patchControlPoints).vertices[(i/4)*4+(i%4)];
            // std::cout << "vertex: " << vertex << '\n';

            // consider ways to make the rendered points bigger than just 1x1 pixel on the screen
        }
    }// UI control for showing vertices

    if(renderParameters->planesEnabled)
    {// UI control for showing axis-aligned planes

        // Planes are axis aligned grids made up of lines

        float quarterIntensity = 0.25f * 255.0f;

        // Draw the horizontal x axis plane (in purple)
        for (int i = -5; i <= 5; i+=2)
        {
            RGBAValue purple = {quarterIntensity, 0.0f, quarterIntensity, 255.0f};
            auto startA = Homogeneous4(i, 0, -5, 1);
            auto endA = Homogeneous4(i, 0, 5, 1);
            DrawLine(startA, endA, purple);
            auto startB = Homogeneous4(-5, 0, i, 1);
            auto endB = Homogeneous4(5, 0, i, 1);
            DrawLine(startB, endB, purple);
        }

        // Draw the vertical y axis plane (in blue)
        for (int i = -5; i <= 5; i+=2)
        {
            RGBAValue blue = {0.0f, quarterIntensity, quarterIntensity, 255.0f};
            auto startA = Homogeneous4(0, i, -5, 1);
            auto endA = Homogeneous4(0, i, 5, 1);
            DrawLine(startA, endA, blue);
            auto startB = Homogeneous4(0, -5, i, 1);
            auto endB = Homogeneous4(0, 5, i, 1);
            DrawLine(startB, endB, blue);
        }

        // Draw the flat plane (in brown)
        for (int i = -5; i <= 5; i++)
        {
            RGBAValue yellow = {quarterIntensity, quarterIntensity, 0.0f, 255.0f};
            auto startA = Homogeneous4(-5, i, 0, 1);
            auto endA = Homogeneous4(5, i, 0, 1);
            DrawLine(startA, endA, yellow);
            auto startB = Homogeneous4(i, -5, 0, 1);
            auto endB = Homogeneous4(i, 5, 0, 1);
            DrawLine(startB, endB, yellow);
        }

        // Refer to RenderWidget.cpp for the precise colours.

    }// UI control for showing axis-aligned planes

    if(renderParameters->netEnabled)
    {// UI control for showing the Bezier control net
     // (control points connected with lines)
    }// UI control for showing the Bezier control net

    // float quarterIntensity = 0.25f * 255.0f;
    // RGBAValue red = {quarterIntensity, 0.0f, 0.0f, 255.0f};
    // DrawCircle(Homogeneous4(0,0,0,1), 10.0f, red);

    if(renderParameters->bezierEnabled)
    {// UI control for showing the Bezier curve
        for (float s = 0.0; s <= 1.0; s += 0.01)
        {// s parameter loop

            for (float t = 0.0; t <= 1.0; t += 0.01)
            { // t parameter loop

                // set the pixel for this parameter value using s, t for colour
            } // t parameter loop
        } // s parameter loop
    }

    // Put the custom framebufer on the screen to display the image
    glDrawPixels(frameBuffer.width, frameBuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.block);

} // BezierPatchRenderWidget::paintGL()


// mouse-handling
void BezierPatchRenderWidget::mousePressEvent(QMouseEvent *event)
    { // BezierPatchRenderWidget::mousePressEvent()
    // store the button for future reference
    int whichButton = int(event->button());
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;

    
    // and we want to force mouse buttons to allow shift-click to be the same as right-click
    unsigned int modifiers = event->modifiers();
    
    // shift-click (any) counts as right click
    if (modifiers & Qt::ShiftModifier)
        whichButton = Qt::RightButton;
    
    // send signal to the controller for detailed processing
    emit BeginScaledDrag(whichButton, x,y);
    } // BezierPatchRenderWidget::mousePressEvent()
    
void BezierPatchRenderWidget::mouseMoveEvent(QMouseEvent *event)
    { // BezierPatchRenderWidget::mouseMoveEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit ContinueScaledDrag(x,y);
    } // BezierPatchRenderWidget::mouseMoveEvent()
    
void BezierPatchRenderWidget::mouseReleaseEvent(QMouseEvent *event)
    { // BezierPatchRenderWidget::mouseReleaseEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit EndScaledDrag(x,y);
    } // BezierPatchRenderWidget::mouseReleaseEvent()
