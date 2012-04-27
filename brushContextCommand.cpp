#include "brushContextCommand.h"
#include "brushContext.h"
#include "brushToolCommand.h"

MPxContext *brushContextCommand::makeObj()
{
    nBrushContext = new brushContext();
    return  nBrushContext;
}

void   *brushContextCommand::creator()
{
    return new brushContextCommand;
}

MStatus brushContextCommand::doEditFlags()
{
    MStatus status = MS::kSuccess;
    MArgParser argData = parser();

    if (argData.isFlagSet(radiusFlag)) {
        double userRadius;
        status = argData.getFlagArgument(radiusFlag, 0, userRadius);
        if (!status)
        {
            status.perror("radius flag parsing failed.");
            return status;
        }
        nBrushContext->setRadius((float)userRadius);
    }

    if (argData.isFlagSet(lockBaseFlag)) {
        bool nlockbase;
        status = argData.getFlagArgument(lockBaseFlag, 0, nlockbase);
        if (!status)
        {
            status.perror("lockBase flag parsing failed.");
            return status;
        }
        nBrushContext->setLockBase(nlockbase);
    }

    if (argData.isFlagSet(resizeBrushFlag))
    {
        bool resize;
        status=argData.getFlagArgument(resizeBrushFlag, 0,resize);
        if (!status) {
            status.perror("resizeBrush flag parsing failed");
            return status;
        }
        nBrushContext->setInteractive(resize);
    }

    if (argData.isFlagSet(intensityFlag))
    {
        double inten;
        status=argData.getFlagArgument(intensityFlag, 0,inten);
        if (!status) {
            status.perror("intensity flag parsing failed");
            return status;
        }
        nBrushContext->setIntensity(inten);
    }
    if (argData.isFlagSet(brushModeFlag))
    {
        int brMode;
        status=argData.getFlagArgument(brushModeFlag, 0,brMode);
        if (!status) {
            status.perror("brushMode flag  parsing failed");
            return status;
        }
        nBrushContext->setBrushMode(brMode);
    }
    return MS::kSuccess;
}

MStatus brushContextCommand::doQueryFlags()
{
    MArgParser argData = parser();

    if (argData.isFlagSet(radiusFlag))
    {
        setResult((float) nBrushContext->getRadius());
    }
    if (argData.isFlagSet(lockBaseFlag))
    {
        setResult(nBrushContext->getLockBase());
    }
    if (argData.isFlagSet(resizeBrushFlag))
    {
        setResult(nBrushContext->getInteractive());
    }
    if (argData.isFlagSet(intensityFlag))
    {
        setResult(nBrushContext->getIntensity());
    }
	if (argData.isFlagSet(brushModeFlag))
    {
        setResult(nBrushContext->getBrushMode());
    }
    return MS::kSuccess;
}

MStatus	brushContextCommand::appendSyntax()
{
    MSyntax mySyntax = syntax();

    if (MS::kSuccess != mySyntax.addFlag(radiusFlag, radiusLongFlag,
                                         MSyntax::kDouble))
    {
        return MS::kFailure;
    }

    if (MS::kSuccess != mySyntax.addFlag(lockBaseFlag, lockBaseLongFlag,
                                         MSyntax::kBoolean))
    {
        return MS::kFailure;
    }

    if (MS::kSuccess != mySyntax.addFlag(resizeBrushFlag, resizeBrushLongFlag,
                                         MSyntax::kBoolean))
    {
        return MS::kFailure;
    }

    if (MS::kSuccess != mySyntax.addFlag(intensityFlag, intensityLongFlag,
                                         MSyntax::kDouble))
    {
        return MS::kFailure;
    }

	if (MS::kSuccess != mySyntax.addFlag(brushModeFlag, brushModeLongFlag,
                                         MSyntax::kLong))
    {
        return MS::kFailure;
    }

    return MS::kSuccess;
}
