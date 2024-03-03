#pragma once

#include "material.hpp"

#include "Vk/lve_model.hpp"


namespace EngineCore
{
    class SubModel;
    class Model
    {
    public:
        Model(Vk::LveDevice& device);
        ~Model() = default;
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        std::vector<SubModel> subModels;

        Vk::LveDevice& lveDevice;
        
    };



    class SubModel
    {
    public:
        SubModel(Vk::LveDevice& device, const Vk::LveModel::Builder& builder);
        ~SubModel() = default;
        SubModel(const SubModel&) = delete;
        SubModel& operator=(const SubModel&) = delete;

    private:
        std::unique_ptr<Vk::LveModel> lveModel;
        Material material;

        friend class Model;
    };
}
