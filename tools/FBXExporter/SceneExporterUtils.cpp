#include <fstream>
#include "SceneExporterUtils.h"

namespace fs = std::filesystem;

namespace Exporter
{

    /* Tab character ("\t") counter */
    int numTabs = 0; 

    /**
    * Print the required number of tabs.
    */
    void PrintTabs() {
        for(int i = 0; i < numTabs; i++)
            printf("\t");
    }

    /**
    * Return a string-based representation based on the attribute type.
    */
    FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) 
    {
        switch(type) { 
        case FbxNodeAttribute::eUnknown: return "unidentified"; 
        case FbxNodeAttribute::eNull: return "null"; 
        case FbxNodeAttribute::eMarker: return "marker"; 
        case FbxNodeAttribute::eSkeleton: return "skeleton"; 
        case FbxNodeAttribute::eMesh: return "mesh"; 
        case FbxNodeAttribute::eNurbs: return "nurbs"; 
        case FbxNodeAttribute::ePatch: return "patch"; 
        case FbxNodeAttribute::eCamera: return "camera"; 
        case FbxNodeAttribute::eCameraStereo: return "stereo"; 
        case FbxNodeAttribute::eCameraSwitcher: return "camera switcher"; 
        case FbxNodeAttribute::eLight: return "light"; 
        case FbxNodeAttribute::eOpticalReference: return "optical reference"; 
        case FbxNodeAttribute::eOpticalMarker: return "marker"; 
        case FbxNodeAttribute::eNurbsCurve: return "nurbs curve"; 
        case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface"; 
        case FbxNodeAttribute::eBoundary: return "boundary"; 
        case FbxNodeAttribute::eNurbsSurface: return "nurbs surface"; 
        case FbxNodeAttribute::eShape: return "shape"; 
        case FbxNodeAttribute::eLODGroup: return "lodgroup"; 
        case FbxNodeAttribute::eSubDiv: return "subdiv"; 
        default: return "unknown"; 
        } 
    }

    /**
    * Print an attribute.
    */
    void PrintAttribute(FbxNodeAttribute* pAttribute) 
    {
        if(!pAttribute) return;

        FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
        FbxString attrName = pAttribute->GetName();
        PrintTabs();
        // Note: to retrieve the character array of a FbxString, use its Buffer() method.
        printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
    }

    /**
    * Print a node, its attributes, and all its children recursively.
    */
    void PrintNode(FbxNode* pNode) 
    {
        PrintTabs();
        const char* nodeName = pNode->GetName();
        FbxDouble3 translation = pNode->LclTranslation.Get(); 
        FbxDouble3 rotation = pNode->LclRotation.Get(); 
        FbxDouble3 scaling = pNode->LclScaling.Get();

        // Print the contents of the node.
        printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n", 
            nodeName, 
            translation[0], translation[1], translation[2],
            rotation[0], rotation[1], rotation[2],
            scaling[0], scaling[1], scaling[2]
        );
        numTabs++;

        // Print the node's attributes.
        for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
            PrintAttribute(pNode->GetNodeAttributeByIndex(i));

        // Recursively print the children.
        for(int j = 0; j < pNode->GetChildCount(); j++)
            PrintNode(pNode->GetChild(j));

        numTabs--;
        PrintTabs();
        printf("</node>\n");
    }

    void IterateNodesDepthFirst(FbxNode* node, std::function<bool(FbxNode*, uint32_t depth)> callback, uint32_t depth)
    {
        if (!node)
            return;

        for (int i = 0; i < node->GetChildCount(); i++)
        {
            auto* current = node->GetChild(i);
            bool go_inside = callback(current, depth);
            if (go_inside)
                IterateNodesDepthFirst(current, callback, depth + 1);
        }
    }

    bool LoadJSONFile(const fs::path& json_path, rapidjson::Document& out_json)
    {
        if (!fs::exists(json_path))
            return false;
        
        std::ifstream file(json_path);
        file.seekg(0, file.end);
        size_t size = file.tellg();
        file.seekg(0, file.beg);
        std::vector<uint8_t> data(size);
        file.read((char*)data.data(), size);
        if (file.bad())
            return false;

        out_json.Parse((char*)data.data(), size);
        if (out_json.HasParseError())
            return false;

        return true;
    }
}