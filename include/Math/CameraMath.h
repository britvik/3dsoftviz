#pragma once

#include <osg/Vec3d>
#include <osg/ref_ptr>
#include <QVector>
#include <QLinkedList>
#include "Data/Node.h"

class CameraMath
{
public:
	static osg::Vec3d getPointOnNextBezierCurve(double time, QVector<osg::Vec3d> * points, double weights[]);
	static int factorial( int n );
	static QVector<osg::ref_ptr<Data::Node> > * getViewExtremes(osg::ref_ptr<osg::Camera> camera, QLinkedList<osg::ref_ptr<Data::Node> > * selectedCluster);
	static osg::Vec3d projectOnScreen(osg::ref_ptr<osg::Camera> camera, osg::Vec3d point);
	static osg::Vec3d getPointOnVector(osg::Vec3d p1, osg::Vec3d p2, float distance);
	static bool isInRect(osg::Vec3d point, float width, float height, float margin);
	static bool isInFOV(osg::Vec3d point, osg::ref_ptr<osg::Camera> camera);
};
