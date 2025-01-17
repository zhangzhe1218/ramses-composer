/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "core/Consistency.h"

#include "core/Project.h"
#include <cassert>

namespace raco::core {

bool Consistency::checkProjectSettings(const Project& project) {
	int settingsCount{0};

	for (const auto& instance : project.instances()) {
		if (instance->getTypeDescription().typeName == ProjectSettings::typeDescription.typeName) {
			settingsCount++;
		}
	}

	assert(settingsCount == 1);
	return settingsCount == 1;
}

}  // namespace raco::core
