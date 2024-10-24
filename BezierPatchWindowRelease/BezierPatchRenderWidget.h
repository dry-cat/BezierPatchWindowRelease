//////////////////////////////////////////////////////////////////////
//
//	University of Leeds
//	COMP 5812M Foundations of Modelling & Rendering
//	User Interface for Coursework
//
//	October, 2024
//
//  -----------------------------
//  Bezier Patch Render Widget
//  -----------------------------
//
//	Provides a widget that displays a fixed image
//	Assumes that the image will be edited (somehow) when Render() is called
//  
////////////////////////////////////////////////////////////////////////

// include guard
#ifndef BEZIER_PATCH_RENDER_WIDGET_H
#define BEZIER_PATCH_RENDER_WIDGET_H

#include <vector>

// include the relevant QT headers
#include <QOpenGLWidget>
#include <QMouseEvent>

// and include all of our own headers that we need
#include "ControlPoints.h"
#include "RenderParameters.h"
#include "RGBAImage.h"

static constexpr int N_PTS = 4;
using Homogeneous4Vector = std::array<Homogeneous4, N_PTS>;
using Homogeneous4x2 = std::array<Homogeneous4Vector, N_PTS>;

// class for a render widget with arcball linked to an external arcball widget
class BezierPatchRenderWidget : public QOpenGLWidget
    { // class BezierPatchRenderWidget
	Q_OBJECT
	private:	
    // the Bezier patch control points to be rendered
    ControlPoints *patchControlPoints;

	// the render parameters to use
	RenderParameters *renderParameters;

    // An image to use as a framebuffer ...
    // ... that we will set individual pixels to
	RGBAImage frameBuffer;

	public:
	// constructor
    BezierPatchRenderWidget
			(
            // the Bezier patch control points to show
            ControlPoints 		*newPatchControlPoints,
			// the render parameters to use
			RenderParameters 	*newRenderParameters,
			// parent widget in visual hierarchy
			QWidget 			*parent
			);
	
	// destructor
    ~BezierPatchRenderWidget();
			
	protected:
	// called when OpenGL context is set up
	void initializeGL();
	// called every time the widget is resized
	void resizeGL(int w, int h);
	// called every time the widget needs painting
	void paintGL();

	// mouse-handling
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

    private:

	Homogeneous4 CalcCubicBezierCurvePoint(float scalar,
		float coeffA, float coeffB, float coeffC, float coeffD,
		const Homogeneous4 &pointA, const Homogeneous4 &pointB,
		const Homogeneous4 &pointC, const Homogeneous4 &pointD);
	Homogeneous4 CalcBezierOrigin(float s, float t, const Homogeneous4x2 &pts);
	bool ShouldHomogeneousClip(const Homogeneous4 &coords);
	Point3 NDCSToDCS(Point3 point);
	void SetPixel(float px, float py, const RGBAValue &color);
	void DrawLine(const Homogeneous4 &A, const Homogeneous4 &B, const RGBAValue &color);

    void forceRepaint();

	signals:
	// these are general purpose signals, which scale the drag to 
	// the notional unit sphere and pass it to the controller for handling
	void BeginScaledDrag(int whichButton, float x, float y);
	// note that Continue & End assume the button has already been set
	void ContinueScaledDrag(float x, float y);
	void EndScaledDrag(float x, float y);

    }; // class BezierPatchRenderWidget

#endif
