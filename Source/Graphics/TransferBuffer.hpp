#ifndef DYNAMICBUFFER
#define DYNAMICBUFFER

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <iostream>

#include "ResourceManager.hpp"

struct TransferBuffer {
	VkBufferUsageFlags usageFlags;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
	VkDeviceSize capacity;
	uint32_t objectCount;
};

// template <typename T> class DynamicBuffer {
// 			VkBufferUsageFlags usageFlags;
// 			VkDeviceSize objectSize;
// 			VkDeviceSize maxBufferCapacity;
// 			VkDeviceSize currentBufferCapacity;
//
// 			VkBuffer stagingBuffer;
// 			VkDeviceMemory stagingBufferMemory;
// 			bool buffersCreated = false;
//
// 			bool checkSize();
// 			void resizeBuffer();
//
// 			const VkDevice &device;
//
// 			public:
// 				VkBuffer buffer;
// 				VkDeviceMemory bufferMemory;
// 				uint32_t objectCount;
// 				std::vector<T> objects;
//
// 				DynamicBuffer();
// 				DynamicBuffer(const VkDevice &_device, VkBufferUsageFlags _usageFlags, uint32_t _maxObjectCount = 10) : resourceManager(_resourceManager), device(_device) {
// 					usageFlags = _usageFlags;
// 					objectSize = sizeof(T);
// 					maxBufferCapacity = objectSize * _maxObjectCount;
// 					currentBufferCapacity = 0;
// 				}
//
// 				void transferOverwrite(std::vector<T> newObjects) {
// 					VkDeviceSize newObjectDataSize = newObjects.size() * objectSize;
// 					objects = newObjects;
// 					objectCount = newObjects.size();
//
// 					if (newObjectDataSize == 0) {
// 						std::cout << "Object data size is 0\n";
// 						return;
// 					}
//
// 					if (newObjectDataSize >= currentBufferCapacity) {
// 						currentBufferCapacity = getMemoryBound(newObjectDataSize);
//
// 						createBuffers(currentBufferCapacity);
// 					}
//
// 					void *data;
// 					vkMapMemory(device, stagingBufferMemory, 0, newObjectDataSize, 0, &data);
// 					memcpy(data, newObjects.data(), (size_t) newObjectDataSize);
// 					vkUnmapMemory(device, stagingBufferMemory);
//
// 					resourceManager->copyBuffer(stagingBuffer, buffer, currentBufferCapacity, 0,0);
// 				}
//
// 				void free() {
// 					vkDestroyBuffer(device, stagingBuffer, nullptr);
// 					vkFreeMemory(device, stagingBufferMemory, nullptr);
//
// 					vkDestroyBuffer(device, buffer, nullptr);
// 					vkFreeMemory(device, bufferMemory, nullptr);
// 				}
//
// 				void createBuffers(VkDeviceSize capacity) {
// 					if (buffersCreated) free();
//
// 					//Create staging buffer
// 					resourceManager->createBuffer(capacity, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
// 					//Create working buffer
// 					resourceManager->createBuffer(capacity, usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
// 					buffersCreated = true;
// 				}
//
// 				VkDeviceSize getMemoryBound(VkDeviceSize memory) {
// 					return std::pow(2, std::ceil(log2(memory))) * 2;
// 				}
//
// };

#endif
