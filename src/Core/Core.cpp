
/*!
 * Core.cpp
 * Projekt 3DVisual
 */

#include "Core/Core.h"
#include "Core/Example.h"

AppCore::Core * AppCore::Core::core;

AppCore::Core::Core(QApplication * app)
{
	//Application initialization
    core = this;

    //Util::ApplicationConfig *appConf = Util::ApplicationConfig::get();

    //messageWindows = new QOSG::MessageWindows();

	//Counting forces for layout algorithm, init layout, viewer and window
    this->alg = new Layout::FRAlgorithm();

    //this->thr = new Layout::LayoutThread(this->alg);
    //this->cg = new Vwr::CoreGraph();
    //this->cw = new QOSG::CoreWindow(0, this->cg, app, this->thr);
    /*this->cw->resize(
    	appConf->getNumericValue (
    		"UI.MainWindow.DefaultWidth",
    		std::auto_ptr<long> (new long(200)),
    		std::auto_ptr<long> (NULL),
    		1024
    	),
    	appConf->getNumericValue (
			"UI.MainWindow.DefaultHeight",
			std::auto_ptr<long> (new long(200)),
			std::auto_ptr<long> (NULL),
			768
		)
    );*/
    //this->cw->show();

	this->alg->SetParameters(10,0.7,1,true);
	this->alg->SetGraph(AppCore::Example::CreateCustomGraph());
	this->thr = new Layout::LayoutThread(this->alg);
    this->thr->start();
    this->thr->play();

    app->exec();
}

AppCore::Core::~Core()
{
}

void AppCore::Core::restartLayout()
{
	// [GrafIT][!] the layout algorithm did not end correctly, what caused more instances
	// to be running, fixed it here + made modifications in FRAlgorithm to make correct ending possible
	this->thr->requestEnd();
	this->thr->wait();
    delete this->thr;

	/*
		-----SEM NAHODIT NOVY GRAF TYPU Data::Graph------
		this->alg->SetGraph(new Data::Graph)
	*/

	

	/*

	*/
    this->alg->SetParameters(10,0.7,1,true);
    this->thr = new Layout::LayoutThread(this->alg);
    //this->cw->setLayoutThread(thr);
    //this->cg->reload(Manager::GraphManager::getInstance()->getActiveGraph());
    this->thr->start();
    this->thr->play();
    //this->messageWindows->closeLoadingDialog();
}

AppCore::Core * AppCore::Core::getInstance(QApplication * app)
{
        if(core == NULL)
        {
                if (app != NULL)
					core = new AppCore::Core(app);
                else
                {
                        qDebug() << "Internal error.";
                        return NULL;
                }
        }

        return core;
}
