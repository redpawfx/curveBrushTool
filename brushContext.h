#ifndef brushContext_Header__
#define brushContext_Header__

#include <maya/MPxContext.h>
#include <maya/MToolsInfo.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MPlug.h>
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
#include <maya/MFnCamera.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MFnDagNode.h>

// openGL is diff per platform
#ifdef WIN32
	#include <WindowsX.h>
	#include <windows.h>
	#include <gl/GLU.h>
#elif OSMac_MachO
 	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
	#include <GL/glx.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <map>

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


private:
    M3dView         view;
    MDagPathArray   dagPathArray;
    float           PI;
    bool            lockBase;
    bool            interactiveResize;
    short int       cursorX,cursorY,oldCursorX,oldCursorY;
    float           Radius;
    bool            intensitySwitch;
    double          intensity;
    bool			firstDraw;
	int				brushMode;
	std::map<unsigned int, MIntArray> cvsInCircle;
	float 			brushMag;
	MVector   		dir3d;

public:
    void            updateGuidLine(void);
    MStatus         getSelectedCurves(MDagPathArray &curvePathArray);
    MStatus         checkCv(MDagPathArray PathArray,std::map<unsigned int,MIntArray> &cvLib );
    MStatus         updateCurve(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib);
    MStatus         updatePosition(MFnNurbsCurve &ptsCurve, int cvNum, double mag, MVector dir3d );
    MStatus         storePosition();
    MStatus         restorePosition();
	MStatus			comb(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib);
	MStatus			pullEnds(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib);
	MStatus			straighten(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib);
	MStatus         scaleCurve(MDagPathArray &curvePathArray,std::map<unsigned int,MIntArray> &cvLib);
	// add more brush functions here

    void            resizeBrush();
    void            setInteractive(bool interactive);
    bool            getInteractive(void);
    void            setRadius(float radius);
    float           getRadius(void);
    void            setLockBase(bool lock);
    bool            getLockBase(void);
	void            setBrushMode(unsigned int mode);
    int			    getBrushMode(void);
    void            setIntensity(double nIntensity);
    double          getIntensity(void);
    void            resetIntensity();
    void            getClassName( MString & name ) const;


};

#endif
