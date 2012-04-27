#ifndef brushToolCommand__
#define brushToolCommand__

#include <maya/MPxCommand.h>
#include <maya/MPxToolCommand.h>
#include "brushContext.h"
#include "brushContextCommand.h"

class brushToolCommand : public MPxToolCommand
{
public:
    brushToolCommand();
    virtual			~brushToolCommand();
    static void*	creator();

    MStatus			doIt(const MArgList& args);
    MStatus			undoIt();
    MStatus			cancel();
    MStatus			parseArgs(const MArgList& args);
    bool			isUndoable() const;
    MStatus			finalize();
    static MSyntax	newSyntax();

    void			setRadius(float newRadius);
    void            setInten(double intensity);
    //void          setNLockBase(bool nlockbase);
    void 			setBrushMode(int brushMode);

private:
    float			radius;     	// brush size
    bool            lockbase;
    double          inten;
    int 			brushModeVal;
};
#endif
