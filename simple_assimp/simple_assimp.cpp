#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stack>
#ifdef TEST_ASSIMP
template<typename T> struct Format {
    static void Output(const T&) {
        return;
    }
};
template<> struct Format<int> {
    static void Output(const int& v) {
        std::cout << v;
    }
};
template<> struct Format<unsigned int> {
    static void Output(const unsigned int& v) {
        std::cout << v;
    }
};
template<> struct Format<const char*> {
    static void Output(const char*& v) {
        std::cout << v;
    }
};
template<> struct Format<long long> {
    static void Output(long long& v) {
        std::cout << v;
    }
};
template<> struct Format<float> {
    static void Output(float& v) {
        std::cout << v;
    }
};
template<> struct Format<double> {
    static void Output(double& v) {
        std::cout << v;
    }
};

template<> struct Format<aiVector3D> {
    static void Output(const aiVector3D& v) {
        std::cout << v.x << ", " << v.y << ", " << v.z;
    }
};
void Println(const char* Message) {
    std::cout << Message << "\n\0";
}
template<typename T> void Println(const char* Message, const T& value) {
    std::cout << Message;  Format<T>::Output(value); std::cout << "\n\0";
}
template<typename... T> void Println(const char* Msg, const T&&... value) {
    std::cout << Msg; Format<T...>::Output(value...); std::cout << "\n\0";
}
static void face(aiFace& face) {
    for (int i = 0; i < face.mNumIndices; i++) {
        auto& index = face.mIndices[i];
        Println("Indice: ", index);
    }
}
static void mesh(aiMesh* mesh) {
    std::cout << "--- new mesh ---\n\0";
    for (int i = 0; i < mesh->mNumVertices; i++) {
        auto& v = mesh->mVertices[i];
        Println("Vertex: ",v );
    }
    Println("--face segment---");
    for (int i = 0; i < mesh->mNumFaces; i++) {
        auto& face = mesh->mFaces[i];
        ::face(face);
    }
    if (mesh->HasNormals()) {
        Println("--normals--");
        for (int i = 0; i < mesh->mNumVertices; i++) {
            Println("Normal: ", mesh->mNormals[i]);
        }
    }
    for (int CoordinateSetIterator = 0; CoordinateSetIterator < AI_MAX_NUMBER_OF_TEXTURECOORDS; CoordinateSetIterator++) {
        if (mesh->HasTextureCoords(CoordinateSetIterator)) {
            std::cout << "Texture Coordinates on Set: " << CoordinateSetIterator << "\n\0";
            for (int CoordinateIterator = 0; CoordinateIterator < mesh->mNumVertices; CoordinateIterator++) {
                Println("Texture Coordinates: ", mesh->mTextureCoords[CoordinateSetIterator][CoordinateIterator]);
            }
        }
    }
}
static void Process(aiNode* Node) {
    std::cout << "Node Details: \n\t";
    std::cout << Node->mName.C_Str() << "\n\t";
    std::cout << Node->mNumMeshes << "\n\t";
    for (int i = 0; i < Node->mNumMeshes; i++) {
        std::cout << Node->mMeshes[i] << ", "; // in our example there is only one node(cube) 
    }
    std::cout << "\n";
}
static void Traver(aiNode* Node) { // Interesting. Trees. Parsing them is weird, but oddly logical. 
    std::stack<aiNode*> Stack;
    
    while (!Stack.empty()) {

    }
}
static void node(aiNode* pRoot) {
    std::stack<aiNode*> Stack;
    Stack.push(pRoot);
    while (!Stack.empty()) {
        aiNode* CurrentNode = Stack.top();

        Stack.pop();

        Process(CurrentNode);

        for (int i = 0; i < CurrentNode->mNumChildren; i++) {
            Stack.push(CurrentNode->mChildren[i]);
        }
    }
}
int main()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\object2.obj", aiPostProcessSteps::aiProcess_Triangulate);
    if (!scene) {
        std::cout << "Could not read file.\n\0";
        return 0;
    }
    std::cout << "- ***Mesh Parse*** -";
    std::cout << "Mesh Count: " << scene->mNumMeshes << "\n\0";
    for (int i = 0; i < scene->mNumMeshes; i++) {
        auto* mesh = scene->mMeshes[i];
        ::mesh(mesh);
    }
    std::cout << "- ***Node Parse*** -\n";
    aiNode* RootTraversal = scene->mRootNode;
    ::node(RootTraversal);
}

#endif