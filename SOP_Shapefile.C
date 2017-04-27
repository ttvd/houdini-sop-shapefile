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

#define SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_NUM "point_shape_num"
#define SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_PART_NUM "point_shape_part_num"
#define SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_TYPE "point_shape_type"


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

    gdp->clearAndDestroy();

    // Get current execution time for parameter evaluation.
    fpreal t = context.getTime();

    // Retrieve shapefile.
    UT_String shape_file_path = "";
    if(!getParamShapefile(shape_file_path, t))
    {
        return processError(context, "Invalid shapefile specified.");
    }

    // Create shapefile handle.
    SHPHandle shp_handle = SHPOpen(shape_file_path.c_str(), "rb");
    if(!shp_handle)
    {
        return processError(context, "Unable to open shapefile for reading.");
    }

    // Retrieve the number of entities and their types.
    int shp_num_entities = 0;
    int shp_entities_type = SHPT_NULL;
    SHPGetInfo(shp_handle, &shp_num_entities, &shp_entities_type, nullptr, nullptr);

    if(!shp_num_entities)
    {
        SHPClose(shp_handle);
        return processError(context, "No shapes were found.");
    }

    // Process all shapes in a file.
    for(int ids = 0; ids < shp_entities_type; ++ids)
    {
        // Retrieve shape object at this index.
        SHPObject* shp_object = SHPReadObject(shp_handle, ids);
        if(!shp_object)
        {
            processWarning("Invalid object.");
            continue;
        }

        // If shape type is invalid, skip.
        if(shp_object->nSHPType == SHPT_NULL)
        {
            processWarning("Skipping a null shape.");
            continue;
        }

        // Make sure we have parts in this shape.
        if(!shp_object->nParts)
        {
            processWarning("Skipping a shape with zero parts.");
            continue;
        }

        switch(shp_object->nSHPType)
        {
            case SHPT_POINT:
            case SHPT_POINTZ:
            {
                if(!addShapePoint(shp_object))
                {
                    processWarning("Skipping an invalid point shape.");
                }

                break;
            }

            case SHPT_ARC:
            {
                break;
            }

            case SHPT_POLYGON:
            {
                break;
            }

            case SHPT_MULTIPOINT:
            {
                break;
            }

            case SHPT_ARCZ:
            {
                break;
            }

            case SHPT_POLYGONZ:
            {
                break;
            }

            case SHPT_MULTIPOINTZ:
            {
                break;
            }

            case SHPT_POINTM:
            case SHPT_ARCM:
            case SHPT_POLYGONM:
            case SHPT_MULTIPOINTM:
            case SHPT_MULTIPATCH:
            default:
            {
                processWarning("Skipping unsupported shape.");
                break;
            }
        }
    }

    // Close the handle.
    SHPClose(shp_handle);
    return error(context);
}


OP_ERROR
SOP_Shapefile::processError(OP_Context& context, const char* reason)
{
    if(reason)
    {
        UT_WorkBuffer buf;
        buf.sprintf("Shapefile: %s", reason);
        addError(SOP_MESSAGE, buf.buffer());
    }

    return error(context);
}


void
SOP_Shapefile::processWarning(const char* reason)
{
    if(reason)
    {
        UT_WorkBuffer buf;
        buf.sprintf("Shapefile: %s", reason);
        addWarning(SOP_MESSAGE, buf.buffer());
    }
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


bool
SOP_Shapefile::getShapeVertexIndices(SHPObject* shp_object, int part_idx, int& vertex_first, int& vertex_last) const
{
    vertex_first = 0;
    vertex_last = 0;

    if(!shp_object || part_idx < 0 || shp_object->nParts >= part_idx)
    {
        return false;
    }

    vertex_first = shp_object->panPartStart[part_idx];
    vertex_last = vertex_first;

    if(part_idx == shp_object->nParts - 1)
    {
        vertex_last = shp_object->nVertices - 1;
    }
    else
    {
        vertex_last = shp_object->panPartStart[part_idx + 1] - 1;
    }

    return true;
}


bool
SOP_Shapefile::addShapePoint(SHPObject* shp_object)
{
    if(!shp_object)
    {
        return false;
    }

    if(shp_object->nSHPType != SHPT_POINT && shp_object->nSHPType != SHPT_POINTZ)
    {
        return false;
    }

    for(int idp = 0; idp < shp_object->nParts; ++idp)
    {
        int vertex_first = 0;
        int vertex_last = 0;

        if(!getShapeVertexIndices(shp_object, idp, vertex_first, vertex_last))
        {
            return false;
        }

        for(int idx = vertex_first; idx <= vertex_last; ++idx)
        {
            float px = shp_object->padfX[idx];
            float py = shp_object->padfY[idx];

            float pz = 0.0f;

            if(shp_object->nSHPType == SHPT_POINTZ)
            {
                pz = shp_object->padfZ[idx];
            }

            GA_Offset point_offset = gdp->appendPoint();
            gdp->setPos3(point_offset, px, py, pz);

            //SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_NUM
            //SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_PART_NUM
            //SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_TYPE
        }
    }
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator(SOP_SHAPEFILE_NAME, SOP_SHAPEFILE_DESCRIPTION,
        SOP_Shapefile::myConstructor, SOP_Shapefile::myTemplateList, 0, 0, 0, OP_FLAG_GENERATOR));
}

