#ifndef VULKMATERIAL_HPP
#define VULKMATERIAL_HPP

struct VulkMaterial {
    std::vector<VulkTexture> textures;

    VkDescriptorPool pool;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSet> sets;
};

#endif //VULKMATERIAL_HPP
