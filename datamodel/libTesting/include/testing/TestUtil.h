/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "core/Context.h"
#include "core/Handles.h"
#include "testing/TestEnvironmentCore.h"
#include "user_types/Animation.h"
#include "user_types/AnimationChannel.h"
#include "user_types/LuaScript.h"
#include "user_types/Node.h"
#include "utils/FileUtils.h"
#include <QCoreApplication>
#include <gtest/gtest.h>
#include <thread>
#include <tuple>

namespace raco {

template <typename T, typename I>
inline std::shared_ptr<T> select(const std::vector<I>& vec) {
	auto it = std::find_if(vec.begin(), vec.end(), [](const auto& i) {
		return std::dynamic_pointer_cast<T>(i);
	});
	if (it != vec.end()) {
		return std::dynamic_pointer_cast<T>(*it);
	}
	return {};
}

template <typename ContextOrCommandInterface>
inline auto createLinkedScene(ContextOrCommandInterface& context, const std::filesystem::path& path) {
	const auto luaScript{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	const auto node{context.createObject(raco::user_types::Node::typeDescription.typeName, "node", "node_id")};
	raco::utils::file::write((path / "lua_script.lua").string(), R"(
function interface()
	OUT.translation = VEC3F
	OUT.rotation3 = VEC3F
	OUT.rotation4 = VEC4F
end
function run()
end
	)");
	context.set({luaScript, {"uri"}}, (path / "lua_script.lua").string());
	auto link = context.addLink({luaScript, {"luaOutputs", "translation"}}, {node, {"translation"}});
	return std::make_tuple(
		std::dynamic_pointer_cast<user_types::LuaScript>(luaScript), std::dynamic_pointer_cast<user_types::Node>(node), link);
}

inline auto createLinkedScene(TestEnvironmentCore& env) {
	return createLinkedScene(env.commandInterface, env.cwd_path_relative());
}

template <typename ContextOrCommandInterface>
inline auto createAnimatedScene(ContextOrCommandInterface& context, const std::filesystem::path& path) {
	const auto anim{context.createObject(raco::user_types::Animation::typeDescription.typeName, "anim", "anim_id")};
	const auto animChannel{context.createObject(raco::user_types::AnimationChannel::typeDescription.typeName, "anim_ch", "anim_ch_id")};
	const auto node{context.createObject(raco::user_types::Node::typeDescription.typeName, "node", "node_id")};

	context.set({anim, {"animationChannels", "Channel 0"}}, animChannel);
	context.set({animChannel, {"uri"}}, (path / "meshes" / "InterpolationTest" / "InterpolationTest.gltf").string());

	auto link = context.addLink({anim, {"animationOutputs", "Ch0.anim_ch"}}, {node, {"translation"}});
	return std::make_tuple(
		std::dynamic_pointer_cast<user_types::Animation>(anim),
		std::dynamic_pointer_cast<user_types::AnimationChannel>(animChannel),
		std::dynamic_pointer_cast<user_types::Node>(node),
		link);
}

inline bool isValueChanged(const raco::core::DataChangeRecorder& recorder, const raco::core::ValueHandle& handle) {
	return recorder.hasValueChanged(handle);
}

inline bool awaitPreviewDirty(const raco::core::DataChangeRecorder& recorder, const raco::core::SEditorObject& obj, long long timeout = 5) {
	const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{timeout};
	auto dirtyObjects{recorder.getPreviewDirtyObjects()};
	int argc = 0;
	QCoreApplication eventLoop_{argc, nullptr};
	while (std::find(dirtyObjects.begin(), dirtyObjects.end(), obj) == dirtyObjects.end()) {
		if (std::chrono::steady_clock::now() > timeoutTS) {
			assert(false && "Timeout");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds{5});
		QCoreApplication::processEvents();
		dirtyObjects = recorder.getPreviewDirtyObjects();
	}
	return true;
}

}  // namespace raco
