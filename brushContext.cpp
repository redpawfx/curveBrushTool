
#include "brushContextCommand.h"
#include "brushContext.h"
#include "brushTool.h"

brushContext::brushContext()
{
	PI=3.1415926f;
	interactiveResize=false;
	lockBase=true;
    cursorX=oldCursorX=100,cursorY=oldCursorY=100;
	Radius=100.0;
	intensitySwitch=false;
	intensity=1.0;
	MVector zero = MVector::zero;
	cvsInCircle=MVectorArray(0,zero );

	MString str("Curve Brush Manipulator");
	setTitleString(str);
	setImage("curveBrushTool.xpm",kImage1);
}

void    brushContext::toolOnSetup(MEvent &newEvent)
{
    MString str("Drag the manipulator around the curves");
	setHelpString(str);

	MPxContext::toolOnSetup(newEvent);
	if(newEvent.getWindowPosition(cursorX,cursorY) == MS::kSuccess)
	{
		dagPathArray.clear();

		MCursor crossHairCursor(MCursor::doubleCrossHairCursor);
	    setCursor(crossHairCursor);
		if(getSelectedCurves(dagPathArray) == MS::kSuccess && dagPathArray.length() > 0)
		{
			/*curveCVPos.clear();
			storePosition();*/
			MGlobal::executeCommand("LockCurveLength",0,0);

			unsigned int i=0;
			MString disInfo("");
			for(;i<dagPathArray.length();i++)
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
	MPxContext::toolOffCleanup();
}

MStatus brushContext::doEnterRegion ( MEvent &newEvent)
{
   if(MPxContext::doEnterRegion(newEvent) == MS::kSuccess)
	 {
	     MString str("Drag the tool around the curves");
	     setHelpString(str);
		 if(newEvent.getWindowPosition(cursorX,cursorY) == MS::kSuccess)
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

	if(MPxContext::doPress(newEvent) == MS::kSuccess)
	  {
		if(newEvent.getWindowPosition(cursorX,cursorY) == MS::kSuccess)
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
MStatus brushContext::setCursor(const MCursor &cursor)
{
	if(MPxContext::setCursor(cursor) == MS::kSuccess)
		return MStatus::kSuccess;
	return MS::kFailure;
}

MStatus brushContext::doRelease(MEvent &newEvent)
{
    MStatus state=MPxContext::doRelease(newEvent);

	MCursor backCursor(MCursor::doubleCrossHairCursor);
	setCursor(backCursor);


	MString command = "undoInfo -closeChunk";
	MGlobal::executeCommand(command);
	return state;
}
MStatus brushContext::doHold(MEvent &newEvent)
{
    if(MPxContext::doHold(newEvent) == MS::kSuccess)
	{
	 if(newEvent.getWindowPosition(cursorX,cursorY) == MS::kSuccess)
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

MStatus brushContext::doDrag(MEvent &newEvent)
{
    if(MPxContext::doDrag(newEvent) == MS::kSuccess)
	  {
		if(newEvent.getWindowPosition(cursorX,cursorY) == MS::kSuccess)
          {

			  if(!newEvent.isModifierShift() && (!newEvent.isModifierControl()) )
			    {
			     interactiveResize=false;
                 intensitySwitch=false;
			     /*curveCVPos.clear();
				 storePosition();*/
				 updateCurve(dagPathArray,cvsInCircle);
                 oldCursorX=cursorX;
				 oldCursorY=cursorY;
			    }
			  else if(newEvent.isModifierShift())
			    {
                interactiveResize=true;
                intensitySwitch=false;
				MCursor resizeCursor(MCursor::crossHairCursor);
				setCursor(resizeCursor);
			    resizeBrush();
			    }
			  else if(newEvent.isModifierControl())
			    {
					intensitySwitch=true;
                    interactiveResize=false;
					MCursor resizeCursor(MCursor::crossHairCursor);
				    setCursor(resizeCursor);
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
	view     =M3dView::active3dView();
	int portW=view.portWidth();
	int portH=view.portHeight();

	view.beginXorDrawing();
	   glClear(GL_CURRENT_BIT);
	   glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       gluOrtho2D(0,portW,0,portH);
	   glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();

       glPushAttrib(GL_CURRENT_BIT );
	   glColor3f(1,0,0);
	   glLineWidth(1.5);
	   glBegin(GL_LINE_LOOP);
	        for(int i=0; i<360; i++)
			{
			  float angle = PI/180*i;
			  glVertex2f( (Radius * cos(angle) +cursorX) ,
				          (Radius * sin(angle) +(portH-cursorY)) );
			}
	   glEnd();


	  glBegin(GL_LINES);
		 glVertex2f( cursorX-Radius*(float)intensity , (float)portH-cursorY );
		 glVertex2f( cursorX+Radius*(float)intensity , (float)portH-cursorY );
	  glEnd();

	  glBegin(GL_LINES);
		 glVertex2f( cursorX , (portH-cursorY)-Radius*(float)intensity );
		 glVertex2f( cursorX , (portH-cursorY)+Radius*(float)intensity );
	  glEnd();


       glFlush();
	   glPopAttrib();

#ifdef _WIN32
	SwapBuffers( view.deviceContext() );
#elif defined (OSMac_)
	::aglSwapBuffers( view.display());
#else
	glXSwapBuffers( view.display(), view.window() );
#endif // _WIN32

	//view.clearOverlayPlane();
	view.endXorDrawing();

	view.refresh(0,1,0);
}



                   //get selected curves and store them in curvePathArray
MStatus brushContext::getSelectedCurves(MDagPathArray &curvePathArray)
{
   MSelectionList list;
   MStatus state;
   if(MGlobal::getActiveSelectionList(list) == MS::kSuccess)
   {
	   MItSelectionList iter(list, MFn::kNurbsCurve,&state);
	   if(state == MS::kFailure)
	   {
		   return MS::kFailure;
	   }
	   for ( ; !iter.isDone(); iter.next() )
		{
			MDagPath item;
			MObject component;
			if( iter.getDagPath(item,component) == MS::kSuccess )
			    curvePathArray.append(item);
			else
				return MS::kFailure;
		}
	   return MS::kSuccess;
   }
   else
	   return MS::kFailure;
}

MStatus brushContext::updateCurve(MDagPathArray curvePathArray,MVectorArray cvLib)
{
   unsigned int i=0;
   MStatus    state=MS::kSuccess;

   if (curvePathArray.length() == 0)
	   return MS::kFailure;

   for(;i<cvLib.length();i++)
   {
     //vectorArray cvlib containts the proper curve and its pt
	 //x is the specific curve and y is the cv num
	 MFnNurbsCurve brushCurve(curvePathArray[(int)cvLib[i].x],&state);
	 if( state == MS::kFailure )
		 return state;

	MString curveName = curvePathArray[(int)cvLib[i].x].fullPathName();

	 state=updatePosition(curveName,brushCurve,(int)cvLib[i].y);
     if(state == MS::kFailure)
	     return state;

     state=brushCurve.updateCurve();
     if(state == MS::kFailure)
	     return state;
    }

   return state;
}

                   //get the single curve and its single cv then move it
MStatus brushContext::updatePosition(MString curveName,MFnNurbsCurve &ptsCurve, int cvNum )
{
	MStatus      state=MS::kSuccess;
	MPoint       pt;

    //get the position of cvNum and put it into pt
    state=ptsCurve.getCV(cvNum,pt,MSpace::kWorld);
	if(state == MS::kFailure)
		 return state;

	MPoint nearClipPt[2],farClipPt[2];

	//get the oldCursor position on the near clip of the camera
	state=view.viewToWorld(oldCursorX,(view.portHeight()-oldCursorY),nearClipPt[0],farClipPt[0]);
    if(state == MS::kFailure)
		 return state;

	//get the cursor position on the near clip of the camera
	state=view.viewToWorld(cursorX,(view.portHeight()-cursorY),nearClipPt[1],farClipPt[1]);
    if(state == MS::kFailure)
		 return state;

	MPoint maxNearViewPoint[2],maxFarViewPoint[2];
	state=view.viewToWorld(0,0,maxNearViewPoint[0],maxFarViewPoint[0]);
    if(state == MS::kFailure)
		 return state;

	state=view.viewToWorld(view.portHeight(),view.portWidth(),maxNearViewPoint[1],maxFarViewPoint[1]);
    if(state == MS::kFailure)
		 return state;

	//get the movement direction of the cursor on the near clip
	MVector   dir3d(0,0,0);
    dir3d=nearClipPt[1]-nearClipPt[0];
	dir3d.normalize();

	MVector dirNearPt(0,0,0);
	dirNearPt=maxNearViewPoint[1]-maxNearViewPoint[0];
	double mag=dirNearPt.length();

    pt+=intensity*mag*dir3d;

	MString command = "move -a ";
	command = command +pt[0] + " " + pt[1]+" " +pt[2]+ " " +curveName+".cv["+cvNum+"]" ;

	MGlobal::executeCommand(command,0,1);


	return state;
}

                   //check which cv is in circle
MStatus brushContext::checkCv(MDagPathArray PathArray,MVectorArray &cvInCircle)
{
	unsigned int i=0;
    MStatus      state=MS::kSuccess;

	for(;i<PathArray.length();i++)
	{
       MFnNurbsCurve nurbsCurve(PathArray[i],&state);

	   MFnDagNode dagNode(PathArray[i]);
	   MString    dagName=dagNode.partialPathName();

	   if( state == MS::kFailure )
		 return state;

       MPointArray pts;
	   nurbsCurve.getCVs(pts,MSpace::kWorld);
       if( state == MS::kFailure )
		 return state;

       unsigned int j;
	   if(!lockBase)
		   j=0;
	   else
		   j=1;
	   for(;j<pts.length();j++)
	   {
			 short int xPos=0,yPos=0;

			 view.worldToView(pts[j],xPos,yPos,&state);
			 yPos=view.portHeight()-yPos;

			 if( state == MS::kFailure )
				 return state;

			 float distanceToCursor=sqrt( (float) (xPos-oldCursorX)*(xPos-oldCursorX)+(yPos-oldCursorY)*(yPos-oldCursorY) );
			 if( distanceToCursor < Radius )
			  {
				MVector cv(i,j,0);
				state=cvInCircle.append(cv);
                if( state == MS::kFailure )
				    return state;
			  }
	   }
	}
	return state;
}
                   //resize the brush size
void    brushContext::resizeBrush(void)
{
        if(Radius >= 10 && Radius <= 400)
	      Radius  += (cursorX-oldCursorX);
		else if( Radius < 10 )
		  Radius  =  10;
		else if( Radius>400 )
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
    if(intensity>=0.0 && intensity <=2.0)
		intensity+=(double)(cursorX-oldCursorX)/100;
	else if(intensity<0.0)
        intensity=0.0;
	else if(intensity>2.0)
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