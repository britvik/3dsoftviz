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
	static QVector<osg::ref_ptr<Data::Node> > * CameraMath::getViewExtremes(osg::ref_ptr<osg::Camera> camera, QLinkedList<osg::ref_ptr<Data::Node> > * selectedCluster);
};
