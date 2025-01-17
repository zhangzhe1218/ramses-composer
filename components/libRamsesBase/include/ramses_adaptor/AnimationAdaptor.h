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

#include "core/Context.h"
#include "ramses_adaptor/ObjectAdaptor.h"
#include "ramses_base/RamsesHandles.h"
#include "user_types/Animation.h"

#include <ramses-logic/AnimationTypes.h>

namespace raco::ramses_adaptor {

class AnimationAdaptor final : public ObjectAdaptor, public ILogicPropertyProvider {
public:
	explicit AnimationAdaptor(SceneAdaptor* sceneAdaptor, raco::user_types::SAnimation animation);

	core::SEditorObject baseEditorObject() noexcept override;
	const core::SEditorObject baseEditorObject() const noexcept override;

	void getLogicNodes(std::vector<rlogic::LogicNode*>& logicNodes) const override;
	const rlogic::Property* getProperty(const std::vector<std::string>& propertyNamesVector) override;
	void onRuntimeError(core::Errors& errors, std::string const& message, core::ErrorLevel level) override;

	bool sync(core::Errors* errors) override;
	void readDataFromEngine(core::DataChangeRecorder& recorder);

private:
	user_types::SAnimation editorObject_;
	std::vector<raco::ramses_base::RamsesAnimationChannelHandle> channelHandles_;
	raco::ramses_base::RamsesAnimationNode animNode_;
	std::array<raco::components::Subscription, 3> settingsSubscriptions_;
	raco::components::Subscription dirtySubscription_;
	raco::components::Subscription nameSubscription_;

	void updateGlobalAnimationSettings();
	void updateGlobalAnimationStats(core::Errors* errors);
};

};	// namespace raco::ramses_adaptor