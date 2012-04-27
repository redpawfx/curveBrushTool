
#include "brushContextCommand.h"
#include "brushContext.h"
#include "brushTool.h"

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
	cout<< "doEnterRegion" << endl;
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

    if (MPxContext::doPress(newEvent) == MS::kSuccess)
    {
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
    }
    else
        return MS::kFailure;
}


MStatus brushContext::doRelease(MEvent &newEvent)
{
    MStatus state=MPxContext::doRelease(newEvent);

    setCursor(MCursor::pencilCursor);
	updateGuidLine();

    MString command = "undoInfo -closeChunk";
    MGlobal::executeCommand(command);
    return state;
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
    MStatus state=MPxContext::helpStateHasChanged(newEvent);
    return state;
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

//get selected curves and store them in curvePathArray
MStatus brushContext::getSelectedCurves(MDagPathArray &curvePathArray)
{
    MSelectionList list;
    MStatus state;
    if (MGlobal::getActiveSelectionList(list) == MS::kSuccess)
    {
        MItSelectionList iter(list, MFn::kNurbsCurve,&state);

        if (state == MS::kFailure)
        {
            return MS::kFailure;
        }
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

MStatus brushContext::updateCurve(MDagPathArray curvePathArray,std::map<unsigned int, MIntArray>cvLib)
{

    MStatus    state=MS::kSuccess;

    if (curvePathArray.length() == 0)
        return MS::kFailure;

	// calculate the motion of the cursor once out here..
	MPoint nearClipPt[2],farClipPt[2];

    //get the oldCursor position on the near clip of the camera
    state=view.viewToWorld(oldCursorX,(oldCursorY),nearClipPt[0],farClipPt[0]);
    if (state == MS::kFailure)
        return state;

    //get the cursor position on the near clip of the camera
    state=view.viewToWorld(cursorX,(cursorY),nearClipPt[1],farClipPt[1]);
    if (state == MS::kFailure)
        return state;

    MPoint maxNearViewPoint[2],maxFarViewPoint[2];
    state=view.viewToWorld(0,0,maxNearViewPoint[0],maxFarViewPoint[0]);
    if (state == MS::kFailure)
        return state;


	double portSquareSize = view.portHeight();
	// we want to base the mag off of a  square viewport
	if (portSquareSize >= view.portWidth())
	{
		portSquareSize = view.portWidth();
	}

    state=view.viewToWorld(portSquareSize,portSquareSize,maxNearViewPoint[1],maxFarViewPoint[1]);
    if (state == MS::kFailure)
        return state;

    //get the movement direction of the cursor on the near clip
    MVector   dir3d(0,0,0);
    dir3d=nearClipPt[1]-nearClipPt[0];
    dir3d.normalize();


	// This helps  normalize the brush stroke depending on how large our clipping planes are...
	MDagPath camPath;
	view.getCamera(camPath);
	MFnCamera cam(camPath);
	double ncp = cam.nearClippingPlane();
    MVector dirNearPt(0,0,0);
    dirNearPt=maxNearViewPoint[1]-maxNearViewPoint[0];
	double mag= dirNearPt.length()*(1/ncp);

	cout << mag << endl;

	map<unsigned int, MIntArray>::iterator  iter;
	for (iter = cvLib.begin(); iter != cvLib.end(); iter++)
	{
		MFnNurbsCurve brushCurve(curvePathArray[iter->first],&state);
		if ( state == MS::kFailure )
			return state;
		unsigned int j = 0;
		for (;j<iter->second.length();j++)
		{
			MString curveName = curvePathArray[iter->first].fullPathName();

			state=updatePosition(curveName,brushCurve,iter->second[j], mag, dir3d);
			if (state == MS::kFailure)
				return state;
		}
		state=brushCurve.updateCurve();
		if (state == MS::kFailure)
			return state;
	}

    return state;
}

//get the single curve and its single cv then move it
MStatus brushContext::updatePosition(MString curveName,MFnNurbsCurve &ptsCurve, int cvNum, double mag, MVector dir3d )
{
    MStatus      state=MS::kSuccess;
    MPoint       pt;

    //get the position of cvNum and put it into pt
    state=ptsCurve.getCV(cvNum,pt,MSpace::kWorld);
    if (state == MS::kFailure)
        return state;

	pt+=intensity*mag*dir3d;

	state=ptsCurve.setCV(cvNum,pt,MSpace::kWorld);

    return state;
}

//check which cv is in circle
MStatus brushContext::checkCv(MDagPathArray PathArray,std::map<unsigned int,MIntArray> &cvsInCircle)
{
    unsigned int i=0;
    MStatus      state=MS::kSuccess;

	cvsInCircle.clear();

    for (;i<PathArray.length();i++)
    {
		MIntArray perCurve;
		cvsInCircle.insert(pair<unsigned int, MIntArray> (i,perCurve));

        MFnNurbsCurve nurbsCurve(PathArray[i],&state);

        MFnDagNode dagNode(PathArray[i]);
        MString    dagName=dagNode.partialPathName();

        if ( state == MS::kFailure )
            return state;

        MPointArray pts;
        nurbsCurve.getCVs(pts,MSpace::kWorld);
        if ( state == MS::kFailure )
            return state;

        unsigned int j;
        if (!lockBase)
            j=0;
        else
            j=1;

        for (;j<pts.length();j++)
        {
            short int xPos=0,yPos=0;

            view.worldToView(pts[j],xPos,yPos,&state);
            //yPos=view.portHeight()-yPos;

            if ( state == MS::kFailure )
                return state;

            float distanceToCursor=sqrt( (float) (xPos-oldCursorX)*(xPos-oldCursorX)+(yPos-oldCursorY)*(yPos-oldCursorY) );
            if ( distanceToCursor < Radius )
            {
                state=cvsInCircle[i].append(j);
                if ( state == MS::kFailure )
                    return state;
            }
        }
    }
    return state;
}
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

//set lockbase mode
void    brushContext::setLockBase(bool lock)
{
    lockBase=lock;
}
//get lockbase mode
bool    brushContext::getLockBase()
{
    return lockBase;
}

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

void brushContext::abortAction()
{
}

void brushContext::deleteAction()
{
}
