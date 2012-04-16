#include "brushContext.h"
#include "brushContextCommand.h"
#include "brushTool.h"
#include <maya/MArgList.h>


brushTool::brushTool()
{
  radius=50;
  inten=1.0;
  lockbase=true;
  setCommandString("curveBrushToolCmd");
}

brushTool::~brushTool()
{}

void*   brushTool::creator()
{
   return new brushTool;
}

MStatus	brushTool::doIt(const MArgList& args)
{
    
	return MS::kSuccess;
}

MStatus	brushTool::parseArgs(const MArgList& args)
{
    MArgDatabase argData(syntax(), args);
	MStatus status;

	if (argData.isFlagSet(radiusFlag))
	{
		double tmp;
		status=argData.getFlagArgument(radiusFlag, 0, tmp);
		if (!status) {
			status.perror("radius flag parsing failed");
			return status;
		}
		radius=(float)tmp;
	}

	if (argData.isFlagSet(lockBaseFlag))
	{
		status=argData.getFlagArgument(lockBaseFlag, 0,lockbase);
		if (!status) {
			status.perror("lockbase flag parsing failed");
			return status;
		}
		
	}

	if (argData.isFlagSet(intensityFlag))
	{
		status=argData.getFlagArgument(intensityFlag, 0,inten);
		if (!status) {
			status.perror("intensity flag parsing failed");
			return status;
		}
		
	}

   
	return MS::kSuccess;
}

bool	brushTool::isUndoable() const
{
  return true;
}

MStatus brushTool::cancel()
{
	return MS::kSuccess;
}

MStatus brushTool::undoIt()
{
	return MS::kSuccess;
}

MStatus	brushTool::finalize()
{
    MArgList command;

	command.addArg(commandString());
	command.addArg(MString(radiusFlag));
	command.addArg(radius);
	command.addArg(MString(intensityFlag));
    command.addArg(inten);
    command.addArg(MString(lockBaseFlag));
	command.addArg(lockbase);
	

	return MPxToolCommand::doFinalize( command );
}

MSyntax	brushTool::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag(radiusFlag, radiusLongFlag, MSyntax::kDouble);
	syntax.addFlag(intensityFlag, intensityLongFlag, MSyntax::kDouble);
	syntax.addFlag(lockBaseFlag, lockBaseLongFlag, MSyntax::kBoolean);


	return syntax;
}
void    brushTool::setRadius(float newRadius)
{
   radius = newRadius;
}

void    brushTool::setInten(double intensity)
{
   inten=intensity;
}