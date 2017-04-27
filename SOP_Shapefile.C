#include "SOP_Shapefile.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
#include <FS/FS_Info.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>


#include <shapefil.h>


#define SOP_SHAPEFILE_NAME "shapefile"
#define SOP_SHAPEFILE_DESCRIPTION "ShapeFile"

#define SOP_SHAPEFILE_PARAM_FILE "shape_file"


static PRM_Name s_name_file(SOP_SHAPEFILE_PARAM_FILE, "Shape File");
static PRM_SpareData s_shape_file_picker(PRM_SpareArgs() << PRM_SpareToken(PRM_SpareData::getFileChooserModeToken(),
    PRM_SpareData::getFileChooserModeValRead()) << PRM_SpareToken(PRM_SpareData::getFileChooserPatternToken(),
    SOP_Shapefile::fileExtensionFilterString()));


PRM_Template
SOP_Shapefile::myTemplateList[] =
{
    PRM_Template(PRM_FILE, 1, &s_name_file, 0, 0, 0, 0, &s_shape_file_picker),
    PRM_Template()
};


OP_Node*
SOP_Shapefile::myConstructor(OP_Network* network, const char* name, OP_Operator* op)
{
    return new SOP_Shapefile(network, name, op);
}


const char*
SOP_Shapefile::fileExtensionFilterString()
{
    return "*.shp";
}


SOP_Shapefile::SOP_Shapefile(OP_Network* network, const char* name, OP_Operator* op) :
    SOP_Node(network, name, op)
{

}


SOP_Shapefile::~SOP_Shapefile()
{

}


OP_ERROR
SOP_Shapefile::cookMySop(OP_Context& context)
{
    if(error() >= UT_ERROR_ABORT)
    {
        return error();
    }

    // Get current execution time for parameter evaluation.
    fpreal t = context.getTime();

    // Retrieve shapefile.
    UT_String shape_file_path = "";
    if(!getParamShapefile(shape_file_path, t))
    {
        processError(context, "Invalid shapefile specified.");
    }

    return error(context);
}


OP_ERROR
SOP_Shapefile::processError(OP_Context& context, const char* reason)
{
    UT_WorkBuffer buf;
    buf.sprintf("Shapefile: %s", reason);
    addError(SOP_MESSAGE, buf.buffer());

    return error(context);
}


bool
SOP_Shapefile::getParamShapefile(UT_String& shape_file, fpreal t) const
{
    shape_file = "";
    evalString(shape_file, SOP_SHAPEFILE_PARAM_FILE, 0, t);

    if(!shape_file || !shape_file.length())
    {
        return false;
    }

    FS_Info file_info(shape_file);
    if(!file_info.fileExists())
    {
        return false;
    }

    return true;
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator(SOP_SHAPEFILE_NAME, SOP_SHAPEFILE_DESCRIPTION,
        SOP_Shapefile::myConstructor, SOP_Shapefile::myTemplateList, 0, 0, 0, OP_FLAG_GENERATOR));
}

