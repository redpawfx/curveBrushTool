#ifndef brushContext_Header__
#define brushContext_Header__

#include <maya/MPxContext.h>
#include <maya/MToolsInfo.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MEvent.h>
#include <maya/MString.h>
#include <maya/MEvent.h>
#include <maya/MStatus.h>
#include <maya/MCursor.h>
#include <maya/M3dView.h>
#include <maya/MGlobal.h>
#include <maya/MColor.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MFnDagNode.h>

#ifdef WIN32
	#include <WindowsX.h>
	#include <windows.h>
	#include <gl/GLU.h>
#else
	#include <GL/glu.h>
	#include <GL/glx.h>
#endif
#include <stdlib.h>
#include <math.h>
//#include <vector>

class brushContext : public MPxContext
{
  public:
	brushContext();
	void	        toolOnSetup(MEvent &newEvent);
	void	        toolOffCleanup();

    MStatus         doEnterRegion ( MEvent &newEvent );
    MStatus         doPress( MEvent &newEvent );
    MStatus         doRelease( MEvent &newEvent );
	MStatus         doDrag( MEvent &newEvent );
	MStatus         doHold( MEvent &newEvent );
    MStatus         helpStateHasChanged( MEvent &newEvent );
    void                abortAction ( );
	void				 deleteAction();

  protected:
	MStatus         setCursor(const MCursor &cursor);

  private:
	M3dView         view;
	MDagPathArray   dagPathArray;
	/*std::vector <MPointArray> curveCVPos;*/
	float           PI;
	bool            lockBase;
	bool            interactiveResize;
	short int       cursorX,cursorY,oldCursorX,oldCursorY;
	float           Radius;
	bool            intensitySwitch;
	double          intensity;
	MVectorArray    cvsInCircle;

  public:
	void            updateGuidLine(void);
	MStatus         getSelectedCurves(MDagPathArray &curvePathArray);
	MStatus         checkCv(MDagPathArray PathArray,MVectorArray &cvInCircle);
    MStatus         updateCurve(MDagPathArray curvePathArray,MVectorArray cvLib);
    MStatus         updatePosition(MString curveName,MFnNurbsCurve &ptsCurve, int cvNum );
	MStatus        storePosition();
	MStatus         restorePosition();

	void            resizeBrush();
	void            setInteractive(bool interactive);
	bool            getInteractive(void);
	void            setRadius(float radius);
	float           getRadius(void);
	void            setLockBase(bool lock);
	bool            getLockBase(void);
	void            setIntensity(double nIntensity);
	double          getIntensity(void);
	void            resetIntensity();
	void            getClassName( MString & name ) const;


};

#endif
