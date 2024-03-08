#pragma once

#include <cassert>
#include <iostream>
#include <vulkan/vulkan.h>

#include "spirv_reflect.h"

std::string ToStringDescriptorType(SpvReflectDescriptorType value);


void PrintDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent);

void PrintModuleInfo(std::ostream& os, const SpvReflectShaderModule& obj);

void PrintDescriptorSet(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent);

void PrintReflectionInfo(const SpvReflectShaderModule& module, const std::vector<SpvReflectDescriptorSet*>& reflectDescriptorSets);
