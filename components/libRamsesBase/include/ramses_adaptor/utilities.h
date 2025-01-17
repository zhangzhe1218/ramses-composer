/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "core/CoreFormatter.h"
#include "core/EditorObject.h"
#include "core/Handles.h"
#include "core/MeshCacheInterface.h"
#include "data_storage/Value.h"
#include "log_system/log.h"
#include "ramses_adaptor/BuildOptions.h"
#include "ramses_base/RamsesHandles.h"
#include "components/DataChangeDispatcher.h"
#include "user_types/Material.h"
#include "user_types/Node.h"
#include "utils/MathUtils.h"

#include <ramses-logic/Property.h>
#include <ramses-logic/RamsesNodeBinding.h>
#include <memory>
#include <type_traits>

namespace raco::ramses_adaptor {

static constexpr const char* defaultVertexShader =
	"#version 300 es\n\
		precision mediump float;\n\
		in vec3 a_Position;\n\
		\n\
		uniform mat4 mvpMatrix;\n\
		void main() {\n\
			gl_Position = mvpMatrix * vec4(a_Position.xyz, 1.0);\n\
		}";

static constexpr const char* defaultVertexShaderWithNormals =
R"(
#version 300 es
precision mediump float;
in vec3 a_Position;
in vec3 a_Normal;
out float lambertian;
uniform mat4 mvpMatrix;
void main() {
	lambertian = mix(0.4, 0.8, max(abs(dot(vec3(1.5, 2.4, 1.0), a_Normal)), 0.0));
	gl_Position = mvpMatrix * vec4(a_Position, 1.0);
}
)";

static constexpr const char* defaultFragmentShader =
	"#version 300 es\n\
		precision mediump float;\n\
		\n\
		out vec4 FragColor;\n\
		\n\
		void main() {\n\
			FragColor = vec4(1.0, 0.0, 0.2, 1.0); \n\
		}";

static constexpr const char* defaultFragmentShaderWithNormals =
R"(
#version 300 es
precision mediump float;
in float lambertian;
out vec4 fragColor;
void main() {
	fragColor = vec4(1.0, 0.5, 0.0, 1.0) * lambertian;
}
)";

static constexpr const char* defaultEffectName = "raco::ramses_adaptor::DefaultEffectWithoutNormals";
static constexpr const char* defaultEffectWithNormalsName = "raco::ramses_adaptor::DefaultEffectWithNormals";
static constexpr const char* defaultAppearanceName = "raco::ramses_adaptor::DefaultAppearanceWithoutNormals";
static constexpr const char* defaultAppearanceWithNormalsName = "raco::ramses_adaptor::DefaultAppearanceWithNormals";
static constexpr const char* defaultIndexDataBufferName = "raco::ramses_adaptor::DefaultIndexDataBuffer";
static constexpr const char* defaultVertexDataBufferName = "raco::ramses_adaptor::DefaultVertexDataBuffer";
static constexpr const char* defaultRenderGroupName = "raco::ramses_adaptor::DefaultRenderGroup";
static constexpr const char* defaultRenderPassName = "raco::ramses_adaptor::DefaultRenderPass";
static constexpr const char* defaultAnimationName = "raco::ramses_adaptor::DefaultAnimation";
static constexpr const char* defaultAnimationChannelName = "raco::ramses_adaptor::DefaultAnimationChannel";
static constexpr const char* defaultAnimationChannelTimestampsName = "raco::ramses_adaptor::DefaultAnimationTimestamps";
static constexpr const char* defaultAnimationChannelKeyframesName = "raco::ramses_adaptor::DefaultAnimationKeyframes";


struct Vec3f {
	float x, y, z;
	bool operator==(const Vec3f& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=(const Vec3f& other) const {
		return x != other.x || y != other.y || z != other.z;
	}
};

struct Rotation final : public Vec3f {
	template <typename DataType>
	static void sync(const std::shared_ptr<DataType>& source, ramses::Node& target) {
		static_assert(std::is_base_of<user_types::EditorObject, DataType>::value);
		Rotation value{from<DataType>(source)};
		auto status = target.setRotation(value.x, value.y, value.z, raco::ramses_adaptor::RAMSES_ROTATION_CONVENTION);
		assert(status == ramses::StatusOK);
	}
	static Rotation from(const ramses::Node& node) {
		Rotation result;
		ramses::ERotationConvention convention;
		auto status = node.getRotation(result.x, result.y, result.z, convention);
		assert(status == ramses::StatusOK);
		return result;
	}
	template <typename DataType>
	static Rotation from(const std::shared_ptr<DataType>& node) {
		static_assert(std::is_base_of<user_types::EditorObject, DataType>::value);
		return {static_cast<float>(node->rotation_->x.asDouble()), static_cast<float>(node->rotation_->y.asDouble()), static_cast<float>(node->rotation_->z.asDouble())};
	}
};

struct Translation final : public Vec3f {
	template <typename DataType>
	static void sync(const std::shared_ptr<DataType>& source, ramses::Node& target) {
		static_assert(std::is_base_of<user_types::EditorObject, DataType>::value);
		Translation value{from<DataType>(source)};
		auto status = target.setTranslation(value.x, value.y, value.z);
		assert(status == ramses::StatusOK);
	}
	static Translation from(ramses::Node& node) {
		Translation result;
		auto status = node.getTranslation(result.x, result.y, result.z);
		assert(status == ramses::StatusOK);
		return result;
	}
	template <typename DataType>
	static Translation from(const std::shared_ptr<DataType>& node) {
		static_assert(std::is_base_of<user_types::EditorObject, DataType>::value);
		return {static_cast<float>(node->translation_->x.asDouble()), static_cast<float>(node->translation_->y.asDouble()), static_cast<float>(node->translation_->z.asDouble())};
	}
};

struct Scaling final : public Vec3f {
	template <typename DataType>
	static void sync(const std::shared_ptr<DataType>& source, ramses::Node& target) {
		static_assert(std::is_base_of<user_types::EditorObject, DataType>::value);
		Scaling value{from<DataType>(source)};
		auto status = target.setScaling(value.x, value.y, value.z);
		assert(status == ramses::StatusOK);
	}
	static Scaling from(const ramses::Node& node) {
		Scaling result;
		auto status = node.getScaling(result.x, result.y, result.z);
		assert(status == ramses::StatusOK);
		return result;
	}
	template <typename DataType>
	static Scaling from(const std::shared_ptr<DataType>& node) {
		static_assert(std::is_base_of<user_types::EditorObject, DataType>::value);
		return {static_cast<float>(node->scale_->x.asDouble()), static_cast<float>(node->scale_->y.asDouble()), static_cast<float>(node->scale_->z.asDouble())};
	}
};

constexpr ramses::EDataType convert(core::MeshData::VertexAttribDataType type) {
	switch (type) {
		case core::MeshData::VertexAttribDataType::VAT_Float:
			return ramses::EDataType::Float;
		case core::MeshData::VertexAttribDataType::VAT_Float2:
			return ramses::EDataType::Vector2F;
		case core::MeshData::VertexAttribDataType::VAT_Float3:
			return ramses::EDataType::Vector3F;
		case core::MeshData::VertexAttribDataType::VAT_Float4:
			return ramses::EDataType::Vector4F;
		default:
			assert(false && "Unknown VertexAttribDataType");
	}
	return ramses::EDataType::Float;
}

template <typename... Args>
inline const rlogic::Property* propertyByNames(const rlogic::Property* property, Args... names);

template <typename T, typename... Args>
inline const rlogic::Property* propertyByNames(const rlogic::Property* property, const T& name, Args... names) {
	if constexpr (sizeof...(Args) > 0) {
		return propertyByNames(property->getChild(name), names...);
	} else {
		return property->getChild(name);
	}
}

inline bool setLuaInputInEngine(rlogic::Property* property, const core::ValueHandle& valueHandle) {
	LOG_TRACE(log_system::RAMSES_ADAPTOR, "{} <= {}", fmt::ptr(property), valueHandle);
	assert(property != nullptr);
	using core::PrimitiveType;

	auto success{false};
	switch (valueHandle.type()) {
		case PrimitiveType::Double:
			success = property->set(static_cast<float>(valueHandle.as<double>()));
			break;
		case PrimitiveType::Int:
			success = property->set(valueHandle.as<int>());
			break;
		case PrimitiveType::Bool:
			success = property->set(valueHandle.as<bool>());
			break;
		case PrimitiveType::Vec2f:
			success = property->set(rlogic::vec2f{valueHandle[0].as<float>(), valueHandle[1].as<float>()});
			break;
		case PrimitiveType::Vec3f:
			success = property->set(rlogic::vec3f{valueHandle[0].as<float>(), valueHandle[1].as<float>(), valueHandle[2].as<float>()});
			break;
		case PrimitiveType::Vec4f:
			success = property->set(rlogic::vec4f{valueHandle[0].as<float>(), valueHandle[1].as<float>(), valueHandle[2].as<float>(), valueHandle[3].as<float>()});
			break;
		case PrimitiveType::Vec2i:
			success = property->set(rlogic::vec2i{valueHandle[0].as<int>(), valueHandle[1].as<int>()});
			break;
		case PrimitiveType::Vec3i:
			success = property->set(rlogic::vec3i{valueHandle[0].as<int>(), valueHandle[1].as<int>(), valueHandle[2].as<int>()});
			break;
		case PrimitiveType::Vec4i:
			success = property->set(rlogic::vec4i{valueHandle[0].as<int>(), valueHandle[1].as<int>(), valueHandle[2].as<int>(), valueHandle[3].as<int>()});
			break;
		case PrimitiveType::String:
			success = property->set(valueHandle.as<std::string>());
			break;
		case PrimitiveType::Table:
			success = true;
			for (size_t i{0}; i < valueHandle.size(); i++) {
				if (property->getType() == rlogic::EPropertyType::Array) {
					success = setLuaInputInEngine(property->getChild(i), valueHandle[i]) && success;
				} else {
					success = setLuaInputInEngine(property->getChild(valueHandle[i].getPropName()), valueHandle[i]) && success;
				}
			}
			break;
	}
	LOG_WARNING_IF(log_system::RAMSES_ADAPTOR, !success, "Script set properties failed: {}", property->getName());
	return success;
}

class ReadFromEngineManager {
public:
	template <typename Type>
	static void setValueFromEngineValue(const core::ValueHandle& valueHandle, Type newValue, core::DataChangeRecorder& recorder) {
		auto oldValue = valueHandle.as<Type>();
		if (oldValue != newValue) {
			valueHandle.valueRef()->set(static_cast<Type>(newValue));
			recorder.recordValueChanged(valueHandle);
		}
	}

	static void setVec2f(const core::ValueHandle& handle, double x, double y, core::DataChangeRecorder& recorder) {
		raco::data_storage::Vec2f& v = handle.valueRef()->asVec2f();

		if (*v.x != x) {
			v.x = x;
			recorder.recordValueChanged(handle[0]);
		}
		if (*v.y != y) {
			v.y = y;
			recorder.recordValueChanged(handle[1]);
		}
	}

	static void setVec3f(const core::ValueHandle& handle, double x, double y, double z, core::DataChangeRecorder& recorder) {
		raco::data_storage::Vec3f& v = handle.valueRef()->asVec3f();

		if (*v.x != x) {
			v.x = x;
			recorder.recordValueChanged(handle[0]);
		}
		if (*v.y != y) {
			v.y = y;
			recorder.recordValueChanged(handle[1]);
		}
		if (*v.z != z) {
			v.z = z;
			recorder.recordValueChanged(handle[2]);
		}
	}

	static void setVec4f(const core::ValueHandle& handle, double x, double y, double z, double w, core::DataChangeRecorder& recorder) {
		raco::data_storage::Vec4f& v = handle.valueRef()->asVec4f();

		if (*v.x != x) {
			v.x = x;
			recorder.recordValueChanged(handle[0]);
		}
		if (*v.y != y) {
			v.y = y;
			recorder.recordValueChanged(handle[1]);
		}
		if (*v.z != z) {
			v.z = z;
			recorder.recordValueChanged(handle[2]);
		}
		if (*v.w != w) {
			v.w = w;
			recorder.recordValueChanged(handle[3]);
		}
	}

	static void setVec2i(const core::ValueHandle& handle, int x, int y, core::DataChangeRecorder& recorder) {
		raco::data_storage::Vec2i& v = handle.valueRef()->asVec2i();

		if (*v.i1_ != x) {
			v.i1_ = x;
			recorder.recordValueChanged(handle[0]);
		}
		if (*v.i2_ != y) {
			v.i2_ = y;
			recorder.recordValueChanged(handle[1]);
		}
	}

	static void setVec3i(const core::ValueHandle& handle, int x, int y, int z, core::DataChangeRecorder& recorder) {
		raco::data_storage::Vec3i& v = handle.valueRef()->asVec3i();

		if (*v.i1_ != x) {
			v.i1_ = x;
			recorder.recordValueChanged(handle[0]);
		}
		if (*v.i2_ != y) {
			v.i2_ = y;
			recorder.recordValueChanged(handle[1]);
		}
		if (*v.i3_ != z) {
			v.i3_ = z;
			recorder.recordValueChanged(handle[2]);
		}
	}

	static void setVec4i(const core::ValueHandle& handle, int x, int y, int z, int w, core::DataChangeRecorder& recorder) {
		raco::data_storage::Vec4i& v = handle.valueRef()->asVec4i();

		if (*v.i1_ != x) {
			v.i1_ = x;
			recorder.recordValueChanged(handle[0]);
		}
		if (*v.i2_ != y) {
			v.i2_ = y;
			recorder.recordValueChanged(handle[1]);
		}
		if (*v.i3_ != z) {
			v.i3_ = z;
			recorder.recordValueChanged(handle[2]);
		}
		if (*v.i4_ != w) {
			v.i4_ = w;
			recorder.recordValueChanged(handle[3]);
		}
	}
};

inline void getLuaOutputFromEngine(const rlogic::Property& property, const core::ValueHandle& valueHandle, core::DataChangeRecorder& recorder) {
	// Don't spam the log with constant messages:
	//LOG_TRACE(log_system::RAMSES_ADAPTOR, "{} => {}", fmt::ptr(&property), valueHandle);
	using core::PrimitiveType;

	// read quaternion rotation data
	if (valueHandle.type() == PrimitiveType::Vec3f && property.getType() == rlogic::EPropertyType::Vec4f) {
		auto [x, y, z, w] = property.get<rlogic::vec4f>().value();
		auto [eulerX, eulerY, eulerZ] = raco::utils::math::quaternionToXYZDegrees(x, y, z, w);
		ReadFromEngineManager::setVec3f(valueHandle, eulerX, eulerY, eulerZ, recorder);
		return;
	}

	switch (valueHandle.type()) {
		case PrimitiveType::Double: {
			ReadFromEngineManager::setValueFromEngineValue<double>(valueHandle, property.get<float>().value(), recorder);
			break;
		}
		case PrimitiveType::Int: {
			ReadFromEngineManager::setValueFromEngineValue(valueHandle, property.get<int>().value(), recorder);
			break;
		}
		case PrimitiveType::Bool: {
			ReadFromEngineManager::setValueFromEngineValue(valueHandle, property.get<bool>().value(), recorder);
			break;
		}
		case PrimitiveType::Vec2f: {
			auto [x, y] = property.get<rlogic::vec2f>().value();
			ReadFromEngineManager::setVec2f(valueHandle, x, y, recorder);
			break;
		}
		case PrimitiveType::Vec3f: {
			auto [x, y, z] = property.get<rlogic::vec3f>().value();
			ReadFromEngineManager::setVec3f(valueHandle, x, y, z, recorder);
			break;
		}
		case PrimitiveType::Vec4f: {
			auto [x, y, z, w] = property.get<rlogic::vec4f>().value();
			ReadFromEngineManager::setVec4f(valueHandle, x, y, z, w, recorder);
			break;
		}
		case PrimitiveType::Vec2i: {
			auto [i1, i2] = property.get<rlogic::vec2i>().value();
			ReadFromEngineManager::setVec2i(valueHandle, i1, i2, recorder);
			break;
		}
		case PrimitiveType::Vec3i: {
			auto [i1, i2, i3] = property.get<rlogic::vec3i>().value();
			ReadFromEngineManager::setVec3i(valueHandle, i1, i2, i3, recorder);
			break;
		}
		case PrimitiveType::Vec4i: {
			auto [i1, i2, i3, i4] = property.get<rlogic::vec4i>().value();
			ReadFromEngineManager::setVec4i(valueHandle, i1, i2, i3, i4, recorder);
			break;
		}
		case PrimitiveType::String: {
			ReadFromEngineManager::setValueFromEngineValue(valueHandle, property.get<std::string>().value(), recorder);
			break;
		}
		case PrimitiveType::Table: 
		case PrimitiveType::Struct:	{
			for (size_t i{0}; i < valueHandle.size(); i++) {
				if (property.getType() == rlogic::EPropertyType::Array) {
					getLuaOutputFromEngine(*property.getChild(i), valueHandle[i], recorder);
				} else {
					getLuaOutputFromEngine(*property.getChild(valueHandle[i].getPropName()), valueHandle[i], recorder);
				}
			}
			break;
		}
	}
}

inline void setUniform(ramses::Appearance* appearance, const core::ValueHandle& valueHandle) {
	LOG_TRACE(raco::log_system::RAMSES_ADAPTOR, "{}.{} = {}", appearance->getName(), valueHandle.getPropName(), valueHandle);
	using core::PrimitiveType;
	ramses::UniformInput input;
	appearance->getEffect().findUniformInput(valueHandle.getPropName().c_str(), input);

	switch (valueHandle.type()) {
		case PrimitiveType::Double:
			appearance->setInputValueFloat(input, valueHandle.as<float>());
			break;
		case PrimitiveType::Int:
			appearance->setInputValueInt32(input, static_cast<int32_t>(valueHandle.as<int>()));
			break;
		case PrimitiveType::Vec2f:
			appearance->setInputValueVector2f(input, valueHandle[0].as<float>(), valueHandle[1].as<float>());
			break;
		case PrimitiveType::Vec3f:
			appearance->setInputValueVector3f(input, valueHandle[0].as<float>(), valueHandle[1].as<float>(), valueHandle[2].as<float>());
			break;
		case PrimitiveType::Vec4f:
			appearance->setInputValueVector4f(input, valueHandle[0].as<float>(), valueHandle[1].as<float>(), valueHandle[2].as<float>(), valueHandle[3].as<float>());
			break;
		case PrimitiveType::Vec2i:
			appearance->setInputValueVector2i(input, static_cast<int32_t>(valueHandle[0].as<int>()), static_cast<int32_t>(valueHandle[1].as<int>()));
			break;
		case PrimitiveType::Vec3i:
			appearance->setInputValueVector3i(input, static_cast<int32_t>(valueHandle[0].as<int>()), static_cast<int32_t>(valueHandle[1].as<int>()), static_cast<int32_t>(valueHandle[2].as<int>()));
			break;
		case PrimitiveType::Vec4i:
			appearance->setInputValueVector4i(input, static_cast<int32_t>(valueHandle[0].as<int>()), static_cast<int32_t>(valueHandle[1].as<int>()), static_cast<int32_t>(valueHandle[2].as<int>()), static_cast<int32_t>(valueHandle[3].as<int>()));
			break;
		default:
			break;
	}
}

inline void setDepthWrite(ramses::Appearance* appearance, const core::ValueHandle& valueHandle) {
	appearance->setDepthWrite(valueHandle.as<bool>() ? ramses::EDepthWrite_Enabled : ramses::EDepthWrite_Disabled);
}

inline void setDepthFunction(ramses::Appearance* appearance, const core::ValueHandle& valueHandle) {
	assert(valueHandle.type() == raco::core::PrimitiveType::Int);
	assert(valueHandle.asInt() >= 0 && valueHandle.asInt() < ramses::EDepthFunc_NUMBER_OF_ELEMENTS);
	appearance->setDepthFunction(static_cast<ramses::EDepthFunc>(valueHandle.asInt()));
}

inline ramses::EDepthWrite getDepthWriteMode(const ramses::Appearance* appearance) {
	ramses::EDepthWrite depthWrite;
	appearance->getDepthWriteMode(depthWrite);
	return depthWrite;
}

inline void setBlendMode(ramses::Appearance* appearance, const core::ValueHandle& options) {
	int colorOp = options.get("blendOperationColor").as<int>();
	assert(colorOp >= 0 && colorOp < ramses::EBlendOperation_NUMBER_OF_ELEMENTS);
	int alphaOp = options.get("blendOperationAlpha").as<int>();
	assert(alphaOp >= 0 && alphaOp < ramses::EBlendOperation_NUMBER_OF_ELEMENTS);
	appearance->setBlendingOperations(static_cast<ramses::EBlendOperation>(colorOp), static_cast<ramses::EBlendOperation>(alphaOp));

	int srcColor = options.get("blendFactorSrcColor").as<int>();
	assert(srcColor >= 0 && srcColor < ramses::EBlendFactor_NUMBER_OF_ELEMENTS);
	int destColor = options.get("blendFactorDestColor").as<int>();
	assert(destColor >= 0 && destColor < ramses::EBlendFactor_NUMBER_OF_ELEMENTS);
	int srcAlpha = options.get("blendFactorSrcAlpha").as<int>();
	assert(srcAlpha >= 0 && srcAlpha < ramses::EBlendFactor_NUMBER_OF_ELEMENTS);
	int destAlpha = options.get("blendFactorDestAlpha").as<int>();
	assert(destAlpha >= 0 && destAlpha < ramses::EBlendFactor_NUMBER_OF_ELEMENTS);
	appearance->setBlendingFactors(
		static_cast<ramses::EBlendFactor>(srcColor),
		static_cast<ramses::EBlendFactor>(destColor),
		static_cast<ramses::EBlendFactor>(srcAlpha),
		static_cast<ramses::EBlendFactor>(destAlpha));
}

inline void setBlendColor(ramses::Appearance* appearance, const core::ValueHandle& color) {
	appearance->setBlendingColor(
		color.get("x").as<float>(),
		color.get("y").as<float>(),
		color.get("z").as<float>(),
		color.get("w").as<float>());
}

inline void setCullMode(ramses::Appearance* appearance, const core::ValueHandle& valueHandle) {
	assert(valueHandle.type() == raco::core::PrimitiveType::Int);
	assert(valueHandle.asInt() >= 0 && valueHandle.asInt() < ramses::ECullMode::ECullMode_NUMBER_OF_ELEMENTS);
	appearance->setCullingMode(static_cast<ramses::ECullMode>(valueHandle.asInt()));
}

inline bool isArrayOfStructs(const rlogic::Property& property) {
	return property.getType() == rlogic::EPropertyType::Array && property.getChildCount() > 0 && property.getChild(0)->getType() == rlogic::EPropertyType::Struct;
}

template <auto T>
struct engine_property_name { static constexpr std::string_view value{}; };
template <>
struct engine_property_name<&user_types::Node::visible_> { static constexpr std::string_view value{"visibility"}; };
template <>
struct engine_property_name<&user_types::Node::translation_> { static constexpr std::string_view value{"translation"}; };
template <>
struct engine_property_name<&user_types::Node::rotation_> { static constexpr std::string_view value{"rotation"}; };
template <>
struct engine_property_name<&user_types::Node::scale_> { static constexpr std::string_view value{"scaling"}; };

};	// namespace raco::ramses_adaptor
