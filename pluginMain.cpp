
#include <maya/MFnPlugin.h>
#include "brushContext.h"
#include "brushContextCommand.h"
#include "brushTool.h"

MStatus initializePlugin( MObject obj )

{
	MStatus   status;
	MFnPlugin plugin( obj, "jc.crystalCG", "1.0", "Any");

	status = plugin.registerContextCommand("curveBrushContext",
										   brushContextCommand::creator,
										   "curveBrushToolCmd",
										   brushTool::creator,
										   brushTool::newSyntax
										  );
	if (!status) {
		MGlobal::displayError("Error registering curveBrushContextCommand");
		return status;
	}
	MGlobal::executeCommand("curveBrushRunTimeCommands");

    return status;
}

MStatus uninitializePlugin( MObject obj )

{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterContextCommand("curveBrushContext","curveBrushToolCmd");
	if (!status) {
		MGlobal::displayError("Error deregistering curvebrushContextCommand");
		return status;
	}

	return status;
}
