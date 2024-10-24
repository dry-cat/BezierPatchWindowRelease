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

#define PRE_MULTIPLY_PROJ_MATRIX 1

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

    // calculate the projection matrix so we can convert from view space to clipping space
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

bool BezierPatchRenderWidget::ShouldHomogeneousClip(const Homogeneous4 &coords) {
    return coords.x <= -coords.w || coords.x >= coords.w ||
        coords.y <= -coords.w || coords.y >= coords.w ||
        coords.z <= -coords.w || coords.z >= coords.w;
}

Point3 BezierPatchRenderWidget::NDCSToDCS(Point3 point) {
    // convert from NDCS to DCS - viewport transformation
    const auto halfwidth = frameBuffer.width * 0.5f;
    const auto halfheight = frameBuffer.height * 0.5f;
    point.x = point.x * halfwidth + halfwidth;
    point.y = point.y * halfheight + halfheight;
    point.z = point.z * 0.5f + 0.5f;
    return point;
}

void BezierPatchRenderWidget::SetPixel(float px, float py, const RGBAValue& color) {
        // convert from model space to view space to clipping space
#if !PRE_MULTIPLY_PROJ_MATRIX
        coords = renderParameters->projMatrix * renderParameters->modelviewMatrix * coords;
#endif // !PRE_MULTIPLY_PROJ_MATRIX

        const auto x = static_cast<long>(px);
        const auto y = static_cast<long>(py);

        // in case clipping in homogeneous coordinates didn't catch out of
        // bounds values, due to floating point error, early return here.
        if (x < 0 || y < 0 || x >= frameBuffer.width || y >= frameBuffer.height) {
            return;
        }

        frameBuffer[y][x] = color;
}

void BezierPatchRenderWidget::DrawLine(const Homogeneous4 &A, const Homogeneous4 &B, const RGBAValue &color) {
    for (float alpha = 0.0f; alpha <= 1.0f; alpha += 0.001f) {
        float beta = 1.0f - alpha;

        Homogeneous4 coords = (alpha*A + beta*B);

#if PRE_MULTIPLY_PROJ_MATRIX
        coords = renderParameters->projMatrix * renderParameters->modelviewMatrix * coords;
#endif // PRE_MULTIPLY_PROJ_MATRIX

        if (!ShouldHomogeneousClip(coords)) {
            Point3 point = NDCSToDCS(coords.Point());
            SetPixel(point.x, point.y, color);
        }
    }
}

Homogeneous4 BezierPatchRenderWidget::CalcCubicBezierCurvePoint(
    float scalar,
    float coeffA, float coeffB, float coeffC, float coeffD,
    const Homogeneous4 &pointA, const Homogeneous4 &pointB,
    const Homogeneous4 &pointC, const Homogeneous4 &pointD) {
    return scalar * (coeffA*pointA + coeffB*pointB + coeffC*pointC + coeffD*pointD);
}

// TODO: rename function
Homogeneous4 BezierPatchRenderWidget::CalcBezierOrigin(
    float s, float t, const Homogeneous4x2 &pts) {
    const auto sPow2 = s*s;
    const auto sPow3 = s*sPow2;
    const auto complementS = 1 - s;
    const auto complementSPow2 = complementS*complementS;
    const auto complementSPow3 = complementS*complementSPow2;

    const auto tPow2 = t*t;
    const auto tPow3 = t*tPow2;
    const auto complementT = 1 - t;
    const auto complementTPow2 = complementT*complementT;
    const auto complementTPow3 = complementT*complementTPow2;

    const auto threeT_ComplementTPow2 = 3*t*complementTPow2;
    const auto threeTPow2_ComplementT = 3*tPow2*complementT;

    return CalcCubicBezierCurvePoint(complementSPow3,
                complementTPow3, threeT_ComplementTPow2,
                threeTPow2_ComplementT, tPow3,
                pts[0][0], pts[0][1], pts[0][2], pts[0][3])
            + CalcCubicBezierCurvePoint(3*s*complementSPow2,
                complementTPow3, threeT_ComplementTPow2,
                threeTPow2_ComplementT, tPow3,
                pts[1][0], pts[1][1], pts[1][2], pts[1][3])
            + CalcCubicBezierCurvePoint(3*sPow2*complementS,
                complementTPow3, threeT_ComplementTPow2,
                threeTPow2_ComplementT, tPow3,
                pts[2][0], pts[2][1], pts[2][2], pts[2][3])
            + CalcCubicBezierCurvePoint(sPow3,
                complementTPow3, threeT_ComplementTPow2,
                threeTPow2_ComplementT, tPow3,
                pts[3][0], pts[3][1], pts[3][2], pts[3][3]);
};

// called every time the widget needs painting
void BezierPatchRenderWidget::paintGL()
{ // BezierPatchRenderWidget::paintGL()

    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    auto perFrameTime1 = high_resolution_clock::now();

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

    // NOTE: hardcored translation of z - 9 to match RenderWidget
    Matrix4 translationMatrix{ { {
        { {1.0f, 0.0f, 0.0f, renderParameters->xTranslate} },
        { {0.0f, 1.0f, 0.0f, renderParameters->yTranslate} },
        { {0.0f, 0.0f, 1.0f, renderParameters->zTranslate - 9} },
        { {0.0f, 0.0f, 0.0f, 1.0f} }
    } } };
    renderParameters->modelviewMatrix = translationMatrix
                                            * renderParameters->rotationMatrix
                                            * renderParameters->modelviewMatrix;

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
            for (float phi = 0.0; phi < 2.f*PI; phi += PI / 30.0) {
                for (float theta = 0.0; theta < 2.f*PI; theta += PI / 30.0) {
                    Homogeneous4 coords =
    #if PRE_MULTIPLY_PROJ_MATRIX
                        renderParameters->projMatrix * renderParameters->modelviewMatrix *
    #endif // PRE_MULTIPLY_PROJ_MATRIX
                        Homogeneous4(vertex.x + radius * cos(phi) * cos(theta),
                                        vertex.y + radius * cos(phi) * sin(theta),
                                        vertex.z + radius * sin(phi), 1.0f);
                    if (!ShouldHomogeneousClip(coords)) {
                        Point3 point = NDCSToDCS(coords.Point());
                        SetPixel(point.x, point.y, color);
                    }
                }
            }

            // consider ways to make the rendered points bigger than just 1x1 pixel on the screen
        }
    }// UI control for showing vertices

    if(renderParameters->netEnabled)
    {// UI control for showing the Bezier control net
        // (control points connected with lines)
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

    // initialise diagonal
    Homogeneous4x2 pts{};
    auto it = std::begin(patchControlPoints->vertices);
    for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
            // std::cout << "i: " << N_PTS - 1 << " j: " << j << " k: " << k << '\n';
#if PRE_MULTIPLY_PROJ_MATRIX
            pts[j][k] = renderParameters->projMatrix * renderParameters->modelviewMatrix * *it;
#else // PRE_MULTIPLY_PROJ_MATRIX
            pts[j][k] = *it;
#endif // PRE_MULTIPLY_PROJ_MATRIX
            ++it;
        }
    }

    auto t1 = high_resolution_clock::now();
    // duration<double, std::milli> setPixelTime{};
    duration<double, std::milli> calcBezierPointTime{};

    struct Fragment {
        Point3 point;
        RGBAValue color;

        // Fragment(Point3 p, RGBAValue c) : point(std::move(p)), color(std::move(c)) {}
    };
    using FragmentVector = std::vector<Fragment>;
    std::vector<FragmentVector> fragments(N_THREADS);
    // std::vector<std::vector<FragmentVector>> fragments(
    //     N_THREADS,
    //     std::vector<FragmentVector>(frameBuffer.width*frameBuffer.height));

    if(renderParameters->bezierEnabled)
    {// UI control for showing the Bezier curve
        omp_set_num_threads(N_THREADS);
        #pragma omp parallel for collapse(2)
        for (int s = 0; s <= 1000; s++) {
            for (int t = 0; t <= 1000; t++) {
                const float alpha = s / 1000.0f;
                const float beta = t / 1000.0f;

                // auto setPixelT1 = high_resolution_clock::now();

                // set the pixel for this parameter value using s, t for colour
                const RGBAValue color = {alpha*255.0f, 0.5f*255.0f, beta*255.0f, 255.0f};
                // std::cout << point << '\n';
                // SetPixel(CalcBezierOrigin(alpha, beta), color);
                auto origin = CalcBezierOrigin(alpha, beta, pts);

                // convert from clipping space to NDCS - perspective division
                if (!ShouldHomogeneousClip(origin)) {
                    Point3 point = NDCSToDCS(origin.Point());
                    // Fragment frag{point, color};

                    fragments[omp_get_thread_num()].push_back({point, color});
                    // SetPixel(point.x, point.y, color);
                }
            }
        }
    }

    // // for (const auto& row : fragments) {
        for (const auto& row : fragments) {
            for (const auto& fragment : row) {
                // std::cout << fragment.point << '\n';
                SetPixel(fragment.point.x, fragment.point.y, fragment.color);
            }
        }
    // // }

    auto t2 = high_resolution_clock::now();

    auto perFrameTime2 = high_resolution_clock::now();

    duration<double, std::milli> perFrameTime = perFrameTime2 - perFrameTime1;
    std::cout << "per frame time ms: " << perFrameTime.count() << "ms\n";
    std::cout << "calc bezier point time ms: " << calcBezierPointTime.count() << "ms\n";
    // std::cout << "set pixel time ms: " << setPixelTime.count() << "ms\n";
    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "overall bezierEnabled ms: " << ms_double.count() << "ms\n";

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
