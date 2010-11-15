#include "Math/CameraMath.h"
#include <osg/Camera>

osg::Vec3d CameraMath::getPointOnNextBezierCurve(double time, QVector<osg::Vec3d> * points, double weights[])
{
	int n = points->size() - 1;
	double denominator = 0;
	double x, y, z;

	x = y = z = 0;

	for (int i = 0; i <= n; i++)
	{
		double x_0 = factorial(n)/(factorial(i) * factorial(n - i));
		double x_1 = pow(time, i);
		double x_2 = pow(1 - time, n - i) * weights[i];	

		double numerator = x_0 * x_1 * x_2;

		x += numerator * points->at(i).x();
		y += numerator * points->at(i).y();
		z += numerator * points->at(i).z();
	}

	for (int i = 0; i <= n; i++)
	{
		double x_0 = factorial(n)/(factorial(i) * factorial(n - i));
		double x_1 = pow(time, i);
		double x_2 = pow(1 - time, n - i) * weights[i];	

		denominator += x_0 * x_1 * x_2;
	}

	x /= denominator;
	y /= denominator;
	z /= denominator;

	return osg::Vec3d(x, y, z);
}

int CameraMath::factorial( int n )
{
	if( n <= 1 )     // base case
		return 1;
	else
		return n * factorial( n - 1 );
}

QVector<osg::ref_ptr<Data::Node> > * CameraMath::getViewExtremes(osg::ref_ptr<osg::Camera> camera, QLinkedList<osg::ref_ptr<Data::Node> > * selectedCluster)
{
	osg::Matrixd mv = camera->getViewMatrix();
	osg::Matrixd mp = camera->getProjectionMatrix();
	osg::Matrixd mw = camera->getViewport()->computeWindowMatrix();
	
	QVector<osg::ref_ptr<Data::Node> > * extremes = new QVector<osg::ref_ptr<Data::Node> >;

	osg::Vec3d leftPosition, rightPosition, topPosition, bottomPosition;

	for (int x = 0; x < 4; x++)
	{
		extremes->push_back(selectedCluster->first());
	}

	osg::Vec3d p = selectedCluster->first()->getCurrentPosition();

	leftPosition = rightPosition = topPosition = bottomPosition = p * mv * mp * mw;

	QLinkedList<osg::ref_ptr<Data::Node> >::iterator i;

	for (i = selectedCluster->begin(); i != selectedCluster->end(); ++i)
	{
		osg::Vec3d position = (*i)->getCurrentPosition() * mv * mp * mw;

		if (position.x() < leftPosition.x())
		{
			extremes->replace(0, *i);
			leftPosition = position;
		}

		if (position.x() > rightPosition.x())
		{
			extremes->replace(1, *i);
			rightPosition = position;
		}

		if (position.y() < topPosition.y())
		{
			extremes->replace(2, *i);
			topPosition = position;
		}

		if (position.y() > bottomPosition.y())
		{
			extremes->replace(3, *i);
			bottomPosition = position;
		}
	}


	return extremes;
}
