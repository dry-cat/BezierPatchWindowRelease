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

    auto& projMatrix = renderParameters->projMatrix;

    // convert from view space to clipping space - projection matrix
    projMatrix.SetIdentity();

    // compute the aspect ratio of the widget
    float aspectRatio = static_cast<float>(renderParameters->windowWidth) /
                            static_cast<float>(renderParameters->windowHeight);

    // TODO: explain
    auto SetOrthoMatrix = [&projMatrix](float r, float t, float n, float f) {
        projMatrix[0][0] = 1.0f / r;
        projMatrix[1][1] = 1.0f / t;
        projMatrix[2][2] = -2.0f / (f - n);
        projMatrix[2][3] = -(f + n)/(f - n);
        projMatrix[3][3] = 1.0f;
    };

    auto SetPerspMatrix = [&projMatrix](float r, float t, float n, float f) {
        projMatrix[0][0] = n / r;
        projMatrix[1][1] = n / t;
        projMatrix[2][2] = -(f + n) / (f - n);
        projMatrix[2][3] = -2.0f * f * n / (f - n);
        projMatrix[3][2] = -1.0f;
        projMatrix[3][3] = 0.0f;
    };

    float nearPlane = 0.01f;
    float farPlane = 200.0f;
    if (renderParameters->orthoProjection) {
        float scale = 10.0f / renderParameters->zTranslate;
        float adjustedScale = aspectRatio * 10.0f / renderParameters->zTranslate;
        if (aspectRatio > 1.0f) {
            SetOrthoMatrix(adjustedScale, scale, nearPlane, farPlane);
        } else {
            SetOrthoMatrix(scale, adjustedScale, nearPlane, farPlane);
        }
    } else {
        float scale = 0.01f;
        float adjustedScale = aspectRatio * 0.01f;
        if (aspectRatio > 1.0f) {
            SetPerspMatrix(adjustedScale, scale, nearPlane, farPlane);
        } else {
            SetPerspMatrix(scale, adjustedScale, nearPlane, farPlane);
        }
    }
} // BezierPatchRenderWidget::resizeGL()


void BezierPatchRenderWidget::SetPixel(Homogeneous4 coords, const RGBAValue &color) {
        // convert from model space to view space to clipping space
        coords = renderParameters->projMatrix * renderParameters->modelviewMatrix * coords;

        // perform per pixel clipping
        if (coords.x < -coords.w || coords.x > coords.w ||
            coords.y < -coords.w || coords.y > coords.w ||
            coords.z < -coords.w || coords.z > coords.w) {
            return;
        }

        // convert from clipping space to NDCS - perspective division
        Point3 P = coords.Point();

        auto width = static_cast<float>(frameBuffer.width);
        auto height = static_cast<float>(frameBuffer.height);
        // convert from NDCS to DCS - viewport transformation
        float N = 0.0f;
        float F = 1.0f;
        float X = 0.0f;
        float Y = 0.0f;
        Matrix4 viewportTransform{ { {
            { {width / 2.0f, 0.0f,          0.0f,           X + width / 2.0f} },
            { {0.0f,         height / 2.0f, 0.0f,           Y + height / 2.0f} },
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

    // if the projection/ortho matrix checkbox has been toggled:
    if(renderParameters->triggerResizeCopy)
    {
        this->resizeGL(renderParameters->windowWidth, renderParameters->windowHeight);

        renderParameters->triggerResizeCopy = false;
    }

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

    renderParameters->modelviewMatrix.SetIdentity();

    renderParameters->modelviewMatrix = renderParameters->rotationMatrix * renderParameters->modelviewMatrix;

    renderParameters->modelviewMatrix[0][3] = renderParameters->xTranslate;
    renderParameters->modelviewMatrix[1][3] = renderParameters->yTranslate;
    // NOTE: hardcored translation of z - 9 to match RenderWidget
    renderParameters->modelviewMatrix[2][3] = renderParameters->zTranslate - 9;

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

    if(renderParameters->verticesEnabled)
    {// UI control for showing vertices
        for(int i = 0 ; i < 16; i++)
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

            Point3 vertex = patchControlPoints->vertices[(i/4)*4+(i%4)];
            double radius = 0.1;
            for (float phi = 0.0; phi < 2.f*PI; phi += PI / 30.0)
                for (float theta = 0.0; theta < 2.f*PI; theta += PI / 30.0)
                    SetPixel(
                        Homogeneous4(
                            vertex.x + radius * cos(phi) * cos(theta),
                            vertex.y + radius * cos(phi) * sin(theta),
                            vertex.z + radius * sin(phi),
                            1.0f),
                        color);

            // consider ways to make the rendered points bigger than just 1x1 pixel on the screen
        }
    }// UI control for showing vertices

    if(renderParameters->netEnabled)
    {// UI control for showing the Bezier control net
        // (control points connected with lines)
        // TODO: Refactor
        RGBAValue color = {0.0f, 1.0f * 255.0f, 0.0f, 255.0f};
        for (int i = 0; i < 4; i++) {
            auto A = Homogeneous4(patchControlPoints->vertices[i*4+0]);
            auto B = Homogeneous4(patchControlPoints->vertices[i*4+1]);
            auto C = Homogeneous4(patchControlPoints->vertices[i*4+2]);
            auto D = Homogeneous4(patchControlPoints->vertices[i*4+3]);
            DrawLine(A, B, color);
            DrawLine(B, C, color);
            DrawLine(C, D, color);
        }

        for (int j = 0; j < 4; j++) {
            auto A = Homogeneous4(patchControlPoints->vertices[0*4+j]);
            auto B = Homogeneous4(patchControlPoints->vertices[1*4+j]);
            auto C = Homogeneous4(patchControlPoints->vertices[2*4+j]);
            auto D = Homogeneous4(patchControlPoints->vertices[3*4+j]);
            DrawLine(A, B, color);
            DrawLine(B, C, color);
            DrawLine(C, D, color);
        }
    }// UI control for showing the Bezier control net

    static constexpr int N_PTS = 4;
    using Homogeneous4Vector = std::vector<Homogeneous4>;
    using Homogeneous4x2 = std::vector<Homogeneous4Vector>;
    using Homogeneous4x3 = std::vector<Homogeneous4x2>;
    Homogeneous4x3 bezPoints(N_PTS, Homogeneous4x2(N_PTS, Homogeneous4Vector(N_PTS)));

    // initialise diagonal
    auto it = std::begin(patchControlPoints->vertices);
    for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
            // std::cout << "i: " << N_PTS - 1 << " j: " << j << " k: " << k << '\n';
            bezPoints[N_PTS - 1][j][k] = *it;
            ++it;
        }
    }

    if(renderParameters->bezierEnabled)
    {// UI control for showing the Bezier curve
        for (float s = 0.0f; s <= 1.0f; s += 0.001f) {
            for (float t = 0.0f; t <= 1.0f; t += 0.001f)
            {
                for (int i = N_PTS - 2; i >= 0; i--) {
                    for (int k = 0; k <= i; k++) {
                        for (int j = 0; j <= i; j++) {
                            // std::cout << "i: " << i << " j: " << j << " k: " << k << '\n';
                            bezPoints[i][j][k] = (1 - s)*(1 - t)*bezPoints[i+1][j][k]
                                                    + s*(1 - t)*bezPoints[i+1][j+1][k]
                                                    + (1 - s)*t*bezPoints[i+1][j][k+1]
                                                    + s*t*bezPoints[i+1][j+1][k+1];
                            // std::cout << bezPoints[i][j][k] << '\n';
                        }
                    }
                }

                // set the pixel for this parameter value using s, t for colour
                RGBAValue color = {s*255.0f, 0.5f*255.0f, t*255.0f, 255.0f};
                Homogeneous4 vertex = bezPoints[0][0][0];
                // std::cout << vertex << '\n';
                SetPixel(vertex, color);
            }
        }
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
