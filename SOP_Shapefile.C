#include "SOP_Shapefile.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>


#define SOP_SHAPEFILE_NAME "shapefile"
#define SOP_SHAPEFILE_DESCRIPTION "ShapeFile"


PRM_Template
SOP_Shapefile::myTemplateList[] =
{
    PRM_Template()
};


OP_Node*
SOP_Shapefile::myConstructor(OP_Network* network, const char* name, OP_Operator* op)
{
    return new SOP_Shapefile(network, name, op);
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

    return error(context);
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator(SOP_SHAPEFILE_NAME, SOP_SHAPEFILE_DESCRIPTION,
        SOP_Shapefile::myConstructor, SOP_Shapefile::myTemplateList, 0, 0, 0, OP_FLAG_GENERATOR));
}

