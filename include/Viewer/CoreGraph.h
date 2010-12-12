/**
*  CoreGraph.h
*  Projekt 3DVisual
*/
#ifndef VIEWER_CORE_GRAPH_DEF
#define VIEWER_CORE_GRAPH_DEF 1

#include <math.h>

#include <osg/ref_ptr>
#include <osg/CullFace>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/Depth>
#include <osg/TextureCubeMap>
#include <osg/AutoTransform>

#include <QMap>
#include <QLinkedList>

#include "Viewer/PerlinNoiseTextureGenerator.h"
#include "Viewer/CameraManipulator.h"
#include "Viewer/DataHelper.h"
#include "Viewer/SkyTransform.h"
#include "Viewer/TextureWrapper.h"
#include "Viewer/EdgeGroup.h"
#include "Viewer/NodeGroup.h"

#include "Layout/Layout.h"
#include "Util/ApplicationConfig.h"
#include "Data/Edge.h"
#include "Data/Node.h"
#include "Data/Graph.h"

namespace Vwr
{
	/*!
	 * \brief
	 * Trieda vykreslujuca aktualny graf.
	 * 
	 * \author
	 * Michal Paprcka
	 * \version 
	 * 3.0
	 * \date 
	 * 7.12.2009
	 */
	class CoreGraph
	{
	public:
		/*!
		 * 
		 * \param in_nodes
		 * Zoznam uzlov
		 * 
		 * \param in_edges
		 * Zoznam hran
		 * 
		 * \param in_types
		 * Zoznam typov	
		 * 
		 * Konstruktor triedy.
		 * 
		 */
        CoreGraph(Data::Graph * graph = 0, osg::ref_ptr<osg::Camera> camera = 0);
		/*!
		 * 
		 * 
		 * Destruktor.
		 * 
		 */
		~CoreGraph(void);


		/**
		*  \fn public  reload(Data::Graph * graph = 0)
		*  \brief
		*  \param   graph  
		*/
		void reload(Data::Graph * graph = 0);

		/**
		*  \fn public  reloadConfig
		*  \brief
		*/
		void reloadConfig();


		/*!
		 * 
		 * 
		 * Aktualizuje pozicie uzlov a hran.
		 * 
		 */
		void update();


		/**
		*  \fn inline public  getCustomNodeList
		*  \brief
		*  \return QLinkedList<osg::ref_ptr<osg::Node> > * 
		*/
		QLinkedList<osg::ref_ptr<osg::Node> > * getCustomNodeList() { return &customNodeList; }
		

		/*!
		 * \brief
		 * Metoda, ktora vracia odkaz na koren grafu.
		 * 
		 * \returns
		 * vrati odkaz na scenu 
		 * 
		 */
		osg::ref_ptr<osg::Group> const getScene()  { return root; }


		/**
		*  \fn inline public  setCamera(osg::ref_ptr<osg::Camera> camera)
		*  \brief Sets current viewing camera to all edges
		*  \param     camera     current camera
		*/
		void setCamera(osg::ref_ptr<osg::Camera> camera) 
		{ 
			this->camera = camera; 

			QMapIterator<qlonglong, osg::ref_ptr<Data::Edge> > i(*in_edges);

			while (i.hasNext()) 
			{
				i.next();
				i.value()->setCamera(camera);
			}
		} 

		osg::ref_ptr<osg::Camera> getCamera() { return camera; }


		/**
		*  \fn public  setEdgeLabelsVisible(bool visible)
		*  \brief If true, edge labels will be visible
		*  \param      visible     edge label visibility
		*/
		void setEdgeLabelsVisible(bool visible);

		/**
		*  \fn public  setNodeLabelsVisible(bool visible)
		*  \brief If true, node labels will be visible
		*  \param       visible     node label visibility
		*/
		void setNodeLabelsVisible(bool visible);


		/**
		*  \fn inline public constant  getNodesFreezed
		*  \brief True, if nodes are freezed
		*  \return bool nodes freeze state
		*/
		bool getNodesFreezed() const { return nodesFreezed; }

		/**
		*  \fn inline public  setNodesFreezed(bool val)
		*  \brief Sets nodes freeze state
		*  \param      val     nodes freeze state
		*/
		void setNodesFreezed(bool val) 
		{ 
			this->nodesFreezed = val; 
			nodesGroup->freezeNodePositions();
			qmetaNodesGroup->freezeNodePositions();
		}

	private:

		Vwr::EdgeGroup * edgesGroup;
		Vwr::EdgeGroup * qmetaEdgesGroup;
		Vwr::NodeGroup * nodesGroup;
		Vwr::NodeGroup * qmetaNodesGroup;
		
		Data::Graph * graph;


		QMap<qlonglong, osg::ref_ptr<Data::Node> > *in_nodes;
		QMap<qlonglong, osg::ref_ptr<Data::Edge> > *in_edges;
		QMap<qlonglong, osg::ref_ptr<Data::Node> > *qmetaNodes;
		QMap<qlonglong, osg::ref_ptr<Data::Edge> > *qmetaEdges;

		Util::ApplicationConfig* appConf;

		osg::ref_ptr<osg::Group> initEdgeLabels();
		osg::ref_ptr<osg::Group> initCustomNodes();

		osg::ref_ptr<osg::Node> createSkyBox();

		osg::ref_ptr<osg::Camera> camera;
		osg::ref_ptr<osg::Group> root;

		bool nodesFreezed;

		QLinkedList<osg::ref_ptr<osg::Node> > customNodeList;

		void synchronize();
		void cleanUp();

		int backgroundPosition;
		int nodesPosition;
		int edgesPosition;
		int qmetaNodesPosition;
		int qmetaEdgesPosition;
		int labelsPosition;
		int customNodesPosition;
	};
}

#endif
