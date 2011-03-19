/*!
 * GraphDAO.cpp
 * Projekt 3DVisual
 */
#include "Model/GraphDAO.h"

Model::GraphDAO::GraphDAO(void)
{
}

Model::GraphDAO::~GraphDAO(void)
{
}

QMap<qlonglong, Data::Graph*> Model::GraphDAO::getGraphs(QSqlDatabase* conn, bool* error)
{
    QMap<qlonglong, Data::Graph*> qgraphs;
    *error = FALSE;

    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::getGraphs] Connection to DB not opened.";
        *error = TRUE;
        return qgraphs;
    }
    
    //get all graphs with their max element id
    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("SELECT g.graph_id, g.graph_name, (CASE WHEN MAX(l.layout_id) IS NOT NULL THEN MAX(l.layout_id) ELSE 0 END) AS layout_id, (CASE WHEN MAX(ele_id) IS NOT NULL THEN MAX(ele_id) ELSE 0 END) AS ele_id FROM ("
        "("
        "SELECT MAX(node_id) AS ele_id, graph_id FROM nodes "
        "GROUP BY graph_id"
        ") UNION ALL ("
        "SELECT MAX(edge_id) AS ele_id, graph_id FROM edges "
        "GROUP BY graph_id"
        ")"
        ") AS foo "
        "RIGHT JOIN graphs AS g ON "
        "g.graph_id = foo.graph_id "
        "LEFT JOIN layouts AS l ON "
        "l.graph_id = g.graph_id "
        "GROUP BY g.graph_id, g.graph_name");

    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::getGraphs] Could not perform query on DB: " << query->lastError().databaseText();
        *error = TRUE;
        return qgraphs;
    }
    
    while(query->next()) {
        qgraphs.insert(query->value(0).toLongLong(), new Data::Graph(query->value(0).toLongLong(),query->value(1).toString(),query->value(2).toLongLong(),query->value(3).toLongLong(),conn));
    }
    return qgraphs;
}

Data::Graph* Model::GraphDAO::getGraph(QSqlDatabase* conn, bool* error2, qlonglong graphID, qlonglong layoutID)
{
	Data::Graph* newGraph;
	Data::GraphLayout* newLayout;
	QSqlQuery* queryNodes;
	QSqlQuery* queryEdges;
	QSqlQuery* queryNodesPositions;
	QString graphName, layoutName;
	bool error = false;
	qlonglong nodeID1, nodeID2, nodeID, edgeID, maxIdEleUsed = 0;
	QMap<qlonglong, Data::Node*> nodes;
	QMap<qlonglong, Data::Node*>::iterator iNodes1;
	QMap<qlonglong, Data::Node*>::iterator iNodes2;
	Data::Node* newNode; 
	osg::Vec3f position;
	QMap< qlonglong, QList<double> > positions;
	QList<double> coordinates;
	QMap<qlonglong, osg::Vec4> nodeColors;
	QMap<qlonglong, osg::Vec4f> edgeColors;

	graphName = Model::GraphDAO::getName(graphID, &error, conn);
	layoutName = Model::GraphLayoutDAO::getName(conn, &error, graphID, layoutID);
	queryNodes = Model::NodeDAO::getNodesQuery(conn, &error, graphID, layoutID);
	queryNodesPositions = Model::NodeDAO::getNodesPositionsQuery(conn, &error, graphID, layoutID);
	queryEdges = Model::EdgeDAO::getEdgesQuery(conn, &error, graphID, layoutID);
	nodeColors = Model::NodeDAO::getColors(conn, &error, graphID, layoutID);
	edgeColors = Model::EdgeDAO::getColors(conn, &error, graphID, layoutID);
		
	if(!error)
	{
		qDebug() << "[Model::GraphDAO::getGraph] Data loaded from database successfully";

		newGraph = new Data::Graph(graphID, graphName, 0, 0, NULL);
		newLayout = new Data::GraphLayout(layoutID, newGraph, layoutName, conn);
		newGraph->selectLayout(newLayout);

 		Data::Type *typeNode = newGraph->addType("node");
		Data::Type *typeEdge = newGraph->addType("edge");
		Data::Type *typeMetaNode = newGraph->getNodeMetaType();
		Data::Type *typeMetaEdge = newGraph->getEdgeMetaType();

		while(queryNodesPositions->next())
		{
			coordinates.clear();
			coordinates << queryNodesPositions->value(2).toDouble() << queryNodesPositions->value(3).toDouble() << queryNodesPositions->value(4).toDouble();
			positions.insert(queryNodesPositions->value(1).toLongLong(), coordinates);
		}

		while(queryNodes->next()) 
		{
			coordinates.clear();

			nodeID = queryNodes->value(0).toLongLong();
			if(maxIdEleUsed < nodeID)
				maxIdEleUsed = nodeID + 1;

			coordinates = positions[queryNodes->value(0).toLongLong()];
			position = osg::Vec3f(coordinates[0], coordinates[1], coordinates[2]);
			newNode = newGraph->addNode(nodeID, queryNodes->value(1).toString(), (queryNodes->value(4).toBool() ? typeMetaNode : typeNode), position);

			//vsetky uzly nastavime fixed, aby sme zachovali layout
			//hodnota, ktora je ulozena v DB: newNode->setFixed(queryNodes->value(5).toBool()); 
			newNode->setFixed(true);
			
			if(nodeColors.contains(nodeID))
			{
				newNode->setColor(nodeColors.value(nodeID));
			}

			nodes.insert(nodeID, newNode);
		}
		
		while(queryEdges->next()) 
		{
			edgeID = queryEdges->value(0).toLongLong();
			if(maxIdEleUsed < edgeID)
				maxIdEleUsed = edgeID + 1;

			nodeID1 = queryEdges->value(3).toLongLong();
			nodeID2 = queryEdges->value(4).toLongLong();
			iNodes1 = nodes.find(nodeID1);
			iNodes2 = nodes.find(nodeID2);
			newGraph->addEdge(edgeID, queryEdges->value(1).toString(), iNodes1.value(), iNodes2.value(), (queryEdges->value(6).toBool() ? typeMetaEdge : typeEdge), queryEdges->value(5).toBool());

			if(edgeColors.contains(edgeID))
			{
				if(newGraph->getEdges()->contains(edgeID))
					newGraph->getEdges()->find(edgeID).value()->setEdgeColor(edgeColors.value(edgeID));
				else
					newGraph->getMetaEdges()->find(edgeID).value()->setEdgeColor(edgeColors.value(edgeID));
			}
		}
	}
	else 
	{
		qDebug() << "[Model::GraphDAO::getGraph] Error while loading data from database";
	}

	newGraph->setEleIdCounter(maxIdEleUsed);
	 
	*error2 = error;
	return newGraph;
}

Data::Graph* Model::GraphDAO::addGraph(QString graph_name, QSqlDatabase* conn)
{
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::addGraph] Connection to DB not opened.";
        return NULL;
    }

	QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("INSERT INTO graphs (graph_name) VALUES (:graph_name) RETURNING graph_id");
    query->bindValue(":graph_name",graph_name);
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::addGraph] Could not perform query on DB: " << query->lastError().databaseText();
        return NULL;
    }

    if(query->next()) {
        Data::Graph* graph = new Data::Graph(query->value(0).toLongLong(),graph_name,0,0,conn);
        graph->setIsInDB();
        qDebug() << "[Model::GraphDAO::addGraph] Graph was added to DB: " << graph->toString();
        return graph;
    } else {
        qDebug() << "[Model::GraphDAO::addGraph] Graph was not added to DB: " << query->lastQuery();
        return NULL;
    }
}


bool Model::GraphDAO::addGraph( Data::Graph* graph, QSqlDatabase* conn )
{
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::addGraph] Connection to DB not opened.";
        return false;
    }
    
    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::addGraph] Invalid parameter - graph is NULL";
        return false;
    }
    
    if(graph->isInDB()) return true; //graph already in DB
    
    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("INSERT INTO graphs (graph_name) VALUES (:graph_name) RETURNING graph_id");
    query->bindValue(":graph_name",graph->getName());
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::addGraph] Could not perform query on DB: " << query->lastError().databaseText();
        return NULL;
    }

    if(query->next()) {
        graph->setId(query->value(0).toLongLong());
        graph->setIsInDB();
        qDebug() << "[Model::GraphDAO::addGraph2] Graph was added to DB and it's ID was set to: " << graph->getId();
        return true;
    } else {
        qDebug() << "[Model::GraphDAO::addGraph2] Graph was not added to DB: " << query->lastQuery();
        return false;
    }
}


bool Model::GraphDAO::removeGraph(Data::Graph* graph, QSqlDatabase* conn)
{
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::removeGraph] Connection to DB not opened.";
        return false;
    }
    
    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::removeGraph] Invalid parameter - graph is NULL";
        return false;
    }

    if(!graph->isInDB()) return true;

    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("DELETE FROM graphs WHERE graph_id = :graph_id");
    query->bindValue(":graph_id",graph->getId());
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::removeGraph] Could not perform query on DB: " << query->lastError().databaseText();
        return false;
    }

	//TODO: mala by byt moznost nastavit isInDB priznak grafu

    return true;
}

QString Model::GraphDAO::getName(qlonglong graphID, bool* error, QSqlDatabase* conn)
{
    *error = FALSE;
	QSqlQuery* query;
	QString name;

	//check if we have connection
    if(conn==NULL || !conn->isOpen()) 
	{ 
        qDebug() << "[Model::GraphDAO::getName] Connection to DB not opened.";
        *error = TRUE;
        return name;
    }

    //get name of graph from DB
    query = new QSqlQuery(*conn);
    query->prepare("SELECT graph_name "
		"FROM graphs "
		"WHERE graph_id = :graph_id");
	query->bindValue(":graph_id", graphID);

    if(!query->exec()) 
	{
        qDebug() << "[Model::GraphDAO::getName] Could not perform query on DB: " << query->lastError().databaseText();
        *error = TRUE;
        return name;
    }
    
	if(query->next()) 
	{
		name = query->value(0).toString();
    }

    return name;
}

QString Model::GraphDAO::setName(QString name,Data::Graph* graph, QSqlDatabase* conn)
{
    //bolo by lepsie prerobit na vracanie error flagu aby sa vedelo ci problem nastal kvoli nedostatocnym parametrom alebo kvoli chybe insertu
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::setName] Connection to DB not opened.";
        return NULL;
    }
    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::setName] Invalid parameter - graph is NULL";
        return NULL;
    }
    
    /*if(!graph->isInDB()) {
        if(!Model::GraphDAO::addGraph(graph, conn)) { //could not insert graph into DB
            qDebug() << "[Model::GraphDAO::setName] Could not update graph name in DB. Graph is not in DB.";
            return NULL;
        }
    }*/

    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("UPDATE graphs SET graph_name = :graph_name WHERE graph_id = :graph_id");
    query->bindValue(":graph_id",graph->getId());
    query->bindValue(":graph_name",name);
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::setName] Could not perform query on DB: " << query->lastError().databaseText();
        return NULL;
    }

    return name;
}

QMap<QString,QString> Model::GraphDAO::getSettings( Data::Graph* graph, QSqlDatabase* conn, bool* error)
{
    QMap<QString,QString> settings;
    *error = FALSE;
    
    if(conn==NULL || !conn->isOpen()) { //check if we have connection
        qDebug() << "[Model::GraphDAO::getSettings] Connection to DB not opened.";
        *error = TRUE;
        return settings;
    }

    if(graph==NULL) {
        qDebug() << "[Model::GraphDAO::getSettings] Invalid parameter - graph is NULL";
        *error = TRUE;
        return settings;
    }

    if(!graph->isInDB()) {
        qDebug() << "[Model::GraphDAO::getSettings] Graph is not in DB";
        *error = TRUE;
        return settings;
    }
    
    QSqlQuery* query = new QSqlQuery(*conn);
    query->prepare("SELECT val_name, val FROM graph_settings WHERE graph_id = :graph_id");
    query->bindValue(":graph_id",graph->getId());
    if(!query->exec()) {
        qDebug() << "[Model::GraphDAO::getSettings] Could not perform query on DB: " << query->lastError().databaseText();
        *error = TRUE;
        return settings;
    }

    while(query->next()) {
        settings.insert(query->value(0).toString(),query->value(1).toString());
    }
    
    return settings;
}

