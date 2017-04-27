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

        //! Process a 2D points shape.
        bool addShapePoint2D(SHPObject* shp_object);

    protected:

        //! Helper function used to retrieve first and last vertex for a given shape part.
        bool getShapeVertexIndices(SHPObject* shp_object, int part_idx, int& vertex_first, int& vertex_last) const;

    protected:

        //! Retrieve shapefile file.
        bool getParamShapefile(UT_String& shape_file, fpreal t) const;

    protected:

        virtual OP_ERROR cookMySop(OP_Context& context);
};
