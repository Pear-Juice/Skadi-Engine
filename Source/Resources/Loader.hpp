#ifndef RESOURCELOADER
#define RESOURCELOADER

#include <filesystem>
#include <assimp/matrix4x4.h>
#include <assimp/scene.h>

#include "Model.hpp"

class Loader {
    public:
        std::tuple<std::vector<Model>, std::vector<Material>> loadModels(std::filesystem::path filePath);
        Texture loadTexture(std::filesystem::path filePath);
        Texture loadTexture(const aiTexture *texture);

    private:
        void processAiNode(const aiScene *scene, aiNode *node, std::vector<Model> &models, std::unordered_map<std::uint32_t, Material> &materials);


        static glm::mat4 Assimp2Glm(const aiMatrix4x4& from)
        {
            return glm::mat4(
                (double)from.a1, (double)from.b1, (double)from.c1, (double)from.d1,
                (double)from.a2, (double)from.b2, (double)from.c2, (double)from.d2,
                (double)from.a3, (double)from.b3, (double)from.c3, (double)from.d3,
                (double)from.a4, (double)from.b4, (double)from.c4, (double)from.d4
            );
        }
        static aiMatrix4x4 Glm2Assimp(const glm::mat4& from)
        {
            return aiMatrix4x4(from[0][0], from[1][0], from[2][0], from[3][0],
                from[0][1], from[1][1], from[2][1], from[3][1],
                from[0][2], from[1][2], from[2][2], from[3][2],
                from[0][3], from[1][3], from[2][3], from[3][3]
            );
        }
};

#endif
