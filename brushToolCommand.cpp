#include "brushContext.h"
#include "brushContextCommand.h"
#include "brushToolCommand.h"
#include <maya/MArgList.h>


brushToolCommand::brushToolCommand()
{
    radius=50;
    inten=1.0;
    lockbase=true;
    brushModeVal = 0; // comb
    setCommandString("curveBrushToolCmd");
}

brushToolCommand::~brushToolCommand()
{}

void*   brushToolCommand::creator()
{
    return new brushToolCommand;
}

MStatus	brushToolCommand::doIt(const MArgList& args)
{

    return MS::kSuccess;
}

MStatus	brushToolCommand::parseArgs(const MArgList& args)
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
    if (argData.isFlagSet(brushModeFlag))
    {
        status=argData.getFlagArgument(brushModeFlag, 0,brushModeVal);
        if (!status) {
            status.perror("brush mode flag parsing failed");
            return status;
        }

    }


    return MS::kSuccess;
}

bool	brushToolCommand::isUndoable() const
{
    return true;
}

MStatus brushToolCommand::cancel()
{
    return MS::kSuccess;
}

MStatus brushToolCommand::undoIt()
{
    return MS::kSuccess;
}

MStatus	brushToolCommand::finalize()
{
    MArgList command;

    command.addArg(commandString());
    command.addArg(MString(radiusFlag));
    command.addArg(radius);
    command.addArg(MString(intensityFlag));
    command.addArg(inten);
    command.addArg(MString(lockBaseFlag));
    command.addArg(lockbase);
    command.addArg(MString(brushModeFlag));
    command.addArg(brushModeVal);


    return MPxToolCommand::doFinalize( command );
}

MSyntax	brushToolCommand::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag(radiusFlag, radiusLongFlag, MSyntax::kDouble);
    syntax.addFlag(intensityFlag, intensityLongFlag, MSyntax::kDouble);
    syntax.addFlag(lockBaseFlag, lockBaseLongFlag, MSyntax::kBoolean);
    syntax.addFlag(brushModeFlag, brushModeLongFlag, MSyntax::kLong);


    return syntax;
}
void    brushToolCommand::setRadius(float newRadius)
{
    radius = newRadius;
}

void    brushToolCommand::setInten(double intensity)
{
    inten=intensity;
}

void  brushToolCommand::setBrushMode(int brushMode)
{
    brushModeVal = brushMode;
}
