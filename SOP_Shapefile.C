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
#define SOP_SHAPEFILE_PARAM_CREATE_SHAPE_GROUPS "create_shape_groups"
#define SOP_SHAPEFILE_PARAM_CREATE_SHAPE_ATTRIBUTES "create_shape_attributes"

#define SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_NUM "point_shape_num"
#define SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_PART_NUM "point_shape_part_num"

#define SOP_SHAPEFILE_ATTRIB_PRIMITIVE_SHAPE_NUM "primitive_shape_num"
#define SOP_SHAPEFILE_ATTRIB_PRIMITIVE_SHAPE_PART_NUM "primitive_shape_part_num"

#define SOP_SHAPEFILE_GROUP_SHAPE_POINT "shape_point"
#define SOP_SHAPEFILE_GROUP_SHAPE_POLYGON "shape_polygon"
#define SOP_SHAPEFILE_GROUP_SHAPE_POLYLINE "shape_polyline"


static PRM_Name s_name_file(SOP_SHAPEFILE_PARAM_FILE, "Shape File");
static PRM_Name s_name_create_shape_groups(SOP_SHAPEFILE_PARAM_CREATE_SHAPE_GROUPS, "Create Shape Groups");
static PRM_Name s_name_create_shape_attributes(SOP_SHAPEFILE_PARAM_CREATE_SHAPE_ATTRIBUTES, "Create Shape Attributes");
static PRM_SpareData s_shape_file_picker(PRM_SpareArgs() << PRM_SpareToken(PRM_SpareData::getFileChooserModeToken(),
    PRM_SpareData::getFileChooserModeValRead()) << PRM_SpareToken(PRM_SpareData::getFileChooserPatternToken(),
    SOP_Shapefile::fileExtensionFilterString()));


static PRM_Default s_default_create_shape_groups(false);
static PRM_Default s_default_create_shape_attributes(false);


PRM_Template
SOP_Shapefile::myTemplateList[] =
{
    PRM_Template(PRM_TOGGLE, 1, &s_name_create_shape_groups, &s_default_create_shape_groups),
    PRM_Template(PRM_TOGGLE, 1, &s_name_create_shape_attributes, &s_default_create_shape_attributes),
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
                if(!addShapePoint(shp_object, t))
                {
                    processWarning("Skipping an invalid point shape.");
                }

                break;
            }

            case SHPT_ARC:
            case SHPT_ARCZ:
            {
                if(!addShapePolyline(shp_object, t))
                {
                    processWarning("Skipping an invalid polyline shape.");
                }

                break;
            }

            case SHPT_POLYGON:
            case SHPT_POLYGONZ:
            {
                if(!addShapePolygon(shp_object, t))
                {
                    processWarning("Skipping an invalid polygon shape.");
                }

                break;
            }

            case SHPT_MULTIPOINT:
            case SHPT_MULTIPOINTZ:
            {
                processWarning("Skipping unsupported mulitpoint shape.");
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
SOP_Shapefile::getParamCreateShapeGroups(fpreal t) const
{
    return evalInt(SOP_SHAPEFILE_PARAM_CREATE_SHAPE_GROUPS, 0, t) != 0;
}


bool
SOP_Shapefile::getParamCreateShapeAttributes(fpreal t) const
{
    return evalInt(SOP_SHAPEFILE_PARAM_CREATE_SHAPE_ATTRIBUTES, 0, t) != 0;
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
SOP_Shapefile::getShapePointPositions(SHPObject* shp_object, int shp_idx, bool use_z,
    UT_Array<UT_Vector3>& point_positions) const
{
    point_positions.clear();

    if(!shp_object)
    {
        return false;
    }

    int vertex_first = 0;
    int vertex_last = 0;

    if(!getShapeVertexIndices(shp_object, shp_idx, vertex_first, vertex_last))
    {
        return false;
    }

    for(int idx = vertex_first; idx <= vertex_last; ++idx)
    {
        float px = shp_object->padfX[idx];
        float py = shp_object->padfY[idx];

        float pz = 0.0f;

        if(use_z)
        {
            pz = shp_object->padfZ[idx];
        }

        point_positions.append(UT_Vector3(px, py, pz));
    }

    return point_positions.size() > 0;
}


bool
SOP_Shapefile::addShapePoint(SHPObject* shp_object, fpreal t)
{
    if(!shp_object)
    {
        return false;
    }

    if(shp_object->nSHPType != SHPT_POINT && shp_object->nSHPType != SHPT_POINTZ)
    {
        return false;
    }

    GA_PointGroup* group_point = nullptr;
    if(getParamCreateShapeGroups(t))
    {
        group_point = findGroupPoint(SOP_SHAPEFILE_GROUP_SHAPE_POINT);
    }

    for(int idp = 0; idp < shp_object->nParts; ++idp)
    {
        UT_Array<UT_Vector3> point_positions;
        if(!getShapePointPositions(shp_object, idp, (shp_object->nSHPType == SHPT_POINTZ), point_positions))
        {
            return false;
        }

        for(int idx = 0; idx < point_positions.size(); ++idx)
        {
            const UT_Vector3& point_position = point_positions(idx);

            GA_Offset point_offset = gdp->appendPoint();
            gdp->setPos3(point_offset, point_position);

            if(group_point)
            {
                group_point->addOffset(point_offset);
            }

            if(getParamCreateShapeAttributes(t))
            {
                setPointAttributeShapeNumber(point_offset, shp_object->nShapeId);
                setPointAttributeShapePartNumber(point_offset, idp);
            }
        }
    }

    return true;
}


bool
SOP_Shapefile::addShapePolygon(SHPObject* shp_object, fpreal t)
{
    if(!shp_object)
    {
        return false;
    }

    if(shp_object->nSHPType != SHPT_POLYGON && shp_object->nSHPType != SHPT_POLYGONZ)
    {
        return false;
    }

    GA_PrimitiveGroup* group_primitive = nullptr;
    if(getParamCreateShapeGroups(t))
    {
        group_primitive = findGroupPrimitive(SOP_SHAPEFILE_GROUP_SHAPE_POLYGON);
    }

    for(int idp = 0; idp < shp_object->nParts; ++idp)
    {
        UT_Array<UT_Vector3> point_positions;
        if(!getShapePointPositions(shp_object, idp, (shp_object->nSHPType == SHPT_POLYGONZ), point_positions))
        {
            return false;
        }

        GU_PrimPoly* prim_poly = GU_PrimPoly::build(gdp, 0, GU_POLY_CLOSED);

        for(int idx = 0; idx < point_positions.size(); ++idx)
        {
            const UT_Vector3& point_position = point_positions(idx);

            GA_Offset point_offset = gdp->appendPoint();
            gdp->setPos3(point_offset, point_position);

            prim_poly->appendVertex(point_offset);
        }

        prim_poly->close();

        if(group_primitive)
        {
            GA_Offset prim_offset = prim_poly->getMapOffset();
            group_primitive->addOffset(prim_offset);
        }

        if(getParamCreateShapeAttributes(t))
        {
            GA_Offset prim_offset = prim_poly->getMapOffset();
            setPrimitiveAttributeShapeNumber(prim_offset, shp_object->nShapeId);
            setPrimitiveAttributeShapePartNumber(prim_offset, idp);
        }
    }

    return true;
}


bool
SOP_Shapefile::addShapePolyline(SHPObject* shp_object, fpreal t)
{
    if(!shp_object)
    {
        return false;
    }

    if(shp_object->nSHPType != SHPT_ARC && shp_object->nSHPType != SHPT_ARCZ)
    {
        return false;
    }

    GA_PrimitiveGroup* group_primitive = nullptr;
    if(getParamCreateShapeGroups(t))
    {
        group_primitive = findGroupPrimitive(SOP_SHAPEFILE_GROUP_SHAPE_POLYLINE);
    }

    for(int idp = 0; idp < shp_object->nParts; ++idp)
    {
        UT_Array<UT_Vector3> point_positions;
        if(!getShapePointPositions(shp_object, idp, (shp_object->nSHPType == SHPT_ARCZ), point_positions))
        {
            return false;
        }

        GU_PrimPoly* prim_poly = GU_PrimPoly::build(gdp, 0, GU_POLY_OPEN);

        for(int idx = 0; idx < point_positions.size(); ++idx)
        {
            const UT_Vector3& point_position = point_positions(idx);

            GA_Offset point_offset = gdp->appendPoint();
            gdp->setPos3(point_offset, point_position);

            prim_poly->appendVertex(point_offset);
        }

        if(group_primitive)
        {
            GA_Offset prim_offset = prim_poly->getMapOffset();
            group_primitive->addOffset(prim_offset);
        }

        if(getParamCreateShapeAttributes(t))
        {
            GA_Offset prim_offset = prim_poly->getMapOffset();
            setPrimitiveAttributeShapeNumber(prim_offset, shp_object->nShapeId);
            setPrimitiveAttributeShapePartNumber(prim_offset, idp);
        }
    }

    return true;
}


GA_PointGroup*
SOP_Shapefile::findGroupPoint(const UT_String& group_name) const
{
    if(!group_name.isstring())
    {
        return nullptr;
    }

    GA_PointGroup* group = gdp->findPointGroup(group_name);

    if(!group)
    {
        group = gdp->newPointGroup(group_name);
    }

    return group;
}


GA_PrimitiveGroup*
SOP_Shapefile::findGroupPrimitive(const UT_String& group_name) const
{
    if(!group_name.isstring())
    {
        return nullptr;
    }

    GA_PrimitiveGroup* group = gdp->findPrimitiveGroup(group_name);

    if(!group)
    {
        group = gdp->newPrimitiveGroup(group_name);
    }

    return group;
}


void
SOP_Shapefile::setPointAttributeShapeNumber(GA_Offset point_offset, int32 shape_number)
{
    setAttribute(point_offset, GA_ATTRIB_POINT, SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_NUM, shape_number);
}


void
SOP_Shapefile::setPointAttributeShapePartNumber(GA_Offset point_offset, int32 shape_part_number)
{
    setAttribute(point_offset, GA_ATTRIB_POINT, SOP_SHAPEFILE_ATTRIB_POINT_SHAPE_PART_NUM, shape_part_number);
}


void
SOP_Shapefile::setPrimitiveAttributeShapeNumber(GA_Offset prim_offset, int32 shape_number)
{
    setAttribute(prim_offset, GA_ATTRIB_PRIMITIVE, SOP_SHAPEFILE_ATTRIB_PRIMITIVE_SHAPE_NUM, shape_number);
}


void
SOP_Shapefile::setPrimitiveAttributeShapePartNumber(GA_Offset prim_offset, int32 shape_part_number)
{
    setAttribute(prim_offset, GA_ATTRIB_PRIMITIVE, SOP_SHAPEFILE_ATTRIB_PRIMITIVE_SHAPE_PART_NUM, shape_part_number);
}


void
SOP_Shapefile::setAttribute(GA_Offset offset, GA_AttributeOwner attrib_owner, const UT_String& attribute_name, int attribute_value)
{
    if(GA_INVALID_OFFSET == offset || !attribute_name.isValidVariableName())
    {
        return;
    }

    GA_Attribute* attribute = gdp->findIntTuple(attrib_owner, attribute_name, 1);
    GA_RWHandleI handle_attribute(attribute);

    if(!handle_attribute.isValid())
    {
        attribute = gdp->addIntTuple(attrib_owner, attribute_name, 1);
        handle_attribute.bind(attribute);

        if(!handle_attribute.isValid())
        {
            return;
        }
    }

    handle_attribute.set(offset, attribute_value);
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator(SOP_SHAPEFILE_NAME, SOP_SHAPEFILE_DESCRIPTION,
        SOP_Shapefile::myConstructor, SOP_Shapefile::myTemplateList, 0, 0, 0, OP_FLAG_GENERATOR));
}

