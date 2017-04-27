#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>


class GU_Detail;
class GA_Primitive;


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

    protected:

        //! Retrieve shapefile file.
        bool getParamShapefile(UT_String& shape_file, fpreal t) const;

    protected:

        virtual OP_ERROR cookMySop(OP_Context& context);
};
