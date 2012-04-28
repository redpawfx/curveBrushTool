
#include "brushContextCommand.h"
#include "brushContext.h"
#include "brushToolCommand.h"

using namespace std;

brushContext::brushContext()
{
    PI=3.1415926f;
    interactiveResize=false;
    lockBase=true;
    cursorX=oldCursorX=0.0,cursorY=oldCursorY=0.0;
    Radius=50.0;
    intensitySwitch=false;
    intensity=0.5;
    cvsInCircle.clear();
	brushMode = 1;
	brushMag = 1.0f;
	dir3d = MVector(0,0,0);

    MString str("Curve Brush Manipulator");
    setTitleString(str);
    setImage("curveBrushTool.xpm",kImage1);
	firstDraw = false;
	setCursor(MCursor::pencilCursor); // set the default cursor
}

void    brushContext::toolOnSetup(MEvent &newEvent)
{
    MString str("Drag the manipulator around the curves");
    setHelpString(str);

    MPxContext::toolOnSetup(newEvent);
    if (newEvent.getPosition(cursorX,cursorY) == MS::kSuccess)
    {
        dagPathArray.clear();

        setCursor(MCursor::pencilCursor);
        if (getSelectedCurves(dagPathArray) == MS::kSuccess && dagPathArray.length() > 0)
        {
            MGlobal::executeCommand("LockCurveLength",0,0);

            unsigned int i=0;
            MString disInfo("");
            for (;i<dagPathArray.length();i++)
            {
                disInfo+=dagPathArray[i].partialPathName();
                disInfo+=" ";
            }

            MGlobal::displayInfo(disInfo);
        }
        else
        {
            MGlobal::displayError("No curve is selected!");
        }
    }
    else
        MGlobal::displayError("Can't get mouse position!");
}

void    brushContext::toolOffCleanup()
{
	// keep here for post tool cleanup
    MPxContext::toolOffCleanup();
}

MStatus brushContext::doEnterRegion ( MEvent &newEvent)
{

    if (MPxContext::doEnterRegion(newEvent) == MS::kSuccess)
    {
        MString str("Drag the tool around the curves");
        setHelpString(str);
        if (newEvent.getPosition(cursorX,cursorY) == MS::kSuccess)
        {
            updateGuidLine();
            return MS::kSuccess;
        }
        else
            return MS::kFailure;
    }
    else
        return MS::kFailure;
}

MStatus brushContext::doPress(MEvent &newEvent)
{

    //if (MPxContext::doPress(newEvent) == MS::kSuccess)
    //{
		firstDraw = true;
		if (newEvent.getPosition(cursorX,cursorY) == MS::kSuccess)
		{
            updateGuidLine();
            //record the press position
            oldCursorX=cursorX;
            oldCursorY=cursorY;

            //check which curve and the num of its cv which is in paint circle
            cvsInCircle.clear();
            checkCv(dagPathArray,cvsInCircle);
            MString command = "undoInfo -openChunk";
            MGlobal::executeCommand(command);
            return MS::kSuccess;
        }
        else
            return MS::kFailure;
    //}
    //else
    //    return MS::kFailure;
}


MStatus brushContext::doRelease(MEvent &newEvent)
{
	cout << "doRelease" << endl;
    //MStatus status=MPxContext::doRelease(newEvent);

	MStatus status;

    setCursor(MCursor::pencilCursor);
	updateGuidLine();

    MString command = "undoInfo -closeChunk";
    MGlobal::executeCommand(command);

	/*
	for (unsigned int i = 0; i<dagPathArray.length(); i++)
	{
		MFnNurbsCurve brushCurve(dagPathArray[i],&status);
		brushCurve.updateCurve();
		MPlug lockLengthPlug = brushCurve.findPlug("lockLength");
		lockLengthPlug.setValue(true);
	}
	*/

    return MStatus::kSuccess;
}
MStatus brushContext::doHold(MEvent &newEvent)
{
    if (MPxContext::doHold(newEvent) == MS::kSuccess)
    {
        if (newEvent.getPosition(cursorX,cursorY) == MS::kSuccess)
		{
            return MS::kSuccess;
        }
        else
            return MS::kFailure;
    }
    else
        return MS::kFailure;
}

MStatus brushContext::doDrag(MEvent &newEvent)
{
    if (MPxContext::doDrag(newEvent) == MS::kSuccess)
    {
		if (!firstDraw)
		{
			//	Clear the guide from the old position.
			updateGuidLine();
		}
		else
		{
			firstDraw = false;
		}
        if (newEvent.getPosition(cursorX,cursorY) == MS::kSuccess)
        {
            if (!newEvent.isModifierShift() && (!newEvent.isModifierControl()) )
            {
				cvsInCircle.clear();
				checkCv(dagPathArray,cvsInCircle);
                interactiveResize=false;
                intensitySwitch=false;
                updateCurve(dagPathArray,cvsInCircle);
                oldCursorX=cursorX;
                oldCursorY=cursorY;
            }
            else if (newEvent.isModifierShift())
            {
                interactiveResize=true;
                intensitySwitch=false;
                setCursor(MCursor::handCursor);
                resizeBrush();
            }
            else if (newEvent.isModifierControl())
            {
                intensitySwitch=true;
                interactiveResize=false;
                setCursor(MCursor::handCursor);
                resetIntensity();
            }
            updateGuidLine();

            return MS::kSuccess;
        }
        else
            return MS::kFailure;
    }
    else
        return MS::kFailure;
}

MStatus brushContext::helpStateHasChanged(MEvent &newEvent)
{
    MStatus status=MPxContext::helpStateHasChanged(newEvent);
    return status;
}



//get selected curves and store them in curvePathArray
MStatus brushContext::getSelectedCurves(MDagPathArray &curvePathArray)
{
    MSelectionList list;
    MStatus status;
    if (MGlobal::getActiveSelectionList(list) == MS::kSuccess)
    {
        MItSelectionList iter(list, MFn::kNurbsCurve,&status);

		CHECK_MSTATUS(status);

        for ( ; !iter.isDone(); iter.next() )
        {
            MDagPath item;
            MObject component;
            if ( iter.getDagPath(item,component) == MS::kSuccess )
                curvePathArray.append(item);
            else
                return MS::kFailure;
        }
        return MS::kSuccess;
    }
    else
        return MS::kFailure;
}

//check which cv is in circle
MStatus brushContext::checkCv(MDagPathArray PathArray,std::map<unsigned int,MIntArray> &cvsInCircle)
{
    unsigned int i=0;
    MStatus  status=MS::kSuccess;

	cvsInCircle.clear();

    for (;i<PathArray.length();i++)
    {
		MIntArray perCurve;

        MFnNurbsCurve nurbsCurve(PathArray[i],&status);

        MFnDagNode dagNode(PathArray[i]);
        MString    dagName=dagNode.partialPathName();

		CHECK_MSTATUS(status);

        MPointArray pts;
        nurbsCurve.getCVs(pts,MSpace::kWorld);
		CHECK_MSTATUS(status);

        unsigned int j;
        if (!lockBase)
            j=0;
        else
            j=1;

		if (brushMode == 2) // pull ends
		{
			j = pts.length()-1;
		}

        for (;j<pts.length();j++)
        {
				short int xPos=0,yPos=0;

				view.worldToView(pts[j],xPos,yPos,&status);

				CHECK_MSTATUS(status);

				float distanceToCursor=sqrt( (float) (xPos-oldCursorX)*(xPos-oldCursorX)+(yPos-oldCursorY)*(yPos-oldCursorY) );
				if ( distanceToCursor < Radius)
				{
					if (brushMode == 3 || brushMode == 4)  // straighten or  scale whole curve
					{
						j=1;
						for (;j<pts.length();j++)
						{
							status=perCurve.append(j);
							CHECK_MSTATUS(status);
						}
						break;
					}
					status=perCurve.append(j);
					CHECK_MSTATUS(status);
				}
        }
        if(perCurve.length())
		{
			cvsInCircle.insert(pair<unsigned int, MIntArray> (i,perCurve));
		}
    }
    return status;
}


MStatus brushContext::updateCurve(MDagPathArray &curvePathArray,std::map<unsigned int, MIntArray> &cvLib)
{

    MStatus    status=MS::kSuccess;

    if (curvePathArray.length() == 0)
        return MS::kFailure;

	// calculate the motion of the cursor once out here..
	MPoint nearClipPt[2],farClipPt[2];

    //get the oldCursor position on the near clip of the camera
    status=view.viewToWorld(oldCursorX,(oldCursorY),nearClipPt[0],farClipPt[0]);
	CHECK_MSTATUS(status);


    //get the cursor position on the near clip of the camera
    status=view.viewToWorld(cursorX,(cursorY),nearClipPt[1],farClipPt[1]);
	CHECK_MSTATUS(status);


    MPoint maxNearViewPoint[2],maxFarViewPoint[2];
    status=view.viewToWorld(0,0,maxNearViewPoint[0],maxFarViewPoint[0]);
	CHECK_MSTATUS(status);

	double portSquareSize = view.portHeight();
	// we want to base the mag off of a  square viewport
	if (portSquareSize >= view.portWidth())
	{
		portSquareSize = view.portWidth();
	}

    status=view.viewToWorld(portSquareSize,portSquareSize,maxNearViewPoint[1],maxFarViewPoint[1]);
	CHECK_MSTATUS(status);

    //get the movement direction of the cursor on the near clip
    dir3d=nearClipPt[1]-nearClipPt[0];
    dir3d.normalize();

	// This helps  normalize the brush stroke depending on how large our clipping planes are...
	MDagPath camPath;
	view.getCamera(camPath);
	MFnCamera cam(camPath);
	double ncp = cam.nearClippingPlane();
    MVector dirNearPt(0,0,0);
    dirNearPt=maxNearViewPoint[1]-maxNearViewPoint[0];
	brushMag = dirNearPt.length()*(1/ncp);


	switch(brushMode)
	{
		case 1: // comb
			status =comb(curvePathArray,cvLib);
			break;
		case 2: // pull ends
			status = pullEnds(curvePathArray,cvLib);
			break;
		case 3: // straighten
			status = straighten(curvePathArray,cvLib);
			break;
		case 4: // scale hair curve
			status = scaleCurve(curvePathArray,cvLib);
			break;
		default:
			status =comb(curvePathArray,cvLib);
			break;
	}

    return status;
}

// standard comb operation
MStatus brushContext::comb(MDagPathArray &curvePathArray,std::map<unsigned int, MIntArray> &cvLib)
{
	MStatus status;
	map<unsigned int, MIntArray>::iterator  iter;
	for (iter = cvLib.begin(); iter != cvLib.end(); iter++)
	{
		MFnNurbsCurve brushCurve(curvePathArray[iter->first],&status);
		CHECK_MSTATUS(status);

		unsigned int j = 0;
		for (;j<iter->second.length();j++)
		{
			status=updatePosition(brushCurve,iter->second[j], brushMag, dir3d);
			CHECK_MSTATUS(status);

		}
		status=brushCurve.updateCurve();
		CHECK_MSTATUS(status);

	}
	return status;
}

// pull ends uses standard comb operation but only on the end CV's (chosen earlier in the process)
MStatus brushContext::pullEnds(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib)
{
	MStatus status;
	status = comb(curvePathArray,cvLib);
	CHECK_MSTATUS(status);
	return status;

}
MStatus brushContext::straighten(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib)
{
	MStatus status;
	map<unsigned int, MIntArray>::iterator  iter;
	for (iter = cvLib.begin(); iter != cvLib.end(); iter++)
	{
		MFnNurbsCurve brushCurve(curvePathArray[iter->first],&status);
		CHECK_MSTATUS(status);

		MPointArray curveCVs;
		brushCurve.getCVs(curveCVs,MSpace::kWorld);
		unsigned int j = 0;
		for (;j< iter->second.length();j++)
		{
			MPoint first,second,third,midPoint;
			first = curveCVs[iter->second[j]-1];
			second = curveCVs[iter->second[j]];
			if( j == iter->second.length()-1) // end point treated special
			{
				third = second + ((curveCVs[0]-second)/curveCVs.length());
			}
			else
			{
				third = curveCVs[iter->second[j]+1];
			}

			midPoint = first+((third-first)/2);
			dir3d = second - midPoint;
			dir3d *= brushMag*intensity;
			brushCurve.setCV(iter->second[j],second+dir3d, MSpace::kWorld);
		}
		status=brushCurve.updateCurve();
	}
	return status;
}
MStatus brushContext::scaleCurve(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib)
{
	MStatus status;
	map<unsigned int, MIntArray>::iterator  iter;
	for (iter = cvLib.begin(); iter != cvLib.end(); iter++)
	{
		MFnNurbsCurve brushCurve(curvePathArray[iter->first],&status);
		CHECK_MSTATUS(status);

		///TODO: need to figure out how to do this properly..
		//MPlug lockLengthPlug = brushCurve.findPlug("lockLength");
		//lockLengthPlug.setValue(false);

		unsigned int j = 0;
		for (;j<iter->second.length();j++)
		{
			MPoint rootCv;
			brushCurve.getCV(0,rootCv,MSpace::kObject);
			MPoint move;
			brushCurve.getCV(iter->second[j],move,MSpace::kObject);
			move -= rootCv;
			dir3d = move;
			dir3d *= 1.f+ brushMag*intensity*0.1;
			brushCurve.setCV(iter->second[j],rootCv+ dir3d, MSpace::kObject);
		}
		status=brushCurve.updateCurve();
		CHECK_MSTATUS(status);

	}
	return status;
}


//get the single curve and its single cv then move it
MStatus brushContext::updatePosition(MFnNurbsCurve &ptsCurve, int cvNum, double mag, MVector dir3d )
{
    MStatus      status=MS::kSuccess;
    MPoint       pt;

    //get the position of cvNum and put it into pt
    status=ptsCurve.getCV(cvNum,pt,MSpace::kWorld);
	CHECK_MSTATUS(status);


	pt+=intensity*mag*dir3d;

	status=ptsCurve.setCV(cvNum,pt,MSpace::kWorld);

    return status;
}



//using opengl to draw a guidline on the viewport's overlay plane
void    brushContext::updateGuidLine()
{
    view =M3dView::active3dView();
    int portW=view.portWidth();
    int portH=view.portHeight();

    view.beginXorDrawing();
    glClear(GL_CURRENT_BIT);
    glPushAttrib(GL_CURRENT_BIT );
	glColor3f(0,1,1);
    glLineWidth(1.5);
    glBegin(GL_LINE_LOOP);
    for (int i=0; i<360; i++)
    {
        float angle = PI/180*i;
        glVertex2f( (Radius * cos(angle) +cursorX) ,
                    (Radius * sin(angle) +(cursorY)) );
    }
    glEnd();

    glBegin(GL_LINES);
    glVertex2f( cursorX-Radius*(float)intensity , (float)cursorY );
    glVertex2f( cursorX+Radius*(float)intensity , (float)cursorY );
    glEnd();

    glBegin(GL_LINES);
    glVertex2f( cursorX , (cursorY)-Radius*(float)intensity );
    glVertex2f( cursorX , (cursorY)+Radius*(float)intensity );

    glEnd();

    glFlush();
	glLineWidth(1.0);
	glPopAttrib();

	/*
#ifdef _WIN32
    SwapBuffers( view.deviceContext() );
#elif defined (OSMac_)
    ::aglSwapBuffers( view.display());
#else
	glXSwapBuffers( view.display(), view.window() );
#endif // _WIN32
*/
    view.endXorDrawing();
    view.refresh(0,1,0);
}


///////////////////////////////////////////////////
/// CONTEXT ATTRIBUTE HANDLING

///////////////////////////////////////////////////
/// BRUSH SIZE

//resize the brush size
void    brushContext::resizeBrush(void)
{
    if (Radius >= 10 && Radius <= 400)
        Radius  += (cursorX-oldCursorX);
    else if ( Radius < 10 )
        Radius  =  10;
    else if ( Radius>400 )
        Radius  =  400;

    oldCursorX=cursorX;

    MString command = "curveBrushContext -e -r ";
    command = command + Radius +" `currentCtx`";

    MGlobal::executeCommand(command);
}

//switch interactive mode
void    brushContext::setInteractive(bool interactive)
{
    interactiveResize = interactive;
}
//get interactive mode
bool    brushContext::getInteractive(void)
{
    return interactiveResize;
}
//set paint radius
void    brushContext::setRadius(float radius)
{
    Radius=radius;
    MToolsInfo::setDirtyFlag(*this);
}

//get paint radius
float   brushContext::getRadius()
{
    return Radius;
}

/////////////////////////////////////////////////////////////
/// LOCKBASE

//set lockbase mode
void    brushContext::setLockBase(bool lock)
{
    lockBase = lock;
}
//get lockbase mode
bool    brushContext::getLockBase()
{
    return lockBase;
}

//////////////////////////////////////////////////////////////
/// BRUSH MODE

// set brush mode
void brushContext::setBrushMode(unsigned int mode)
{
	brushMode = mode;
}

// get brush mode
int brushContext::getBrushMode()
{
	return brushMode;
}

/////////////////////////////////////////////////////////
/// INTENSITY
void    brushContext::setIntensity(double nIntensity)
{
    intensity=nIntensity;
}

void    brushContext::resetIntensity()
{
    if (intensity>=0.0 && intensity <=2.0)
        intensity+=(double)(cursorX-oldCursorX)/100;
    else if (intensity<0.0)
        intensity=0.0;
    else if (intensity>2.0)
        intensity=2.0;

    oldCursorX=cursorX;

    MString command = "curveBrushContext -e -ity ";
    command = command + intensity +" `currentCtx`";

    MGlobal::displayInfo(command);

    MGlobal::executeCommand(command);

}
double  brushContext::getIntensity(void)
{
    return intensity;
}

void    brushContext::getClassName( MString & name ) const
{
    name.set("curveBrush");
}

