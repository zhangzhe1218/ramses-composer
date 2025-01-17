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

#include "data_storage/BasicAnnotations.h"

#include "user_types/Node.h"
#include "user_types/Prefab.h"

namespace raco::user_types {

class PrefabInstance;
using SPrefabInstance = std::shared_ptr<PrefabInstance>;

class PrefabInstance : public Node {
public:
	static inline const TypeDescriptor typeDescription = { "PrefabInstance", false };
	TypeDescriptor const& getTypeDescription() const override {
		return typeDescription;
	}

	PrefabInstance(PrefabInstance const& other) : Node(other), template_(other.template_) {
		fillPropertyDescription();
	}

	PrefabInstance(std::string name = std::string(), std::string id = std::string())
		: Node(name, id)
	{
				fillPropertyDescription();
	}

	void fillPropertyDescription() {
		properties_.emplace_back("template", &template_);
		properties_.emplace_back("mapToInstance", &mapToInstance_);
	}

	Property<SPrefab, DisplayNameAnnotation> template_{nullptr, DisplayNameAnnotation("Prefab Template")};

	static SEditorObject mapToInstance(SEditorObject obj, SPrefab prefab, SPrefabInstance instance);
	static SEditorObject mapFromInstance(SEditorObject obj, SPrefabInstance instance);

	void removePrefabInstanceChild(BaseContext& context, const SEditorObject& prefabChild);

	void addChildMapping(BaseContext& context, const SEditorObject& prefabChild, const SEditorObject& instanceChild);

	// Maps from Prefab children objects -> PrefabInstance children
	Property<Table, ArraySemanticAnnotation, HiddenProperty> mapToInstance_;
};

}