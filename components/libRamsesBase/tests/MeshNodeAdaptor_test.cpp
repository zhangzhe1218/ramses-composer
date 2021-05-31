/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <gtest/gtest.h>

#include "RamsesBaseFixture.h"
#include "ramses_adaptor/MeshNodeAdaptor.h"
#include "ramses_adaptor/SceneAdaptor.h"
#include "ramses_adaptor/utilities.h"

using namespace raco;
using raco::ramses_adaptor::MeshNodeAdaptor;
using raco::ramses_adaptor::SceneAdaptor;
using raco::user_types::Material;
using raco::user_types::Mesh;
using raco::user_types::MeshNode;
using raco::user_types::SMeshNode;
using raco::user_types::ValueHandle;

class MeshNodeAdaptorFixture : public RamsesBaseFixture<> {
protected:

	template <int MESH_NODE_AMOUNT>
	void runMeshNodeConstructionRoutine() {
		std::array<raco::core::SEditorObject, MESH_NODE_AMOUNT> meshNodes;
		auto material = context.createObject(Material::typeDescription.typeName, "Material");
		auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
		context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());

		for (int i = 0; i < MESH_NODE_AMOUNT; ++i) {
			meshNodes[i] = context.createObject(MeshNode::typeDescription.typeName, std::to_string(i));
			context.set(ValueHandle{meshNodes[i], {"mesh"}}, mesh);
			context.set(ValueHandle{meshNodes[i]}.get("materials")[0].get("material"), material);
		}

		context.set({material, {"uriVertex"}}, cwd_path().append("shaders/basic.vert").string());
		context.set({material, {"uriFragment"}}, cwd_path().append("shaders/basic.frag").string());
		dispatch();

		auto selectedMeshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};
		auto geometryBindings{select<ramses::GeometryBinding>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_GeometryBinding)};
		EXPECT_EQ(selectedMeshNodes.size(), MESH_NODE_AMOUNT);
		EXPECT_EQ(geometryBindings.size(), MESH_NODE_AMOUNT);

		for (int i = 0; i < MESH_NODE_AMOUNT; ++i) {
			auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), std::to_string(i).c_str());
			EXPECT_STREQ(std::to_string(i).c_str(), ramsesMeshNode->getName());
			EXPECT_TRUE(ramsesMeshNode->getAppearance() != nullptr);
			EXPECT_STREQ((std::to_string(i) + "_Appearance").c_str(), ramsesMeshNode->getAppearance()->getName());
			EXPECT_STRNE(raco::ramses_adaptor::defaultEffectName, ramsesMeshNode->getAppearance()->getEffect().getName());
			EXPECT_TRUE(ramsesMeshNode->getGeometryBinding() != nullptr);
		}
	}
};

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_constructs_ramsesMeshNode) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode Name");

	dispatch();
	auto meshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};

	EXPECT_EQ(meshNodes.size(), 1);
	EXPECT_TRUE(meshNodes[0]->getAppearance() != nullptr);
	EXPECT_TRUE(meshNodes[0]->getGeometryBinding() != nullptr);
	EXPECT_STREQ("MeshNode Name", meshNodes[0]->getName());
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_name_change) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode Name");

	dispatch();

	auto meshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};

	EXPECT_EQ(meshNodes.size(), 1);
	EXPECT_TRUE(meshNodes[0]->getAppearance() != nullptr);
	EXPECT_TRUE(meshNodes[0]->getGeometryBinding() != nullptr);
	EXPECT_STREQ("MeshNode Name", meshNodes[0]->getName());

	context.set({meshNode, {"objectName"}}, std::string("Changed"));
	dispatch();

	EXPECT_STREQ("Changed", meshNodes[0]->getName());
	EXPECT_STREQ("Changed_Appearance", meshNodes[0]->getAppearance()->getName());
	EXPECT_STREQ("Changed_GeometryBinding", meshNodes[0]->getGeometryBinding()->getName());
}


TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_withEmptyMesh_constructs_ramsesMeshNode) {
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode Name");
	context.set(ValueHandle{meshNode}.get("mesh"), mesh);

	dispatch();
	auto meshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};
	auto geometryBindings{select<ramses::GeometryBinding>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_GeometryBinding)};

	EXPECT_EQ(meshNodes.size(), 1);
	EXPECT_TRUE(meshNodes[0]->getAppearance() != nullptr);
	EXPECT_TRUE(meshNodes[0]->getGeometryBinding() != nullptr);
	EXPECT_STREQ("MeshNode Name", meshNodes[0]->getName());
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_withEmptyMesh_createdAfterMeshNode_constructs_ramsesMeshNode) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode Name");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	context.set(ValueHandle{meshNode}.get("mesh"), mesh);

	dispatch();
	auto meshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};
	auto geometryBindings{select<ramses::GeometryBinding>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_GeometryBinding)};

	EXPECT_EQ(meshNodes.size(), 1);
	EXPECT_TRUE(meshNodes[0]->getAppearance() != nullptr);
	EXPECT_TRUE(meshNodes[0]->getGeometryBinding() != nullptr);
	EXPECT_STREQ("MeshNode Name", meshNodes[0]->getName());
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_withEmptyMaterial_constructs_ramsesMeshNode) {
	auto material = context.createObject(Material::typeDescription.typeName, "Material");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");

	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	context.set(ValueHandle{meshNode}.get("materials")[0].get("material"), material);
	// env.context.set(ValueHandle{meshNode, { "materials", "material", "material"}}), material);
	dispatch();

	auto meshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};
	auto geometryBindings{select<ramses::GeometryBinding>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_GeometryBinding)};
	EXPECT_EQ(meshNodes.size(), 1);
	EXPECT_EQ(geometryBindings.size(), 1);

	auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), "MeshNode");
	EXPECT_TRUE(ramsesMeshNode->getAppearance() != nullptr);
	EXPECT_STREQ(raco::ramses_adaptor::defaultEffectWithNormalsName, ramsesMeshNode->getAppearance()->getEffect().getName());
	EXPECT_TRUE(ramsesMeshNode->getGeometryBinding() != nullptr);
	EXPECT_STREQ("MeshNode", ramsesMeshNode->getName());
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_withMaterial_constructs_ramsesMeshNode) {
	runMeshNodeConstructionRoutine<1>();
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_ten_MeshNodes_withSameMaterial_andSameMesh_construction) {
	runMeshNodeConstructionRoutine<10>();
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_ten_MeshNodes_withSameMaterial_andSameMesh_propertyUnsetting) {
	constexpr auto MESH_NODE_AMOUNT = 10;
	std::array<raco::core::SEditorObject, MESH_NODE_AMOUNT> meshNodes;
	auto material = context.createObject(Material::typeDescription.typeName, "Material");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());

	for (int i = 0; i < MESH_NODE_AMOUNT; ++i) {
		meshNodes[i] = context.createObject(MeshNode::typeDescription.typeName, std::to_string(i));
		context.set(ValueHandle{meshNodes[i], {"mesh"}}, mesh);
		context.set(ValueHandle{meshNodes[i]}.get("materials")[0].get("material"), material);
	}

	context.set({material, {"uriVertex"}}, cwd_path().append("shaders/basic.vert").string());
	context.set({material, {"uriFragment"}}, cwd_path().append("shaders/basic.frag").string());
	dispatch();

	context.deleteObjects({mesh, material});
	dispatch();

	auto selectedMeshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};
	auto geometryBindings{select<ramses::GeometryBinding>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_GeometryBinding)};
	EXPECT_EQ(selectedMeshNodes.size(), MESH_NODE_AMOUNT);
	EXPECT_EQ(geometryBindings.size(), MESH_NODE_AMOUNT);

	for (int i = 0; i < MESH_NODE_AMOUNT; ++i) {
		auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), std::to_string(i).c_str());
		EXPECT_STREQ(raco::ramses_adaptor::defaultEffectName, ramsesMeshNode->getAppearance()->getEffect().getName());
		// TODO: Compare Ramses mesh with default sceneAdaptor mesh here
	}
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_dynamicMaterial_constructs_ramsesMeshNode) {
	auto material = context.createObject(Material::typeDescription.typeName, "Material");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");

	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	context.set(ValueHandle{meshNode}.get("materials")[0].get("material"), material);
	// env.context.set(ValueHandle{meshNode, { "materials", "material", "material"}}), material);
	dispatch();

	// precondition
	{
		auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), "MeshNode");
		EXPECT_STREQ(raco::ramses_adaptor::defaultEffectWithNormalsName, ramsesMeshNode->getAppearance()->getEffect().getName());
	}

	context.set({material, {"uriVertex"}}, cwd_path().append("shaders/basic.vert").string());
	context.set({material, {"uriFragment"}}, cwd_path().append("shaders/basic.frag").string());
	dispatch();

	auto meshNodes{select<ramses::MeshNode>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_MeshNode)};
	auto geometryBindings{select<ramses::GeometryBinding>(*sceneContext.scene(), ramses::ERamsesObjectType::ERamsesObjectType_GeometryBinding)};
	EXPECT_EQ(meshNodes.size(), 1);
	EXPECT_EQ(geometryBindings.size(), 1);

	auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), "MeshNode");
	EXPECT_TRUE(ramsesMeshNode->getAppearance() != nullptr);
	EXPECT_STREQ("Material", ramsesMeshNode->getAppearance()->getEffect().getName());
	EXPECT_TRUE(ramsesMeshNode->getGeometryBinding() != nullptr);
	EXPECT_STREQ("MeshNode", ramsesMeshNode->getName());
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_depthWrite_disabled) {
	auto material = context.createObject(Material::typeDescription.typeName, "Material");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	auto meshNode = create<MeshNode>("MeshNode");

	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	context.set({material, {"uriVertex"}}, cwd_path().append("shaders/basic.vert").string());
	context.set({material, {"uriFragment"}}, cwd_path().append("shaders/basic.frag").string());

	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	context.set(meshNode->getMaterialHandle(0), material);
	context.set(meshNode->getMaterialOptionsHandle(0).get("depthwrite"), false);
	dispatch();

	auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), "MeshNode");
	EXPECT_TRUE(ramsesMeshNode->getAppearance() != nullptr);
	EXPECT_EQ(ramses::EDepthWrite_Disabled, raco::ramses_adaptor::getDepthWriteMode(ramsesMeshNode->getAppearance()));
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_depthWrite_enabled) {
	auto material = context.createObject(Material::typeDescription.typeName, "Material");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");

	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	context.set({material, {"uriVertex"}}, cwd_path().append("shaders/basic.vert").string());
	context.set({material, {"uriFragment"}}, cwd_path().append("shaders/basic.frag").string());
	context.set({material, {"depthwrite"}}, true);

	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	context.set(ValueHandle{meshNode}.get("materials")[0].get("material"), material);
	dispatch();

	auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), "MeshNode");
	EXPECT_TRUE(ramsesMeshNode->getAppearance() != nullptr);
	EXPECT_EQ(ramses::EDepthWrite_Enabled, raco::ramses_adaptor::getDepthWriteMode(ramsesMeshNode->getAppearance()));
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_materialReset_and_depthWriteDisable) {
	auto material = context.createObject(Material::typeDescription.typeName, "Material");
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");

	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	context.set({material, {"uriVertex"}}, cwd_path().append("shaders/basic.vert").string());
	context.set({material, {"uriFragment"}}, cwd_path().append("shaders/basic.frag").string());
	context.set({material, {"depthwrite"}}, true);

	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	context.set(ValueHandle{meshNode}.get("materials")[0].get("material"), material);
	dispatch();

	context.set(ValueHandle{meshNode}.get("materials")[0].get("material"), core::SEditorObject{});
	context.set({material, {"depthwrite"}}, false);
	dispatch();

	auto ramsesMeshNode = select<ramses::MeshNode>(*sceneContext.scene(), "MeshNode");
	EXPECT_TRUE(ramsesMeshNode->getAppearance() != nullptr);
	EXPECT_STREQ(raco::ramses_adaptor::defaultEffectWithNormalsName, ramsesMeshNode->getAppearance()->getEffect().getName());
	EXPECT_EQ(ramses::EDepthWrite_Enabled, raco::ramses_adaptor::getDepthWriteMode(ramsesMeshNode->getAppearance()));
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_dynamicCreation_meshBeforeMeshNode) {
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	dispatch();
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");
	dispatch();
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	dispatch();
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	dispatch();

	// TODO: Need a way to check actual mesh resources in ramses, we only check for no error
	EXPECT_TRUE(true);
}

TEST_F(MeshNodeAdaptorFixture, inContext_userType_MeshNode_dynamicCreation_meshNodeBeforeMesh) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");
	dispatch();
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	dispatch();
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	dispatch();
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	dispatch();

	// TODO: Need a way to check actual mesh resources in ramses, we only check for no error
	EXPECT_TRUE(true);
}

TEST_F(MeshNodeAdaptorFixture, inContext_user_type_MeshNode_meshDeletion_meshNodeDataIsEmpty) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");
	dispatch();
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	dispatch();
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	dispatch();
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	dispatch();

	context.deleteObjects({mesh});

	// TODO: Need a way to check actual mesh resources in ramses, we only check for no error
	EXPECT_TRUE(true);
}

TEST_F(MeshNodeAdaptorFixture, inContext_user_type_MeshNode_submeshSelection_wrongSubmeshIndexCreatesErrorTooHigh) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");
	dispatch();
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	dispatch();
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
  	dispatch();
	context.set(ValueHandle{mesh, {"bakeMeshes"}}, false);
	dispatch();
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	dispatch();
	context.set(ValueHandle{mesh, {"meshIndex"}}, 1);
	dispatch();

	ASSERT_EQ(context.errors().getError(ValueHandle{mesh}).level(), core::ErrorLevel::ERROR);
}

TEST_F(MeshNodeAdaptorFixture, inContext_user_type_MeshNode_submeshSelection_wrongSubmeshIndexCreatesErrorTooLow) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");
	dispatch();
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	dispatch();
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	dispatch();
	context.set(ValueHandle{mesh, {"bakeMeshes"}}, false);
	dispatch();
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	dispatch();
	context.set(ValueHandle{mesh, {"meshIndex"}}, -1);
	dispatch();

	ASSERT_EQ(context.errors().getError(ValueHandle{mesh}).level(), core::ErrorLevel::ERROR);
}

TEST_F(MeshNodeAdaptorFixture, inContext_user_type_MeshNode_submeshSelection_correctSubmeshIndexFixesError) {
	auto meshNode = context.createObject(MeshNode::typeDescription.typeName, "MeshNode");
	dispatch();
	auto mesh = context.createObject(Mesh::typeDescription.typeName, "Mesh");
	dispatch();
	context.set(ValueHandle{meshNode, {"mesh"}}, mesh);
	dispatch();
	context.set(ValueHandle{mesh, {"bakeMeshes"}}, false);
	dispatch();
	context.set(ValueHandle{mesh, {"uri"}}, cwd_path().append("meshes/Duck.glb").string());
	dispatch();
	context.set(ValueHandle{mesh, {"meshIndex"}}, 1);
	dispatch();
	context.set(ValueHandle{mesh, {"meshIndex"}}, 0);
	dispatch();

	ASSERT_EQ(context.errors().getError(ValueHandle{mesh}).level(), core::ErrorLevel::INFORMATION);
}