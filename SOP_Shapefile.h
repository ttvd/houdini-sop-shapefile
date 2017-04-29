#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>


class GU_Detail;
class GA_Primitive;

typedef struct tagSHPObject SHPObject;

class SOP_Shapefile : public SOP_Node
{
    public:

        static OP_Node* myConstructor(OP_Network* network, const char* name, OP_Operator* op);
        static PRM_Template myTemplateList[];
        static const char* fileExtensionFilterString();

    protected:

        SOP_Shapefile(OP_Network* network, const char* name, OP_Operator* op);
        virtual ~SOP_Shapefile();

    protected:

        //! Return and process an error.
        OP_ERROR processError(OP_Context& context, const char* reason);

        //! Process a warning.
        void processWarning(const char* reason);

    protected:

        //! Process a points shape.
        bool addShapePoint(SHPObject* shp_object, fpreal t);

    protected:

        //! Helper function used to retrieve first and last vertex for a given shape part.
        bool getShapeVertexIndices(SHPObject* shp_object, int part_idx, int& vertex_first, int& vertex_last) const;

    protected:

        //! Find or create if not found a point group with a specified name.
        GA_PointGroup* findGroupPoint(const UT_String& group_name) const;

        //! Find or create if not found a primitive group with a specified name.
        GA_PrimitiveGroup* findGroupPrimitive(const UT_String& group_name) const;

    protected:

        //! Set shape number (and create an attribute if necessary) on a point.
        void setPointAttributeShapeNumber(GA_Offset point_offset, int shape_number);

        //! Set shape part number (and create an attribute if necessary) on a point.
        void setPointAttributeShapePartNumber(GA_Offset point_offset, int shape_part_number);

        //! Helper function to set an integer attribute on a point.
        void setPointAttribute(GA_Offset point_offset, const UT_String& attribute_name, int attribute_value);

    protected:

        //! Retrieve shapefile file.
        bool getParamShapefile(UT_String& shape_file, fpreal t) const;

        //! Retrieve whether to create shape groups.
        bool getParamCreateShapeGroups(fpreal t) const;

        //! Retrieve whether to create shape attributes.
        bool getParamCreateShapeAttributes(fpreal t) const;

    protected:

        virtual OP_ERROR cookMySop(OP_Context& context);
};
