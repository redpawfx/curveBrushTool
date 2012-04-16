#ifndef brushContextCommand__
#define brushContextCommand__

#include <maya/MPxContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MArgDatabase.h>
#include "brushContext.h"

static const char *radiusFlag = "-r";
static const char *radiusLongFlag = "-radius";
static const char *lockBaseFlag = "-lb";
static const char *lockBaseLongFlag = "-lockBase";
static const char *resizeBrushFlag = "-rsb";
static const char *resizeBrushLongFlag = "-resizeBrush";
static const char *intensityFlag = "-ity";
static const char *intensityLongFlag = "-intensity";

class brushContextCommand : public MPxContextCommand
{
	public:
		                       brushContextCommand() {};
		virtual  MPxContext  * makeObj();
		virtual  MStatus	   appendSyntax();

	public:
		static void*           creator();
		static MSyntax         newSyntax();

		virtual	MStatus		   doEditFlags();
	    virtual MStatus		   doQueryFlags(); 

    protected:
		brushContext*          nBrushContext;
};

#endif
